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

/**
 * @file announcement-types.h
 * @brief Data structures for DAB announcement support and switching
 *
 * This file implements data structures for announcement support as defined in:
 * - ETSI EN 300 401 V2.1.1 Clause 8.1.6 (Announcement Support and Switching)
 * - ETSI EN 300 401 V2.1.1 Table 14 (Announcement types)
 * - FIG 0/18: Announcement Support (ASu flags)
 * - FIG 0/19: Announcement Switching (ASw flags)
 *
 * Announcement Priority Order (ETSI EN 300 401 Section 8.1.6.1):
 * Priority 1 (Highest): Alarm
 * Priority 2: Road Traffic flash
 * Priority 3: Transport flash
 * Priority 4: Warning/Service
 * Priority 5: News flash
 * Priority 6: Area weather flash
 * Priority 7: Event announcement
 * Priority 8: Special event
 * Priority 9: Programme information
 * Priority 10: Sport report
 * Priority 11: Financial report
 *
 * Thailand DAB+ specific considerations:
 * - Emergency Alert (Alarm) announcements have highest priority per NBTC requirements
 * - Traffic announcements critical for Bangkok metro area congestion management
 * - Support for Thai character set (Profile 0x0E) in announcement labels
 * - NBTC compliance per ผว. 104-2567 (Broadcasting Business Act)
 */

#ifndef __ANNOUNCEMENT_TYPES_H__
#define __ANNOUNCEMENT_TYPES_H__

#include <cstdint>
#include <vector>
#include <chrono>

/**
 * @brief Announcement types as defined in ETSI EN 300 401 Table 14
 *
 * These types are transmitted in FIG 0/18 (Announcement Support) and
 * FIG 0/19 (Announcement Switching) with corresponding bit positions
 * in the 16-bit announcement support/switching flags.
 *
 * Bit Position Mapping:
 * - Bit 0 (MSB): Alarm
 * - Bit 1: Road Traffic flash
 * - Bit 2: Transport flash
 * - Bit 3: Warning/Service
 * - Bit 4: News flash
 * - Bit 5: Area weather flash
 * - Bit 6: Event announcement
 * - Bit 7: Special event
 * - Bit 8: Programme information
 * - Bit 9: Sport report
 * - Bit 10: Financial report
 * - Bits 11-15: Reserved for future use
 */
enum class AnnouncementType : uint8_t {
    Alarm = 0,           ///< Bit 0 - Emergency warning (highest priority)
    RoadTraffic = 1,     ///< Bit 1 - Road traffic flash
    TransportFlash = 2,  ///< Bit 2 - Transport flash (public transport)
    Warning = 3,         ///< Bit 3 - Warning/Service
    News = 4,            ///< Bit 4 - News flash
    Weather = 5,         ///< Bit 5 - Area weather flash
    Event = 6,           ///< Bit 6 - Event announcement
    SpecialEvent = 7,    ///< Bit 7 - Special event
    ProgrammeInfo = 8,   ///< Bit 8 - Programme information
    Sport = 9,           ///< Bit 9 - Sport report
    Financial = 10,      ///< Bit 10 - Financial report
    MAX_TYPE = 10        ///< Maximum valid announcement type value
};

/**
 * @brief Announcement state machine states
 *
 * State transitions follow ETSI EN 300 401 Section 8.1.6.3:
 * Idle -> AnnouncementDetected (FIG 0/19 received with ASw != 0x0000)
 * AnnouncementDetected -> SwitchingToAnnouncement (user accepts or auto-switch)
 * SwitchingToAnnouncement -> PlayingAnnouncement (subchannel tuned)
 * PlayingAnnouncement -> AnnouncementEnding (FIG 0/19 with ASw = 0x0000)
 * AnnouncementEnding -> RestoringOriginalService (switching back)
 * RestoringOriginalService -> Idle (original service restored)
 */
