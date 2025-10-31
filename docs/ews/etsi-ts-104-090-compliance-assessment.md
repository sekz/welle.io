# ETSI TS 104 090 V1.1.2 Compliance Assessment
## Emergency Warning System (EWS) Requirements for welle.io DAB+ Receiver

**Document Version:** 1.0
**Date:** 2025-10-15
**Standard:** ETSI TS 104 090 V1.1.2 (2025-02) - Emergency Warning System minimum requirements and test specifications for receivers

---

## Executive Summary

This document assesses the compliance of **welle.io DAB+ receiver** against **ETSI TS 104 090 V1.1.2** requirements for Emergency Warning System (EWS) receivers. The standard defines minimum requirements and test specifications for domestic and in-vehicle receivers designed to handle emergency alert broadcasts via DAB.

### Current Status

| Category | Status | Notes |
|----------|--------|-------|
| **FIG Decoding** | ⚠️ **PARTIAL** | FIG 0/7 (OE) and FIG 0/15 (EWS ensemble flag) need verification |
| **Alert Switching** | ⚠️ **PARTIAL** | Announcement manager exists but needs EWS-specific extensions |
| **Location Codes** | ❌ **MISSING** | No location code input, validation, or matching |
| **Sleep/Monitor Mode** | ❌ **MISSING** | No duty cycle implementation for battery-powered operation |
| **Alert Stages** | ❌ **MISSING** | L1Start/Update/Repeat/Critical, L2Start/Update/Repeat not supported |
| **OE Signalling** | ❌ **MISSING** | Other Ensemble alerts not implemented |
| **Test Capability** | ✅ **READY** | Can load ETI files for testing |

### Compliance Summary

- **Current Implementation:** ~30% compliant
- **Major Gaps:** Location handling, sleep mode, alert stages, OE signalling
- **Estimated Effort:** 4-6 weeks (160-240 hours)
- **Priority:** **HIGH** (required for NBTC Thailand EWS compliance)

---

## 1. Standard Overview

### 1.1 Scope (ETSI TS 104 090 Clause 1)

The standard defines:
- **Domestic receivers**: Battery or mains-powered, with sleep/monitor duty cycle
- **In-vehicle receivers**: Require 2+ tuners, GNSS location awareness
- **Test specifications**: 7 tests using 9 ETI files (EWS1.eti - EWS9.eti)
- **Certification mark**: Optional EWS logo for compliant receivers

### 1.2 Key Requirements

**Clause 5 - Domestic Receivers:**
1. FIG 0/15 detection (EWS ensemble identification)
2. Location code entry with checksum validation
3. Sleep/monitor duty cycle (FIG 0/10 time sync)
4. Alert area matching (location code comparison)
5. Alert stages handling (8 stages from L1Start to Test)
6. Other Ensemble (OE) alert monitoring
7. Concurrent alerts priority management

**Clause 7 - Test Specifications:**
- Test 1: Set-up behaviour (with/without location code)
- Test 2: Alert area matching (6 location code sets LC1-LC6)
- Test 3: Alert stages (8 stages)
- Test 4: Other ensemble alerts
- Test 5: Sleep mode EWS selection
- Test 6: Sleep mode alert response
- Test 7: Concurrent alerts

---

## 2. Current Implementation Analysis

### 2.1 Existing Announcement Support

welle.io has basic announcement support:

**✅ Implemented:**
- `AnnouncementManager` class ([announcement-manager.h](../../src/backend/announcement-manager.h))
- FIG 0/18 decoding (announcement support)
- FIG 0/19 decoding (active announcements)
- Cluster 0xFF (Alarm) special handling with Al flag
- Priority-based switching (11 announcement types)
- User preferences (enable/disable types)
- Switching state machine (6 states)

**Code Location:**
```
src/backend/announcement-manager.cpp    (620 lines)
src/backend/announcement-manager.h      (470 lines)
src/backend/announcement-types.h
src/backend/fic-handler.cpp             (FIG decoding)
src/welle-gui/radio_controller.cpp      (UI integration)
```

### 2.2 Gap Analysis

