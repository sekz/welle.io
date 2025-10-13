import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root

    property bool mockMode: true
    property var mockData: []
    property var filteredData: []
    property var enabledTypes: []  // Track enabled announcement types

    title: qsTr("Announcement History") + " / " + qsTr("ประวัติการแจ้งเตือน")

    Component.onCompleted: {
        if (mockMode) {
            generateMockData()
            updateEnabledTypes()
            filteredData = mockData
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                text: root.title
                font.pixelSize: 16
                font.bold: true
                Layout.fillWidth: true
            }
            Label {
                text: "[MOCK]"
                color: "#FF9800"
                visible: mockMode
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // Statistics Panel
        Pane {
            Layout.fillWidth: true
            Layout.preferredHeight: 80

            RowLayout {
                anchors.fill: parent
                spacing: 20

                Column {
                    spacing: 2
                    Label {
                        text: qsTr("Total")
                        font.pixelSize: 11
                        opacity: 0.7
                    }
                    Label {
                        text: mockData.length.toString()
                        font.pixelSize: 24
                        font.bold: true
                    }
                }

                Column {
                    spacing: 2
                    Label {
                        text: qsTr("Most Frequent")
                        font.pixelSize: 11
                        opacity: 0.7
                    }
                    Label {
                        text: getMostFrequentType()
                        font.pixelSize: 14
                    }
                }
            }
        }

        // Filters
        GroupBox {
            title: qsTr("Filters")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Search service...")
                    onTextChanged: applyFilters()
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        id: typeFilterRepeater
                        model: announcementTypes

                        CheckBox {
                            id: typeCheckbox
                            text: modelData.nameTh
                            checked: true
                            property int announcementType: modelData.type

                            onCheckedChanged: {
                                updateEnabledTypes()
                                applyFilters()
                            }

                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                color: modelData.color
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: -16
                            }
                        }
                    }
                }
            }
        }

        // History List
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: filteredData
            spacing: 4
            clip: true

            ScrollBar.vertical: ScrollBar {}

            delegate: ItemDelegate {
                width: listView.width
                height: 70

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 12

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: getTypeColor(modelData.type)
                        opacity: 0.2

                        Label {
                            anchors.centerIn: parent
                            text: getTypeName(modelData.type, false).substring(0, 1)
                            font.pixelSize: 18
                            font.bold: true
                            color: getTypeColor(modelData.type)
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        RowLayout {
                            Label {
                                text: getTypeName(modelData.type, true)
                                font.bold: true
                            }
                            Label {
                                text: "(" + getTypeName(modelData.type, false) + ")"
                                font.pixelSize: 11
                                opacity: 0.7
                            }
                        }

                        Label {
                            text: modelData.serviceName
                            font.pixelSize: 12
                        }

                        Label {
                            text: ThaiDateFormatter.format(modelData.startTime, "medium") +
                                  " • " + formatDuration(modelData.durationSeconds)
                            font.pixelSize: 11
                            opacity: 0.7
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                text: qsTr("No announcements")
                visible: listView.count === 0
                opacity: 0.5
            }
        }

        // Export Button
        Button {
            Layout.fillWidth: true
            text: qsTr("Export to CSV")
            onClicked: exportDialog.open()
        }
    }

    // Announcement types data
    property var announcementTypes: [
        {type: 0, nameEn: "Alarm", nameTh: "เตือนภัย", color: "#FF0000"},
        {type: 1, nameEn: "Traffic", nameTh: "จราจร", color: "#FFCC00"},
        {type: 2, nameEn: "Transport", nameTh: "ข้อมูลการขนส่ง", color: "#2196F3"},
        {type: 3, nameEn: "Warning", nameTh: "คำเตือน", color: "#FF9800"},
        {type: 4, nameEn: "News", nameTh: "ข่าว", color: "#F44336"},
        {type: 5, nameEn: "Weather", nameTh: "สภาพอากาศ", color: "#03A9F4"},
        {type: 6, nameEn: "Event", nameTh: "กิจกรรม", color: "#9C27B0"},
        {type: 7, nameEn: "Special Event", nameTh: "กิจกรรมพิเศษ", color: "#E91E63"},
        {type: 8, nameEn: "Programme Info", nameTh: "ข้อมูลรายการ", color: "#009688"},
        {type: 9, nameEn: "Sport", nameTh: "กีฬา", color: "#4CAF50"},
        {type: 10, nameEn: "Finance", nameTh: "การเงิน", color: "#795548"}
    ]

    // Helper functions
    function generateMockData() {
        var services = ["Radio Thailand", "FM 95.0", "NBT Radio", "Traffic Radio", "Bangkok FM"]
        var now = new Date()
        mockData = []

        for (var i = 0; i < 20; i++) {
            var type = Math.floor(Math.random() * 11)
            var duration = 30 + Math.floor(Math.random() * 300)
            var hoursAgo = Math.floor(Math.random() * 72)
            var startTime = new Date(now.getTime() - hoursAgo * 3600000)
            var endTime = new Date(startTime.getTime() + duration * 1000)

            mockData.push({
                type: type,
                serviceName: services[Math.floor(Math.random() * services.length)],
                startTime: startTime,
                endTime: endTime,
                durationSeconds: duration
            })
        }

        mockData.sort(function(a, b) { return b.startTime - a.startTime })
    }

    function getTypeName(type, isThai) {
        for (var i = 0; i < announcementTypes.length; i++) {
            if (announcementTypes[i].type === type) {
                return isThai ? announcementTypes[i].nameTh : announcementTypes[i].nameEn
            }
        }
        return "Unknown"
    }

    function getTypeColor(type) {
        for (var i = 0; i < announcementTypes.length; i++) {
            if (announcementTypes[i].type === type) {
                return announcementTypes[i].color
            }
        }
        return "#757575"
    }

    function formatDuration(seconds) {
        if (seconds < 60) {
            return seconds + " วินาที"
        } else {
            var minutes = Math.floor(seconds / 60)
            var secs = seconds % 60
            return minutes + " นาที " + secs + " วินาที"
        }
    }

    function getMostFrequentType() {
        if (mockData.length === 0) return "-"

        var counts = {}
        for (var i = 0; i < mockData.length; i++) {
            var type = mockData[i].type
            counts[type] = (counts[type] || 0) + 1
        }

        var maxType = 0
        var maxCount = 0
        for (var t in counts) {
            if (counts[t] > maxCount) {
                maxCount = counts[t]
                maxType = parseInt(t)
            }
        }

        return getTypeName(maxType, true) + " (" + maxCount + ")"
    }

    function updateEnabledTypes() {
        enabledTypes = []
        for (var i = 0; i < typeFilterRepeater.count; i++) {
            var checkbox = typeFilterRepeater.itemAt(i)
            if (checkbox && checkbox.checked) {
                enabledTypes.push(checkbox.announcementType)
            }
        }
    }

    function applyFilters() {
        if (!mockMode) return

        filteredData = mockData.filter(function(item) {
            if (enabledTypes.indexOf(item.type) === -1) return false
            if (searchField.text && item.serviceName.toLowerCase().indexOf(searchField.text.toLowerCase()) === -1) {
                return false
            }
            return true
        })
    }

    Dialog {
        id: exportDialog
        title: qsTr("Export to CSV")
        standardButtons: Dialog.Ok | Dialog.Cancel

        Label {
            text: qsTr("Export functionality will be implemented when connected to backend.")
        }
    }
}
