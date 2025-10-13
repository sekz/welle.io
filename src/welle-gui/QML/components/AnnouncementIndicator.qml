/*
 *    Copyright (C) 2025
 *    Thailand DAB+ Implementation
 *
 *    Announcement indicator banner for welle.io Thailand DAB+ receiver.
 *    Displays active announcements with color-coded priority.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import io.welle 1.0

// Banner that appears when announcement is active
Rectangle {
    id: root

    property bool isActive: radioController.isInAnnouncement
    property int currentType: radioController.currentAnnouncementType
    property int duration: radioController.announcementDuration
    property string serviceName: radioController.announcementServiceName

    visible: opacity > 0
    opacity: isActive ? 1.0 : 0.0
    height: isActive ? 60 : 0
    color: getAnnouncementColor(currentType)
    z: 100  // Float above other content

    Behavior on opacity {
        NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
    }

    Behavior on height {
        NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 16

        // Icon
        Label {
            text: getAnnouncementIcon(currentType)
            font.family: "Material Icons"
            font.pixelSize: 32
            color: "white"
        }

        // Info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                text: qsTr("Announcement Active") + ": " + getAnnouncementNameThai(currentType)
                font.pixelSize: 16
                font.bold: true
                color: "white"
            }

            Label {
                text: serviceName + " • " + formatDuration(duration)
                font.pixelSize: 12
                color: "white"
                opacity: 0.9
            }
        }

        // Return button
        Button {
            text: qsTr("Return to Service")
            visible: radioController.allowManualAnnouncementReturn
            onClicked: radioController.returnFromAnnouncement()

            background: Rectangle {
                color: parent.pressed ? "#ffffff" : "transparent"
                border.color: "white"
                border.width: 2
                radius: 4
            }

            contentItem: Label {
                text: parent.text
                font.pixelSize: 14
                color: parent.pressed ? root.color : "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    // Announcement types with Thai names and colors
    property var announcementTypes: [
        {type: 0, nameEn: "Alarm", nameTh: "เตือนภัย", icon: "\ue855", color: "#FF0000"},
        {type: 1, nameEn: "Traffic", nameTh: "จราจร", icon: "\ue558", color: "#FFCC00"},
        {type: 2, nameEn: "Transport", nameTh: "ข้อมูลการขนส่ง", icon: "\ue531", color: "#2196F3"},
        {type: 3, nameEn: "Warning", nameTh: "คำเตือน", icon: "\ue002", color: "#FF9800"},
        {type: 4, nameEn: "News", nameTh: "ข่าว", icon: "\ue639", color: "#F44336"},
        {type: 5, nameEn: "Weather", nameTh: "สภาพอากาศ", icon: "\ue430", color: "#03A9F4"},
        {type: 6, nameEn: "Event", nameTh: "กิจกรรม", icon: "\ue878", color: "#9C27B0"},
        {type: 7, nameEn: "Special Event", nameTh: "กิจกรรมพิเศษ", icon: "\ue885", color: "#E91E63"},
        {type: 8, nameEn: "Programme Info", nameTh: "ข้อมูลรายการ", icon: "\ue88e", color: "#009688"},
        {type: 9, nameEn: "Sport", nameTh: "กีฬา", icon: "\ue52f", color: "#4CAF50"},
        {type: 10, nameEn: "Finance", nameTh: "การเงิน", icon: "\ue227", color: "#795548"}
    ]

    function getAnnouncementColor(type) {
        for (var i = 0; i < announcementTypes.length; i++) {
            if (announcementTypes[i].type === type) {
                return announcementTypes[i].color
            }
        }
        return "#757575"  // Gray for unknown
    }

    function getAnnouncementIcon(type) {
        for (var i = 0; i < announcementTypes.length; i++) {
            if (announcementTypes[i].type === type) {
                return announcementTypes[i].icon
            }
        }
        return "\ue88e"  // Info icon for unknown
    }

    function getAnnouncementNameThai(type) {
        for (var i = 0; i < announcementTypes.length; i++) {
            if (announcementTypes[i].type === type) {
                return announcementTypes[i].nameTh
            }
        }
        return "ไม่ทราบ"
    }

    function formatDuration(seconds) {
        if (seconds < 60) {
            return seconds + " " + qsTr("seconds")
        } else {
            var minutes = Math.floor(seconds / 60)
            var secs = seconds % 60
            return minutes + " " + qsTr("min") + " " + secs + " " + qsTr("s")
        }
    }
}
