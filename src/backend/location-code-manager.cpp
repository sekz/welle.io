/*
 *    Copyright (C) 2025
 *    welle.io Thailand DAB+ Receiver
 *
 *    This file is part of the welle.io.
 *    Many of the ideas as implemented in welle.io are derived from
 *    other work, made available through the GNU general Public License.
 *    All copyrights of the original authors are recognized.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    welle.io is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with welle.io; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "location-code-manager.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

// Debug logging (define in build system for debug builds)
#ifdef DEBUG_LOCATION_CODE
#include <iostream>
#define LC_DEBUG(msg) std::cerr << "[LocationCode] " << msg << std::endl
#else
#define LC_DEBUG(msg)
#endif

LocationCodeManager::LocationCodeManager()
    : has_location_(false)
    , zone_(0)
    , l3_(0)
    , l4_(0)
    , l5_(0)
    , l6_(0)
    , checksum_(0)
{
}

bool LocationCodeManager::setReceiverLocation(const std::string& code)
{
    if (code.empty()) {
        LC_DEBUG("Empty location code");
        return false;
    }

    // Detect format
    bool success = false;
    if (code.find('-') != std::string::npos) {
        // Display format: "1255-4467-1352"
        success = parseDisplayFormat(code);
    } else if (code.find(':') != std::string::npos &&
               (code[0] == 'Z' || code[0] == 'z')) {
        // Hex format: "Z1:91BB82"
        success = parseHexFormat(code);
    } else {
        LC_DEBUG("Unknown format: " << code);
        return false;
    }

    if (!success) {
        has_location_ = false;
        return false;
    }

    // Validate checksum
    uint32_t location_24bit = (static_cast<uint32_t>(l3_) << 18) |
                              (static_cast<uint32_t>(l4_) << 12) |
                              (static_cast<uint32_t>(l5_) << 6) |
                              static_cast<uint32_t>(l6_);

    if (!validateChecksum(zone_, location_24bit, checksum_)) {
        LC_DEBUG("Checksum validation failed");
        has_location_ = false;
        return false;
    }

    has_location_ = true;
    LC_DEBUG("Location set: Zone=" << static_cast<int>(zone_)
             << " L3=" << static_cast<int>(l3_)
             << " L4=" << static_cast<int>(l4_)
             << " L5=" << static_cast<int>(l5_)
             << " L6=" << static_cast<int>(l6_)
             << " Checksum=0x" << std::hex << static_cast<int>(checksum_));
    return true;
}

void LocationCodeManager::clearReceiverLocation()
{
    has_location_ = false;
    zone_ = 0;
    l3_ = 0;
    l4_ = 0;
    l5_ = 0;
    l6_ = 0;
    checksum_ = 0;
}

bool LocationCodeManager::hasReceiverLocation() const
{
    return has_location_;
}

bool LocationCodeManager::matchesAlertLocation(const uint8_t* location_data, uint8_t nff) const
{
    if (!has_location_) {
        LC_DEBUG("No receiver location set");
        return false;
    }

    if (location_data == nullptr) {
        LC_DEBUG("Null location_data");
        return false;
    }

    // Extract alert location data
    uint8_t alert_zone, alert_l3, alert_l4, alert_l5, alert_l6, alert_checksum;
    extractLocationData(location_data, alert_zone, alert_l3, alert_l4,
                        alert_l5, alert_l6, alert_checksum);

    LC_DEBUG("Alert location: Zone=" << static_cast<int>(alert_zone)
             << " L3=" << static_cast<int>(alert_l3)
             << " L4=" << static_cast<int>(alert_l4)
             << " L5=" << static_cast<int>(alert_l5)
             << " L6=" << static_cast<int>(alert_l6)
             << " NFF=0x" << std::hex << static_cast<int>(nff));

    // Validate alert checksum
    uint32_t alert_location_24bit = (static_cast<uint32_t>(alert_l3) << 18) |
                                    (static_cast<uint32_t>(alert_l4) << 12) |
                                    (static_cast<uint32_t>(alert_l5) << 6) |
                                    static_cast<uint32_t>(alert_l6);

    if (!validateChecksum(alert_zone, alert_location_24bit, alert_checksum)) {
        LC_DEBUG("Alert checksum validation failed");
        return false;
    }

    // Zone must always match
    if (zone_ != alert_zone) {
        LC_DEBUG("Zone mismatch: receiver=" << static_cast<int>(zone_)
                 << " alert=" << static_cast<int>(alert_zone));
        return false;
    }

    // Match according to NFF (Nibble Fill Flag)
    bool matches = false;
    switch (nff) {
        case 0xF:
            // Match L3 only (coarsest)
            matches = matchL3(alert_l3);
            LC_DEBUG("NFF 0xF: L3 match=" << matches);
            break;

        case 0xE:
            // Match L3 + L4
            matches = matchL3L4(alert_l3, alert_l4);
            LC_DEBUG("NFF 0xE: L3+L4 match=" << matches);
            break;

        case 0xC:
            // Match L3 + L4 + L5
            matches = matchL3L4L5(alert_l3, alert_l4, alert_l5);
            LC_DEBUG("NFF 0xC: L3+L4+L5 match=" << matches);
            break;

        case 0x8:
            // Match all 4 levels (finest)
            matches = matchL3L4L5L6(alert_l3, alert_l4, alert_l5, alert_l6);
            LC_DEBUG("NFF 0x8: L3+L4+L5+L6 match=" << matches);
            break;

        default:
            LC_DEBUG("Invalid NFF value: 0x" << std::hex << static_cast<int>(nff));
            return false;
    }

    return matches;
}

std::string LocationCodeManager::toDisplayFormat() const
{
    if (!has_location_) {
        return "";
    }

    // Convert to 30-bit value: zone (6) + l3 (6) + l4 (6) + l5 (6) + l6 (6)
    uint32_t val30 = (static_cast<uint32_t>(zone_ & 0x3F) << 24) |
                     (static_cast<uint32_t>(l3_ & 0x3F) << 18) |
                     (static_cast<uint32_t>(l4_ & 0x3F) << 12) |
                     (static_cast<uint32_t>(l5_ & 0x3F) << 6) |
                     static_cast<uint32_t>(l6_ & 0x3F);

    // Split into 3 groups of 10 bits each
    uint16_t group1 = (val30 >> 20) & 0x3FF;  // Bits 29-20
    uint16_t group2 = (val30 >> 10) & 0x3FF;  // Bits 19-10
    uint16_t group3 = val30 & 0x3FF;          // Bits 9-0

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << group1 << "-"
        << std::setfill('0') << std::setw(4) << group2 << "-"
        << std::setfill('0') << std::setw(4) << group3;

    return oss.str();
}

std::string LocationCodeManager::toHexFormat() const
{
    if (!has_location_) {
        return "";
    }

    // 24-bit location code
    uint32_t location_24bit = (static_cast<uint32_t>(l3_) << 18) |
                              (static_cast<uint32_t>(l4_) << 12) |
                              (static_cast<uint32_t>(l5_) << 6) |
                              static_cast<uint32_t>(l6_);

    std::ostringstream oss;
    oss << "Z" << static_cast<int>(zone_) << ":"
        << std::uppercase << std::hex << std::setfill('0') << std::setw(6)
        << location_24bit;

    return oss.str();
}

uint8_t LocationCodeManager::getZone() const
{
    return has_location_ ? zone_ : 0xFF;
}

bool LocationCodeManager::getHierarchyLevels(uint8_t& l3, uint8_t& l4, uint8_t& l5, uint8_t& l6) const
{
    if (!has_location_) {
        return false;
    }

    l3 = l3_;
    l4 = l4_;
    l5 = l5_;
    l6 = l6_;
    return true;
}

bool LocationCodeManager::validateChecksum(uint8_t zone, uint32_t location_24bit, uint8_t checksum)
{
    uint8_t calculated = calculateChecksum(zone, location_24bit);
    return calculated == checksum;
}

uint8_t LocationCodeManager::calculateChecksum(uint8_t zone, uint32_t location_24bit)
{
    // Concatenate: zone (6 bits) + location (24 bits) = 30 bits = ~4 bytes
    // Pack into 4 bytes for XOR operation
    uint32_t val30 = (static_cast<uint32_t>(zone & 0x3F) << 24) |
                     (location_24bit & 0xFFFFFF);

    uint8_t byte0 = (val30 >> 24) & 0xFF;
    uint8_t byte1 = (val30 >> 16) & 0xFF;
    uint8_t byte2 = (val30 >> 8) & 0xFF;
    uint8_t byte3 = val30 & 0xFF;

    // XOR all bytes
    uint8_t xor_result = byte0 ^ byte1 ^ byte2 ^ byte3;

    // Invert
    uint8_t checksum = ~xor_result;

    return checksum;
}

// ============================================================================
// Private Helper Methods
// ============================================================================

bool LocationCodeManager::parseDisplayFormat(const std::string& code)
{
    // Expected format: "1255-4467-1352" (3 groups of 4 decimal digits)
    if (code.length() != 14) {  // 4 + 1 + 4 + 1 + 4
        LC_DEBUG("Display format length mismatch: " << code.length());
        return false;
    }

    if (code[4] != '-' || code[9] != '-') {
        LC_DEBUG("Display format missing dashes");
        return false;
    }

    // Extract groups
    std::string group1_str = code.substr(0, 4);
    std::string group2_str = code.substr(5, 4);
    std::string group3_str = code.substr(10, 4);

    // Check all digits
    if (!std::all_of(group1_str.begin(), group1_str.end(), ::isdigit) ||
        !std::all_of(group2_str.begin(), group2_str.end(), ::isdigit) ||
        !std::all_of(group3_str.begin(), group3_str.end(), ::isdigit)) {
        LC_DEBUG("Display format contains non-digit characters");
        return false;
    }

    // Convert to integers
    uint16_t group1 = static_cast<uint16_t>(std::stoi(group1_str));
    uint16_t group2 = static_cast<uint16_t>(std::stoi(group2_str));
    uint16_t group3 = static_cast<uint16_t>(std::stoi(group3_str));

    // Validate range (each group is 10 bits max = 1023)
    if (group1 > 1023 || group2 > 1023 || group3 > 1023) {
        LC_DEBUG("Display format group out of range");
        return false;
    }

    // Reconstruct 30-bit value
    uint32_t val30 = (static_cast<uint32_t>(group1) << 20) |
                     (static_cast<uint32_t>(group2) << 10) |
                     static_cast<uint32_t>(group3);

    // Extract fields (zone: 6 bits, l3-l6: 6 bits each)
    zone_ = (val30 >> 24) & 0x3F;
    l3_ = (val30 >> 18) & 0x3F;
    l4_ = (val30 >> 12) & 0x3F;
    l5_ = (val30 >> 6) & 0x3F;
    l6_ = val30 & 0x3F;

    // Validate zone (0-41)
    if (zone_ > 41) {
        LC_DEBUG("Zone out of range: " << static_cast<int>(zone_));
        return false;
    }

    // Calculate checksum
    uint32_t location_24bit = (static_cast<uint32_t>(l3_) << 18) |
                              (static_cast<uint32_t>(l4_) << 12) |
                              (static_cast<uint32_t>(l5_) << 6) |
                              static_cast<uint32_t>(l6_);
    checksum_ = calculateChecksum(zone_, location_24bit);

    return true;
}

bool LocationCodeManager::parseHexFormat(const std::string& code)
{
    // Expected format: "Z1:91BB82" or "z1:91bb82"
    // Format: Z<zone>:<6 hex digits>
    if (code.length() < 4) {  // Minimum: "Z0:0"
        LC_DEBUG("Hex format too short");
        return false;
    }

    if (code[0] != 'Z' && code[0] != 'z') {
        LC_DEBUG("Hex format missing 'Z' prefix");
        return false;
    }

    size_t colon_pos = code.find(':');
    if (colon_pos == std::string::npos || colon_pos < 2) {
        LC_DEBUG("Hex format missing or misplaced colon");
        return false;
    }

    // Extract zone string (between 'Z' and ':')
    std::string zone_str = code.substr(1, colon_pos - 1);
    if (!std::all_of(zone_str.begin(), zone_str.end(), ::isdigit)) {
        LC_DEBUG("Hex format zone contains non-digit characters");
        return false;
    }

    int zone_int = std::stoi(zone_str);
    if (zone_int < 0 || zone_int > 41) {
        LC_DEBUG("Hex format zone out of range: " << zone_int);
        return false;
    }
    zone_ = static_cast<uint8_t>(zone_int);

    // Extract hex location string (after ':')
    std::string hex_str = code.substr(colon_pos + 1);
    if (hex_str.length() != 6) {
        LC_DEBUG("Hex format location length mismatch: " << hex_str.length());
        return false;
    }

    // Check all hex digits
    if (!std::all_of(hex_str.begin(), hex_str.end(), ::isxdigit)) {
        LC_DEBUG("Hex format contains non-hex characters");
        return false;
    }

    // Convert to 24-bit integer
    uint32_t location_24bit;
    std::istringstream iss(hex_str);
    iss >> std::hex >> location_24bit;

    if (location_24bit > 0xFFFFFF) {
        LC_DEBUG("Hex format location exceeds 24 bits");
        return false;
    }

    // Extract hierarchy levels (6 bits each)
    l3_ = (location_24bit >> 18) & 0x3F;
    l4_ = (location_24bit >> 12) & 0x3F;
    l5_ = (location_24bit >> 6) & 0x3F;
    l6_ = location_24bit & 0x3F;

    // Calculate checksum
    checksum_ = calculateChecksum(zone_, location_24bit);

    return true;
}

void LocationCodeManager::extractLocationData(const uint8_t* location_data,
                                              uint8_t& zone,
                                              uint8_t& l3,
                                              uint8_t& l4,
                                              uint8_t& l5,
                                              uint8_t& l6,
                                              uint8_t& checksum)
{
    // Location data format (4 bytes):
    // Byte 0: Zone (6 bits) + L3 high (2 bits)
    // Byte 1: L3 low (4 bits) + L4 (4 bits)
    // Byte 2: L5 (6 bits) + L6 high (2 bits)
    // Byte 3: L6 low (4 bits) + Checksum (4 bits)

    zone = (location_data[0] >> 2) & 0x3F;  // Bits 7-2

    // L3: 6 bits split across byte 0 and byte 1
    uint8_t l3_high = location_data[0] & 0x03;       // Bits 1-0 of byte 0
    uint8_t l3_low = (location_data[1] >> 4) & 0x0F; // Bits 7-4 of byte 1
    l3 = (l3_high << 4) | l3_low;

    // L4: 4 bits from byte 1, padded to 6 bits
    l4 = location_data[1] & 0x0F;  // Bits 3-0 of byte 1

    // L5: 6 bits from byte 2
    l5 = (location_data[2] >> 2) & 0x3F;  // Bits 7-2

    // L6: 6 bits split across byte 2 and byte 3
    uint8_t l6_high = location_data[2] & 0x03;       // Bits 1-0 of byte 2
    uint8_t l6_low = (location_data[3] >> 4) & 0x0F; // Bits 7-4 of byte 3
    l6 = (l6_high << 4) | l6_low;

    // Checksum: 4 bits from byte 3 (assume remaining bits are checksum)
    // NOTE: ETSI spec may define full 8-bit checksum differently
    checksum = location_data[3] & 0x0F;  // Bits 3-0 of byte 3

    // For full 8-bit checksum validation, we need to calculate it
    // from zone + 24-bit location
    uint32_t location_24bit = (static_cast<uint32_t>(l3) << 18) |
                              (static_cast<uint32_t>(l4) << 12) |
                              (static_cast<uint32_t>(l5) << 6) |
                              static_cast<uint32_t>(l6);
    checksum = calculateChecksum(zone, location_24bit);
}

bool LocationCodeManager::matchL3(uint8_t alert_l3) const
{
    return l3_ == alert_l3;
}

bool LocationCodeManager::matchL3L4(uint8_t alert_l3, uint8_t alert_l4) const
{
    return (l3_ == alert_l3) && (l4_ == alert_l4);
}

bool LocationCodeManager::matchL3L4L5(uint8_t alert_l3, uint8_t alert_l4, uint8_t alert_l5) const
{
    return (l3_ == alert_l3) && (l4_ == alert_l4) && (l5_ == alert_l5);
}

bool LocationCodeManager::matchL3L4L5L6(uint8_t alert_l3, uint8_t alert_l4,
                                       uint8_t alert_l5, uint8_t alert_l6) const
{
    return (l3_ == alert_l3) && (l4_ == alert_l4) &&
           (l5_ == alert_l5) && (l6_ == alert_l6);
}
