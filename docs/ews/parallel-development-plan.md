# EWS Parallel Development Plan
## Multi-Agent Concurrent Implementation Strategy

**Version:** 1.0
**Date:** 2025-10-15
**Estimated Duration:** 4-6 weeks
**Team Size:** 5 parallel agents (can work simultaneously)

---

## Overview

This plan divides EWS compliance implementation into **5 independent workstreams** that can be developed in parallel by specialized agents. Each workstream has minimal dependencies on others.

### Parallel Workstreams

```
┌─────────────────────────────────────────────────────────────┐
│                    EWS Compliance Project                    │
├─────────────────┬─────────────────┬─────────────────────────┤
│   Agent 1       │   Agent 2       │   Agent 3               │
│ Backend Dev     │ Backend Dev     │ Backend Dev             │
│ Location Codes  │ Sleep Monitor   │ FIG Extensions          │
│ (2-3 weeks)     │ (1-2 weeks)     │ (1-2 weeks)             │
├─────────────────┼─────────────────┼─────────────────────────┤
│   Agent 4       │   Agent 5       │                         │
│ Frontend Dev    │ Test Automator  │                         │
│ GUI/QML         │ Test Suite      │                         │
│ (2 weeks)       │ (2-3 weeks)     │                         │
└─────────────────┴─────────────────┴─────────────────────────┘
```

---

## Workstream 1: Location Code Management
**Agent:** `backend-developer` (Backend specialist)
**Duration:** 2-3 weeks
**Priority:** CRITICAL (blocks testing)
**Dependencies:** None

### Deliverables

#### 1.1 Location Code Parser & Validator
**Files:** `src/backend/location-code-manager.h`, `src/backend/location-code-manager.cpp`

```cpp
class LocationCodeManager {
public:
    // Parse from user input
    bool setReceiverLocation(const std::string& code);

    // Format conversion
    std::string toDisplayFormat();  // "1255-4467-1352"
    std::string toHexFormat();      // "Z1:91BB82"
    uint8_t getZone() const;
    uint32_t getLocation24bit() const;

    // Validation
    bool validateChecksum(const std::string& code);
    uint8_t calculateChecksum(uint8_t zone, uint32_t location);

    // Matching
    bool matchesAlertLocation(const uint8_t* location_data,
                             uint8_t nff_level);

private:
    uint8_t zone_;          // 0-41
    uint32_t location_;     // 24-bit
    uint8_t checksum_;      // 8-bit

    // NFF decoding helpers
    bool matchL3(uint8_t alert_l3);
    bool matchL4(uint16_t alert_l4);
    bool matchL5(uint32_t alert_l5);
    bool matchL6(uint32_t alert_l6);
};
```

#### 1.2 NFF Encoding Support
**Reference:** ETSI TS 104 089 Annex B

```cpp
// NFF (Nibble Fill Flag) encoding for location hierarchies
struct NFFloLocationEncoding {
    uint8_t nff;        // 0-3 (L3, L4, L5, L6)
    uint8_t num_bytes;  // Varies by NFF
    std::vector<uint8_t> encoded_locations;

    // Decode from FIG 0/19 location field
    static NFFloLocationEncoding decode(const uint8_t* data, uint8_t length);

    // Check if receiver location matches any encoded location
    bool matches(const LocationCodeManager& receiver_loc) const;
};
```

#### 1.3 Unit Tests
**Files:** `src/tests/location-code-test.cpp`

```cpp
TEST(LocationCodeManager, ParseDisplayFormat) {
    LocationCodeManager lcm;
    EXPECT_TRUE(lcm.setReceiverLocation("1255-4467-1352"));
    EXPECT_EQ(lcm.getZone(), 1);
    EXPECT_EQ(lcm.toHexFormat(), "Z1:91BB82");
}

TEST(LocationCodeManager, ChecksumValidation) {
    LocationCodeManager lcm;
    EXPECT_TRUE(lcm.validateChecksum("1255-4467-1352"));  // Valid
    EXPECT_FALSE(lcm.validateChecksum("1255-4467-1353")); // Invalid
}

TEST(LocationCodeManager, NFFloMatching) {
    LocationCodeManager lcm;
    lcm.setReceiverLocation("1255-4467-1352");

    // LC1: Exact match
    uint8_t lc1[] = {0x01, 0x91, 0xBB, 0x82}; // NFF=0, 1 location
    EXPECT_TRUE(lcm.matchesAlertLocation(lc1, 0));

    // LC2: L6 surrounding (should not match)
    uint8_t lc2[] = {0x01, 0x91, 0xBB, 0x81}; // Different L6
    EXPECT_FALSE(lcm.matchesAlertLocation(lc2, 0));
}
```