| Requirement | Current Status | Gap Description |
|-------------|---------------|-----------------|
| **FIG 0/15 Detection** | ⚠️ VERIFY | Need to confirm FIG 0/15 (EWS ensemble flag) is decoded |
| **Location Code Input** | ❌ MISSING | No UI for entering location code (format: `1255-4467-1352` or `Z1:91BB82`) |
| **Location Checksum** | ❌ MISSING | No checksum calculation/validation (ETSI TS 104 089 Annex A) |
| **Alert Area Matching** | ❌ MISSING | No location code comparison logic |
| **Sleep/Monitor Duty Cycle** | ❌ MISSING | No time-sync to FIG 0/10 minute edge |
| **Alert Stages** | ❌ MISSING | Only basic start/stop, no L1Start/Update/Repeat/Critical stages |
| **IId (Stage Identifier)** | ❌ MISSING | Not parsed from FIG 0/19 |
| **OE Signalling** | ❌ MISSING | FIG 0/7 OE field not processed |
| **OE Other Ensemble Alerts** | ❌ MISSING | Cannot monitor alerts from other ensembles |
| **Concurrent Alerts** | ⚠️ PARTIAL | Priority exists but no proper concurrent handling |
| **ETI File Playback** | ✅ EXISTS | Can load ETI files via `raw_file.cpp` |

---

## 3. Required Modifications

### 3.1 Backend Modifications

#### 3.1.1 FIG Decoder Enhancements

**File:** `src/backend/fib-processor.cpp`

```cpp
// Add FIG 0/15 (EWS ensemble indicator) decoding
void FIBProcessor::decodeFIG_0_15(const uint8_t* data, uint16_t length) {
    // Bit 0: EWS flag (1 = ensemble carries EWS)
    bool ews_flag = (data[0] & 0x80) != 0;

    // Notify RadioController
    radioInterface.onEnsembleEWSFlag(ews_flag);
}

// Add FIG 0/7 OE (Other Ensemble) field extraction
// Currently missing - needed for Test 4
void FIBProcessor::decodeFIG_0_7_OE_field(const uint8_t* data) {
    // Extract OE flag from FIG 0/7 byte 1 bit 0
    bool oe_flag = (data[1] & 0x01) != 0;

    // If OE=1, extract OE EId from bytes 2-3
    if (oe_flag) {
        uint16_t oe_eid = (data[2] << 8) | data[3];
        // Pass to AnnouncementManager for OE alert monitoring
    }
}
```

**FIG 0/19 Enhancement - Add IId (Stage Identifier):**
```cpp
struct ActiveAnnouncement {
    uint8_t cluster_id;
    uint8_t subchannel_id;
    AnnouncementFlags active_flags;
    uint8_t stage_id;  // ← ADD THIS (IId from FIG 0/19)
    // Maps to: 0=L1Start, 1=L1Update, 2=L1Repeat, 3=L1Critical,
    //          4=L2Start, 5=L2Update, 6=L2Repeat, 7=Test
};
```

#### 3.1.2 Location Code Manager

**New File:** `src/backend/location-code-manager.h` / `.cpp`

```cpp
class LocationCodeManager {
public:
    // Parse location code from user input
    // Formats: "1255-4467-1352" or "Z1:91BB82"
    bool setReceiverLocation(const std::string& code);

    // Validate checksum (ETSI TS 104 089 Annex A)
    bool validateChecksum(const std::string& code);

    // Check if alert location matches receiver location
    // Supports NFF encoding (4 levels: L3, L4, L5, L6)
    bool matchesAlertLocation(const uint8_t* location_data, uint8_t nff);

    // Convert between formats
    std::string toDisplayFormat();    // "1255-4467-1352"
    std::string toHexFormat();        // "Z1:91BB82"

private:
    uint8_t zone_;          // 0-41
    uint32_t location_;     // 24-bit location code
    uint8_t checksum_;      // 8-bit checksum
};
```

#### 3.1.3 Sleep/Monitor Duty Cycle

**New File:** `src/backend/sleep-monitor.h` / `.cpp`

