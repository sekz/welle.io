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

#ifndef LOCATION_CODE_MANAGER_H
#define LOCATION_CODE_MANAGER_H

#include <cstdint>
#include <string>
#include <vector>

/**
 * @file location-code-manager.h
 * @brief DAB Emergency Warning System Location Code Manager
 *
 * Manages receiver location codes for EWS announcement filtering.
 * Implements ETSI TS 104 090 V1.1.2 location code format and matching.
 *
 * Location Code Format:
 * - Display: "1255-4467-1352" (3 groups of 4 decimal digits)
 * - Hex: "Z1:91BB82" (Zone + 24-bit location code)
 * - Binary structure:
 *   - Zone (0-41): 6 bits
 *   - L3 (Level 3): 6 bits  [most significant]
 *   - L4 (Level 4): 6 bits
 *   - L5 (Level 5): 6 bits
 *   - L6 (Level 6): 6 bits  [least significant]
 *   - Checksum: 8 bits
 *
 * Nibble Fill Flag (NFF) Matching:
 * - NFF 0xF: Match L3 only (coarsest, e.g., region)
 * - NFF 0xE: Match L3 + L4 (e.g., province)
 * - NFF 0xC: Match L3 + L4 + L5 (e.g., district)
 * - NFF 0x8: Match L3 + L4 + L5 + L6 (finest, e.g., sub-district)
 *
 * References:
 * - ETSI TS 104 090 V1.1.2 Section 5.1: Location code requirements
 * - Annex B: Location code format and checksum algorithm
 */

/**
 * @class LocationCodeManager
 * @brief Manages receiver location and matches against alert location codes
 *
 * Responsibilities:
 * 1. Parse and validate location codes (display/hex format)
 * 2. Store receiver location (zone + 4 hierarchy levels)
 * 3. Match alert location codes with NFF (Nibble Fill Flag)
 * 4. Validate checksums
 * 5. Convert between display and hex formats
 *
 * Thread Safety: This class is NOT thread-safe. Caller must synchronize access.
 *
 * Usage Example:
 * @code
 *   LocationCodeManager lcm;
 *
 *   // Set receiver location (user input)
 *   if (!lcm.setReceiverLocation("1255-4467-1352")) {
 *       std::cerr << "Invalid location code" << std::endl;
 *   }
 *
 *   // Check if alert matches receiver location
 *   uint8_t alert_data[4] = {0x09, 0x1B, 0xB8, 0x20}; // From FIG 0/19
 *   uint8_t nff = 0xE;  // Match L3 + L4
 *   if (lcm.matchesAlertLocation(alert_data, nff)) {
 *       // Switch to announcement
 *   }
 *
 *   // Display location to user
 *   std::string display = lcm.toDisplayFormat();  // "1255-4467-1352"
 * @endcode
 */
class LocationCodeManager {
public:
    /**
     * @brief Default constructor - no location set
     */
    LocationCodeManager();

    /**
     * @brief Destructor
     */
    ~LocationCodeManager() = default;

    // ========================================================================
    // Configuration Methods
    // ========================================================================

    /**
     * @brief Set receiver location from display format
     * @param code Location code in format "1255-4467-1352" or "Z1:91BB82"
     * @return true if valid and set, false if invalid format or checksum
     *
     * Accepts two formats:
     * - Display: "1255-4467-1352" (3 groups of 4 decimal digits)
     * - Hex: "Z1:91BB82" (Zone decimal + colon + 24-bit hex)
     *
     * Validates:
     * - Format correctness
     * - Zone range (0-41)
     * - Checksum validity
     */
    bool setReceiverLocation(const std::string& code);

    /**
     * @brief Clear receiver location (no location set)
     *
     * After clearing, all matchesAlertLocation() calls return false.
     */
    void clearReceiverLocation();

    /**
     * @brief Check if receiver location is set
     * @return true if location set, false otherwise
     */
    bool hasReceiverLocation() const;

    // ========================================================================
    // Alert Matching
    // ========================================================================

    /**
     * @brief Check if alert location matches receiver location
     * @param location_data Location data from FIG 0/19 (4 bytes)
     * @param nff Nibble Fill Flag (determines matching granularity)
     * @return true if alert matches receiver location, false otherwise
     *
     * Location data format (4 bytes):
     * - Byte 0: Zone (6 bits) + L3 high (2 bits)
     * - Byte 1: L3 low (4 bits) + L4 (4 bits)
     * - Byte 2: L5 (6 bits) + L6 high (2 bits)
     * - Byte 3: L6 low (4 bits) + Checksum high (4 bits)
     *
     * NFF (Nibble Fill Flag) matching:
     * - 0xF: Match L3 only
     * - 0xE: Match L3 + L4
     * - 0xC: Match L3 + L4 + L5
     * - 0x8: Match L3 + L4 + L5 + L6
     *
     * Returns false if:
     * - No receiver location set
     * - Invalid NFF value
     * - Zone mismatch
     * - Hierarchy level mismatch (according to NFF)
     * - Invalid checksum in location_data
     */
    bool matchesAlertLocation(const uint8_t* location_data, uint8_t nff) const;

