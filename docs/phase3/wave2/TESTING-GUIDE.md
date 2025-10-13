# GUI Fixes Testing Guide

**Version:** Phase 3 Wave 2B
**Date:** 2025-10-13
**Target:** QA Testers, Developers, Maintainers

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Test Environment Setup](#test-environment-setup)
- [Test Cases](#test-cases)
  - [TC-001: Announcement Settings Access](#tc-001-announcement-settings-access)
  - [TC-002: Announcement History Access](#tc-002-announcement-history-access)
  - [TC-003: Quick Selection Buttons](#tc-003-quick-selection-buttons)
  - [TC-004: Individual Announcement Type Toggle](#tc-004-individual-announcement-type-toggle)
  - [TC-005: Manual Channel Dropdown Visibility](#tc-005-manual-channel-dropdown-visibility)
  - [TC-006: Manual Channel Dropdown Width](#tc-006-manual-channel-dropdown-width)
  - [TC-007: Widget Loading from Add Menu](#tc-007-widget-loading-from-add-menu)
  - [TC-008: Widget Persistence](#tc-008-widget-persistence)
  - [TC-009: Console Error Checking](#tc-009-console-error-checking)
  - [TC-010: Memory Leak Testing](#tc-010-memory-leak-testing)
- [Regression Testing](#regression-testing)
- [Performance Testing](#performance-testing)
- [Compatibility Testing](#compatibility-testing)
- [Test Reporting](#test-reporting)

---

## Prerequisites

### Required Software
- welle.io built from commit `6dc0ec26` or later
- Qt6 (6.2 or higher) with QML modules
- Linux, Windows, or macOS operating system

### Optional Software
- Qt6 Charts module (`qml6-module-qtcharts` on Ubuntu)
- SDR hardware (RTL-SDR, Airspy, etc.) for full functionality testing
- DAB/DAB+ signal source (live broadcast or recorded I/Q file)

### Test Environment
- Clean user profile (to test first-run experience)
- Existing user profile (to test upgrade path)
- Console access for debug output verification

---

## Test Environment Setup

### 1. Build Latest Version

```bash
cd /home/seksan/workspace/welle.io
git pull
git log -1 --oneline  # Verify at least commit 6dc0ec26
./build-script.sh build
```

### 2. Prepare Test Environment

**For clean install testing:**
```bash
# Backup existing settings
mv ~/.config/welle.io ~/.config/welle.io.backup

# Start fresh
./build/src/welle-io/welle-io
```

**For upgrade testing:**
```bash
# Keep existing settings
./build/src/welle-io/welle-io
```

### 3. Enable Console Debugging

**Linux/macOS:**
```bash
# Run from terminal to see console output
./build/src/welle-io/welle-io 2>&1 | tee test-output.log
```

**Windows:**
```cmd
# Run from Command Prompt
welle-io.exe 2>&1 | tee test-output.log
```

---

## Test Cases

### TC-001: Announcement Settings Access

**Objective:** Verify announcement settings dialog is accessible and opens correctly.

**Priority:** P1 (Critical)

**Prerequisites:**
- Application running
- Main window visible

**Test Steps:**

1. Locate the menu button (☰) in the application toolbar
2. Click the menu button
3. Observe the menu items list
4. Locate "Announcement Settings" menu item
5. Click "Announcement Settings"
6. Observe the dialog that appears

**Expected Results:**

- ✅ Menu button is visible and clickable
- ✅ Menu opens showing list of items
- ✅ "Announcement Settings" item is present in menu
- ✅ Dialog opens when menu item clicked
- ✅ Dialog shows "Announcement Settings" title
- ✅ Dialog contains announcement type list with checkboxes
- ✅ Dialog contains "Critical Only", "All Types", "Clear All" buttons
- ✅ Dialog contains "Close" or "OK" button

**Pass Criteria:**
All expected results must be achieved.

**Failure Symptoms:**
- Menu item missing
- Menu item present but grayed out
- Menu item clickable but dialog doesn't open
- Dialog opens but shows error message
- Console shows "Component could not be loaded" error

**Test Evidence:**
Screenshot of:
1. Menu with "Announcement Settings" item visible
2. Opened Announcement Settings dialog

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-002: Announcement History Access

**Objective:** Verify announcement history view is accessible and displays correctly.

**Priority:** P1 (Critical)

**Prerequisites:**
- Application running
- Main window visible

**Test Steps:**

1. Click the menu button (☰)
2. Locate "Announcement History" menu item
3. Click "Announcement History"
4. Observe the view that appears
5. Check for history entries (may be empty on first run)
6. Close the history view

**Expected Results:**

- ✅ "Announcement History" menu item present
- ✅ History view opens when clicked
- ✅ View shows "Announcement History" title
- ✅ View has list area for history entries
- ✅ If no history: Shows "No announcements yet" or similar message
- ✅ If history exists: Shows timestamp and announcement details
- ✅ View has close button

**Pass Criteria:**
All expected results must be achieved.

**Failure Symptoms:**
- Menu item missing
- History view doesn't open
- History view opens but crashes app
- Console shows errors

**Test Evidence:**
Screenshot of:
1. Menu with "Announcement History" item visible
2. Opened Announcement History view (empty or with entries)

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-003: Quick Selection Buttons

**Objective:** Verify "Critical Only", "All Types", and "Clear All" buttons work correctly.

**Priority:** P0 (Critical)

**Prerequisites:**
- Announcement Settings dialog open
- Console output visible

**Test Steps:**

**Part 1: Clear All Button**
1. Click "Clear All" button
2. Observe all checkboxes
3. Check console for signals

**Part 2: All Types Button**
4. Click "All Types" button
5. Observe all checkboxes
6. Check console for signals

**Part 3: Critical Only Button**
7. Click "Critical Only" button
8. Observe checkboxes
9. Verify only critical types checked

**Part 4: State Persistence**
10. Close dialog
11. Reopen Announcement Settings
12. Verify checkbox states match previous selection

**Expected Results:**

**After "Clear All":**
- ✅ All 11 checkboxes become unchecked
- ✅ Change happens immediately (no delay)
- ✅ Console shows: `announcementTypesChanged()` signal emitted

**After "All Types":**
- ✅ All 11 checkboxes become checked
- ✅ Change happens immediately
- ✅ Console shows: `announcementTypesChanged()` signal emitted

**After "Critical Only":**
- ✅ Alarm (1) checkbox: CHECKED
- ✅ Road Traffic (2) checkbox: CHECKED
- ✅ Transport Flash (3) checkbox: CHECKED
- ✅ Warning/Service (4) checkbox: CHECKED
- ✅ News Flash (5) checkbox: UNCHECKED
- ✅ Area Weather (6) checkbox: UNCHECKED
- ✅ Event Announcement (7) checkbox: UNCHECKED
- ✅ Special Event (8) checkbox: UNCHECKED
- ✅ Programme Info (9) checkbox: UNCHECKED
- ✅ Sport Report (10) checkbox: UNCHECKED
- ✅ Financial Report (11) checkbox: UNCHECKED

**After Reopen:**
- ✅ Checkbox states persist (match last selection)

**Pass Criteria:**
All three buttons must work correctly and states must persist.

**Failure Symptoms:**
- Buttons do nothing when clicked
- Checkboxes don't update
- Wrong checkboxes get checked
- State doesn't persist after closing dialog
- Console shows no signals

**Test Evidence:**
Screenshots of:
1. After "Clear All" clicked (all unchecked)
2. After "All Types" clicked (all checked)
3. After "Critical Only" clicked (4 checked, 7 unchecked)
4. Console output showing signals

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-004: Individual Announcement Type Toggle

**Objective:** Verify individual checkboxes can be toggled and state persists.

**Priority:** P1 (High)

**Prerequisites:**
- Announcement Settings dialog open

**Test Steps:**

1. Click "Clear All" to reset state
2. Manually check "News Flash" (type 5) checkbox
3. Observe checkbox becomes checked
4. Manually uncheck "News Flash"
5. Observe checkbox becomes unchecked
6. Check multiple types: Alarm (1), Weather (6), Sport (10)
7. Close dialog
8. Reopen Announcement Settings
9. Verify the 3 types are still checked

**Expected Results:**

- ✅ Individual checkboxes respond to clicks
- ✅ Checking a checkbox enables that type
- ✅ Unchecking a checkbox disables that type
- ✅ Multiple selections work simultaneously
- ✅ State persists after close/reopen
- ✅ Console shows `announcementTypesChanged()` signal for each change

**Pass Criteria:**
Manual checkbox toggling works and state persists.

**Failure Symptoms:**
- Checkbox doesn't respond to click
- Wrong checkbox gets toggled
- State doesn't persist

**Test Evidence:**
Screenshot showing 3 selected types persisting after reopen.

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-005: Manual Channel Dropdown Visibility

**Objective:** Verify manual channel dropdown shows visible text.

**Priority:** P1 (High)

**Prerequisites:**
- Application running
- Expert Mode enabled in settings

**Test Steps:**

1. Open Settings (if not already in Expert Mode)
2. Enable "Expert Mode" option
3. Close Settings
4. Locate "Manual channel" dropdown near top of window
5. Observe the currently selected channel text
6. Click dropdown to open channel list
7. Observe text in dropdown list items
8. Verify text color contrasts with background

**Expected Results:**

- ✅ Manual channel dropdown visible in Expert Mode
- ✅ Current selection text is readable (not blank)
- ✅ Text has good contrast with background (not invisible)
- ✅ Dropdown list items show channel names clearly
- ✅ Text color: Visible color (not white-on-white or black-on-black)
- ✅ Font size: Readable size (not microscopic)

**Pass Criteria:**
Channel names must be clearly visible with good contrast.

**Failure Symptoms:**
- Dropdown shows blank text
- Text is same color as background (invisible)
- Text is barely visible (poor contrast)
- Only partial text visible

**Test Evidence:**
Screenshot showing:
1. Closed dropdown with visible selected channel
2. Open dropdown with visible list items

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-006: Manual Channel Dropdown Width

**Objective:** Verify dropdown is wide enough to show full channel names.

**Priority:** P2 (Medium)

**Prerequisites:**
- Expert Mode enabled
- Manual channel dropdown visible

**Test Steps:**

Test each channel selection:

1. Select channel "5A"
   - Verify full text "5A" visible (not "5")

2. Select channel "7C"
   - Verify full text "7C" visible (not "7")

3. Select channel "10A"
   - Verify full text "10A" visible (not "10")

4. Select channel "11B"
   - Verify full text "11B" visible (not "11")

5. Select channel "12B"
   - Verify full text "12B" visible (not "12")

6. Select channel "12C"
   - Verify full text "12C" visible (not "12")

7. Select channel "13E"
   - Verify full text "13E" visible (not "13")

8. Verify indicator arrow (▼) is visible
9. Verify no text overlap with arrow

**Expected Results:**

For all channels:
- ✅ Full channel name visible
- ✅ No truncation (letter suffix shown)
- ✅ Text doesn't overlap indicator arrow
- ✅ Visual spacing looks professional
- ✅ Width accommodates longest channel name

**Pass Criteria:**
All channel names display fully without truncation.

**Failure Symptoms:**
- Channel "12B" shows as "12"
- Letter suffix cut off
- Text overlaps arrow
- Dropdown too narrow

**Test Evidence:**
Screenshots of:
1. Short channel "5A" selected
2. Long channel "12C" selected
3. Measuring width with ruler/grid overlay

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-007: Widget Loading from Add Menu

**Objective:** Verify all widgets can be loaded from Add (+) menu.

**Priority:** P0 (Critical)

**Prerequisites:**
- Application running
- Main window visible
- Console output visible
- QtCharts module installed (for some widgets)

**Test Steps:**

For each widget in Add menu:

**Widget 1: Service Overview**
1. Click Add (+) button
2. Select "Service Overview"
3. Verify widget appears in grid

**Widget 2: Service Details**
4. Click Add (+) button
5. Select "Service Details"
6. Verify widget appears in grid

**Widget 3: MOT Slide Show**
7. Click Add (+) button
8. Select "MOT Slide Show"
9. Verify widget appears in grid
10. Check console for errors

**Widget 4: Spectrum Graph**
11. Click Add (+) button
12. Select "Spectrum"
13. Verify widget appears (or QtCharts error)
14. Check console output

**Widget 5: Null Symbol Graph**
15. Click Add (+) button
16. Select "Null Symbol"
17. Verify widget appears in grid
18. Check console for errors

**Widget 6: Constellation Graph**
19. Click Add (+) button
20. Select "Constellation"
21. Verify widget appears (or QtCharts error)

**Widget 7: Impulse Response Graph**
22. Click Add (+) button
23. Select "Impulse Response"
24. Verify widget appears (or QtCharts error)

**Widget 8: Raw Recorder**
25. Click Add (+) button
26. Select "Raw Recorder"
27. Verify widget appears in grid

**Widget 9: Text Output**
28. Click Add (+) button
29. Select "Text Output"
30. Verify widget appears in grid

**Expected Results:**

For widgets WITHOUT QtCharts dependency:
- ✅ Widget appears in grid immediately
- ✅ Widget has proper title/header
- ✅ Widget displays content (may be empty without DAB signal)
- ✅ No console errors
- ✅ Widget is interactive (can be resized/moved)

For widgets WITH QtCharts dependency:
- ✅ IF QtCharts installed: Widget appears normally
- ✅ IF QtCharts NOT installed: Error message "module QtCharts is not installed"
- ✅ Other widgets still work (error doesn't break app)

**Console Output:**
```
Debug: === addComponent called with path: qrc:/QML/expertviews/SpectrumGraph.qml row: -1 column: -1
Debug: === Creating component at row: 1 column: 0
Debug: === Component ready, creating object...
Debug: === Object created successfully, type: SpectrumGraph_QMLTYPE_123
Debug: === Component added successfully, new children count: 3
```

**No "Settings is not a type" errors!**

**Pass Criteria:**
All widgets without QtCharts dependency must load successfully.
Widgets with QtCharts dependency may fail if module not installed (acceptable).

**Failure Symptoms:**
- Widget doesn't appear after selection
- Console shows "Settings is not a type"
- Console shows "Component could not be loaded"
- Application crashes when selecting widget

**Test Evidence:**
Screenshots of:
1. Add menu with all widget options
2. Grid with multiple widgets loaded
3. Console output showing successful component creation
4. Console showing NO "Settings is not a type" errors

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-008: Widget Persistence

**Objective:** Verify widget layout persists across application restarts.

**Priority:** P2 (Medium)

**Prerequisites:**
- Application running
- At least 3 widgets loaded

**Test Steps:**

1. Load 3 widgets: MOT Slide Show, Spectrum, Text Output
2. Verify all 3 visible in grid
3. Note their positions (row, column)
4. Close application completely
5. Restart application
6. Observe grid layout

**Expected Results:**

- ✅ All 3 widgets reload automatically
- ✅ Widget positions match previous layout
- ✅ Widget settings preserved (if any)
- ✅ No "lost widgets" requiring re-add
- ✅ Grid arrangement identical to before restart

**Pass Criteria:**
Layout persists exactly across restart.

**Failure Symptoms:**
- Widgets disappear after restart
- Widgets reset to default layout
- Some widgets missing

**Test Evidence:**
Screenshots of:
1. Before restart (with 3 widgets)
2. After restart (same 3 widgets in same positions)

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-009: Console Error Checking

**Objective:** Verify no critical errors or warnings in console output.

**Priority:** P1 (High)

**Prerequisites:**
- Application running with console output visible

**Test Steps:**

1. Start application from terminal
2. Perform normal operations for 5 minutes:
   - Open menus
   - Load widgets
   - Change settings
   - Switch channels
3. Review console output
4. Categorize any errors/warnings

**Expected Results:**

**Must NOT appear:**
- ❌ "Settings is not a type"
- ❌ "Component could not be loaded"
- ❌ "Cannot read property 'implicitHeight' of null" (if fix applied)
- ❌ Any QML engine errors about missing types
- ❌ Segmentation faults
- ❌ Uncaught exceptions

**Acceptable warnings:**
- ⚠️ "module QtCharts is not installed" (if QtCharts not installed)
- ⚠️ DAB signal quality warnings
- ⚠️ Hardware device warnings (if no SDR connected)

**Pass Criteria:**
No critical errors related to GUI components.

**Failure Symptoms:**
- Repeated "Settings is not a type" errors
- QML engine errors on every widget load
- Loader implicitHeight warnings every second

**Test Evidence:**
Console log file showing:
1. Application startup output
2. Widget loading operations
3. No critical GUI errors

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

### TC-010: Memory Leak Testing

**Objective:** Verify no memory leaks when loading/unloading widgets.

**Priority:** P2 (Medium)

**Prerequisites:**
- Linux system with `top` or `htop` command
- Or Windows with Task Manager
- Application running

**Test Steps:**

1. Start application
2. Note initial memory usage (RSS)
3. Load all 9 widgets
4. Note memory usage with widgets loaded
5. Remove all widgets (use - remove button)
6. Wait 10 seconds
7. Note memory usage after removal
8. Repeat steps 3-7 ten times
9. Compare memory usage over iterations

**Expected Results:**

- ✅ Initial memory: ~25-30 MB
- ✅ With 9 widgets: ~40-50 MB
- ✅ After removal: Returns close to initial (~30-35 MB)
- ✅ After 10 iterations: Memory doesn't grow significantly (< 10 MB increase)
- ✅ No "memory balloon" effect

**Pass Criteria:**
Memory usage remains stable after multiple load/unload cycles.

**Failure Symptoms:**
- Memory keeps growing with each cycle
- Memory never returns after widget removal
- Application slows down over time
- System reports memory leak warnings

**Test Evidence:**
Table of memory measurements:

| Iteration | Initial | With Widgets | After Removal |
|-----------|---------|--------------|---------------|
| 1         | 28 MB   | 45 MB        | 30 MB         |
| 5         | 30 MB   | 46 MB        | 31 MB         |
| 10        | 31 MB   | 47 MB        | 32 MB         |

**Status:** [ ] PASS [ ] FAIL [ ] BLOCKED

**Notes:**
_______________________________________________________

---

## Regression Testing

### Ensure Existing Features Still Work

After GUI fixes, verify core functionality not affected:

#### RT-001: Station Scanning
- [ ] Scan button works
- [ ] Stations discovered
- [ ] Service list populates

#### RT-002: Station Playback
- [ ] Selecting station starts audio
- [ ] Audio quality good
- [ ] Station switching works

#### RT-003: Device Selection
- [ ] RTL-SDR detected (if connected)
- [ ] Device dropdown works
- [ ] Device switching works

#### RT-004: Settings Persistence
- [ ] Volume setting saves
- [ ] Theme setting saves
- [ ] Device settings save

#### RT-005: Expert Mode
- [ ] Expert mode toggle works
- [ ] Expert widgets appear/disappear
- [ ] Manual channel works (tested above)

**All Regression Tests Must Pass**

---

## Performance Testing

### Benchmarks

Measure and record:

| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| Widget load time | < 100 ms | ____ ms | [ ] PASS [ ] FAIL |
| Dialog open time | < 50 ms | ____ ms | [ ] PASS [ ] FAIL |
| Signal propagation | < 10 ms | ____ ms | [ ] PASS [ ] FAIL |
| Memory with 0 widgets | < 35 MB | ____ MB | [ ] PASS [ ] FAIL |
| Memory with 9 widgets | < 55 MB | ____ MB | [ ] PASS [ ] FAIL |
| Application startup | < 3 sec | ____ sec | [ ] PASS [ ] FAIL |

**Measuring widget load time:**
```
Check console debug output for timestamps:
[14:23:45.123] === addComponent called
[14:23:45.125] === Component added successfully
Time = 125 - 123 = 2 ms ✅
```

---

## Compatibility Testing

### Operating Systems

Test on multiple OS platforms:

#### Linux
- [ ] Ubuntu 22.04 LTS
- [ ] Ubuntu 24.04 LTS
- [ ] Fedora 40
- [ ] Arch Linux (rolling)
- [ ] Debian 12

#### Windows
- [ ] Windows 10
- [ ] Windows 11

#### macOS
- [ ] macOS 13 (Ventura)
- [ ] macOS 14 (Sonoma)

### Qt Versions

Test with different Qt versions:

- [ ] Qt 6.2
- [ ] Qt 6.5
- [ ] Qt 6.8 (latest)

### SDR Hardware

Test with different devices:

- [ ] RTL-SDR V3
- [ ] Airspy R2
- [ ] SoapySDR (with HackRF)
- [ ] No hardware (file playback)

---

## Test Reporting

### Test Summary Template

```
==========================================
GUI Fixes Test Report
==========================================

Test Date: 2025-10-13
Tester: [Your Name]
Build: Commit 6dc0ec26
OS: Ubuntu 24.04 LTS
Qt Version: 6.8.0

TEST RESULTS:
-------------
TC-001: Announcement Settings Access       [ ] PASS [ ] FAIL
TC-002: Announcement History Access        [ ] PASS [ ] FAIL
TC-003: Quick Selection Buttons            [ ] PASS [ ] FAIL
TC-004: Individual Announcement Toggle     [ ] PASS [ ] FAIL
TC-005: Manual Channel Visibility          [ ] PASS [ ] FAIL
TC-006: Manual Channel Width               [ ] PASS [ ] FAIL
TC-007: Widget Loading                     [ ] PASS [ ] FAIL
TC-008: Widget Persistence                 [ ] PASS [ ] FAIL
TC-009: Console Error Checking             [ ] PASS [ ] FAIL
TC-010: Memory Leak Testing                [ ] PASS [ ] FAIL

REGRESSION TESTS:
-----------------
RT-001: Station Scanning                   [ ] PASS [ ] FAIL
RT-002: Station Playback                   [ ] PASS [ ] FAIL
RT-003: Device Selection                   [ ] PASS [ ] FAIL
RT-004: Settings Persistence               [ ] PASS [ ] FAIL
RT-005: Expert Mode                        [ ] PASS [ ] FAIL

PERFORMANCE:
------------
Widget Load Time: ____ ms (Target: < 100ms)
Dialog Open Time: ____ ms (Target: < 50ms)
Memory Usage: ____ MB (Target: < 55MB)

ISSUES FOUND:
-------------
1. [Issue description]
2. [Issue description]

NOTES:
------
[Additional observations]

OVERALL VERDICT:
----------------
[ ] APPROVE FOR RELEASE
[ ] APPROVE WITH MINOR ISSUES
[ ] REQUIRES FIXES

Signature: _______________
Date: _______________
```

---

## Automated Testing Scripts

### Quick Test Script

```bash
#!/bin/bash
# quick-test.sh - Run quick sanity checks

echo "Starting welle.io quick test..."

# Start app in background
./build/src/welle-io/welle-io &
APP_PID=$!

sleep 5  # Wait for startup

# Check if app is running
if ps -p $APP_PID > /dev/null; then
    echo "✅ Application started successfully"
else
    echo "❌ Application failed to start"
    exit 1
fi

# Check memory usage
MEM=$(ps -o rss= -p $APP_PID)
if [ "$MEM" -lt 100000 ]; then  # < 100 MB
    echo "✅ Memory usage acceptable: ${MEM}KB"
else
    echo "⚠️  High memory usage: ${MEM}KB"
fi

# Kill app
kill $APP_PID

echo "Quick test complete!"
```

### Console Error Check Script

```bash
#!/bin/bash
# check-errors.sh - Check console output for errors

echo "Checking console output for errors..."

if grep -q "Settings is not a type" test-output.log; then
    echo "❌ FAIL: Found 'Settings is not a type' error"
    exit 1
else
    echo "✅ PASS: No 'Settings is not a type' errors"
fi

if grep -q "Component could not be loaded" test-output.log; then
    echo "❌ FAIL: Found component loading errors"
    exit 1
else
    echo "✅ PASS: No component loading errors"
fi

echo "Console check complete!"
```

---

## Contact

**Questions about testing?**
- Developer: Seksan Poltree <seksan.poltree@gmail.com>
- GitHub: https://github.com/AlbrechtL/welle.io

---

**End of Testing Guide**
