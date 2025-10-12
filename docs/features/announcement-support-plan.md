# Announcement Support & Switching Implementation Plan
## ETSI EN 300 401 V2.1.1 (2017-01) - FIG 0/18 & FIG 0/19

**Project:** welle.io Thailand DAB+ Receiver  
**Feature:** Announcement Support (ASu) and Announcement Switching (ASw)  
**Standard:** ETSI EN 300 401 V2.1.1 Section 8.1.6  
**Priority:** Medium (Enhancement)  
**Complexity:** High (Multi-component feature)  
**Date:** 2025-10-13

---

## 1. Executive Summary

This document outlines the implementation plan for DAB Announcement Support and Switching functionality in welle.io Thailand receiver. The feature allows automatic switching to announcement subchannels (e.g., emergency alerts, traffic reports, news flashes) when broadcasters activate announcements, then returning to the original programme when the announcement ends.

### Key Features

- ✅ Parse FIG 0/18 (Announcement Support) - which services support announcements
- ✅ Parse FIG 0/19 (Announcement Switching) - active announcements and target subchannel
- ✅ Automatic subchannel switching when announcement activates (ASw flags set)
- ✅ Automatic return to original channel when announcement ends (ASw = 0x0000)
- ✅ Support for 11 announcement types (Alarm, Traffic, Travel, Warning, News, Weather, Event, Special, Programme, Sports, Finance)
- ✅ Priority-based switching (Alarm highest priority)
- ✅ User preferences (allow/block announcement types)
- ✅ Visual indication in GUI (announcement active, type, countdown)

### Current State

**Existing Code (Partial Support):**
- FIG 0/18 parser exists but data is **discarded** (`FIBProcessor::FIG0Extension18()`)
- FIG 0/19 parser exists but data is **discarded** (`FIBProcessor::FIG0Extension19()`)
- No data structures to store announcement information
- No switching logic in radio receiver
- No GUI components for announcement visualization

**What Needs Implementation:**
- Data structures to store announcement support/switching info
- Business logic for automatic subchannel switching
- State machine to manage announcement lifecycle
- GUI components to display announcement status
- User preferences for announcement types
- Integration with RadioController and ProgrammeHandler

---

## 2. Standard Requirements Analysis

### 2.1 FIG 0/18: Announcement Support

**Purpose:** Declares which services support announcements and in which clusters they participate.

**Structure (ETSI EN 300 401 §8.1.6.1):**
```
Byte 0-1:  Service Identifier (SId) - 16 bits
Byte 2-3:  Announcement support flags (ASu) - 16 bits
Byte 4:    Number of clusters (5 bits) + Rfa (3 bits)
Byte 5-n:  Cluster Id list (8 bits each)
```

**ASu Flags (ETSI EN 300 401 Table 14):**
```
Bit  Type        Description                     Priority
15   Rfu         Reserved for future use         -
14   Rfu         Reserved for future use         -
13   Rfu         Reserved for future use         -
12   Rfu         Reserved for future use         -
11   Rfu         Reserved for future use         -
10   Finance     Financial report                11 (Lowest)
9    Sports      Sport report                    10
8    Programme   Programme information           9
7    Special     Special event                   8
6    Event       Event announcement              7
5    Weather     Weather flash                   6
4    News        News flash                      5
3    Warning     Warning                         4
2    Travel      Travel news                     3
1    Traffic     Traffic flash                   2
0    Alarm       Alarm announcement              1 (Highest)
```

**Example:**
```
SId: 0x4001
ASu: 0x0001 (binary: 0000 0000 0000 0001)
     └─ Bit 0 set: Service supports Alarm announcements
NumClusters: 1
ClusterId: 1
```

**Transmission Rate:** Rate B minimum (every 192ms / 8 frames)

### 2.2 FIG 0/19: Announcement Switching

**Purpose:** Signals which announcements are currently active and which subchannel carries the announcement.

**Structure (ETSI EN 300 401 §8.1.6.2):**
```
Byte 0:    Cluster Id (8 bits)
Byte 1-2:  Announcement switching flags (ASw) - 16 bits
Byte 3:
  Bits 7-2: SubChId (6 bits) - Subchannel carrying announcement
  Bit 1:    Region flag (1 bit) - shall be zero
  Bit 0:    New flag (1 bit) - shall be 1
```

**ASw Flags (ETSI EN 300 401 Table 15):**
- Same bit mapping as ASu flags
- **ASw = 0x0000:** No announcement active in this cluster
- **ASw != 0x0000:** Announcement(s) active, bits indicate which types

**Switching Behavior:**
- When ASw changes from 0x0000 to non-zero: **Announcement STARTS** → Switch to SubChId
- When ASw changes from non-zero to 0x0000: **Announcement ENDS** → Return to original service
- Multiple bits can be set (e.g., 0x0003 = Alarm + Traffic)

**Transmission Rate:**
- Normal (stable): Rate B (every 192ms)
- Transition (activation/deactivation): Rate A_B (faster) for 2 seconds

**Example Scenario:**