```cpp
class SleepMonitorController {
public:
    // Synchronize to FIG 0/10 time (minute edge)
    void syncToEnsembleTime(const EnsembleTime& time);

    // Enter sleep mode (low-power)
    void enterSleepMode();

    // Wake up 5 seconds before minute edge
    void scheduleWakeup();

    // Monitor FIC for alerts during wake window
    void monitorFICForAlerts();

    // Handle alert detected in sleep mode
    void onAlertDetectedInSleep(const ActiveAnnouncement& ann);

private:
    bool is_sleeping_;
    std::chrono::steady_clock::time_point next_wakeup_;
    Timer wakeup_timer_;
};
```

#### 3.1.4 Alert Stages Support

**File:** `src/backend/announcement-types.h`

```cpp
enum class AlertStage : uint8_t {
    L1Start    = 0,  // Level 1 Start
    L1Update   = 1,  // Level 1 Update
    L1Repeat   = 2,  // Level 1 Repeat
    L1Critical = 3,  // Level 1 Critical
    L2Start    = 4,  // Level 2 Start
    L2Update   = 5,  // Level 2 Update
    L2Repeat   = 6,  // Level 2 Repeat
    Test       = 7,  // Test announcement
    Unknown    = 255
};

const char* getAlertStageName(AlertStage stage);
```

#### 3.1.5 Other Ensemble (OE) Alert Monitoring

**File:** `src/backend/announcement-manager.h`

```cpp
class AnnouncementManager {
public:
    // Monitor other ensemble for alerts
    void addOtherEnsemble(uint16_t oe_eid);

    // Process OE alert from FIG 0/7
    void processOtherEnsembleAlert(uint16_t oe_eid,
                                    const ActiveAnnouncement& ann);

    // Check if should switch to OE alert
    bool shouldSwitchToOEAlert(const ActiveAnnouncement& ann);

private:
    std::map<uint16_t, std::vector<ActiveAnnouncement>> oe_alerts_;
};
```

### 3.2 GUI Modifications

#### 3.2.1 Location Code Input Dialog

**New File:** `src/welle-gui/QML/settingpages/EWSSettings.qml`

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    GroupBox {
        title: qsTr("Emergency Warning System (EWS)")

        ColumnLayout {
            Label {
                text: qsTr("Receiver Location Code:")
                font.bold: true
            }

            Label {
                text: qsTr("Enter your location code (format: 1255-4467-1352)")
                font.pointSize: 9
                color: "gray"
            }

            TextField {
                id: locationCodeInput
                placeholderText: "1255-4467-1352"
                validator: RegExpValidator {
                    regExp: /\d{4}-\d{4}-\d{4}/
                }
                onTextChanged: {
                    // Validate checksum
                    locationCodeValid = radioController.validateLocationCode(text)
                }
            }

            Label {
                text: locationCodeValid ?
                      qsTr("✓ Valid location code") :
                      qsTr("✗ Invalid checksum")
                color: locationCodeValid ? "green" : "red"
            }

            Button {
                text: qsTr("Set Location")
                enabled: locationCodeValid
                onClicked: {
                    radioController.setReceiverLocation(locationCodeInput.text)
                }
            }
        }
    }

    GroupBox {
        title: qsTr("Alert Preferences")

        CheckBox {
            text: qsTr("Enable sleep mode (battery saving)")
            checked: true
            onCheckedChanged: {
                radioController.setSleepModeEnabled(checked)
            }
        }
    }
}
```

#### 3.2.2 Alert Stage Display

**File:** `src/welle-gui/QML/RadioView.qml`

```qml
// Add alert stage indicator
Rectangle {
    visible: radioController.isInAnnouncement
    color: getStageColor(radioController.currentAlertStage)

    Text {
        text: radioController.currentAlertStageName
        // Shows: "Level 1 Start", "Level 1 Critical", etc.
    }
}
```

### 3.3 RadioController Integration

**File:** `src/welle-gui/radio_controller.h`

```cpp
class CRadioController : public RadioControllerInterface {
    Q_OBJECT