### Success Criteria
- [ ] Parse both formats: `1255-4467-1352` and `Z1:91BB82`
- [ ] Checksum validation: 100% accurate
- [ ] NFF matching: All 4 levels (L3, L4, L5, L6) supported
- [ ] Unit tests: >95% code coverage
- [ ] Performance: < 1ms per match operation

---

## Workstream 2: Sleep/Monitor Duty Cycle
**Agent:** `backend-developer` (System programming specialist)
**Duration:** 1-2 weeks
**Priority:** HIGH (required for Tests 5-6)
**Dependencies:** None (independent of other workstreams)

### Deliverables

#### 2.1 Sleep Monitor Controller
**Files:** `src/backend/sleep-monitor.h`, `src/backend/sleep-monitor.cpp`

```cpp
class SleepMonitorController {
public:
    SleepMonitorController(RadioControllerInterface& radio);

    // Configuration
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Time synchronization (from FIG 0/10)
    void syncToEnsembleTime(uint32_t mjd, uint8_t hour,
                           uint8_t minute, uint8_t second);

    // Sleep/wake cycle
    void enterSleepMode();
    void exitSleepMode();
    bool isSleeping() const;

    // Wakeup scheduling
    void scheduleNextWakeup();  // 5s before minute edge

    // FIC monitoring during wake window
    void monitorFICForAlerts();

    // Alert handling in sleep mode
    void onAlertDetected(const ActiveAnnouncement& ann);

private:
    RadioControllerInterface& radio_;
    bool enabled_;
    bool sleeping_;

    // Timing
    std::chrono::steady_clock::time_point last_ensemble_time_;
    std::chrono::steady_clock::time_point next_wakeup_;
    Timer wakeup_timer_;

    // State
    uint8_t current_minute_;
    bool synchronized_;

    // Helpers
    void calculateNextMinuteEdge();
    void powerDownComponents();
    void powerUpComponents();
};
```

#### 2.2 FIG 0/10 Time Decoder Enhancement
**Files:** `src/backend/fib-processor.cpp` (modify existing)

```cpp
void FIBProcessor::decodeFIG_0_10(const uint8_t* data, uint16_t length) {
    // Existing MJD/time parsing...
    uint32_t mjd = ...;
    uint8_t hour = ...;
    uint8_t minute = ...;
    uint8_t second = ...;

    // NEW: Notify sleep monitor for synchronization
    if (sleep_monitor_) {
        sleep_monitor_->syncToEnsembleTime(mjd, hour, minute, second);
    }

    // Existing code continues...
}
```

#### 2.3 Low-Power Mode Integration
**Platform-specific implementations:**

```cpp
// src/backend/power-manager-linux.cpp
class PowerManagerLinux : public PowerManagerInterface {
public:
    void enterLowPowerMode() override {
        // Reduce CPU frequency
        // Disable unused peripherals
        // Reduce SDR sample rate
    }

    void exitLowPowerMode() override {
        // Restore full performance
    }
};

// src/backend/power-manager-android.cpp
class PowerManagerAndroid : public PowerManagerInterface {
    // Android-specific wake locks
};
```

#### 2.4 Unit Tests
**Files:** `src/tests/sleep-monitor-test.cpp`

```cpp
TEST(SleepMonitor, WakeupScheduling) {
    MockRadioController radio;
    SleepMonitorController monitor(radio);

    // Sync to 12:34:56
    monitor.syncToEnsembleTime(58849, 12, 34, 56);

    // Should schedule wakeup at 12:34:55 (5s before next minute)
    EXPECT_EQ(monitor.getNextWakeupSecond(), 55);
}

TEST(SleepMonitor, AlertInSleep) {
    SleepMonitorController monitor(radio);
    monitor.enterSleepMode();

    ActiveAnnouncement ann;
    ann.cluster_id = 5;
    ann.active_flags.set(AnnouncementType::Alarm);

    monitor.onAlertDetected(ann);

    // Should wake up immediately for alarm
    EXPECT_FALSE(monitor.isSleeping());
}
```

### Success Criteria
- [ ] FIG 0/10 time sync: ±1 second accuracy
- [ ] Wakeup timing: Within 5s window before minute edge
- [ ] Power consumption: <50% of full operation (target)
- [ ] Alert detection: 0% missed alerts in sleep mode
- [ ] Unit tests: >90% coverage

---

## Workstream 3: FIG Decoder Extensions
**Agent:** `backend-developer` (Protocol specialist)
**Duration:** 1-2 weeks
**Priority:** HIGH (required for Tests 4, 5, 6)
**Dependencies:** None

### Deliverables

#### 3.1 FIG 0/15 Decoder (EWS Ensemble Flag)
**Files:** `src/backend/fib-processor.cpp` (add new function)