**Before Alarm (Inactive):**
```
ClusterId: 1
ASw: 0x0000 (no announcement)
SubChId: 18
```
**Receiver action:** Play normal service

**During Alarm (Active):**
```
ClusterId: 1
ASw: 0x0001 (Alarm active)
SubChId: 18
```
**Receiver action:** Switch to subchannel 18, play emergency content

**After Alarm (Inactive):**
```
ClusterId: 1
ASw: 0x0000 (no announcement)
SubChId: 18
```
**Receiver action:** Return to original service

### 2.3 Cluster Concept

**What is a Cluster?**
- A cluster is a group of services that share announcement information
- Services in the same cluster receive the same announcements
- Cluster IDs: 1-254 (0 reserved, 255 special "all services" cluster)

**Example:**
- **Cluster 1:** Bangkok region services (all receive Bangkok traffic announcements)
- **Cluster 2:** Northern region services (all receive Northern traffic announcements)
- Service can belong to multiple clusters

---

## 3. Architecture Design

### 3.1 Data Structures

#### 3.1.1 Announcement Support Information (from FIG 0/18)

**File:** `src/backend/dab-constants.h`

```cpp
// Announcement type enum (ETSI EN 300 401 Table 14)
enum class AnnouncementType : uint8_t {
    Alarm = 0,      // Bit 0 - Highest priority
    Traffic = 1,    // Bit 1
    Travel = 2,     // Bit 2
    Warning = 3,    // Bit 3
    News = 4,       // Bit 4
    Weather = 5,    // Bit 5
    Event = 6,      // Bit 6
    Special = 7,    // Bit 7
    Programme = 8,  // Bit 8
    Sports = 9,     // Bit 9
    Finance = 10    // Bit 10
};

// Announcement support flags (16 bits)
struct AnnouncementSupportFlags {
    uint16_t flags = 0x0000;
    
    bool supports(AnnouncementType type) const {
        return (flags & (1 << static_cast<uint8_t>(type))) != 0;
    }
    
    void set(AnnouncementType type) {
        flags |= (1 << static_cast<uint8_t>(type));
    }
    
    void clear(AnnouncementType type) {
        flags &= ~(1 << static_cast<uint8_t>(type));
    }
    
    std::vector<AnnouncementType> getActiveTypes() const;
};

// Announcement cluster information
struct AnnouncementCluster {
    uint8_t cluster_id = 0;
    std::vector<uint32_t> service_ids;  // Services in this cluster
};

// Service announcement support (from FIG 0/18)
struct ServiceAnnouncementSupport {
    uint32_t service_id = 0;
    AnnouncementSupportFlags support_flags;
    std::vector<uint8_t> cluster_ids;  // Clusters this service belongs to
};
```

#### 3.1.2 Announcement Switching Information (from FIG 0/19)

**File:** `src/backend/dab-constants.h`

```cpp
// Active announcement information (from FIG 0/19)
struct ActiveAnnouncement {
    uint8_t cluster_id = 0;
    AnnouncementSupportFlags active_flags;  // ASw flags
    uint8_t subchannel_id = 0;              // SubChId carrying announcement
    bool new_flag = true;                   // Always 1 per standard
    bool region_flag = false;               // Always 0 (regional not supported)
    
    // Derived information
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_update;
    
    bool isActive() const {
        return active_flags.flags != 0x0000;
    }
    
    AnnouncementType getHighestPriorityType() const;
};
```

#### 3.1.3 Extended Service Structure

**File:** `src/backend/dab-constants.h`

```cpp
// Extend existing Service struct
struct Service {
    // ... existing fields ...
    
    // Announcement support (NEW)
    AnnouncementSupportFlags announcement_support;
    std::vector<uint8_t> announcement_clusters;
};
```

### 3.2 State Machine Design

#### 3.2.1 Announcement State

```cpp
enum class AnnouncementState {
    Idle,                    // No announcement, playing normal service
    AnnouncementDetected,    // ASw != 0x0000 detected
    SwitchingToAnnouncement, // Switching to announcement subchannel
    PlayingAnnouncement,     // Playing announcement content
    AnnouncementEnded,       // ASw = 0x0000 detected
    ReturningToService       // Switching back to original service
};
```

#### 3.2.2 State Transitions

```
┌──────────────────────────────────────────────────────────┐
│                         IDLE                              │
│              (Playing normal service)                     │
└──────────────────┬───────────────────────────────────────┘
                   │
                   │ FIG 0/19: ASw != 0x0000
                   │
                   ▼
┌──────────────────────────────────────────────────────────┐
│              ANNOUNCEMENT_DETECTED                        │
│     (Check priority, user preferences)                    │
└──────────────────┬───────────────────────────────────────┘
                   │
                   │ Decision: Switch
                   │
                   ▼
┌──────────────────────────────────────────────────────────┐
│           SWITCHING_TO_ANNOUNCEMENT                       │
│    (Store current service, tune to SubChId)               │
└──────────────────┬───────────────────────────────────────┘
                   │
                   │ Subchannel tuned
                   │
                   ▼
┌──────────────────────────────────────────────────────────┐
│             PLAYING_ANNOUNCEMENT                          │
│         (Play announcement content)                       │
└──────────────────┬───────────────────────────────────────┘
                   │
                   │ FIG 0/19: ASw = 0x0000
                   │
                   ▼
┌──────────────────────────────────────────────────────────┐
│              ANNOUNCEMENT_ENDED                           │
│            (Prepare to return)                            │
└──────────────────┬───────────────────────────────────────┘
                   │
                   │ Initiate return
                   │
                   ▼
┌──────────────────────────────────────────────────────────┐
│            RETURNING_TO_SERVICE                           │
│      (Restore original service)                           │
└──────────────────┬───────────────────────────────────────┘
                   │
                   │ Service restored
                   │
                   ▼
                  IDLE
```

