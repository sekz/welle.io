/*
 *    Copyright (C) 2024
 *    welle.io Thailand DAB+ Compliance
 *
 *    This file is part of the welle.io Thailand DAB+ compliance implementation.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ScrollView {
    id: thaiEPGViewer

    property bool preferThai: true
    property string thaiFont: "Noto Sans Thai"
    property color primaryTextColor: "#333333"
    property color secondaryTextColor: "#666666"
    property color timeTextColor: "#2196F3"
    property color backgroundEvenColor: "#FAFAFA"
    property color backgroundOddColor: "white"
    property color currentProgrammeColor: "#E8F5E8"
    property alias model: epgListView.model

    signal programmeSelected(var programmeInfo)
    signal programmeContextMenu(var programmeInfo, point position)

    contentWidth: availableWidth

    ListView {
        id: epgListView
        width: parent.width

        delegate: Rectangle {
            id: programmeDelegate
            width: ListView.view.width
            height: Math.max(100, contentColumn.implicitHeight + 20)

            property bool isCurrent: {
                if (!model.start_time || !model.end_time) return false
                var now = new Date()
                var startTime = new Date(model.start_time)
                var endTime = new Date(model.end_time)
                return now >= startTime && now <= endTime
            }

            color: isCurrent ? currentProgrammeColor :
                   (index % 2 === 0 ? backgroundEvenColor : backgroundOddColor)

            border.color: isCurrent ? "#4CAF50" : "#E0E0E0"
            border.width: isCurrent ? 2 : 1

            MouseArea {
                anchors.fill: parent
                onClicked: programmeSelected(model)
                onPressAndHold: programmeContextMenu(model, Qt.point(mouseX, mouseY))

                // Right-click context menu
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onPressed: {
                    if (mouse.button === Qt.RightButton) {
                        programmeContextMenu(model, Qt.point(mouseX, mouseY))
                    }
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                // Time column
                Column {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredWidth: 80
                    spacing: 4

                    Text {
                        text: formatTime(model.start_time)
                        font.pixelSize: 12
                        font.weight: Font.Bold
                        color: timeTextColor
                        width: parent.width
                        horizontalAlignment: Text.AlignCenter
                    }

                    Text {
                        text: "—"
                        font.pixelSize: 10
                        color: secondaryTextColor
                        width: parent.width
                        horizontalAlignment: Text.AlignCenter
                    }

                    Text {
                        text: formatTime(model.end_time)
                        font.pixelSize: 12
                        font.weight: Font.Bold
                        color: timeTextColor
                        width: parent.width
                        horizontalAlignment: Text.AlignCenter
                    }

                    Text {
                        text: calculateDuration(model.start_time, model.end_time)
                        font.pixelSize: 9
                        color: secondaryTextColor
                        width: parent.width
                        horizontalAlignment: Text.AlignCenter
                        font.italic: true
                    }
                }

                // Content column
                Column {
                    id: contentColumn
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: 8

                    // Programme title
                    Text {
                        text: preferThai ?
                            (model.programme_title_thai || model.programme_title_english || "ไม่ระบุชื่อรายการ") :
                            (model.programme_title_english || model.programme_title_thai || "Programme Title Not Available")
                        font.family: (preferThai && model.programme_title_thai) ? thaiFont : "Open Sans"
                        font.pixelSize: 14
                        font.weight: Font.Bold
                        color: primaryTextColor
                        width: parent.width
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }

                    // Alternative title (other language)
                    Text {
                        text: {
                            if (preferThai && model.programme_title_thai && model.programme_title_english) {
                                return model.programme_title_english
                            } else if (!preferThai && model.programme_title_english && model.programme_title_thai) {
                                return model.programme_title_thai
                            }
                            return ""
                        }
                        visible: text.length > 0
                        font.family: visible ? (preferThai ? "Open Sans" : thaiFont) : "Open Sans"
                        font.pixelSize: 12
                        color: secondaryTextColor
                        width: parent.width
                        wrapMode: Text.WordWrap
                        maximumLineCount: 1
                        elide: Text.ElideRight
                    }

                    // Programme description
                    Text {
                        text: preferThai ?
                            (model.description_thai || model.description_english || "") :
                            (model.description_english || model.description_thai || "")
                        visible: text.length > 0
                        font.family: (preferThai && model.description_thai) ? thaiFont : "Open Sans"
                        font.pixelSize: 10
                        color: secondaryTextColor
                        width: parent.width
                        wrapMode: Text.WordWrap
                        maximumLineCount: 3
                        elide: Text.ElideRight
                    }

                    // Programme metadata
                    RowLayout {
                        width: parent.width
                        spacing: 15

                        // Programme type
                        Row {
                            spacing: 4
                            visible: model.programme_type || model.programme_type_thai

                            Rectangle {
                                width: 12
                                height: 12
                                radius: 2
                                color: getProgrammeTypeColor(model.programme_type_code || 0)
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: preferThai ?
                                    (model.programme_type_thai || model.programme_type || "ไม่ระบุ") :
                                    (model.programme_type || model.programme_type_thai || "None")
                                font.family: preferThai ? thaiFont : "Open Sans"
                                font.pixelSize: 9
                                color: secondaryTextColor
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        // Rating or age classification
                        Rectangle {
                            visible: model.age_rating
                            width: ratingText.implicitWidth + 6
                            height: 16
                            radius: 3
                            color: "#FF9800"

                            Text {
                                id: ratingText
                                text: model.age_rating || ""
                                font.pixelSize: 8
                                font.weight: Font.Bold
                                color: "white"
                                anchors.centerIn: parent
                            }
                        }

                        // Live indicator
                        Rectangle {
                            visible: model.is_live || false
                            width: liveText.implicitWidth + 8
                            height: 16
                            radius: 3
                            color: "#F44336"

                            Text {
                                id: liveText
                                text: preferThai ? "สด" : "LIVE"
                                font.family: preferThai ? thaiFont : "Open Sans"
                                font.pixelSize: 8
                                font.weight: Font.Bold
                                color: "white"
                                anchors.centerIn: parent
                            }
                        }

                        // Spacer
                        Item { Layout.fillWidth: true }
                    }
                }

                // Current programme indicator
                Rectangle {
                    visible: isCurrent
                    width: 6
                    height: parent.height * 0.6
                    radius: 3
                    color: "#4CAF50"
                    Layout.alignment: Qt.AlignVCenter

                    SequentialAnimation on opacity {
                        running: isCurrent
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 1000 }
                        NumberAnimation { to: 1.0; duration: 1000 }
                    }
                }
            }
        }

        // Section headers for date grouping
        section.property: "date_section"
        section.criteria: ViewSection.FullString
        section.delegate: Rectangle {
            width: ListView.view.width
            height: 40
            color: "#E0E0E0"

            Text {
                text: formatDateSection(section)
                font.family: preferThai ? thaiFont : "Open Sans"
                font.pixelSize: 14
                font.weight: Font.Bold
                color: primaryTextColor
                anchors.left: parent.left
                anchors.leftMargin: 15
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // Empty state
    Item {
        anchors.fill: parent
        visible: epgListView.count === 0

        Column {
            anchors.centerIn: parent
            spacing: 16

            Text {
                text: "📺"
                font.pixelSize: 48
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: 0.5
            }

            Text {
                text: preferThai ? "ไม่พบตารางรายการ" : "No programme schedule available"
                font.family: preferThai ? thaiFont : "Open Sans"
                font.pixelSize: 16
                color: secondaryTextColor
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: preferThai ?
                    "ตารางรายการอาจยังไม่พร้อมใช้งาน" :
                    "Programme information may not be available yet"
                font.family: preferThai ? thaiFont : "Open Sans"
                font.pixelSize: 12
                color: secondaryTextColor
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: 0.8
            }
        }
    }

    // Helper functions
    function formatTime(timeString) {
        if (!timeString) return ""
        var date = new Date(timeString)
        return Qt.formatTime(date, "hh:mm")
    }

    function formatDateSection(section) {
        if (!section) return ""

        var date = new Date(section)
        var today = new Date()
        var tomorrow = new Date(today)
        tomorrow.setDate(today.getDate() + 1)

        if (date.toDateString() === today.toDateString()) {
            return preferThai ? "วันนี้" : "Today"
        } else if (date.toDateString() === tomorrow.toDateString()) {
            return preferThai ? "พรุ่งนี้" : "Tomorrow"
        } else {
            if (preferThai) {
                return Qt.formatDate(date, "วันddd ที่ d MMM yyyy")
            } else {
                return Qt.formatDate(date, "dddd, MMMM d, yyyy")
            }
        }
    }

    function calculateDuration(startTime, endTime) {
        if (!startTime || !endTime) return ""

        var start = new Date(startTime)
        var end = new Date(endTime)
        var durationMs = end - start
        var minutes = Math.floor(durationMs / (1000 * 60))

        if (minutes < 60) {
            return preferThai ? minutes + " นาที" : minutes + " min"
        } else {
            var hours = Math.floor(minutes / 60)
            var remainingMinutes = minutes % 60
            if (remainingMinutes === 0) {
                return preferThai ? hours + " ชั่วโมง" : hours + " hr"
            } else {
                return preferThai ?
                    hours + " ชม. " + remainingMinutes + " นาที" :
                    hours + "h " + remainingMinutes + "m"
            }
        }
    }

    function getProgrammeTypeColor(typeCode) {
        // Color coding based on programme type
        switch (typeCode) {
            case 1: return "#F44336"  // News - Red
            case 2: return "#FF9800"  // Current Affairs - Orange
            case 4: return "#4CAF50"  // Sport - Green
            case 5: return "#2196F3"  // Education - Blue
            case 6: return "#9C27B0"  // Drama - Purple
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15: return "#E91E63" // Music - Pink
            case 16: return "#00BCD4"  // Weather - Cyan
            case 18: return "#FFC107"  // Children - Amber
            case 20: return "#795548"  // Religion - Brown
            default: return "#9E9E9E"  // Other - Grey
        }
    }
}