```cpp
void FIBProcessor::decodeFIG_0_15(const uint8_t* data, uint16_t length) {
    // ETSI EN 300 401 Section 8.1.11.1
    // FIG 0/15: Programme Type (also carries EWS flag)

    uint8_t pd = (data[0] & 0x80) >> 7;  // P/D flag

    // Byte 1, bit 7: EWS flag
    // 0 = No EWS on this ensemble
    // 1 = Ensemble carries EWS
    bool ews_flag = (data[1] & 0x80) != 0;

    #ifdef DEBUG_FIG
    std::clog << "FIG 0/15: EWS flag = "
              << (ews_flag ? "1 (EWS enabled)" : "0 (no EWS)")
              << std::endl;
    #endif

    // Notify RadioController
    radioInterface.onEnsembleEWSFlag(ews_flag);
}
```

#### 3.2 FIG 0/7 OE Field Extraction
**Files:** `src/backend/fib-processor.cpp` (enhance existing)

```cpp
void FIBProcessor::decodeFIG_0_7(const uint8_t* data, uint16_t length) {
    // Existing service component decoding...

    // NEW: Extract OE (Other Ensemble) flag
    // ETSI EN 300 401 Figure 18: FIG type 0/7
    // Byte 1, bit 0: OE flag
    bool oe_flag = (data[1] & 0x01) != 0;

    if (oe_flag) {
        // Bytes 2-3: OE EId (Other Ensemble ID)
        uint16_t oe_eid = (data[2] << 8) | data[3];

        #ifdef DEBUG_FIG
        std::clog << "FIG 0/7: OE flag set, EId = 0x"
                  << std::hex << oe_eid << std::dec
                  << std::endl;
        #endif

        // Pass to AnnouncementManager
        announcement_manager_->addOtherEnsemble(oe_eid);
    }

    // Existing code continues...
}
```

#### 3.3 FIG 0/19 Enhancement (Add IId Field)
**Files:** `src/backend/announcement-types.h` (modify struct)

```cpp
struct ActiveAnnouncement {
    uint8_t cluster_id;
    uint8_t subchannel_id;
    AnnouncementFlags active_flags;

    // NEW: Stage identifier (IId)
    uint8_t stage_id;  // 0-7

    // Helper to get stage name
    const char* getStageName() const {
        static const char* names[] = {
            "L1Start", "L1Update", "L1Repeat", "L1Critical",
            "L2Start", "L2Update", "L2Repeat", "Test"
        };
        return (stage_id <= 7) ? names[stage_id] : "Unknown";
    }

    // Existing methods...
    AnnouncementType getHighestPriorityType() const;
    bool isActive() const;
};
```

**Files:** `src/backend/fib-processor.cpp` (enhance FIG 0/19 parsing)

```cpp
void FIBProcessor::decodeFIG_0_19(const uint8_t* data, uint16_t length) {
    // Existing parsing...

    uint8_t cluster_id = data[0];
    uint16_t asw_flags = (data[1] << 8) | data[2];
    uint8_t subchannel_id = ...;

    // NEW: Extract IId (Stage Identifier)
    // ETSI TS 104 089 Section 5.2.1.2
    // Bits in ASw flags indicate stage
    uint8_t stage_id = 0;
    // Decode from ASw flags based on announcement type
    // (Implementation depends on announcement type priority)

    ActiveAnnouncement ann;
    ann.cluster_id = cluster_id;
    ann.subchannel_id = subchannel_id;
    ann.active_flags.flags = asw_flags;
    ann.stage_id = stage_id;  // NEW

    announcements.push_back(ann);
}
```

#### 3.4 Unit Tests
**Files:** `src/tests/fig-decoder-test.cpp`

```cpp
TEST(FIGDecoder, FIG_0_15_EWSFlag) {
    uint8_t fig_data[] = {
        0x00,  // FIG 0/15
        0x80,  // EWS flag = 1
        // ...
    };

    MockRadioController radio;
    FIBProcessor proc(radio);
    proc.decodeFIG_0_15(fig_data, sizeof(fig_data));

    EXPECT_TRUE(radio.wasEWSFlagSet());
}

TEST(FIGDecoder, FIG_0_7_OEField) {
    uint8_t fig_data[] = {
        0x00,        // FIG 0/7
        0x01,        // OE flag = 1
        0xD0, 0x01,  // OE EId = 0xD001
        // ...
    };

    FIBProcessor proc(radio);
    proc.decodeFIG_0_7(fig_data, sizeof(fig_data));

    EXPECT_TRUE(proc.hasOtherEnsemble(0xD001));
}

TEST(FIGDecoder, FIG_0_19_StageId) {
    uint8_t fig_data[] = {
        0x05,        // Cluster ID
        0x00, 0x01,  // ASw flags (L1Start)
        0x03,        // SubChId
        // ...
    };

    FIBProcessor proc(radio);
    std::vector<ActiveAnnouncement> anns =
        proc.decodeFIG_0_19(fig_data, sizeof(fig_data));

    EXPECT_EQ(anns[0].stage_id, 0);  // L1Start
    EXPECT_STREQ(anns[0].getStageName(), "L1Start");
}
```