### 3.3 Component Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      GUI Layer                              │
│  - Announcement indicator widget                            │
│  - Announcement type display                                │
│  - User preferences (enable/disable types)                  │
│  - "Return to service" button (manual override)             │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        │ Qt signals/slots
                        │
┌───────────────────────▼─────────────────────────────────────┐
│                   Radio Controller                          │
│  - AnnouncementManager (NEW)                                │
│  - Receives FIG 0/18/19 data from FIBProcessor              │
│  - Manages announcement state machine                       │
│  - Makes switching decisions                                │
│  - Notifies GUI of state changes                            │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        │ API calls
                        │
┌───────────────────────▼─────────────────────────────────────┐
│                   FIB Processor                             │
│  - FIG0Extension18() - parse announcement support (MODIFY)  │
│  - FIG0Extension19() - parse announcement switching (MODIFY)│
│  - Store announcement data (NEW)                            │
│  - Notify RadioController of updates (NEW)                  │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        │ FIB data
                        │
┌───────────────────────▼─────────────────────────────────────┐
│                  OFDM Processor                             │
│              (No changes needed)                            │
└─────────────────────────────────────────────────────────────┘
```

---

## 4. Implementation Plan

### Phase 1: Backend Foundation (5-7 days)

#### Task 1.1: Data Structure Implementation
**File:** `src/backend/announcement-manager.h` (NEW)  
**File:** `src/backend/announcement-manager.cpp` (NEW)  
**File:** `src/backend/dab-constants.h` (MODIFY)

**Deliverables:**
- [ ] Define `AnnouncementType` enum
- [ ] Implement `AnnouncementSupportFlags` struct
- [ ] Implement `ServiceAnnouncementSupport` struct
- [ ] Implement `ActiveAnnouncement` struct
- [ ] Implement `AnnouncementCluster` struct
- [ ] Extend `Service` struct with announcement fields
- [ ] Add priority mapping for announcement types

**Estimated Effort:** 1 day

#### Task 1.2: FIG 0/18 Parser Enhancement
**File:** `src/backend/fib-processor.cpp` (MODIFY)  
**File:** `src/backend/fib-processor.h` (MODIFY)

**Current Code:**
```cpp
void FIBProcessor::FIG0Extension18(uint8_t *d) {
    // ... parsing code ...
    (void)SId;        // DISCARDED!
    (void)AsuFlags;   // DISCARDED!
}
```

**New Code:**
```cpp
void FIBProcessor::FIG0Extension18(uint8_t *d) {
    // Parse FIG 0/18 data
    int16_t offset = 16;
    int16_t Length = getBits_5(d, 3);
    
    while (offset / 8 < Length - 1) {
        uint16_t SId = getBits(d, offset, 16);
        uint16_t AsuFlags = getBits(d, offset + 16, 16);
        int16_t NumClusters = getBits_5(d, offset + 35);
        
        // Extract cluster IDs
        std::vector<uint8_t> cluster_ids;
        for (int i = 0; i < NumClusters; i++) {
            uint8_t cluster_id = getBits_8(d, offset + 40 + i * 8);
            cluster_ids.push_back(cluster_id);
        }
        
        // Store announcement support info (NEW)
        ServiceAnnouncementSupport support;
        support.service_id = SId;
        support.support_flags.flags = AsuFlags;
        support.cluster_ids = cluster_ids;
        
        storeAnnouncementSupport(support);  // NEW METHOD
        
        offset += 40 + NumClusters * 8;
    }
    
    // Notify RadioController of update (NEW)
    myRadioInterface.onAnnouncementSupportUpdate();
}
```

**Deliverables:**
- [ ] Store announcement support data in FIBProcessor
- [ ] Add `storeAnnouncementSupport()` method
- [ ] Add `getAnnouncementSupport(SId)` method
- [ ] Notify RadioController when FIG 0/18 received
- [ ] Handle multiple clusters per service

**Estimated Effort:** 2 days

#### Task 1.3: FIG 0/19 Parser Enhancement
**File:** `src/backend/fib-processor.cpp` (MODIFY)  
**File:** `src/backend/fib-processor.h` (MODIFY)

**Current Code:**
```cpp
void FIBProcessor::FIG0Extension19(uint8_t *d) {
    // ... parsing code ...
    (void)clusterId;   // DISCARDED!
    (void)aswFlags;    // DISCARDED!
    (void)subChId;     // DISCARDED!
}
```

**New Code:**
```cpp
void FIBProcessor::FIG0Extension19(uint8_t *d) {
    int16_t offset = 16;
    int16_t Length = getBits_5(d, 3);
    
    std::vector<ActiveAnnouncement> announcements;
    
    while (offset / 8 < Length - 1) {
        ActiveAnnouncement ann;
        ann.cluster_id = getBits_8(d, offset);
        ann.new_flag = getBits_1(d, offset + 24);
        ann.region_flag = getBits_1(d, offset + 25);
        ann.subchannel_id = getBits_6(d, offset + 26);
        ann.active_flags.flags = getBits(d, offset + 8, 16);
        ann.last_update = std::chrono::steady_clock::now();
        
        // Detect state change (NEW)
        bool was_active = isAnnouncementActive(ann.cluster_id);
        bool now_active = ann.isActive();
        
        if (!was_active && now_active) {
            // ANNOUNCEMENT STARTED
            ann.start_time = ann.last_update;
        }
        
        announcements.push_back(ann);
        
        if (ann.region_flag) {
            offset += 40;
        } else {
            offset += 32;
        }
    }
    
    // Store and notify (NEW)
    updateActiveAnnouncements(announcements);
    myRadioInterface.onAnnouncementSwitchingUpdate(announcements);
}
```

**Deliverables:**
- [ ] Store active announcement data in FIBProcessor
- [ ] Add `updateActiveAnnouncements()` method
- [ ] Add `getActiveAnnouncements()` method
- [ ] Detect announcement start/end transitions
- [ ] Notify RadioController when FIG 0/19 received
- [ ] Handle regional announcements (if region_flag set)

**Estimated Effort:** 2 days

#### Task 1.4: RadioController Interface Extension
**File:** `src/backend/radio-controller.h` (MODIFY)

**Add to RadioControllerInterface:**
```cpp
/* When announcement support information is updated (FIG 0/18) */
virtual void onAnnouncementSupportUpdate() = 0;

