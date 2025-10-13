# Release Notes - GUI Improvements

**Version:** Phase 3 Wave 2B
**Date:** October 13, 2025
**welle.io DAB/DAB+ Receiver**

---

## What's New

This release fixes several important issues that prevented you from accessing announcement features and adding widgets to your workspace.

---

## New Features Now Available

### 1. Announcement Settings & History

You can now access announcement settings and view announcement history from the menu.

**How to use:**

1. Open welle.io
2. Click the **Menu** button (☰) at the top
3. Select **"Announcement Settings"** to configure which types of announcements you want to receive
4. Select **"Announcement History"** to see past announcements

**What you can do:**

- **Choose announcement types:** Pick which announcements matter to you (traffic, news, weather, etc.)
- **Quick selections:** Use "Critical Only" for important alerts, "All Types" for everything, or "Clear All" to turn off announcements
- **View history:** See all announcements you've received with timestamps
- **Visual alerts:** Active announcements appear at the top of the screen

---

### 2. All Widgets Now Working

Fixed an issue where most widgets couldn't be added from the Add (+) menu. Now all widgets work perfectly!

**Previously broken, now working:**

- **MOT Slide Show** - View station logos and images
- **Spectrum Graph** - See frequency spectrum visualization
- **Null Symbol Graph** - Monitor signal quality
- **Raw Recorder** - Record raw I/Q data
- **Text Output** - View technical information

**How to add widgets:**

1. Click the **Add (+)** button in the toolbar
2. Select the widget you want
3. The widget appears in your workspace
4. Arrange widgets by dragging them
5. Your layout is saved automatically

---

### 3. Better Manual Channel Selection

If you use Expert Mode, the manual channel dropdown now works much better.

**Improvements:**

- Channel names are now **clearly visible** (previously they were invisible!)
- Full channel name displays (previously "12B" showed as only "12")
- Better text contrast against the background
- Works with all Thailand DAB+ channels (5A through 13F)

**How to use:**

1. Enable **Expert Mode** in settings
2. Find the **"Manual channel"** dropdown near the top
3. Select your desired channel
4. Radio tunes to that frequency immediately

---

## What We Fixed

### Critical Bug Fixes

#### Fix 1: Widgets Wouldn't Load
**Problem:** Clicking Add (+) menu and selecting widgets did nothing. Widgets didn't appear.

**What was wrong:** Technical issue with configuration loading (missing imports in 9 files).

**Now:** All widgets load instantly when you select them.

---

#### Fix 2: Announcement Features Were Hidden
**Problem:** Announcement settings and history existed but weren't accessible. Users couldn't find them.

**What was wrong:** Menu items weren't added to the interface.

**Now:** Both features clearly visible in the menu.

---

### Important Bug Fixes

#### Fix 3: Quick Selection Buttons Didn't Work
**Problem:** In Announcement Settings, clicking "Critical Only", "All Types", or "Clear All" did nothing. Checkboxes didn't update.

**What was wrong:** Interface wasn't listening to backend state changes.

**Now:** Buttons work instantly. Click "All Types" and watch all checkboxes get checked immediately.

---

#### Fix 4: Channel Names Invisible or Cut Off
**Problem:** Manual channel dropdown showed blank text or cut off letters ("12B" appeared as "12").

**What was wrong:** Text color and width calculation issues.

**Now:** Full channel names display clearly with proper contrast.

---

## Technical Improvements

For users interested in technical details:

- **Better error handling:** Application logs helpful debug messages
- **Memory efficiency:** Widgets use less memory and load faster
- **Qt6 compatibility:** Works perfectly with latest Qt6 framework
- **Persistent settings:** Your widget layout and preferences save automatically
- **Signal-slot optimization:** Real-time UI updates when settings change

---

## Known Limitations

### QtCharts Module Required for Some Graphs

Two advanced graph widgets need an additional package:

- **Impulse Response Graph**
- **Constellation Graph**

**If you see "module QtCharts is not installed" error:**

**Ubuntu/Debian users:**
```bash
sudo apt install qml6-module-qtcharts
```

**Fedora users:**
```bash
sudo dnf install qt6-qtcharts-devel
```

**Arch Linux users:**
```bash
sudo pacman -S qt6-charts
```