### Success Criteria
- [ ] FIG 0/15: EWS flag correctly extracted
- [ ] FIG 0/7: OE field correctly parsed
- [ ] FIG 0/19: IId (stage) correctly decoded
- [ ] Unit tests: 100% coverage of new code
- [ ] Integration: No regressions in existing FIG decoding

---

## Workstream 4: GUI/QML Extensions
**Agent:** `cpp-qt-developer` (Qt/QML specialist)
**Duration:** 2 weeks
**Priority:** MEDIUM (needed for user interaction)
**Dependencies:** Workstream 1 (location code backend)

### Deliverables

#### 4.1 EWS Settings Page
**Files:** `src/welle-gui/QML/settingpages/EWSSettings.qml`

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0

Page {
    title: qsTr("Emergency Warning System")

    ScrollView {
        anchors.fill: parent

        ColumnLayout {
            width: parent.width
            spacing: 20

            // Location Code Input
            GroupBox {
                title: qsTr("Receiver Location")
                Layout.fillWidth: true

                ColumnLayout {
                    width: parent.width

                    Label {
                        text: qsTr("Enter your location code:")
                        font.bold: true
                    }

                    Label {
                        text: qsTr("Format: 1255-4467-1352 or Z1:91BB82")
                        font.pointSize: 9
                        color: "gray"
                    }

                    TextField {
                        id: locationInput
                        Layout.fillWidth: true
                        placeholderText: "1255-4467-1352"

                        validator: RegExpValidator {
                            regExp: /(\d{4}-\d{4}-\d{4})|(Z\d{1,2}:[0-9A-F]{6})/
                        }

                        onTextChanged: {
                            validationIcon.visible = true
                            validationLabel.text = qsTr("Validating...")

                            // Async validation
                            validationTimer.restart()
                        }
                    }

                    RowLayout {
                        Image {
                            id: validationIcon
                            source: locationCodeValid ?
                                    "qrc:/icons/check.svg" :
                                    "qrc:/icons/error.svg"
                            width: 16
                            height: 16
                            visible: false
                        }

                        Label {
                            id: validationLabel
                            text: locationCodeValid ?
                                  qsTr("✓ Valid location code") :
                                  qsTr("✗ Invalid checksum")
                            color: locationCodeValid ? "green" : "red"
                        }
                    }

                    Button {
                        text: qsTr("Set Location")
                        enabled: locationCodeValid
                        Layout.alignment: Qt.AlignRight

                        onClicked: {
                            radioController.setReceiverLocation(locationInput.text)
                            successPopup.open()
                        }
                    }

                    // Current location display
                    Rectangle {
                        Layout.fillWidth: true
                        height: 60
                        color: "#f0f0f0"
                        radius: 5
                        visible: radioController.hasReceiverLocation

                        ColumnLayout {
                            anchors.centerIn: parent

                            Label {
                                text: qsTr("Current Location:")
                                font.pointSize: 9
                                color: "gray"
                            }

                            Label {
                                text: radioController.receiverLocationDisplay
                                font.pointSize: 12
                                font.bold: true
                            }
                        }
                    }
                }
            }

            // Sleep Mode Settings
            GroupBox {
                title: qsTr("Power Saving")
                Layout.fillWidth: true

                ColumnLayout {
                    width: parent.width

                    CheckBox {
                        id: sleepModeCheckbox
                        text: qsTr("Enable sleep mode (battery saving)")
                        checked: radioController.sleepModeEnabled

                        onCheckedChanged: {
                            radioController.setSleepModeEnabled(checked)
                        }
                    }

                    Label {
                        text: qsTr("Sleep mode reduces power consumption by monitoring " +
                                   "alerts only at minute edges (recommended for battery-powered devices)")
                        font.pointSize: 9
                        color: "gray"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        visible: sleepModeCheckbox.checked
                    }

                    // Sleep status indicator
                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        color: radioController.isSleeping ? "#e0f7fa" : "#fff3e0"
                        radius: 5
                        visible: sleepModeCheckbox.checked

                        RowLayout {
                            anchors.centerIn: parent

                            Image {
                                source: radioController.isSleeping ?
                                        "qrc:/icons/sleep.svg" :
                                        "qrc:/icons/awake.svg"
                                width: 24
                                height: 24
                            }

                            Label {
                                text: radioController.isSleeping ?
                                      qsTr("Sleeping") :
                                      qsTr("Monitoring")
                                font.bold: true
                            }
                        }
                    }
                }
            }

            // Alert Preferences
            GroupBox {
                title: qsTr("Alert Types")
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    columnSpacing: 10
                    rowSpacing: 5

                    Repeater {
                        model: radioController.announcementTypeList

                        CheckBox {
                            text: modelData.name
                            checked: modelData.enabled

                            onCheckedChanged: {
                                radioController.setAnnouncementTypeEnabled(
                                    modelData.type, checked)
                            }
                        }
                    }
                }
            }
        }
    }

    // Validation timer
    Timer {
        id: validationTimer
        interval: 500
        repeat: false

        onTriggered: {
            locationCodeValid = radioController.validateLocationCode(
                locationInput.text)
            validationIcon.visible = true
        }
    }

    // Success popup
    Popup {
        id: successPopup
        anchors.centerIn: parent
        width: 300
        height: 150
        modal: true

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 20

            Image {
                source: "qrc:/icons/success.svg"
                width: 48
                height: 48
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: qsTr("Location set successfully!")
                font.pointSize: 12
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignHCenter
                onClicked: successPopup.close()
            }
        }
    }

    // Settings persistence
    Settings {
        property alias locationCode: locationInput.text
        property alias sleepModeEnabled: sleepModeCheckbox.checked
    }
}
```

#### 4.2 Alert Stage Indicator
**Files:** `src/welle-gui/QML/components/AlertStageIndicator.qml`

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root

    property string stageName: ""
    property int stageId: 0

    width: parent.width
    height: 60
    color: getStageColor(stageId)
    radius: 8

    visible: radioController.isInAnnouncement

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 15

        // Alert icon (animated)
        Image {
            source: "qrc:/icons/alert.svg"
            width: 40
            height: 40

            RotationAnimator on rotation {
                from: 0
                to: 360
                duration: 2000
                loops: Animation.Infinite
                running: root.visible
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            Label {
                text: qsTr("EMERGENCY ALERT")
                font.bold: true
                font.pointSize: 14
                color: "white"
            }

            Label {
                text: stageName
                font.pointSize: 12
                color: "white"
            }

            Label {
                text: qsTr("Duration: %1s").arg(
                    radioController.announcementDuration)
                font.pointSize: 10
                color: "white"
                opacity: 0.8
            }
        }

        Button {
            text: qsTr("Return to Service")
            enabled: radioController.allowManualReturn

            onClicked: {
                radioController.returnToOriginalService()
            }
        }
    }

    function getStageColor(stageId) {
        switch (stageId) {
            case 0: return "#ff5252"  // L1Start - Red
            case 1: return "#ff6e40"  // L1Update - Deep Orange
            case 2: return "#ff9100"  // L1Repeat - Orange
            case 3: return "#ff1744"  // L1Critical - Deep Red (flashing)
            case 4: return "#ffab00"  // L2Start - Amber
            case 5: return "#ffc400"  // L2Update - Yellow
            case 6: return "#ffd740"  // L2Repeat - Light Yellow
            case 7: return "#00bfa5"  // Test - Teal
            default: return "#9e9e9e" // Unknown - Gray
        }
    }

    // Flashing animation for L1Critical
    SequentialAnimation on opacity {
        running: stageId === 3 && root.visible
        loops: Animation.Infinite

        NumberAnimation { to: 0.5; duration: 500 }
        NumberAnimation { to: 1.0; duration: 500 }
    }
}
```