/* When announcement switching information is updated (FIG 0/19)
 * announcements contains all active announcements in the ensemble */
virtual void onAnnouncementSwitchingUpdate(
    const std::vector<ActiveAnnouncement>& announcements) = 0;
```

**Estimated Effort:** 0.5 day

### Phase 2: Announcement Manager (4-6 days)

#### Task 2.1: AnnouncementManager Class
**File:** `src/backend/announcement-manager.h` (NEW)  
**File:** `src/backend/announcement-manager.cpp` (NEW)

**Class Design:**
```cpp
class AnnouncementManager {
public:
    AnnouncementManager(RadioControllerInterface& controller);
    
    // Configuration
    void setUserPreferences(const AnnouncementPreferences& prefs);
    void enableAnnouncementType(AnnouncementType type, bool enable);
    
    // Announcement support (FIG 0/18)
    void updateAnnouncementSupport(const ServiceAnnouncementSupport& support);
    ServiceAnnouncementSupport getServiceSupport(uint32_t service_id) const;
    
    // Announcement switching (FIG 0/19)
    void updateActiveAnnouncements(const std::vector<ActiveAnnouncement>& announcements);
    std::vector<ActiveAnnouncement> getActiveAnnouncements() const;
    
    // Switching logic
    bool shouldSwitchToAnnouncement(const ActiveAnnouncement& ann) const;
    void switchToAnnouncement(const ActiveAnnouncement& ann);
    void returnToOriginalService();
    
    // State management
    AnnouncementState getState() const;
    bool isInAnnouncement() const;
    ActiveAnnouncement getCurrentAnnouncement() const;
    
private:
    // State machine
    AnnouncementState state_;
    
    // Current playback context
    uint32_t original_service_id_;
    uint16_t original_subchannel_id_;
    ActiveAnnouncement current_announcement_;
    
    // Configuration
    AnnouncementPreferences user_prefs_;
    
    // Data storage
    std::unordered_map<uint32_t, ServiceAnnouncementSupport> service_support_;
    std::unordered_map<uint8_t, ActiveAnnouncement> active_announcements_;
    
    // Helper methods
    int getAnnouncementPriority(AnnouncementType type) const;
    bool isAnnouncementTypeEnabled(AnnouncementType type) const;
    void transitionState(AnnouncementState new_state);
};
```

**User Preferences:**
```cpp
struct AnnouncementPreferences {
    // Enable/disable announcement feature entirely
    bool enabled = true;
    
    // Enable/disable specific announcement types
    std::unordered_map<AnnouncementType, bool> type_enabled;
    
    // Priority threshold (only switch for announcements with priority >= threshold)
    // 1 = only Alarm, 2 = Alarm + Traffic, etc.
    int priority_threshold = 1;  // Default: only Alarm
    
    // Allow manual return (user can cancel announcement)
    bool allow_manual_return = true;
    
    // Auto-return timeout (return to service if announcement exceeds this duration)
    std::chrono::seconds max_announcement_duration{300};  // 5 minutes
    
