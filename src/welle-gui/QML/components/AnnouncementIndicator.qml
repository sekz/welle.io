import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Banner that appears when announcement is active
Rectangle {
    id: root

    property bool isActive: radioController.isInAnnouncement
    property int currentType: radioController.activeAnnouncementType
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

        // Info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                text: qsTr("Announcement Active") + ": " + getAnnouncementName(currentType)
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

    // Announcement types (ETSI EN 300 401 Table 14) and their display colors
    readonly property var announcementTypes: [
        {type: 0, name: qsTr("Alarm"), color: "#FF0000"},
        {type: 1, name: qsTr("Road Traffic"), color: "#FFCC00"},
        {type: 2, name: qsTr("Transport Flash"), color: "#2196F3"},
        {type: 3, name: qsTr("Warning/Service"), color: "#FF9800"},
        {type: 4, name: qsTr("News Flash"), color: "#F44336"},
        {type: 5, name: qsTr("Area Weather"), color: "#03A9F4"},
        {type: 6, name: qsTr("Event"), color: "#9C27B0"},
        {type: 7, name: qsTr("Special Event"), color: "#E91E63"},
        {type: 8, name: qsTr("Programme Information"), color: "#009688"},
        {type: 9, name: qsTr("Sport Report"), color: "#4CAF50"},
        {type: 10, name: qsTr("Financial Report"), color: "#795548"}
    ]

    function getAnnouncementColor(type) {
        for (var i = 0; i < announcementTypes.length; i++) {
            if (announcementTypes[i].type === type) {
                return announcementTypes[i].color
            }
        }
        return "#757575"  // Gray for unknown
    }

    function getAnnouncementName(type) {
        for (var i = 0; i < announcementTypes.length; i++) {
            if (announcementTypes[i].type === type) {
                return announcementTypes[i].name
            }
        }
        return qsTr("Unknown")
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