#### 4.3 RadioController Integration
**Files:** `src/welle-gui/radio_controller.h` (add Q_PROPERTY)

```cpp
class CRadioController : public RadioControllerInterface {
    Q_OBJECT

    // Existing properties...

    // NEW: EWS properties
    Q_PROPERTY(QString receiverLocationDisplay
               READ getReceiverLocationDisplay
               NOTIFY receiverLocationChanged)
    Q_PROPERTY(bool hasReceiverLocation
               READ hasReceiverLocation
               NOTIFY receiverLocationChanged)
    Q_PROPERTY(bool sleepModeEnabled
               READ isSleepModeEnabled
               WRITE setSleepModeEnabled
               NOTIFY sleepModeStateChanged)
    Q_PROPERTY(bool isSleeping
               READ isSleeping
               NOTIFY sleepModeStateChanged)
    Q_PROPERTY(QString currentAlertStage
               READ getCurrentAlertStageName
               NOTIFY alertStageChanged)
    Q_PROPERTY(int announcementDuration
               READ getAnnouncementDuration
               NOTIFY announcementDurationChanged)
    Q_PROPERTY(bool allowManualReturn
               READ getAllowManualReturn
               CONSTANT)

public:
    // Location code methods
    Q_INVOKABLE bool validateLocationCode(const QString& code);
    Q_INVOKABLE void setReceiverLocation(const QString& code);
    QString getReceiverLocationDisplay() const;
    bool hasReceiverLocation() const;

    // Sleep mode methods
    Q_INVOKABLE void setSleepModeEnabled(bool enabled);
    bool isSleepModeEnabled() const;
    bool isSleeping() const;

    // Alert stage methods
    QString getCurrentAlertStageName() const;
    int getAnnouncementDuration() const;
    bool getAllowManualReturn() const;

    // Announcement type list (for UI)
    Q_INVOKABLE QVariantList getAnnouncementTypeList() const;
    Q_INVOKABLE void setAnnouncementTypeEnabled(int type, bool enabled);

signals:
    void receiverLocationChanged();
    void sleepModeStateChanged();
    void alertStageChanged(const QString& stage);
    void announcementDurationChanged();

private:
    LocationCodeManager location_manager_;
    SleepMonitorController sleep_monitor_;

    // Timer for duration updates
    QTimer announcement_duration_timer_;
};
```