    AnnouncementPreferences() {
        // Default: only enable Alarm (highest priority)
        type_enabled[AnnouncementType::Alarm] = true;
        type_enabled[AnnouncementType::Traffic] = false;
        type_enabled[AnnouncementType::Travel] = false;
        type_enabled[AnnouncementType::Warning] = false;
        type_enabled[AnnouncementType::News] = false;
        type_enabled[AnnouncementType::Weather] = false;
        type_enabled[AnnouncementType::Event] = false;
        type_enabled[AnnouncementType::Special] = false;
        type_enabled[AnnouncementType::Programme] = false;
        type_enabled[AnnouncementType::Sports] = false;
        type_enabled[AnnouncementType::Finance] = false;
    }
};
```

**Deliverables:**
- [ ] Implement AnnouncementManager class
- [ ] Implement state machine with 6 states
- [ ] Implement switching decision logic (priority, user prefs)
- [ ] Implement subchannel switching coordination
- [ ] Implement timeout handling (max announcement duration)
- [ ] Add thread safety (mutex for state access)

**Estimated Effort:** 3 days

#### Task 2.2: Integration with Radio Receiver
**File:** `src/welle-gui/radio_controller.cpp` (MODIFY)  
**File:** `src/welle-gui/radio_controller.h` (MODIFY)

**Add to RadioController class:**
```cpp
private:
    std::unique_ptr<AnnouncementManager> announcementManager_;
    
public:
    // RadioControllerInterface implementation
    void onAnnouncementSupportUpdate() override;
    void onAnnouncementSwitchingUpdate(
        const std::vector<ActiveAnnouncement>& announcements) override;
    
    // Public API for GUI
    AnnouncementManager& getAnnouncementManager();
    bool isInAnnouncement() const;
    ActiveAnnouncement getCurrentAnnouncement() const;
```

**Deliverables:**
- [ ] Integrate AnnouncementManager into RadioController
- [ ] Implement callback handlers for FIG 0/18/19
- [ ] Coordinate subchannel switching with existing tuning logic
- [ ] Emit Qt signals for GUI updates

**Estimated Effort:** 2 days

#### Task 2.3: Unit Tests for Announcement Logic
**File:** `src/tests/announcement_tests.h` (NEW)  
**File:** `src/tests/announcement_tests.cpp` (NEW)

**Test Coverage:**
- [ ] Test FIG 0/18 parsing (valid/invalid data)
- [ ] Test FIG 0/19 parsing (valid/invalid data)
- [ ] Test announcement type priority ordering
- [ ] Test user preference filtering
- [ ] Test state machine transitions
- [ ] Test switching logic (when to switch, when to ignore)
- [ ] Test timeout handling
- [ ] Test multi-cluster scenarios
- [ ] Test concurrent announcements (different clusters)
- [ ] Test edge cases (malformed FIG data, ASw = 0xFFFF, etc.)

**Estimated Effort:** 2 days

### Phase 3: GUI Implementation (5-7 days)

#### Task 3.1: Announcement Indicator Widget
**File:** `src/welle-gui/QML/components/AnnouncementIndicator.qml` (NEW)

**Visual Design:**
```qml
// Announcement banner overlay
Rectangle {
    id: announcementBanner
    visible: radioController.isInAnnouncement
    color: getAnnouncementColor(announcementType)  // Red for Alarm, Orange for Traffic, etc.
    height: 80
    anchors.top: parent.top
    
    Row {
        spacing: 10
        
        // Icon
        Image {
            source: getAnnouncementIcon(announcementType)
            width: 48
            height: 48
        }
        
        // Text
        Column {
            Text {
                text: getAnnouncementTypeName(announcementType)
                font.bold: true
                font.pixelSize: 18
                color: "white"
            }
            Text {
                text: qsTr("Announcement in progress")
                font.pixelSize: 14
                color: "white"
            }
            Text {
                text: qsTr("Duration: ") + formatDuration(announcementDuration)
                font.pixelSize: 12
                color: "white"
            }
        }
        
        // Return button
        Button {
            text: qsTr("Return to Service")
            visible: announcementManager.allowManualReturn
            onClicked: announcementManager.returnToOriginalService()
        }
    }
}
```

**Color Coding (ETSI recommendation):**
- Alarm: Red (#FF0000)
- Warning: Orange (#FFA500)
- Traffic: Yellow (#FFFF00)
- News: Blue (#0000FF)
- Weather: Light Blue (#87CEEB)
- Other: Gray (#808080)

**Deliverables:**
- [ ] Create AnnouncementIndicator.qml component
- [ ] Add announcement type icons (11 types)
- [ ] Add color coding per announcement type
- [ ] Add duration counter
- [ ] Add "Return to Service" button
- [ ] Add animations (fade in/out)

**Estimated Effort:** 2 days

#### Task 3.2: Announcement Preferences Dialog
**File:** `src/welle-gui/QML/settingpages/AnnouncementSettings.qml` (NEW)

**UI Elements:**
```qml
GroupBox {
    title: qsTr("Announcement Settings")
    
    Column {
        spacing: 10
        
        // Master enable/disable
        CheckBox {
            text: qsTr("Enable automatic announcement switching")
            checked: announcementManager.enabled
            onCheckedChanged: announcementManager.setEnabled(checked)
        }
        
        // Individual announcement types
        Label {
            text: qsTr("Announcement Types:")
            font.bold: true
        }
        
        Repeater {
            model: announcementTypes  // [Alarm, Traffic, Travel, ...]
            
            CheckBox {
                text: modelData.name + " (Priority: " + modelData.priority + ")"
                checked: announcementManager.isTypeEnabled(modelData.type)
                enabled: announcementManager.enabled
                onCheckedChanged: announcementManager.setTypeEnabled(modelData.type, checked)
            }
        }
        
        // Priority threshold slider
        Label {
            text: qsTr("Priority Threshold: ") + prioritySlider.value
        }
        Slider {
            id: prioritySlider
            from: 1
            to: 11
            stepSize: 1
            value: announcementManager.priorityThreshold
            onValueChanged: announcementManager.setPriorityThreshold(value)
        }
        
        // Max duration
        SpinBox {
            from: 30
            to: 600
            stepSize: 30
            value: announcementManager.maxAnnouncementDuration
            textFromValue: function(value) { return value + " seconds" }
            onValueModified: announcementManager.setMaxDuration(value)
        }
    }
}
```

**Deliverables:**
- [ ] Create AnnouncementSettings.qml page
- [ ] Add master enable/disable toggle
- [ ] Add checkboxes for 11 announcement types
- [ ] Add priority threshold slider
- [ ] Add max duration spinner
- [ ] Add "Allow manual return" checkbox
- [ ] Save preferences to QSettings

**Estimated Effort:** 2 days

#### Task 3.3: Announcement Log/History
**File:** `src/welle-gui/QML/components/AnnouncementHistory.qml` (NEW)

**Features:**
- Display list of recent announcements (last 50)
- Show type, start time, duration, cluster ID
- Allow filtering by type
- Export to CSV

**Deliverables:**
- [ ] Create AnnouncementHistory.qml component
- [ ] Add ListView with announcement records
- [ ] Add filtering controls
- [ ] Add export button
- [ ] Store history in AnnouncementManager (circular buffer)

**Estimated Effort:** 1 day

#### Task 3.4: Thai Language Translations
**File:** `src/welle-gui/i18n/th_TH.ts` (MODIFY)

**Add Translations:**
```xml
<!-- Announcement types in Thai -->
<translation>
    <source>Alarm</source>
    <translation>สัญญาณเตือนภัย</translation>