    Q_PROPERTY(QString receiverLocation READ getReceiverLocation WRITE setReceiverLocation NOTIFY receiverLocationChanged)
    Q_PROPERTY(bool sleepModeEnabled READ isSleepModeEnabled WRITE setSleepModeEnabled)
    Q_PROPERTY(QString currentAlertStage READ getCurrentAlertStageName)

public:
    // Location code management
    Q_INVOKABLE bool validateLocationCode(const QString& code);
    Q_INVOKABLE void setReceiverLocation(const QString& code);
    QString getReceiverLocation() const;

    // Sleep mode control
    Q_INVOKABLE void setSleepModeEnabled(bool enabled);
    bool isSleepModeEnabled() const;

    // Alert stage queries
    QString getCurrentAlertStageName() const;

signals:
    void receiverLocationChanged(const QString& location);
    void alertStageChanged(const QString& stage);
    void sleepModeStateChanged(bool sleeping);

private:
    LocationCodeManager location_manager_;
    SleepMonitorController sleep_monitor_;
};
```

---

## 4. Test Plan

### 4.1 Test Environment Setup

**Requirements:**
- ETI file playback capability (already exists in welle.io)
- 9 test ETI files from ETSI TS 104 090 Annex A
- RF signal generator (optional, for radiated testing)
- Timing device (stopwatch or automated timing)

**Download Test Files:**
```bash
# From ETSI docbox (free download)
wget https://docbox.etsi.org/Broadcast/Open/EWSv1_1.zip
unzip EWSv1_1.zip

# Files included:
# EWS1.eti - EWS9.eti (9 test streams)
# Location code sets (LC1-LC8)
```

### 4.2 Test Execution

#### Test 1: Set-up Behaviour (Clause 7.3)

**Objective:** Verify initialization with/without location code

**Steps:**
1. Factory reset welle.io
2. Load `EWS1.eti`
3. Test WITHOUT location code → should NOT switch to alerts
4. Test WITH location code `1255-4467-1352` → should switch to Alert 1, Alert 2

**Expected Results:** See Table 1 (page 12 of standard)

**Implementation Status:** ❌ **BLOCKED** - No location code input

---

#### Test 2: Alert Area Matching (Clause 7.4)

**Objective:** Verify location code matching with different area sets

**Steps:**
1. Set location code `1255-4467-1352`
2. Load `EWS2.eti`
3. Verify switching only for matching locations (LC1, LC2, LC5, LC6)

**Expected Results:** See Table 2 (page 13 of standard)

**Implementation Status:** ❌ **BLOCKED** - No location matching

---

#### Test 3: Alert Stages (Clause 7.5)

**Objective:** Verify 8 alert stages (L1Start → Test)

**Steps:**
1. Set location code `1255-4467-1352`
2. Load `EWS3.eti`
3. Verify display changes for each stage:
   - 0m30: "Level 1 Start"
   - 0m50: "Level 1 Update"
   - 1m10: "Level 1 Repeat"
   - 1m30: "Level 1 Critical"
   - 1m50: "Level 2 Start"
   - ...

**Expected Results:** See Table 3 (page 14 of standard)

**Implementation Status:** ❌ **BLOCKED** - No stage support

---

#### Test 4: Other Ensemble Alerts (Clause 7.6)

**Objective:** Verify OE (Other Ensemble) alert monitoring

**Steps:**
1. Use 2 ETI players simultaneously
2. Load `EWS3.eti` on player 1 (EId D001)
3. Load `EWS4.eti` on player 2 (EId D002)
4. Verify switching to alerts from ensemble D001 when service from D001 is playing

**Expected Results:** See Table 4 (page 15 of standard)

**Implementation Status:** ❌ **BLOCKED** - No OE support

---

#### Test 5: Sleep Mode EWS Selection (Clause 7.7)

**Objective:** Verify ensemble selection in sleep mode

**Steps:**
1. Load `EWS5.eti` + `EWS6.eti`
2. Put receiver to sleep
3. At 2m20: reduce signal level of EWS5 to -85 dBm
4. At 4m00: verify switched to EWS6 (stronger signal)

**Expected Results:** See Table 5 (page 17 of standard)

**Implementation Status:** ❌ **BLOCKED** - No sleep mode

---

#### Test 6: Sleep Mode Alert Response (Clause 7.8)

**Objective:** Verify wake-up on alert during sleep mode

**Steps:**
1. Continue from Test 5
2. Load `EWS7.eti`
3. Verify receiver wakes up at minute edges to check for alerts
4. Verify automatic switch to announcement

**Expected Results:** See Table 6 (page 18 of standard)

**Implementation Status:** ❌ **BLOCKED** - No sleep mode

---

#### Test 7: Concurrent Alerts (Clause 7.9)

**Objective:** Verify priority handling with multiple concurrent alerts

**Steps:**
1. Load `EWS8.eti` + `EWS9.eti` (2 ensembles)
2. Select "Service 1"
3. At 0m30: verify switches to L1Start (higher priority)
4. At 0m40: verify switches to L2Start (Level 2 overrides Level 1)
5. Test user cancellation of alert

**Expected Results:** See Table 7 (page 19 of standard)

**Implementation Status:** ⚠️ **PARTIAL** - Priority exists but no concurrent handling

---

### 4.3 Automated Testing

**Recommended Approach:**
```python
# Python test script using welle-cli
import subprocess
import time