### Success Criteria
- [ ] Location code input: User-friendly validation with real-time feedback
- [ ] Sleep mode toggle: Clear indication of sleep/wake state
- [ ] Alert stage indicator: Visually distinct for 8 stages
- [ ] Settings persistence: Location code saved across sessions
- [ ] Responsive UI: <100ms interaction latency

---

## Workstream 5: Test Automation
**Agent:** `test-automator` (Test framework specialist)
**Duration:** 2-3 weeks
**Priority:** HIGH (validation required)
**Dependencies:** Workstreams 1-3 (backend features)

### Deliverables

#### 5.1 ETI Test Runner
**Files:** `src/tests/ews-test-runner.cpp`, `src/tests/ews-test-runner.h`

```cpp
class EWSTestRunner {
public:
    struct TestResult {
        int test_number;
        std::string test_name;
        bool passed;
        std::string details;
        std::chrono::seconds duration;
    };

    EWSTestRunner(const std::string& welle_cli_path);

    // Run single test
    TestResult runTest(int test_num, const std::string& eti_file);

    // Run all 7 tests
    std::vector<TestResult> runAllTests();

    // Generate HTML report
    void generateReport(const std::vector<TestResult>& results,
                       const std::string& output_path);

private:
    std::string welle_cli_path_;
    std::string test_data_dir_;

    // Test implementations
    TestResult runTest1_SetupBehaviour();
    TestResult runTest2_AreaMatching();
    TestResult runTest3_AlertStages();
    TestResult runTest4_OtherEnsemble();
    TestResult runTest5_SleepSelection();
    TestResult runTest6_SleepResponse();
    TestResult runTest7_ConcurrentAlerts();

    // Validation helpers
    bool validateServiceSwitch(const std::string& expected_service,
                              std::chrono::seconds at_time);
    bool validateAudioFrequency(int expected_hz,
                               std::chrono::seconds at_time);
    bool validateSleepWake(std::chrono::seconds expected_wakeup);
};
```

#### 5.2 Audio Frequency Validator
**Files:** `src/tests/audio-validator.cpp`

```cpp
class AudioFrequencyValidator {
public:
    AudioFrequencyValidator(const std::string& audio_device);

    // Start recording
    void startRecording();

    // Stop recording
    void stopRecording();

    // Detect dominant frequency at timestamp
    int detectFrequency(std::chrono::seconds timestamp);

    // Check if specific frequency present
    bool hasFrequency(int hz, std::chrono::seconds timestamp,
                     int tolerance_hz = 10);

private:
    std::string audio_device_;
    std::vector<int16_t> recorded_samples_;
    int sample_rate_;

    // FFT for frequency analysis
    std::vector<double> performFFT(const int16_t* samples, size_t count);
    int findPeakFrequency(const std::vector<double>& fft_result);
};
```

#### 5.3 Test Suite (Google Test)
**Files:** `src/tests/ews-compliance-test.cpp`