</translation>
<translation>
    <source>Traffic</source>
    <translation>ข่าวจราจร</translation>
</translation>
<translation>
    <source>Travel</source>
    <translation>ข่าวการเดินทาง</translation>
</translation>
<!-- ... 8 more types ... -->

<!-- UI strings -->
<translation>
    <source>Announcement in progress</source>
    <translation>กำลังเล่นประกาศ</translation>
</translation>
<translation>
    <source>Return to Service</source>
    <translation>กลับไปยังรายการปกติ</translation>
</translation>
```

**Deliverables:**
- [ ] Add Thai translations for 11 announcement types
- [ ] Add Thai translations for UI strings
- [ ] Add Thai translations for settings dialog

**Estimated Effort:** 1 day

### Phase 4: Testing & Documentation (3-4 days)

#### Task 4.1: Integration Testing
**File:** `src/tests/announcement_integration_tests.cpp` (NEW)

**Test Scenarios:**
- [ ] End-to-end: FIG 0/18 → FIG 0/19 → switch → return
- [ ] User cancels announcement manually
- [ ] Announcement timeout (max duration exceeded)
- [ ] Multiple announcements in different clusters
- [ ] Priority-based switching (higher priority preempts lower)
- [ ] User has disabled announcement type
- [ ] Service does not support announcement type
- [ ] Announcement on same subchannel as current service (no switch needed)

**Estimated Effort:** 2 days

#### Task 4.2: Field Testing with ODR-DabMux
**Setup:**
1. Configure ODR-DabMux with announcement clusters (use Announcement.md config)
2. Activate/deactivate announcements via REST API
3. Verify welle.io switches correctly
4. Test with real Thailand DAB+ frequencies (12B, 12C, 12D)

**Test Matrix:**
| Announcement Type | Service | Cluster | SubCh | Expected Behavior |
|-------------------|---------|---------|-------|-------------------|
| Alarm (0x0001) | All | 1 | 18 | Switch immediately |
| Traffic (0x0002) | All | 1 | 18 | Switch if enabled |
| Multiple (0x0003) | All | 1 | 18 | Show highest priority |

**Deliverables:**
- [ ] Test with ODR-DabMux setup
- [ ] Verify timing (switch delay <500ms)
- [ ] Verify return to service
- [ ] Measure audio glitches during switch
- [ ] Test with Thailand character sets (Thai labels in announcements)

**Estimated Effort:** 1 day

#### Task 4.3: Documentation
**File:** `docs/features/announcement-support.md` (NEW)

**Content:**
- Feature overview
- User guide (how to enable/configure)
- Developer guide (API reference)
- Standard compliance details (ETSI EN 300 401 sections)
- Troubleshooting (common issues)
- Thailand-specific considerations

**File:** `README.md` (MODIFY)
- Add "Announcement Support" to feature list

**Estimated Effort:** 1 day

---

## 5. Implementation Roadmap

### Timeline (Total: 17-24 days)

```
Week 1: Backend Foundation
├─ Day 1: Data structures
├─ Day 2-3: FIG 0/18 parser
└─ Day 4-5: FIG 0/19 parser

