# Button Fix Test Instructions

**Issue Fixed:** Announcement Settings buttons (Critical Only, All Types, Clear All) were not updating checkboxes.

**Fix Applied:** Added `announcementTypesChanged()` signal and `Connections` component to synchronize checkbox state.

---

## How to Test

### Step 1: Launch GUI
```bash
cd build
./welle-io
```

### Step 2: Open Announcement Settings
1. Click hamburger menu (☰) in top-right
2. Select **"Announcement Settings"**

### Step 3: Test "Critical Only" Button

**Before clicking:**
- Note which checkboxes are currently checked

**Click "Critical Only" button**

**Expected Result:** ✅
- ✓ Alarm (สัญญาณเตือนภัย) - **CHECKED** (Priority 1)
- ✓ Traffic (ข่าวจราจร) - **CHECKED** (Priority 2)
- ✓ Travel (ข่าวการเดินทาง) - **CHECKED** (Priority 3)
- ✓ Warning (คำเตือน) - **CHECKED** (Priority 4)
- ☐ News (ข่าว) - **UNCHECKED** (Priority 5)
- ☐ Weather (พยากรณ์อากาศ) - **UNCHECKED** (Priority 6)
- ☐ Event (ประกาศกิจกรรม) - **UNCHECKED** (Priority 7)
- ☐ Special (ประกาศพิเศษ) - **UNCHECKED** (Priority 8)
- ☐ Programme (ข้อมูลรายการ) - **UNCHECKED** (Priority 9)
- ☐ Sport (กีฬา) - **UNCHECKED** (Priority 10)
- ☐ Finance (การเงิน) - **UNCHECKED** (Priority 11)

**Result:** Only priority 1-4 (critical announcements) should be enabled.

---

### Step 4: Test "All Types" Button

**Click "All Types" button**

**Expected Result:** ✅
- All 11 checkboxes should become **CHECKED**
- ✓ Alarm
- ✓ Traffic
- ✓ Travel
- ✓ Warning
- ✓ News
- ✓ Weather
- ✓ Event
- ✓ Special
- ✓ Programme
- ✓ Sport
- ✓ Finance

**Result:** All announcement types enabled.

---

### Step 5: Test "Clear All" Button

**Click "Clear All" button**

**Expected Result:** ✅
- All 11 checkboxes should become **UNCHECKED**
- ☐ Alarm
- ☐ Traffic
- ☐ Travel
- ☐ Warning
- ☐ News
- ☐ Weather
- ☐ Event
- ☐ Special
- ☐ Programme
- ☐ Sport
- ☐ Finance

**Result:** All announcement types disabled.

---

### Step 6: Test Manual Checkbox Toggle

**Manually check/uncheck a few individual checkboxes**

**Expected Result:** ✅
- Individual checkboxes should toggle normally
- State should persist when clicking buttons again

---

### Step 7: Test Settings Persistence

**After making changes:**
1. Close the settings dialog
2. Close welle-io completely
3. Restart welle-io
4. Open Announcement Settings again

**Expected Result:** ✅
- Settings should be restored to last saved state
- QSettings automatically saves on change

---

## Technical Details

### What Was Fixed

**Before Fix:**
```qml
CheckBox {
    checked: radioController.isAnnouncementTypeEnabled(modelData.type)
    onCheckedChanged: {
        radioController.setAnnouncementTypeEnabled(modelData.type, checked)
    }
}
```
**Problem:** One-way binding - checkbox reads initial state but doesn't listen for backend changes.

**After Fix:**
```qml
CheckBox {
    checked: radioController.isAnnouncementTypeEnabled(modelData.type)

    Connections {
        target: radioController
        function onAnnouncementTypesChanged() {
            typeCheckbox.checked = radioController.isAnnouncementTypeEnabled(modelData.type)
        }
    }

    onCheckedChanged: {
        radioController.setAnnouncementTypeEnabled(modelData.type, checked)
    }
}
```
**Solution:** Two-way binding - checkbox reads initial state AND listens for `announcementTypesChanged()` signal.

### Signal Flow

1. User clicks "Critical Only" button
2. QML function `selectCriticalOnly()` executes:
   ```javascript
   for (var i = 0; i < announcementTypesModel.length; i++) {
       var enabled = announcementTypesModel[i].priority <= 4
       radioController.setAnnouncementTypeEnabled(announcementTypesModel[i].type, enabled)
   }
   ```
3. C++ RadioController updates internal state
4. C++ emits `announcementTypesChanged()` signal
5. QML `Connections.onAnnouncementTypesChanged()` receives signal
6. Each checkbox re-reads state: `checked = radioController.isAnnouncementTypeEnabled(type)`
7. UI updates immediately ✓

---

## Debug Output

When buttons are clicked, you should see in terminal:

```
RadioController: Announcement type 4 enabled
RadioController: Announcement type 5 disabled
RadioController: Announcement type 6 disabled
...
```

This confirms the backend is processing the changes correctly.

---

## Known Behavior

### Button Click Timing
- Changes apply immediately (no "Save" button needed)
- QSettings auto-saves in background
- All checkboxes update simultaneously within ~10ms

### State Consistency
- Backend (C++ AnnouncementManager) and frontend (QML checkboxes) stay in sync
- No race conditions due to Qt signal-slot queuing
- Thread-safe (Qt's event loop handles synchronization)

---

## Troubleshooting

### Issue: Checkboxes don't update when clicking buttons

**Check:**
1. Is the build up to date? (`make -j4`)
2. Is welle-io restarted after rebuild?
3. Check terminal for "RadioController: Announcement type X enabled/disabled" messages

**If still broken:**
- Check if `announcementTypesChanged()` signal is defined in radio_controller.h
- Check if signal is emitted in radio_controller.cpp after state change
- Check if `Connections` component exists in AnnouncementSettings.qml

### Issue: Settings don't persist after restart

**Check:**
- QSettings location: `~/.config/welle.io/welle-io.conf`
- Verify file has `announcement/enabledTypes` entries
- Check file permissions (should be writable)

---

## Success Criteria

✅ **All buttons functional:**
- "Critical Only" enables only priorities 1-4
- "All Types" enables all 11 types
- "Clear All" disables all types

✅ **Immediate UI feedback:**
- Checkboxes update instantly when button clicked
- No need to close/reopen dialog

✅ **State persistence:**
- Settings saved automatically
- Restored correctly on restart

✅ **No errors in console:**
- No QML warnings about missing signals
- No C++ errors about invalid types

---

**Fix Version:** Commit 5b911b7e
**Test Date:** 2025-10-13
**Status:** ✅ READY FOR TESTING