```cpp
#include <gtest/gtest.h>
#include "ews-test-runner.h"

class EWSComplianceTest : public ::testing::Test {
protected:
    void SetUp() override {
        runner_ = std::make_unique<EWSTestRunner>(
            "./build/src/welle-cli/welle-cli");

        // Set location code for all tests
        system("./build/src/welle-cli/welle-cli --set-location 1255-4467-1352");
    }

    std::unique_ptr<EWSTestRunner> runner_;
};

TEST_F(EWSComplianceTest, Test1_SetupBehaviour) {
    auto result = runner_->runTest(1, "test-data/ews/EWS1.eti");

    EXPECT_TRUE(result.passed) << result.details;

    // Specific validations
    EXPECT_TRUE(result.details.find("Switched to Alert 1 at 1:05")
                != std::string::npos);
    EXPECT_TRUE(result.details.find("Switched to Alert 2 at 1:25")
                != std::string::npos);
}

TEST_F(EWSComplianceTest, Test2_AreaMatching) {
    auto result = runner_->runTest(2, "test-data/ews/EWS2.eti");

    EXPECT_TRUE(result.passed) << result.details;

    // LC1, LC5, LC6 should match
    EXPECT_TRUE(result.details.find("LC1: MATCHED") != std::string::npos);
    EXPECT_TRUE(result.details.find("LC5: MATCHED") != std::string::npos);
    EXPECT_TRUE(result.details.find("LC6: MATCHED") != std::string::npos);

    // LC2, LC3, LC4 should NOT match
    EXPECT_TRUE(result.details.find("LC2: NOT MATCHED") != std::string::npos);
    EXPECT_TRUE(result.details.find("LC3: NOT MATCHED") != std::string::npos);
    EXPECT_TRUE(result.details.find("LC4: NOT MATCHED") != std::string::npos);
}

TEST_F(EWSComplianceTest, Test3_AlertStages) {
    auto result = runner_->runTest(3, "test-data/ews/EWS3.eti");

    EXPECT_TRUE(result.passed) << result.details;

    // All 8 stages should be detected
    std::vector<std::string> stages = {
        "L1Start", "L1Update", "L1Repeat", "L1Critical",
        "L2Start", "L2Update", "L2Repeat", "Test"
    };

    for (const auto& stage : stages) {
        EXPECT_TRUE(result.details.find(stage) != std::string::npos)
            << "Stage " << stage << " not detected";
    }
}

TEST_F(EWSComplianceTest, Test4_OtherEnsemble) {
    auto result = runner_->runTest(4, "test-data/ews/EWS3.eti,test-data/ews/EWS4.eti");

    EXPECT_TRUE(result.passed) << result.details;

    // Should switch to tuned ensemble alerts only
    EXPECT_TRUE(result.details.find("Switched to ensemble D001 alert")
                != std::string::npos);
    EXPECT_TRUE(result.details.find("Ignored ensemble D002 alert")
                != std::string::npos);
}

TEST_F(EWSComplianceTest, Test5_SleepSelection) {
    auto result = runner_->runTest(5, "test-data/ews/EWS5.eti,test-data/ews/EWS6.eti");

    EXPECT_TRUE(result.passed) << result.details;

    // Should wake at minute edges
    EXPECT_TRUE(result.details.find("Woke at 2:00") != std::string::npos);
    EXPECT_TRUE(result.details.find("Woke at 4:00") != std::string::npos);

    // Should switch to stronger ensemble
    EXPECT_TRUE(result.details.find("Switched to EWS6 (stronger signal)")
                != std::string::npos);
}

TEST_F(EWSComplianceTest, Test6_SleepResponse) {
    auto result = runner_->runTest(6, "test-data/ews/EWS7.eti");

    EXPECT_TRUE(result.passed) << result.details;

    // Should wake for alerts
    EXPECT_TRUE(result.details.find("Woke for L1Start at 1:00")
                != std::string::npos);
}

TEST_F(EWSComplianceTest, Test7_ConcurrentAlerts) {
    auto result = runner_->runTest(7, "test-data/ews/EWS8.eti,test-data/ews/EWS9.eti");

    EXPECT_TRUE(result.passed) << result.details;

    // Should prioritize higher priority alerts
    EXPECT_TRUE(result.details.find("Switched to L2Start (higher priority)")
                != std::string::npos);
}

// Run all tests
TEST_F(EWSComplianceTest, RunAllTests) {
    auto results = runner_->runAllTests();

    int passed = 0;
    for (const auto& result : results) {
        if (result.passed) passed++;
    }

    // Generate HTML report
    runner_->generateReport(results, "ews-test-report.html");

    // All tests must pass
    EXPECT_EQ(passed, 7) << "Only " << passed << "/7 tests passed";
}
```

#### 5.4 CI/CD Integration
**Files:** `.github/workflows/ews-compliance.yml`