Week 2: Announcement Manager
├─ Day 6-8: AnnouncementManager class
├─ Day 9-10: RadioController integration
└─ Day 11-12: Unit tests

Week 3: GUI Implementation
├─ Day 13-14: Announcement indicator widget
├─ Day 15-16: Preferences dialog
└─ Day 17: Announcement history

Week 4: Testing & Polish
├─ Day 18-19: Integration testing
├─ Day 20: Field testing
├─ Day 21: Documentation
└─ Day 22-24: Bug fixes and polish
```

### Milestones

**M1 (Day 5):** FIG 0/18 and FIG 0/19 data captured and stored  
**M2 (Day 12):** Announcement switching logic functional (backend only)  
**M3 (Day 17):** GUI components complete  
**M4 (Day 21):** All tests passing  
**M5 (Day 24):** Feature ready for release

---

## 6. Risk Assessment

### Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Subchannel switching causes audio glitches | High | Medium | Buffer management, crossfade |
| FIG 0/19 updates too slow (>500ms delay) | Medium | Medium | Optimize FIB processing |
| State machine race conditions | Medium | High | Comprehensive mutex locking |
| Multiple announcements conflict | Low | High | Priority-based arbitration |
| ODR-DabMux compatibility issues | Low | Medium | Test with multiple mux versions |

### User Experience Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| User annoyed by frequent announcements | High | High | Strict default preferences (Alarm only) |
| User confused by automatic switching | Medium | Medium | Clear visual indication |
| Announcement doesn't end (stuck) | Low | High | Timeout mechanism (max duration) |
| Thai labels not displayed correctly | Low | Medium | Test with ThaiServiceParser |

---

## 7. Success Criteria

### Functional Requirements

- ✅ Parse FIG 0/18 and extract announcement support info
- ✅ Parse FIG 0/19 and extract active announcement info
- ✅ Automatically switch to announcement subchannel when ASw != 0x0000
- ✅ Automatically return to original service when ASw = 0x0000
- ✅ Respect user preferences (disabled types are ignored)
- ✅ Handle multiple announcements (priority-based)
- ✅ Provide visual indication of announcement state
- ✅ Allow manual return to service
- ✅ Support all 11 announcement types
- ✅ Support Thai language UI

### Performance Requirements

- Switch latency: <500ms (from ASw detection to audio output)
- Return latency: <500ms (from ASw=0 detection to original service)
- Audio glitches: <50ms dropout during switch
- FIG processing overhead: <5% CPU increase
- Memory footprint: <2MB additional

### Quality Requirements

- Zero crashes during announcement switching
- Zero audio buffer underruns
- 100% ETSI EN 300 401 compliance
- 100% test coverage for state machine
- Clear user documentation (English + Thai)

---

## 8. Future Enhancements (Post-MVP)

### Phase 2 Features (Nice-to-have)

1. **Smart Announcement History Analytics**
   - Track announcement frequency per service
   - Show "Most interrupted service" statistics
   - Predict announcement patterns

2. **Regional Announcement Support**
   - Parse region_flag in FIG 0/19
   - Filter announcements by user's location
   - Requires GPS integration

3. **Advanced Audio Handling**
   - Crossfade between service and announcement (smooth transition)
   - Volume ducking (reduce music volume, play announcement over it)
   - Record announcements to file

4. **Network Integration**
   - Forward announcements to mobile app via WiFi
   - Push notifications for Alarm announcements
   - Cloud backup of announcement history

5. **AI-Powered Features**
   - Transcribe announcement audio to text (Thai speech recognition)
   - Summarize announcement content
   - Translate Thai announcements to English

---

## 9. References

### Standards

1. **ETSI EN 300 401 V2.1.1 (2017-01)**
   - Section 5.2: Fast Information Channel
   - Section 8.1.6.1: FIG 0/18 (Announcement support)
   - Section 8.1.6.2: FIG 0/19 (Announcement switching)
   - Table 14: Announcement support flags (ASu)
   - Table 15: Announcement switching flags (ASw)

2. **ETSI TS 101 756**
   - Registered tables (announcement types)

### Implementation Examples

1. **ODR-DabMux** (Transmitter side)
   - `src/fig/FIG0_18.cpp` - FIG 0/18 generation
   - `src/fig/FIG0_19.cpp` - FIG 0/19 generation
   - `src/MuxElements.cpp` - AnnouncementCluster class

2. **welle.io Current Code**
   - `src/backend/fib-processor.cpp` - FIG parsers (baseline)
   - `src/backend/dab-constants.h` - Data structures

3. **Other DAB Receivers**
   - SDR-J DAB+ (reference implementation)
   - dablin (Linux command-line DAB player)

### Related Documentation

- `Announcement.md` - ODR-DabMux compliance verification
- NBTC Thailand DAB+ regulations (emergency alert requirements)
- Thailand Civil Defense Act (alarm announcement procedures)

---

## 10. Appendix: Code Examples

### Example 1: Announcement Type to String

```cpp
// File: src/backend/announcement-manager.cpp

