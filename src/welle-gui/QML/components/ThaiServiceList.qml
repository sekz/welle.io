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

ListView {
    id: thaiServiceList

    property bool preferThai: true
    property string thaiFont: "Noto Sans Thai"
    property color primaryTextColor: "#333333"
    property color secondaryTextColor: "#666666"
    property color metaTextColor: "#999999"
    property color selectedBackgroundColor: "#E3F2FD"
    property color hoverBackgroundColor: "#F5F5F5"

    signal serviceSelected(var serviceInfo)
    signal serviceContextMenu(var serviceInfo, point position)

    delegate: ItemDelegate {
        id: serviceDelegate
        width: parent ? parent.width : 0
        height: 80

        property bool isSelected: ListView.isCurrentItem
        property bool isHovered: hovered

        background: Rectangle {
            color: isSelected ? selectedBackgroundColor :
                   isHovered ? hoverBackgroundColor : "transparent"
            border.color: isSelected ? "#2196F3" : "transparent"
            border.width: 1
            radius: 4

            Behavior on color {
                ColorAnimation { duration: 150 }
            }
        }

        onClicked: {
            thaiServiceList.currentIndex = index
            serviceSelected(model)
        }

        onPressAndHold: {
            serviceContextMenu(model, Qt.point(mouseX, mouseY))
        }

        MouseArea {
            id: rightClickArea
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                serviceContextMenu(model, Qt.point(mouseX, mouseY))
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            // Service icon/indicator
            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: model.signal_strength > 70 ? "#4CAF50" :
                       model.signal_strength > 40 ? "#FF9800" : "#F44336"
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.topMargin: 8
            }

            // Service information
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: 4

                // Primary service label (Thai or English based on preference)
                Text {
                    id: primaryLabel
                    text: preferThai && model.thai_label ? model.thai_label :
                          (model.english_label || model.thai_label || "ไม่ระบุชื่อสถานี")
                    font.family: (preferThai && model.thai_label) ? thaiFont : "Open Sans"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: primaryTextColor
                    wrapMode: Text.WordWrap
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Secondary service label (alternate language)
                Text {
                    id: secondaryLabel
                    text: {
                        if (preferThai && model.thai_label && model.english_label) {
                            return model.english_label
                        } else if (!preferThai && model.english_label && model.thai_label) {
                            return model.thai_label
                        }
                        return ""
                    }
                    visible: text.length > 0
                    font.family: visible ? (preferThai ? "Open Sans" : thaiFont) : "Open Sans"
                    font.pixelSize: 12
                    color: secondaryTextColor
                    wrapMode: Text.WordWrap
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Programme type and metadata
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: {
                            var programmeType = preferThai ?
                                (model.programme_type_thai || model.programme_type_english || "ไม่ระบุ") :
                                (model.programme_type_english || model.programme_type_thai || "None")
                            return "ประเภท: " + programmeType
                        }
                        font.family: preferThai ? thaiFont : "Open Sans"
                        font.pixelSize: 10
                        color: metaTextColor
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    // Emergency indicator
                    Rectangle {
                        visible: model.emergency_support || false
                        width: 12
                        height: 12
                        radius: 6
                        color: "#F44336"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            anchors.centerIn: parent
                            text: "!"
                            color: "white"
                            font.pixelSize: 8
                            font.weight: Font.Bold
                        }

                        ToolTip.text: preferThai ? "รองรับการแจ้งเตือนฉุกเฉิน" : "Emergency Alert Support"
                        ToolTip.visible: emergencyMouseArea.containsMouse

                        MouseArea {
                            id: emergencyMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }

                    // Traffic indicator
                    Rectangle {
                        visible: model.traffic_support || false
                        width: 12
                        height: 12
                        radius: 2
                        color: "#2196F3"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            anchors.centerIn: parent
                            text: "T"
                            color: "white"
                            font.pixelSize: 7
                            font.weight: Font.Bold
                        }

                        ToolTip.text: preferThai ? "รองรับข้อมูลจราจร" : "Traffic Information Support"
                        ToolTip.visible: trafficMouseArea.containsMouse

                        MouseArea {
                            id: trafficMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }
                }
            }

            // Signal strength indicator
            Column {
                Layout.alignment: Qt.AlignVCenter
                spacing: 2

                Rectangle {
                    width: 40
                    height: 16
                    color: model.signal_strength > 70 ? "#4CAF50" :
                           model.signal_strength > 40 ? "#FF9800" : "#F44336"
                    radius: 8

                    Text {
                        anchors.centerIn: parent
                        text: (model.signal_strength || 0) + "%"
                        color: "white"
                        font.pixelSize: 8
                        font.weight: Font.Medium
                    }
                }

                Text {
                    text: preferThai ? "สัญญาณ" : "Signal"
                    font.family: preferThai ? thaiFont : "Open Sans"
                    font.pixelSize: 8
                    color: metaTextColor
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }

        // Selection highlight
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 4
            color: "#2196F3"
            visible: isSelected
        }
    }

    // Empty state
    Item {
        anchors.fill: parent
        visible: thaiServiceList.count === 0

        Column {
            anchors.centerIn: parent
            spacing: 16

            Text {
                text: "📻"
                font.pixelSize: 48
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: 0.5
            }

            Text {
                text: preferThai ? "ไม่พบสถานีวิทยุ" : "No radio stations found"
                font.family: preferThai ? thaiFont : "Open Sans"
                font.pixelSize: 16
                color: metaTextColor
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: preferThai ?
                    "กรุณาตรวจสoba การเชื่อมต่อและสัญญาณ" :
                    "Please check your connection and signal"
                font.family: preferThai ? thaiFont : "Open Sans"
                font.pixelSize: 12
                color: metaTextColor
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    // Scroll indicators
    ScrollBar.vertical: ScrollBar {
        active: true

        background: Rectangle {
            color: "#F0F0F0"
            radius: 4
        }

        contentItem: Rectangle {
            color: "#BDBDBD"
            radius: 4
        }
    }

    // Accessibility
    Keys.onReturnPressed: {
        if (currentIndex >= 0 && currentIndex < count) {
            serviceSelected(model.get(currentIndex))
        }
    }

    Keys.onSpacePressed: {
        if (currentIndex >= 0 && currentIndex < count) {
            serviceSelected(model.get(currentIndex))
        }
    }
}