enum class AnnouncementState : uint8_t {
    Idle = 0,                       ///< No announcement, playing normal service
    AnnouncementDetected,           ///< FIG 0/19 received, announcement available
    SwitchingToAnnouncement,        ///< Switching to announcement subchannel
    PlayingAnnouncement,            ///< Playing announcement audio
    AnnouncementEnding,             ///< Announcement ended, preparing to restore
    RestoringOriginalService        ///< Switching back to original service
};

/**
 * @brief 16-bit announcement support/switching flags (ASu/ASw)
 *
 * ASu (Announcement Support): Indicates which announcement types a service supports
 * ASw (Announcement Switching): Indicates which announcement types are currently active
 *
 * Per ETSI EN 300 401:
 * - ASu is transmitted in FIG 0/18
 * - ASw is transmitted in FIG 0/19
 * - Bit positions correspond to AnnouncementType enum values
 * - ASw = 0x0000 indicates no active announcements (end of announcement)
 */
struct AnnouncementSupportFlags {
    uint16_t flags = 0;  ///< Bit field: bit N set means announcement type N is supported/active

    /**
     * @brief Set support/active flag for given announcement type
     * @param type The announcement type to enable
     */
    void set(AnnouncementType type) {
        if (static_cast<uint8_t>(type) <= static_cast<uint8_t>(AnnouncementType::MAX_TYPE)) {
            flags |= (1 << static_cast<uint8_t>(type));
        }
    }

    /**
     * @brief Clear support/active flag for given announcement type
     * @param type The announcement type to disable
     */
    void clear(AnnouncementType type) {
        if (static_cast<uint8_t>(type) <= static_cast<uint8_t>(AnnouncementType::MAX_TYPE)) {
            flags &= ~(1 << static_cast<uint8_t>(type));
        }
    }

    /**
     * @brief Check if given announcement type is supported/active
     * @param type The announcement type to check
     * @return true if the type is supported/active, false otherwise
     */
    bool supports(AnnouncementType type) const {
        if (static_cast<uint8_t>(type) > static_cast<uint8_t>(AnnouncementType::MAX_TYPE)) {
            return false;
        }
        return (flags & (1 << static_cast<uint8_t>(type))) != 0;
    }

    /**
     * @brief Check if any announcement types are supported/active
     * @return true if flags != 0x0000, false otherwise
     */
    bool hasAny() const {
        return flags != 0;
    }

    /**
     * @brief Get list of all active announcement types
     * @return Vector of announcement types that are currently set
     */
    std::vector<AnnouncementType> getActiveTypes() const {
        std::vector<AnnouncementType> active;
        for (uint8_t i = 0; i <= static_cast<uint8_t>(AnnouncementType::MAX_TYPE); ++i) {
            if (flags & (1 << i)) {
                active.push_back(static_cast<AnnouncementType>(i));
            }
        }
        return active;
    }
};

/**
 * @brief Active announcement information (parsed from FIG 0/19)
 *
 * FIG 0/19 structure (ETSI EN 300 401 Section 6.3.5):
 * - Cluster ID: Identifies the announcement cluster
 * - ASw flags: Active announcement types (16 bits)
 * - SubChId: Subchannel carrying the announcement
 * - New flag: Indicates new announcement
 * - Region flag: Indicates if announcement is region-specific
 *
 * When ASw = 0x0000, the announcement has ended.
 */
struct ActiveAnnouncement {
    uint8_t cluster_id = 0;                                   ///< Announcement cluster ID (0-255)
    AnnouncementSupportFlags active_flags;                    ///< ASw: Active announcement types
    uint8_t subchannel_id = 0;                                ///< SubChId carrying announcement
    bool new_flag = false;                                    ///< New flag from FIG 0/19
    bool region_flag = false;                                 ///< Region flag from FIG 0/19
    std::chrono::steady_clock::time_point start_time;         ///< When announcement was first detected
    std::chrono::steady_clock::time_point last_update;        ///< Last FIG 0/19 update time

