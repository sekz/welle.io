import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Page {
    id: root

    title: qsTr("Announcement Settings")

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                text: root.title
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 16

            // Master enable switch
            GroupBox {
                title: qsTr("General")
                Layout.fillWidth: true
                Layout.margins: 16

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Switch {
                        id: enableSwitch
                        text: qsTr("Enable automatic announcement switching")
                        checked: radioController.announcementEnabled
                        onCheckedChanged: {
                            if (checked !== radioController.announcementEnabled) {
                                radioController.announcementEnabled = checked
                            }
                        }
                    }

                    Label {
                        text: qsTr("Automatically switch to announcement broadcasts when they become active")
                        font.pixelSize: 11
                        opacity: 0.7
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            // Announcement types
            GroupBox {
                title: qsTr("Announcement Types")
                Layout.fillWidth: true
                Layout.margins: 16
                enabled: enableSwitch.checked

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    Label {
                        text: qsTr("Select which announcement types to receive:")
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    // Type checkboxes
                    GridLayout {
                        id: typeGrid
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Repeater {
                            id: typeRepeater
                            model: announcementTypesModel

                            RowLayout {
                                spacing: 8
                                Layout.fillWidth: true

                                CheckBox {
                                    id: typeCheckbox
                                    text: modelData.name
                                    checked: radioController.isAnnouncementTypeEnabled(modelData.type)

                                    // Update checked state when announcement settings change
                                    Connections {
                                        target: radioController
                                        function onAnnouncementTypesChanged() {
                                            typeCheckbox.checked = radioController.isAnnouncementTypeEnabled(modelData.type)
                                        }
                                    }

                                    onCheckedChanged: {
                                        if (checked !== radioController.isAnnouncementTypeEnabled(modelData.type)) {
                                            radioController.setAnnouncementTypeEnabled(modelData.type, checked)
                                        }
                                    }
                                }

                                Rectangle {
                                    width: 24
                                    height: 24
                                    radius: 12
                                    color: modelData.color
                                    opacity: 0.3

                                    Label {
                                        anchors.centerIn: parent
                                        text: modelData.priority.toString()
                                        font.pixelSize: 10
                                        font.bold: true
                                        color: modelData.color
                                    }
                                }
                            }
                        }
                    }

                    // Quick selection buttons
                    RowLayout {
                        spacing: 8
                        Layout.topMargin: 8

                        Button {
                            text: qsTr("Critical Only")
                            onClicked: selectCriticalOnly()
                        }

                        Button {
                            text: qsTr("All Types")
                            onClicked: selectAllTypes()
                        }

                        Button {
                            text: qsTr("Clear All")
                            onClicked: clearAllTypes()
                        }
                    }
                }
            }

            // Priority threshold
            GroupBox {
                title: qsTr("Priority Settings")
                Layout.fillWidth: true
                Layout.margins: 16
                enabled: enableSwitch.checked

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Label {
                        text: qsTr("Minimum announcement priority:")
                        font.pixelSize: 12
                    }

                    RowLayout {
                        spacing: 16

                        Slider {
                            id: prioritySlider
                            Layout.fillWidth: true
                            from: 1
                            to: 11
                            stepSize: 1
                            value: radioController.minAnnouncementPriority
                            onValueChanged: {
                                if (value !== radioController.minAnnouncementPriority) {
                                    radioController.minAnnouncementPriority = value
                                }
                            }
                        }

                        Label {
                            text: prioritySlider.value.toFixed(0)
                            font.pixelSize: 16
                            font.bold: true
                            Layout.minimumWidth: 30
                        }
                    }

                    Label {
                        text: qsTr("Only announcements with priority %1 or higher will be received").arg(prioritySlider.value.toFixed(0))
                        font.pixelSize: 11
                        opacity: 0.7
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            // Duration settings
            GroupBox {
                title: qsTr("Duration Settings")
                Layout.fillWidth: true
                Layout.margins: 16
                enabled: enableSwitch.checked

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    RowLayout {
                        spacing: 12

                        Label {
                            text: qsTr("Maximum duration:")
                            font.pixelSize: 12
                        }

                        SpinBox {
                            id: durationSpinBox
                            from: 30
                            to: 600
                            stepSize: 30
                            value: radioController.maxAnnouncementDuration
                            onValueModified: {
                                radioController.maxAnnouncementDuration = value
                            }

                            textFromValue: function(value, locale) {
                                return value + " " + qsTr("seconds")
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    Label {
                        text: qsTr("Automatically return to the original service after this duration")
                        font.pixelSize: 11
                        opacity: 0.7
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Switch {
                        text: qsTr("Allow manual return to service")
                        checked: radioController.allowManualAnnouncementReturn
                        onCheckedChanged: {
                            if (checked !== radioController.allowManualAnnouncementReturn) {
                                radioController.allowManualAnnouncementReturn = checked
                            }
                        }
                    }
                }
            }

            // Action buttons
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 16
                Layout.topMargin: 24
                spacing: 12

                Button {
                    text: qsTr("Reset to Defaults")
                    onClicked: resetDialog.open()
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("Save Settings")
                    highlighted: true
                    onClicked: {
                        radioController.saveAnnouncementSettings()
                        saveNotification.show()
                    }
                }
            }
        }
    }

    // Announcement types model
    property var announcementTypesModel: [
        {type: 0, name: qsTr("Alarm"), priority: 1, color: "#FF0000"},
        {type: 1, name: qsTr("Road Traffic"), priority: 2, color: "#FFCC00"},
        {type: 2, name: qsTr("Transport Flash"), priority: 3, color: "#2196F3"},
        {type: 3, name: qsTr("Warning/Service"), priority: 4, color: "#FF9800"},
        {type: 4, name: qsTr("News Flash"), priority: 5, color: "#F44336"},
        {type: 5, name: qsTr("Area Weather"), priority: 6, color: "#03A9F4"},
        {type: 6, name: qsTr("Event"), priority: 7, color: "#9C27B0"},
        {type: 7, name: qsTr("Special Event"), priority: 8, color: "#E91E63"},
        {type: 8, name: qsTr("Programme Information"), priority: 9, color: "#009688"},
        {type: 9, name: qsTr("Sport Report"), priority: 10, color: "#4CAF50"},
        {type: 10, name: qsTr("Financial Report"), priority: 11, color: "#795548"}
    ]

    // Helper functions
    function selectCriticalOnly() {
        for (var i = 0; i < announcementTypesModel.length; i++) {
            var enabled = announcementTypesModel[i].priority <= 4
            radioController.setAnnouncementTypeEnabled(announcementTypesModel[i].type, enabled)
        }
    }

    function selectAllTypes() {
        for (var i = 0; i < announcementTypesModel.length; i++) {
            radioController.setAnnouncementTypeEnabled(announcementTypesModel[i].type, true)
        }
    }

    function clearAllTypes() {
        for (var i = 0; i < announcementTypesModel.length; i++) {
            radioController.setAnnouncementTypeEnabled(announcementTypesModel[i].type, false)
        }
    }

    // Reset confirmation dialog
    Dialog {
        id: resetDialog
        title: qsTr("Reset to Defaults")
        standardButtons: Dialog.Yes | Dialog.No
        modal: true

        Label {
            text: qsTr("Are you sure you want to reset all announcement settings to their default values?")
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            radioController.resetAnnouncementSettings()
            saveNotification.show()
        }
    }

    // Save notification
    Popup {
        id: saveNotification
        x: (parent.width - width) / 2
        y: parent.height - height - 50
        width: 300
        height: 50
        modal: false
        closePolicy: Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#4CAF50"
            radius: 4
        }

        contentItem: Label {
            text: qsTr("Settings saved successfully")
            color: "white"
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        function show() {
            open()
            hideTimer.start()
        }

        Timer {
            id: hideTimer
            interval: 2000
            onTriggered: saveNotification.close()
        }
    }
}