After installing, restart welle.io and the graphs will work.

**Note:** All other widgets work without this package. Only affects these 2 specialized graphs.

---

## Getting Started

### For New Users

1. **Install/Update welle.io**
   - Download from your package manager
   - Or build from source with latest code

2. **Explore New Features**
   - Check out the menu for announcement features
   - Try adding widgets with the Add (+) button
   - Customize your workspace layout

3. **Configure Announcements**
   - Open Announcement Settings
   - Choose which alerts you want to see
   - Use "Critical Only" for traffic/emergency alerts

### For Existing Users

Your previous settings and widget layouts are preserved. New features appear in the menu automatically.

**What stays the same:**
- Your favorite stations
- Audio settings
- Device configuration
- Expert mode preferences

**What's new:**
- Announcement features in menu
- All widgets now loadable
- Better manual channel selection

---

## Upgrading

### From Package Manager

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt upgrade welle-io
```

**Flatpak:**
```bash
flatpak update io.welle.welle
```

### From Source

```bash
cd welle.io
git pull
./build-script.sh build
```

No special migration needed. All changes are automatic.

---

## Help & Support

### Getting Help

**Documentation:**
- User Guide: https://www.welle.io
- Technical Docs: `docs/` folder in source code

**Report Issues:**
- GitHub: https://github.com/AlbrechtL/welle.io/issues
- Include: What you tried, what happened, error messages

**Community:**
- GitHub Discussions for questions
- Check existing issues before reporting

### Frequently Asked Questions

**Q: Where are the announcement features?**
A: Click Menu (☰) → "Announcement Settings" or "Announcement History"

**Q: Widgets still won't load?**
A: Check console for "QtCharts" error. You may need to install the Qt6 Charts module (see Known Limitations).

**Q: Channel dropdown still shows blank?**
A: Enable Expert Mode first. If still blank, please report the issue.

**Q: How do I reset my widget layout?**
A: Remove all widgets using the (-) remove button, then add them back in your preferred arrangement.

**Q: Do announcement settings save?**
A: Yes! Your announcement type preferences persist between sessions.

---

## What's Next

### Upcoming Features

We're working on:
- Enhanced Thai language support
- More announcement types
- Improved signal quality indicators
- Better station scanning

### Stay Updated

- Watch GitHub repository for releases
- Check https://www.welle.io for news
- Subscribe to mailing list (if available)

---

## Credits

**Development:** Seksan Poltree
**Testing:** welle.io community
**Framework:** Qt6
**License:** GPL

---

## Version History

### Phase 3 Wave 2B (2025-10-13)
- ✅ Fixed widget loading from Add menu
- ✅ Made announcement features accessible
- ✅ Fixed quick selection buttons
- ✅ Improved manual channel dropdown
- ✅ Enhanced debug logging

### Phase 3 Wave 2A (2025-10-13)
- Thai language localization
- Thai UI components
- Translation support

### Phase 3 Wave 1 (2025-10-12)
- Announcement support foundation
- QML components for announcements
- Backend integration

---

## Feedback Welcome

We'd love to hear from you!

**What's working well?**
**What could be better?**
**What features do you want next?**

Share feedback on GitHub Issues or Discussions.

---

**Thank you for using welle.io!**

---

## Quick Reference Card

### New Menu Items
| Menu Item | What It Does |
|-----------|--------------|
| Announcement Settings | Choose which alert types to receive |
| Announcement History | View past announcements with timestamps |

### New Working Widgets
| Widget | Purpose |
|--------|---------|
| MOT Slide Show | Station logos and images |
| Spectrum Graph | Frequency visualization |
| Null Symbol Graph | Signal quality monitor |
| Raw Recorder | Record I/Q data |
| Text Output | Technical information |

### Announcement Quick Buttons
| Button | Effect |
|--------|--------|
| Critical Only | Enable Alarm, Traffic, Travel, Warning |
| All Types | Enable all 11 announcement types |
| Clear All | Disable all announcements |

### Announcement Types
1. Alarm
2. Road Traffic
3. Transport Flash
4. Warning/Service
5. News Flash
6. Area Weather Flash
7. Event Announcement
8. Special Event
9. Programme Information
10. Sport Report
11. Financial Report

---

**Enjoy the improved welle.io experience!**