    // ========================================================================
    // Format Conversion
    // ========================================================================

    /**
     * @brief Convert to display format
     * @return Location code in format "1255-4467-1352"
     *
     * Returns empty string if no location set.
     */
    std::string toDisplayFormat() const;

    /**
     * @brief Convert to hex format
     * @return Location code in format "Z1:91BB82"
     *
     * Returns empty string if no location set.
     */
    std::string toHexFormat() const;

    /**
     * @brief Get zone number
     * @return Zone (0-41), or 0xFF if no location set
     */
    uint8_t getZone() const;

    /**
     * @brief Get hierarchy levels
     * @param l3 Output: Level 3 (0-63)
     * @param l4 Output: Level 4 (0-63)
     * @param l5 Output: Level 5 (0-63)
     * @param l6 Output: Level 6 (0-63)
     * @return true if location set, false otherwise
     */
    bool getHierarchyLevels(uint8_t& l3, uint8_t& l4, uint8_t& l5, uint8_t& l6) const;

    // ========================================================================
    // Validation
    // ========================================================================

    /**
     * @brief Validate checksum of location code
     * @param zone Zone (0-41)
     * @param location_24bit 24-bit location code (L3|L4|L5|L6)
     * @param checksum Checksum to validate
     * @return true if checksum valid, false otherwise
     *
     * Checksum algorithm (ETSI TS 104 090 Annex B):
     * 1. Concatenate: zone (6 bits) + location (24 bits) = 30 bits
     * 2. XOR bytes: checksum = byte0 ^ byte1 ^ byte2 ^ byte3
     * 3. Invert: checksum = ~checksum
     *
     * Static method for testing purposes.
     */
    static bool validateChecksum(uint8_t zone, uint32_t location_24bit, uint8_t checksum);

    /**
     * @brief Calculate checksum for location code
     * @param zone Zone (0-41)
     * @param location_24bit 24-bit location code (L3|L4|L5|L6)
     * @return Calculated checksum (8 bits)
     */
    static uint8_t calculateChecksum(uint8_t zone, uint32_t location_24bit);

private:
    // ========================================================================
    // Internal State
    // ========================================================================

    bool has_location_;        ///< true if location is set
    uint8_t zone_;             ///< Zone (0-41), 6 bits
    uint8_t l3_;               ///< Level 3 (0-63), 6 bits
    uint8_t l4_;               ///< Level 4 (0-63), 6 bits
    uint8_t l5_;               ///< Level 5 (0-63), 6 bits
    uint8_t l6_;               ///< Level 6 (0-63), 6 bits
    uint8_t checksum_;         ///< Checksum (8 bits)

    // ========================================================================
    // Helper Methods
    // ========================================================================

    /**
     * @brief Parse display format "1255-4467-1352"
     * @param code Input string
     * @return true if valid format, false otherwise
     */
    bool parseDisplayFormat(const std::string& code);

    /**
     * @brief Parse hex format "Z1:91BB82"
     * @param code Input string
     * @return true if valid format, false otherwise
     */
    bool parseHexFormat(const std::string& code);

    /**
     * @brief Extract location data from FIG 0/19 bytes
     * @param location_data 4-byte location data
     * @param zone Output: zone
     * @param l3 Output: L3
     * @param l4 Output: L4
     * @param l5 Output: L5
     * @param l6 Output: L6
     * @param checksum Output: checksum
     */
    static void extractLocationData(const uint8_t* location_data,
                                    uint8_t& zone,
                                    uint8_t& l3,
                                    uint8_t& l4,
                                    uint8_t& l5,
                                    uint8_t& l6,
                                    uint8_t& checksum);

    /**
     * @brief Match L3 level
     * @param alert_l3 Alert L3 value
     * @return true if matches receiver L3
     */
    bool matchL3(uint8_t alert_l3) const;

    /**
     * @brief Match L3 + L4 levels
     * @param alert_l3 Alert L3 value
     * @param alert_l4 Alert L4 value
     * @return true if matches receiver L3 + L4
     */
    bool matchL3L4(uint8_t alert_l3, uint8_t alert_l4) const;

    /**
     * @brief Match L3 + L4 + L5 levels
     * @param alert_l3 Alert L3 value
     * @param alert_l4 Alert L4 value
     * @param alert_l5 Alert L5 value
     * @return true if matches receiver L3 + L4 + L5
     */
    bool matchL3L4L5(uint8_t alert_l3, uint8_t alert_l4, uint8_t alert_l5) const;

    /**
     * @brief Match all 4 levels
     * @param alert_l3 Alert L3 value
     * @param alert_l4 Alert L4 value
     * @param alert_l5 Alert L5 value
     * @param alert_l6 Alert L6 value
     * @return true if matches receiver L3 + L4 + L5 + L6
     */
    bool matchL3L4L5L6(uint8_t alert_l3, uint8_t alert_l4, uint8_t alert_l5, uint8_t alert_l6) const;
};

#endif // LOCATION_CODE_MANAGER_H