```yaml
name: EWS Compliance Tests

on:
  push:
    branches: [ feature/ews-compliance ]
  pull_request:
    branches: [ master ]

jobs:
  ews-tests:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential cmake qt6-base-dev \
          libfftw3-dev libfaad-dev librtlsdr-dev

    - name: Download EWS test files
      run: |
        mkdir -p test-data/ews
        cd test-data/ews
        wget https://docbox.etsi.org/Broadcast/Open/EWSv1_1.zip
        unzip EWSv1_1.zip

    - name: Build welle.io
      run: |
        mkdir build && cd build
        cmake .. -DRTLSDR=1 -DBUILD_WELLE_CLI=ON
        make -j$(nproc)

    - name: Run EWS compliance tests
      run: |
        cd build
        ctest -R EWSCompliance --output-on-failure

    - name: Generate test report
      run: |
        ./build/src/tests/ews-test-runner --generate-report

    - name: Upload test report
      uses: actions/upload-artifact@v2
      with:
        name: ews-test-report
        path: ews-test-report.html
```

### Success Criteria
- [ ] All 7 tests automated
- [ ] Audio frequency validation: ±10 Hz accuracy
- [ ] Test execution time: <15 minutes for full suite
- [ ] HTML report generation: Pass/fail with details
- [ ] CI/CD integration: Automatic testing on PR

---

## Integration & Coordination

### Weekly Sync Points

**Week 1 Checkpoint:**
- Agent 1 (Location): Parser complete, validation working
- Agent 2 (Sleep): FIG 0/10 sync implemented
- Agent 3 (FIG): FIG 0/15, 0/7 OE decoded
- Agent 4 (GUI): Location input UI mockup
- Agent 5 (Test): Test runner framework ready

**Week 2 Checkpoint:**
- Agent 1: NFF matching algorithm complete
- Agent 2: Wakeup scheduling working
- Agent 3: FIG 0/19 IId parsing complete
- Agent 4: EWS settings page integrated
- Agent 5: Tests 1-3 automated

**Week 3 Checkpoint:**
- Agent 1: Unit tests passing, integration ready
- Agent 2: Low-power mode implemented
- Agent 3: All FIG enhancements complete
- Agent 4: Alert stage indicator working
- Agent 5: Tests 4-7 automated

**Week 4 Final Integration:**
- All agents: Merge branches
- Full system test with all 7 ETI files
- Bug fixes and refinements
- Documentation updates

### Communication Protocol

**Daily Standups (Async):**
```markdown
Agent [N]: [Name]
- Yesterday: [Completed tasks]
- Today: [Planned tasks]
- Blockers: [Any dependencies/issues]
```

**Shared Resources:**
- Git branches: `feature/ews-[workstream]`
- Test data: Shared `test-data/ews/` directory
- Documentation: Update `docs/ews/` as you go

### Dependency Management

```
Location Codes (Agent 1)
    ↓ (provides LocationCodeManager)
GUI (Agent 4)
    ↓ (uses LocationCodeManager in UI)
Test Suite (Agent 5)

Sleep Monitor (Agent 2)
    ↓ (uses FIG 0/10 from Agent 3)
FIG Extensions (Agent 3)
    ↓ (provides time sync, OE data)
Test Suite (Agent 5)
```

**Critical Path:**
1. Agent 1 → Agent 4 (location UI needs backend)
2. Agent 3 → Agent 2 (sleep needs FIG 0/10)
3. Agents 1-3 → Agent 5 (tests need backend features)

**Workaround:** Use mock implementations during development:
```cpp
class MockLocationCodeManager : public LocationCodeManager {
    bool matchesAlertLocation(...) override { return true; }
};
```

---

## Success Metrics

### Code Quality
- [ ] Unit test coverage: >90%
- [ ] Integration test coverage: 100% (all 7 ETSI tests)
- [ ] Code review: 2+ approvals per PR
- [ ] Static analysis: 0 critical issues (cppcheck, clang-tidy)

### Performance
- [ ] Location matching: <1ms
- [ ] Sleep mode power: <50% of active mode
- [ ] FIG decoding: <5ms per FIB
- [ ] UI responsiveness: <100ms

### Compliance
- [ ] ETSI TS 104 090: 7/7 tests passed
- [ ] NBTC Thailand: Compatible with Thai characters
- [ ] Documentation: Complete API docs + user guide

---

## Risk Mitigation

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| **Location code algorithm complexity** | Medium | High | Agent 1 can consult ETSI reference implementation |
| **Sleep mode battery measurement** | Medium | Medium | Use power profiler, document results |
| **OE alerts require 2 tuners** | High | Low | Document limitation, implement single-tuner fallback |
| **Test ETI files download failure** | Low | Critical | Mirror files in project repo |
| **Agent merge conflicts** | Medium | Medium | Daily sync + clear API boundaries |

---

## Next Steps

1. **Create Git branches** for each workstream
2. **Assign agents** (can be same person with different roles)
3. **Download test files** immediately
4. **Kick off all 5 workstreams in parallel**
5. **Daily async standups** via GitHub issues/discussions

---

**Document Status:** READY FOR EXECUTION
**Approval:** Pending
**Start Date:** TBD

