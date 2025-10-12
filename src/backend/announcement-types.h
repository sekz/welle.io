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
 * - FIG 0/18: Announcement Support
 * - FIG 0/19: Announcement Switching
 * 
 * Thailand DAB+ specific considerations:
 * - Emergency Alert (Alarm) announcements have highest priority
 * - Traffic announcements critical for Bangkok metro area
 * - Support for Thai character set (Profile 0x0E) in announcement labels
 * - NBTC compliance per ผว. 104-2567
 */

#ifndef __ANNOUNCEMENT_TYPES_H__
#define __ANNOUNCEMENT_TYPES_H__

#include <cstdint>
#include <vector>
#include <set>
#include <string>
#include <chrono>

/**
 * @brief Announcement types as defined in ETSI EN 300 401 Table 14
 * 
 * These types are transmitted in FIG 0/18 (Announcement Support) and
 * FIG 0/19 (Announcement Switching) with corresponding bit positions
 * in the 16-bit announcement support/switching flags.
 */
enum class AnnouncementType : uint8_t {
    Alarm = 0,           ///< Emergency warning (highest priority)
    Traffic = 1,         ///< Traffic flash
    Transport = 2,       ///< Transport flash (public transport)
    Warning = 3,         ///< Warning/Service
    News = 4,            ///< News flash
    Weather = 5,         ///< Weather forecast
    Event = 6,           ///< Event announcement
    Special = 7,         ///< Special event
    ProgramInfo = 8,     ///< Programme information
    Sport = 9,           ///< Sport news
    Finance = 10         ///< Financial news
};

/**
 * @brief Announcement state machine states
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
 * @brief 16-bit announcement support/switching flags
 */
struct AnnouncementSupportFlags {
    uint16_t flags = 0;  ///< Bit field of supported announcement types
    
    void set(AnnouncementType type) {
        flags |= (1 << static_cast<uint8_t>(type));
    }
    
    void clear(AnnouncementType type) {
        flags &= ~(1 << static_cast<uint8_t>(type));
    }
    
    bool supports(AnnouncementType type) const {
        return (flags & (1 << static_cast<uint8_t>(type))) != 0;
    }
    
    bool hasAny() const {
        return flags != 0;
    }
};

/**
 * @brief Active announcement information (parsed from FIG 0/19)
 */
struct ActiveAnnouncement {
    uint8_t cluster_id = 0;                           ///< Announcement cluster ID
    AnnouncementSupportFlags active_flags;            ///< Active announcement types
    uint8_t subchannel_id = 0;                        ///< Subchannel carrying announcement
    bool is_new_flag = false;                         ///< New flag from FIG 0/19
    bool is_region_flag = false;                      ///< Region flag from FIG 0/19
    std::chrono::system_clock::time_point start_time;      ///< When announcement started
    std::chrono::system_clock::time_point last_update;     ///< Last FIG 0/19 received
    
    ActiveAnnouncement() 
        : start_time(std::chrono::system_clock::now())
        , last_update(std::chrono::system_clock::now()) {}
    
    bool isActive() const {
        return active_flags.flags != 0x0000;
    }
};

#endif // __ANNOUNCEMENT_TYPES_H__
