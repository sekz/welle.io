import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// List of recent announcements (ETSI EN 300 401, FIG 0/19) with a simple
// filter by announcement type and service name.
Page {
    id: root

    title: qsTr("Announcement History")

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

    // Types hidden by the filter check boxes (key: type, value: true)
    property var hiddenTypes: ({})

    readonly property var historyData: radioController.announcementHistory
    readonly property var filteredData: filterHistory(historyData, searchField.text, hiddenTypes)

    function typeInfo(type) {
        for (var i = 0; i < announcementTypes.length; i++) {
            if (announcementTypes[i].type === type)
                return announcementTypes[i]
        }
        return {type: type, name: qsTr("Unknown"), color: "#757575"}
    }

    function formatDuration(seconds) {
        if (seconds < 60)
            return qsTr("%1 s").arg(seconds)
        return qsTr("%1 min %2 s").arg(Math.floor(seconds / 60)).arg(seconds % 60)
    }

    function filterHistory(data, text, hidden) {
        var needle = text.toLowerCase()
        return data.filter(function(item) {
            if (hidden[item.type])
                return false
            return needle === "" || item.serviceName.toLowerCase().indexOf(needle) !== -1
        })
    }

    function mostFrequentType(data) {
        var counts = {}
        var best = -1
        for (var i = 0; i < data.length; i++) {
            var t = data[i].type
            counts[t] = (counts[t] || 0) + 1
            if (best < 0 || counts[t] > counts[best])
                best = t
        }
        return best < 0 ? "-" : typeInfo(best).name + " (" + counts[best] + ")"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Column {
                Label { text: qsTr("Total"); opacity: 0.7 }
                Label { text: historyData.length.toString(); font.bold: true }
            }

            Column {
                Label { text: qsTr("Most Frequent"); opacity: 0.7 }
                Label { text: mostFrequentType(historyData) }
            }
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search service...")
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: announcementTypes

                CheckBox {
                    text: modelData.name
                    checked: true
                    onToggled: {
                        var hidden = Object.assign({}, root.hiddenTypes)
                        if (checked)
                            delete hidden[modelData.type]
                        else
                            hidden[modelData.type] = true
                        root.hiddenTypes = hidden
                    }
                }
            }
        }

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

                contentItem: RowLayout {
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 12
                        Layout.preferredHeight: 12
                        radius: 6
                        color: typeInfo(modelData.type).color
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            text: typeInfo(modelData.type).name
                            font.bold: true
                        }
                        Label {
                            text: modelData.serviceName
                        }
                        Label {
                            text: Qt.formatDateTime(modelData.startTime, Qt.locale().dateTimeFormat(Locale.ShortFormat))
                                  + " • " + formatDuration(modelData.durationSeconds)
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
    }
}
