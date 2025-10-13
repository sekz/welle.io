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

#ifndef ANNOUNCEMENT_TYPE_HELPER_H
#define ANNOUNCEMENT_TYPE_HELPER_H

#include <QObject>
#include "../backend/announcement-types.h"

/**
 * @brief QML helper for accessing announcement type data
 *
 * This class provides a bridge between C++ AnnouncementType enum and QML.
 * It exposes announcement types, names, priorities, and colors to QML
 * components, ensuring consistency across the application.
 *
 * P1-1 FIX (IMPROVE-001): Centralize Announcement Type Definitions
 * Previously, announcement types were hardcoded in:
 * - radio_controller.cpp:1274-1276 (loop 0-10)
 * - AnnouncementIndicator.qml:94-106
 * - AnnouncementHistory.qml:216-228
 * - AnnouncementSettings.qml:290-302
 *
 * Now all QML components use this helper via Q_ENUM and Q_INVOKABLE methods.
 */
class AnnouncementTypeHelper : public QObject
{
    Q_OBJECT

public:
    // Expose AnnouncementType enum to QML
    enum Type {
        Alarm = 0,
        RoadTraffic = 1,
        TransportFlash = 2,
        Warning = 3,
        News = 4,
        Weather = 5,
        Event = 6,
        SpecialEvent = 7,
        ProgrammeInfo = 8,
        Sport = 9,
        Financial = 10,
        MAX_TYPE = 10
    };
    Q_ENUM(Type)

    explicit AnnouncementTypeHelper(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief Get English name for announcement type
     * @param type Announcement type (0-10)
     * @return English name string
     */
    Q_INVOKABLE QString getTypeName(int type) const {
        if (type < 0 || type > MAX_TYPE) {
            return tr("UNKNOWN");
        }
        return QString::fromUtf8(getAnnouncementTypeName(static_cast<AnnouncementType>(type)));
    }

    /**
     * @brief Get Thai name for announcement type
     * @param type Announcement type (0-10)
     * @return Thai name string in UTF-8
     */
    Q_INVOKABLE QString getTypeNameThai(int type) const {
        if (type < 0 || type > MAX_TYPE) {
            return tr("ไม่ทราบประเภท");
        }
        return QString::fromUtf8(getAnnouncementTypeNameThai(static_cast<AnnouncementType>(type)));
    }

    /**
     * @brief Get priority level for announcement type
     * @param type Announcement type (0-10)
     * @return Priority level (1-11, where 1 is highest)
     */
    Q_INVOKABLE int getPriority(int type) const {
        if (type < 0 || type > MAX_TYPE) {
            return 99; // Invalid priority
        }
        return getAnnouncementPriority(static_cast<AnnouncementType>(type));
    }

    /**
     * @brief Get color for announcement type (for UI display)
     * @param type Announcement type (0-10)
     * @return HTML color code string
     */
    Q_INVOKABLE QString getColor(int type) const {
        // Color scheme for announcement types
        // Critical announcements: Red/Orange tones
        // Informational: Blue/Green tones
        switch (type) {
            case Alarm: return "#FF0000";           // Red - Highest priority
            case RoadTraffic: return "#FFCC00";     // Yellow - Traffic
            case TransportFlash: return "#2196F3";  // Blue - Transport
            case Warning: return "#FF9800";         // Orange - Warning
            case News: return "#F44336";            // Red - News
            case Weather: return "#03A9F4";         // Light Blue - Weather
            case Event: return "#9C27B0";           // Purple - Event
            case SpecialEvent: return "#E91E63";    // Pink - Special
            case ProgrammeInfo: return "#009688";   // Teal - Info
            case Sport: return "#4CAF50";           // Green - Sport
            case Financial: return "#795548";       // Brown - Finance
            default: return "#757575";              // Gray - Unknown
        }
    }

    /**
     * @brief Get icon code for announcement type (Material Design Icons)
     * @param type Announcement type (0-10)
     * @return Unicode icon code string
     */
    Q_INVOKABLE QString getIcon(int type) const {
        switch (type) {
            case Alarm: return "\ue855";           // warning icon
            case RoadTraffic: return "\ue558";     // traffic icon
            case TransportFlash: return "\ue531";  // directions_bus icon
            case Warning: return "\ue002";         // error_outline icon
            case News: return "\ue639";            // article icon
            case Weather: return "\ue430";         // wb_sunny icon
            case Event: return "\ue878";           // event icon
            case SpecialEvent: return "\ue885";    // stars icon
            case ProgrammeInfo: return "\ue88e";   // info icon
            case Sport: return "\ue52f";           // sports_soccer icon
            case Financial: return "\ue227";       // attach_money icon
            default: return "\ue88e";              // info icon (fallback)
        }
    }

    /**
     * @brief Get maximum valid announcement type value
     * @return Maximum type value (10)
     */
    Q_INVOKABLE int getMaxType() const {
        return MAX_TYPE;
    }

    /**
     * @brief Get complete type data as QVariantMap
     * @param type Announcement type (0-10)
     * @return QVariantMap with all type data
     */
    Q_INVOKABLE QVariantMap getTypeData(int type) const {
        QVariantMap data;
        data["type"] = type;
        data["nameEn"] = getTypeName(type);
        data["nameTh"] = getTypeNameThai(type);
        data["priority"] = getPriority(type);
        data["color"] = getColor(type);
        data["icon"] = getIcon(type);
        return data;
    }

    /**
     * @brief Get all announcement types as list of QVariantMap
     * @return QVariantList containing all type data
     */
    Q_INVOKABLE QVariantList getAllTypes() const {
        QVariantList types;
        for (int i = 0; i <= MAX_TYPE; ++i) {
            types.append(getTypeData(i));
        }
        return types;
    }
};

#endif // ANNOUNCEMENT_TYPE_HELPER_H