    // EWS location data (ETSI TS 104 090)
    bool has_location_data = false;                           ///< true if location_data is valid
    uint8_t location_data[4] = {0};                           ///< Location code (4 bytes from FIG 0/19)
    uint8_t location_nff = 0;                                 ///< Nibble Fill Flag (0xF, 0xE, 0xC, 0x8)

    /**
     * @brief Default constructor initializes timestamps to now
     */
    ActiveAnnouncement()
        : start_time(std::chrono::steady_clock::now())
        , last_update(std::chrono::steady_clock::now()) {}

    /**
     * @brief Check if announcement is currently active
     * @return true if ASw != 0x0000, false if announcement has ended
     */
    bool isActive() const {
        return active_flags.flags != 0x0000;
    }

    /**
     * @brief Get the highest priority announcement type currently active
     * @return The highest priority AnnouncementType, or Alarm if none active
     *
     * Priority order follows ETSI EN 300 401 Section 8.1.6.1:
     * Alarm (0) has highest priority, Financial (10) has lowest.
     */
    AnnouncementType getHighestPriorityType() const {
        if (!isActive()) {
            return AnnouncementType::Alarm; // Default to highest priority
        }

        // Scan from highest priority (0) to lowest priority (10)
        for (uint8_t i = 0; i <= static_cast<uint8_t>(AnnouncementType::MAX_TYPE); ++i) {
            if (active_flags.supports(static_cast<AnnouncementType>(i))) {
                return static_cast<AnnouncementType>(i);
            }
        }

        return AnnouncementType::Alarm; // Should never reach here if isActive() is true
    }
};

/**
 * @brief Service announcement support configuration (parsed from FIG 0/18)
 *
 * FIG 0/18 structure (ETSI EN 300 401 Section 6.3.4):
 * - SId: Service ID (16-bit or 32-bit)
 * - ASu flags: Supported announcement types (16 bits)
 * - Number of clusters: Count of announcement clusters
 * - Cluster IDs: List of cluster IDs this service participates in
 *
 * A service can support multiple announcement clusters, allowing it to
 * receive announcements from different sources (e.g., local and national).
 */
struct ServiceAnnouncementSupport {
    uint32_t service_id = 0;                    ///< Service ID (SId)
    AnnouncementSupportFlags support_flags;     ///< ASu: Supported announcement types
    std::vector<uint8_t> cluster_ids;           ///< List of cluster IDs (0-255)

    /**
     * @brief Check if service supports given announcement type
     * @param type The announcement type to check
     * @return true if service supports this announcement type
     */
    bool supportsType(AnnouncementType type) const {
        return support_flags.supports(type);
    }

    /**
     * @brief Check if service participates in given cluster
     * @param cluster_id The cluster ID to check
     * @return true if service is in this cluster
     */
    bool inCluster(uint8_t cluster_id) const {
        for (auto id : cluster_ids) {
            if (id == cluster_id) {
                return true;
            }
        }
        return false;
    }
};

/**
 * @brief Get English name for announcement type
 * @param type The announcement type
 * @return English name string (e.g., "Alarm", "Road Traffic")
 */
const char* getAnnouncementTypeName(AnnouncementType type);

/**
 * @brief Get Thai name for announcement type
 * @param type The announcement type
 * @return Thai name string in UTF-8 encoding
 */
const char* getAnnouncementTypeNameThai(AnnouncementType type);

/**
 * @brief Get priority level for announcement type
 * @param type The announcement type
 * @return Priority level (1-11, where 1 is highest priority)
 *
 * Priority mapping per ETSI EN 300 401 Section 8.1.6.1:
 * - Alarm: Priority 1 (highest)
 * - RoadTraffic: Priority 2
 * - TransportFlash: Priority 3
 * - Warning: Priority 4
 * - News: Priority 5
 * - Weather: Priority 6
 * - Event: Priority 7
 * - SpecialEvent: Priority 8
 * - ProgrammeInfo: Priority 9
 * - Sport: Priority 10
 * - Financial: Priority 11 (lowest)
 */
int getAnnouncementPriority(AnnouncementType type);

#endif // __ANNOUNCEMENT_TYPES_H__