const char* AnnouncementManager::getAnnouncementTypeName(AnnouncementType type) {
    switch (type) {
        case AnnouncementType::Alarm:      return "Alarm";
        case AnnouncementType::Traffic:    return "Traffic";
        case AnnouncementType::Travel:     return "Travel";
        case AnnouncementType::Warning:    return "Warning";
        case AnnouncementType::News:       return "News";
        case AnnouncementType::Weather:    return "Weather";
        case AnnouncementType::Event:      return "Event";
        case AnnouncementType::Special:    return "Special";
        case AnnouncementType::Programme:  return "Programme";
        case AnnouncementType::Sports:     return "Sports";
        case AnnouncementType::Finance:    return "Finance";
        default:                           return "Unknown";
    }
}

const char* AnnouncementManager::getAnnouncementTypeNameThai(AnnouncementType type) {
    switch (type) {
        case AnnouncementType::Alarm:      return "สัญญาณเตือนภัย";
        case AnnouncementType::Traffic:    return "ข่าวจราจร";
        case AnnouncementType::Travel:     return "ข่าวการเดินทาง";
        case AnnouncementType::Warning:    return "คำเตือน";
        case AnnouncementType::News:       return "ข่าว";
        case AnnouncementType::Weather:    return "พยากรณ์อากาศ";
        case AnnouncementType::Event:      return "ประกาศกิจกรรม";
        case AnnouncementType::Special:    return "ประกาศพิเศษ";
        case AnnouncementType::Programme:  return "ข้อมูลรายการ";
        case AnnouncementType::Sports:     return "ข่าวกีฬา";
        case AnnouncementType::Finance:    return "ข่าวการเงิน";
        default:                           return "ไม่ทราบประเภท";
    }
}
```

### Example 2: Switching Decision Logic

```cpp
// File: src/backend/announcement-manager.cpp

bool AnnouncementManager::shouldSwitchToAnnouncement(const ActiveAnnouncement& ann) const {
    // Check 1: Feature enabled?
    if (!user_prefs_.enabled) {
        return false;
    }
    
    // Check 2: Already in announcement?
    if (state_ == AnnouncementState::PlayingAnnouncement) {
        // Only switch if new announcement has higher priority
        auto current_priority = getAnnouncementPriority(
            current_announcement_.getHighestPriorityType());
        auto new_priority = getAnnouncementPriority(
            ann.getHighestPriorityType());
        return new_priority < current_priority;  // Lower number = higher priority
    }
    
    // Check 3: Announcement type enabled?
    AnnouncementType type = ann.getHighestPriorityType();
    if (!isAnnouncementTypeEnabled(type)) {
        return false;
    }
    
    // Check 4: Priority threshold?
    int priority = getAnnouncementPriority(type);
    if (priority > user_prefs_.priority_threshold) {
        return false;  // Priority too low
    }
    
    // Check 5: Service supports this announcement type?
    auto support = getServiceSupport(original_service_id_);
    if (!support.support_flags.supports(type)) {
        return false;  // Service doesn't support this announcement
    }
    
    // All checks passed - switch!
    return true;
}
```

### Example 3: State Machine Transition

```cpp
// File: src/backend/announcement-manager.cpp

void AnnouncementManager::switchToAnnouncement(const ActiveAnnouncement& ann) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Save current state
    original_service_id_ = radioController_.getCurrentServiceId();
    original_subchannel_id_ = radioController_.getCurrentSubchannelId();
    
    // Update state
    transitionState(AnnouncementState::SwitchingToAnnouncement);
    current_announcement_ = ann;
    
    // Perform switch
    radioController_.tuneToSubchannel(ann.subchannel_id);
    
    // State will transition to PlayingAnnouncement once subchannel is tuned
    transitionState(AnnouncementState::PlayingAnnouncement);
    
    // Notify GUI
    emit announcementStarted(ann);
    
    // Start timeout timer
    timeout_timer_.start(user_prefs_.max_announcement_duration);
}

void AnnouncementManager::returnToOriginalService() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ != AnnouncementState::PlayingAnnouncement) {
        return;  // Not in announcement
    }
    
    // Update state
    transitionState(AnnouncementState::ReturningToService);
    
    // Perform return
    radioController_.tuneToService(original_service_id_);
    
    // State will transition to Idle once service is tuned
    transitionState(AnnouncementState::Idle);
    
    // Notify GUI
    emit announcementEnded(current_announcement_);
    
    // Clear current announcement
    current_announcement_ = ActiveAnnouncement();
}
```

---

**Document Version:** 1.0  
**Last Updated:** 2025-10-13  
**Author:** Implementation Planning Team  
**Status:** DRAFT - Ready for Review
