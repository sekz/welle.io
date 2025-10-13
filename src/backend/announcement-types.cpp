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

#include "announcement-types.h"
#include <iostream>

// For Qt translation if Qt is existing
#ifdef QT_CORE_LIB
    #include <QtGlobal>
#else
    #define QT_TR_NOOP(x) (x)
#endif

/**
 * @brief Get English name for announcement type
 *
 * Names follow ETSI EN 300 401 V2.1.1 Table 14
 * These strings are marked for Qt translation with QT_TR_NOOP
 */
const char* getAnnouncementTypeName(AnnouncementType type)
{
    const char* typeName = "";

    switch (type) {
        case AnnouncementType::Alarm:
            typeName = QT_TR_NOOP("Alarm");
            break;
        case AnnouncementType::RoadTraffic:
            typeName = QT_TR_NOOP("Road Traffic");
            break;
        case AnnouncementType::TransportFlash:
            typeName = QT_TR_NOOP("Transport Flash");
            break;
        case AnnouncementType::Warning:
            typeName = QT_TR_NOOP("Warning/Service");
            break;
        case AnnouncementType::News:
            typeName = QT_TR_NOOP("News Flash");
            break;
        case AnnouncementType::Weather:
            typeName = QT_TR_NOOP("Area Weather");
            break;
        case AnnouncementType::Event:
            typeName = QT_TR_NOOP("Event Announcement");
            break;
        case AnnouncementType::SpecialEvent:
            typeName = QT_TR_NOOP("Special Event");
            break;
        case AnnouncementType::ProgrammeInfo:
            typeName = QT_TR_NOOP("Programme Information");
            break;
        case AnnouncementType::Sport:
            typeName = QT_TR_NOOP("Sport Report");
            break;
        case AnnouncementType::Financial:
            typeName = QT_TR_NOOP("Financial Report");
            break;
        default:
            typeName = QT_TR_NOOP("UNKNOWN");
            std::clog << "AnnouncementTypes: Unknown announcement type: "
                      << static_cast<int>(type) << std::endl;
            break;
    }

    return typeName;
}

/**
 * @brief Get Thai name for announcement type
 *
 * Thai translations for ETSI EN 300 401 Table 14 announcement types
 * These are direct UTF-8 encoded Thai strings, suitable for display
 * in Thai DAB+ receivers per NBTC requirements
 */
const char* getAnnouncementTypeNameThai(AnnouncementType type)
{
    const char* thaiName = "";

    switch (type) {
        case AnnouncementType::Alarm:
            // "การเตือนภัยฉุกเฉิน" (Emergency Alert)
            thaiName = "การเตือนภัยฉุกเฉิน";
            break;
        case AnnouncementType::RoadTraffic:
            // "ข่าวจราจร" (Traffic News)
            thaiName = "ข่าวจราจร";
            break;
        case AnnouncementType::TransportFlash:
            // "ข่าวการเดินทาง" (Transport News)
            thaiName = "ข่าวการเดินทาง";
            break;
        case AnnouncementType::Warning:
            // "คำเตือน/บริการ" (Warning/Service)
            thaiName = "คำเตือน/บริการ";
            break;
        case AnnouncementType::News:
            // "ข่าวด่วน" (News Flash)
            thaiName = "ข่าวด่วน";
            break;
        case AnnouncementType::Weather:
            // "พยากรณ์อากาศ" (Weather Forecast)
            thaiName = "พยากรณ์อากาศ";
            break;
        case AnnouncementType::Event:
            // "ประกาศเหตุการณ์" (Event Announcement)
            thaiName = "ประกาศเหตุการณ์";
            break;
        case AnnouncementType::SpecialEvent:
            // "เหตุการณ์พิเศษ" (Special Event)
            thaiName = "เหตุการณ์พิเศษ";
            break;
        case AnnouncementType::ProgrammeInfo:
            // "ข้อมูลรายการ" (Programme Information)
            thaiName = "ข้อมูลรายการ";
            break;
        case AnnouncementType::Sport:
            // "ข่าวกีฬา" (Sport News)
            thaiName = "ข่าวกีฬา";
            break;
        case AnnouncementType::Financial:
            // "ข่าวการเงิน" (Financial News)
            thaiName = "ข่าวการเงิน";
            break;
        default:
            // "ไม่ทราบประเภท" (Unknown Type)
            thaiName = "ไม่ทราบประเภท";
            std::clog << "AnnouncementTypes: Unknown announcement type for Thai translation: "
                      << static_cast<int>(type) << std::endl;
            break;
    }

    return thaiName;
}

/**
 * @brief Get priority level for announcement type
 *
 * Priority levels are defined in ETSI EN 300 401 Section 8.1.6.1
 * Lower numbers indicate higher priority.
 *
 * Priority mapping:
 * - Alarm (0) -> Priority 1 (highest)
 * - RoadTraffic (1) -> Priority 2
 * - TransportFlash (2) -> Priority 3
 * - Warning (3) -> Priority 4
 * - News (4) -> Priority 5
 * - Weather (5) -> Priority 6
 * - Event (6) -> Priority 7
 * - SpecialEvent (7) -> Priority 8
 * - ProgrammeInfo (8) -> Priority 9
 * - Sport (9) -> Priority 10
 * - Financial (10) -> Priority 11 (lowest)
 *
 * @param type The announcement type
 * @return Priority level (1-11, where 1 is highest priority)
 */
int getAnnouncementPriority(AnnouncementType type)
{
    // Direct mapping: type value + 1 = priority level
    // This works because the enum values are ordered by priority
    uint8_t typeValue = static_cast<uint8_t>(type);

    // Validate type is in valid range
    if (typeValue > static_cast<uint8_t>(AnnouncementType::MAX_TYPE)) {
        std::clog << "AnnouncementTypes: Invalid announcement type for priority: "
                  << static_cast<int>(type) << std::endl;
        return 99; // Return very low priority for invalid types
    }

    // Priority = type value + 1
    // Alarm (0) -> 1 (highest priority)
    // Financial (10) -> 11 (lowest priority)
    return typeValue + 1;
}