def test_ews_compliance():
    tests = [
        ("Test 1", "EWS1.eti", test_1_setup_behaviour),
        ("Test 2", "EWS2.eti", test_2_area_matching),
        # ...
    ]

    for name, eti_file, test_func in tests:
        print(f"Running {name}...")
        result = test_func(eti_file)
        print(f"{name}: {'PASS' if result else 'FAIL'}")

def test_1_setup_behaviour(eti_file):
    # Start welle-cli with ETI file
    proc = subprocess.Popen([
        "./welle-cli",
        "-F", f"rawfile,{eti_file}",
        "-D"  # Dump mode
    ])

    # Monitor output for expected service switches
    time.sleep(90)  # Wait for 1m30
    # Check if switched to Alert 1
    # ...

    proc.kill()
    return True  # or False

# Run tests
test_ews_compliance()
```

---

## 5. Implementation Roadmap

### Phase 1: Core EWS Support (2-3 weeks)

**Week 1:**
- [ ] Implement `LocationCodeManager`
  - Parse location code formats
  - Checksum validation
  - Location matching algorithm
- [ ] Add FIG 0/15 decoding
- [ ] Enhance FIG 0/19 with IId (stage identifier)
- [ ] Add alert stage enum and names

**Week 2:**
- [ ] Implement alert stage handling in `AnnouncementManager`
- [ ] Add location code input UI
- [ ] Integrate location matching into switching logic
- [ ] Test with EWS1.eti, EWS2.eti

**Week 3:**
- [ ] Implement FIG 0/7 OE field extraction
- [ ] Add OE alert monitoring
- [ ] Test with EWS3.eti, EWS4.eti

### Phase 2: Sleep/Monitor Mode (1-2 weeks)

**Week 4:**
- [ ] Implement `SleepMonitorController`
- [ ] FIG 0/10 time synchronization
- [ ] Minute-edge wakeup timer
- [ ] Low-power mode integration
- [ ] Test with EWS5.eti, EWS6.eti, EWS7.eti

### Phase 3: Concurrent Alerts & Testing (1-2 weeks)

**Week 5:**
- [ ] Implement concurrent alert priority handling
- [ ] User cancellation support
- [ ] Test with EWS8.eti, EWS9.eti

**Week 6:**
- [ ] Full test suite execution
- [ ] Bug fixes and refinements
- [ ] Documentation updates

### Phase 4: Certification (optional)

- [ ] Apply for ETSI EWS Certification Mark
- [ ] Third-party testing
- [ ] NBTC Thailand approval (if required)

---

## 6. Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| **FIG 0/15 not decoded** | Medium | High | Verify FIBProcessor, add if missing |
| **Location code complexity** | Low | Medium | Use ETSI reference implementation |
| **Sleep mode battery drain** | Medium | High | Thorough testing with power meter |
| **OE alerts require 2 tuners** | High | Medium | Document limitation for domestic receivers |
| **Test ETI files not available** | Low | Critical | Download from ETSI docbox immediately |
| **Timing precision** | Medium | Medium | Use steady_clock, not system_clock |

---

## 7. Conclusion

### 7.1 Summary

welle.io has a **solid foundation** with existing announcement support, but requires **significant extensions** to meet ETSI TS 104 090 compliance:

**Strengths:**
- ✅ FIG 0/18, FIG 0/19 decoding exists
- ✅ Priority-based switching implemented
- ✅ State machine architecture sound
- ✅ ETI file playback ready

**Critical Gaps:**
- ❌ Location code management (MANDATORY)
- ❌ Sleep/monitor duty cycle (MANDATORY for domestic)
- ❌ Alert stages support (MANDATORY)
- ❌ OE signalling (MANDATORY for Test 4)

### 7.2 Recommendations

**Priority 1 (Immediate):**
1. Implement location code manager
2. Add alert stage support
3. Verify FIG 0/15 decoding

**Priority 2 (Next Sprint):**
4. Implement sleep/monitor mode
5. Add OE alert monitoring
6. Execute test suite

**Priority 3 (Future):**
7. Optimize battery consumption
8. Add advanced UI features
9. Apply for certification

### 7.3 Estimated Effort

- **Development:** 4-6 weeks (160-240 hours)
- **Testing:** 2-3 weeks (80-120 hours)
- **Documentation:** 1 week (40 hours)
- **Total:** **7-10 weeks** (280-400 hours)

### 7.4 Next Steps

1. **Download test ETI files** from ETSI docbox
2. **Create feature branch:** `git checkout -b feature/ews-compliance`
3. **Start with Phase 1** (Location code manager)
4. **Run Test 1** to validate approach
5. **Iterate** through remaining tests

---

## Appendices

### Appendix A: Location Code Format

**Format 1 (Display):** `1255-4467-1352`
- Part 1: `1255` = Zone (1) + L3 code (255)
- Part 2: `4467` = L4 code
- Part 3: `1352` = L5 code + checksum

**Format 2 (Hex):** `Z1:91BB82`
- Zone: `Z1` (0-41)
- Location: `91BB82` (24-bit hex)

**Checksum Algorithm (ETSI TS 104 089 Annex A):**
```python
def calculate_checksum(zone, location_24bit):
    data = [(zone << 24) | location_24bit]
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum & 0xFF
```

### Appendix B: Test ETI File Contents

| File | Duration | Ensembles | Alerts | Purpose |
|------|----------|-----------|--------|---------|
| EWS1.eti | 4 min | 1 (D001) | Alert 1, Alert 2 | Basic switching |
| EWS2.eti | 4 min | 1 (D001) | L1Start (6 locations) | Location matching |
| EWS3.eti | 4 min | 1 (D001) | 8 stages | Stage progression |
| EWS4.eti | 4 min | 1 (D002) | OE alerts | Other ensemble |
| EWS5.eti | 10 min | 1 (D001) | L1Start (4 times) | Sleep mode selection |
| EWS6.eti | 10 min | 1 (D002) | L1Start | Sleep mode backup |
| EWS7.eti | 10 min | 1 (D001) | 8 stages + long trigger | Sleep mode response |
| EWS8.eti | 10 min | 1 (D001) | Concurrent alerts | Priority handling |
| EWS9.eti | 10 min | 1 (D002) | Concurrent OE alerts | Concurrent OE |

### Appendix C: Reference Documents

- **ETSI TS 104 090 V1.1.2** (2025-02): Main standard
- **ETSI TS 104 089**: EWS Definition and rules of behaviour
- **ETSI EN 300 401** V2.1.1: DAB system specification
- **ETSI TS 103 461**: DAB receiver minimum requirements

### Appendix D: Contact Information

- **ETSI Docbox:** https://docbox.etsi.org/Broadcast/Open/EWSv1_1.zip
- **World DAB:** https://www.worlddab.org/
- **NBTC Thailand:** https://www.nbtc.go.th/

---

**Document Status:** DRAFT
**Review Required:** Yes
**Approval:** Pending

