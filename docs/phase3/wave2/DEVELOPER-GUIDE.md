# Developer Guide - GUI Integration Patterns

**Version:** Phase 3 Wave 2B
**Date:** 2025-10-13
**Audience:** Developers, Contributors, Maintainers

---

## Table of Contents

- [Overview](#overview)
- [Qt/QML Best Practices](#qtqml-best-practices)
- [Common Patterns](#common-patterns)
- [Anti-Patterns to Avoid](#anti-patterns-to-avoid)
- [Code Examples](#code-examples)
- [Debugging Techniques](#debugging-techniques)
- [Contributing Guidelines](#contributing-guidelines)

---

## Overview

This guide documents the Qt/QML patterns and practices learned from fixing GUI integration issues in Phase 3 Wave 2B. Use these patterns when:

- Adding new QML components
- Creating dynamic UI elements
- Connecting C++ backend to QML frontend
- Implementing persistent settings
- Styling custom widgets

---

## Qt/QML Best Practices

### 1. Loader Null Safety

**Problem:** Accessing `Loader.item` properties before component fully loaded causes TypeError.

**Anti-Pattern:**
```qml
Loader {
    id: myLoader
    source: "MyComponent.qml"

    // ❌ WRONG - Can crash!
    height: item.implicitHeight
    width: item.implicitWidth
}
```

**Best Practice:**
```qml
Loader {
    id: myLoader
    source: "MyComponent.qml"

    // ✅ CORRECT - Null-safe
    height: (status === Loader.Ready && item) ? item.implicitHeight : undefined
    width: (status === Loader.Ready && item) ? item.implicitWidth : undefined
}
```

**Why it matters:**
- `item` is null until component loaded
- `status === Loader.Ready` doesn't guarantee `item` is non-null
- Both checks required for safety

**Alternative (Qt6.2+):**
```qml
// Using optional chaining (requires Qt 6.2+)
height: item?.implicitHeight ?? undefined
width: item?.implicitWidth ?? undefined
```

**When to use:**
- Any time you access `Loader.item` properties
- When binding to item methods: `item.someMethod()`
- When passing item to functions: `doSomething(item)`

---

### 2. Qt.labs.settings Import

**Problem:** Using `Settings {}` type without importing the module causes "Settings is not a type" error.

**Anti-Pattern:**
```qml
import QtQuick
import QtQuick.Controls

// ❌ WRONG - Missing import!
Item {
    Settings {
        id: settings
        property alias visible: root.visible
    }
}
```

**Best Practice:**
```qml
import QtQuick
import QtQuick.Controls
import Qt.labs.settings 1.0  // ✅ CORRECT - Explicit import with version

Item {
    Settings {
        id: settings
        category: "MyComponent"  // Good: Use category for organization
        property alias visible: root.visible
    }
}
```

**Why it matters:**
- QML engine validates all types at component creation
- Missing import causes `Qt.createComponent()` to fail
- Error happens at runtime, not compile time
- Affects dynamic component loading (Add menu widgets)

**Scope and organization:**
```qml
Settings {
    id: settings
    category: "SpectrumGraph"  // Namespace settings by component

    // Good: Use property aliases for automatic sync
    property alias visible: spectrumGraph.visible
    property alias fftSize: spectrumGraph.fftSize

    // Good: Store complex types as JSON strings
    property string windowGeometry: JSON.stringify({x: 0, y: 0, w: 400, h: 300})
}
```

**Qt6 compatibility:**
- `Qt.labs.settings` is same in Qt5 and Qt6
- Module is considered "stable enough" despite "labs" naming
- No breaking changes expected

---

### 3. Signal-Slot Pattern for C++ ↔ QML

**Problem:** QML UI doesn't update when C++ backend state changes programmatically.

**Anti-Pattern:**
```qml
// ❌ WRONG - Only reads once at initialization!
CheckBox {
    id: myCheckbox
    checked: radioController.getSomeSetting()
    onClicked: radioController.setSomeSetting(checked)
}
```

**Best Practice:**

**Step 1: Add signal to C++ class** (`radio_controller.h`)
```cpp
class RadioController : public QObject {
    Q_OBJECT

signals:
    void someSettingChanged();  // ✅ CORRECT - Notify QML of changes

public slots:
    void setSomeSetting(bool value);
    bool getSomeSetting() const;
};
```

**Step 2: Emit signal when state changes** (`radio_controller.cpp`)
```cpp
void RadioController::setSomeSetting(bool value) {
    bool oldValue = getSomeSetting();

    // Update backend state
    backend_->setSetting(value);

    // ✅ CORRECT - Only emit if actually changed
    if (oldValue != value) {
        emit someSettingChanged();
    }
}
```

**Step 3: Listen in QML** (`MyComponent.qml`)
```qml
CheckBox {
    id: myCheckbox
    checked: radioController.getSomeSetting()

    // User clicks checkbox → update backend
    onClicked: radioController.setSomeSetting(checked)

    // ✅ CORRECT - Backend changes → update UI
    Connections {
        target: radioController
        function onSomeSettingChanged() {
            myCheckbox.checked = radioController.getSomeSetting()
        }
    }
}
```

**Why it matters:**
- QML property bindings are one-time unless explicitly updated
- C++ signals are the standard Qt mechanism for state change notification
- Avoids polling/timers which waste CPU
- Thread-safe (Qt handles cross-thread signals)

**Guard against infinite loops:**
```cpp
void RadioController::setSomeSetting(bool value) {
    // ✅ CORRECT - Only emit if changed (prevents loops)
    bool oldValue = getSomeSetting();
    backend_->setSetting(value);

    if (oldValue != value) {
        emit someSettingChanged();
    }

    // ❌ WRONG - Emits even if unchanged (can cause loops)
    // emit someSettingChanged();
}
```

---

### 4. ComboBox Custom Styling

**Problem:** Default ComboBox text may be invisible with custom themes.

**Anti-Pattern:**
```qml
// ❌ WRONG - Relies on theme defaults
ComboBox {
    id: combo
    model: ["Option 1", "Option 2"]
    // No contentItem specified
    // Text color depends on theme
}
```

**Best Practice:**
```qml
ComboBox {
    id: combo
    model: ["Option 1", "Option 2"]

    // ✅ CORRECT - Explicit contentItem
    contentItem: Text {
        leftPadding: 0
        rightPadding: combo.indicator.width + combo.spacing
        text: combo.displayText
        font: combo.font
        color: combo.enabled ? TextStyle.textColor
               : Qt.rgba(TextStyle.textColor.r,
                        TextStyle.textColor.g,
                        TextStyle.textColor.b,
                        0.3)
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
```

**Better approach (using opacity):**
```qml
ComboBox {
    id: combo
    model: ["Option 1", "Option 2"]

    contentItem: Text {
        leftPadding: 0
        rightPadding: combo.indicator.width + combo.spacing
        text: combo.displayText
        font: combo.font
        color: TextStyle.textColor  // ✅ CORRECT - Simple color
        opacity: combo.enabled ? 1.0 : 0.3  // ✅ CORRECT - Opacity for disabled
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
```

**Why it matters:**
- Qt Universal/Material themes may have unexpected defaults
- Custom themes may not style ComboBox properly
- Disabled state needs visual distinction
- Text needs to contrast with background

**Width calculation:**
```qml
ComboBox {
    id: combo
    property bool sizeToContents: true
    property int modelWidth: 0  // Calculated by TextMetrics

    // ✅ CORRECT - Self-documenting width
    readonly property int indicatorSpace: indicator ? indicator.width + spacing : 0
    readonly property int contentPadding: leftPadding + rightPadding
    readonly property int widthMargin: 6  // Visual breathing room
    readonly property int calculatedWidth: modelWidth + indicatorSpace +
                                           contentPadding + widthMargin

    width: sizeToContents ? calculatedWidth : implicitWidth

    // ❌ WRONG - Magic number
    // width: sizeToContents ? modelWidth + 60 : implicitWidth
}
```

---

### 5. Dynamic Component Creation

**Problem:** Creating components dynamically requires careful error handling.

**Anti-Pattern:**
```qml
function addWidget(path) {
    // ❌ WRONG - No error checking!
    var component = Qt.createComponent(path);
    var object = component.createObject(parent);
    // What if component failed to load?
    // What if createObject returned null?
}
```

**Best Practice:**
```qml
function addWidget(path) {
    console.debug("=== addComponent called with path:", path);

    // ✅ CORRECT - Check for duplicates first
    for (var i = 0; i < children.length; i++) {
        if (children[i].objectName === path) {
            console.debug("=== Component already exists - skipping");
            return;
        }
    }

    // ✅ CORRECT - Create component with error checking
    var component = Qt.createComponent(path);

    if (component.status === Component.Error) {
        console.error("=== ERROR creating component:", component.errorString());
        return;
    }

    if (component.status !== Component.Ready) {
        console.warn("=== Component not ready, status:", component.status);
        return;
    }

    console.debug("=== Component ready, creating object...");

    // ✅ CORRECT - Create object with proper parent and properties
    var object = component.createObject(parent, {
        "objectName": path,
        "Layout.row": Qt.binding(function() { return rowIndex }),
        "Layout.column": Qt.binding(function() { return columnIndex })
    });

    if (object === null) {
        console.error("=== ERROR: createObject returned null");
        return;
    }

    console.debug("=== Object created successfully, type:", object.toString());
    console.debug("=== Component added successfully");
}
```

**Why it matters:**
- Component loading can fail (missing file, syntax error, missing imports)
- Object creation can fail (invalid parent, missing properties)
- Without checks, app crashes or behaves unpredictably
- Good logging helps diagnose issues

**Component lifecycle:**
```
1. Qt.createComponent(path)
   ├─ Component.Loading → Wait
   ├─ Component.Ready → Proceed
   └─ Component.Error → Abort

2. component.createObject(parent, properties)
   ├─ Returns object → Success
   └─ Returns null → Abort

3. Object destroyed when parent destroyed (Qt parent-child)
```

---

### 6. Qt Resource System

**Problem:** QML components not found at runtime even though files exist.

**Anti-Pattern:**
```qml
// ❌ WRONG - Relative filesystem path
Loader {
    source: "../components/MyComponent.qml"
}
```

**Best Practice:**

**Step 1: Register in resources.qrc**
```xml
<RCC>
    <qresource prefix="/">
        <!-- ✅ CORRECT - Register the QML file -->
        <file>QML/components/MyComponent.qml</file>
    </qresource>
</RCC>
```

**Step 2: Use qrc:// URL**
```qml
// ✅ CORRECT - Qt resource URL
Loader {
    source: "qrc:/QML/components/MyComponent.qml"
}
```

**Why it matters:**
- Resources compiled into binary (single executable)
- Works across all platforms (Windows/Linux/macOS)
- No filesystem path issues
- Faster loading (no disk I/O)

**Resource organization:**
```
resources.qrc
├─ QML/
│  ├─ components/
│  │  ├─ WButton.qml
│  │  ├─ WComboBox.qml
│  │  └─ AnnouncementIndicator.qml
│  ├─ expertviews/
│  │  ├─ SpectrumGraph.qml
│  │  └─ NullSymbolGraph.qml
│  └─ settingpages/
│     ├─ AnnouncementSettings.qml
│     └─ RTLSDRSettings.qml
└─ images/
   └─ icons/
```

---

### 7. Modern Qt6 Import Syntax

**Problem:** Old-style versioned imports are deprecated in Qt6.

**Anti-Pattern:**
```qml
// ❌ OLD STYLE - Qt5 versioned imports
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
```

**Best Practice:**
```qml
// ✅ CORRECT - Qt6 version-less imports
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Exception: Qt.labs modules still need version
import Qt.labs.settings 1.0
```

**Why it matters:**
- Qt6 ignores version numbers anyway
- Cleaner syntax
- Future-proof (Qt7 ready)
- Recommended by Qt documentation

**Migration:**
```bash
# Find all files with versioned imports
grep -r "import QtQuick 2" src/welle-gui/QML/

# Replace manually or with sed
sed -i 's/import QtQuick 2\.[0-9]\+/import QtQuick/' *.qml
sed -i 's/import QtQuick\.Controls 2\.[0-9]\+/import QtQuick.Controls/' *.qml
sed -i 's/import QtQuick\.Layouts 1\.[0-9]\+/import QtQuick.Layouts/' *.qml
```

---

## Common Patterns

### Pattern 1: Persistent Widget Layout

**Use case:** Save user's widget arrangement across application restarts.

**Implementation:**

```qml
// GridLayout with persistent layout
GridLayout {
    id: componentGrid
    columns: 2
    rowSpacing: 5
    columnSpacing: 5

    function saveLayout() {
        var layout = []
        for (var i = 0; i < children.length; i++) {
            layout.push({
                path: children[i].objectName,
                row: children[i].Layout.row,
                column: children[i].Layout.column
            })
        }
        settings.layout = JSON.stringify(layout)
    }

    function restoreLayout() {
        if (settings.layout) {
            var layout = JSON.parse(settings.layout)
            for (var i = 0; i < layout.length; i++) {
                addComponent(layout[i].path, layout[i].row, layout[i].column)
            }
        }
    }

    Component.onCompleted: restoreLayout()
    Component.onDestruction: saveLayout()

    Settings {
        id: settings
        category: "ComponentGrid"
        property string layout: ""
    }
}
```

---

### Pattern 2: Lazy Loading Dialogs

**Use case:** Load dialog components only when needed to reduce startup time.

**Implementation:**

```qml
// Main window
Item {
    // ✅ CORRECT - Lazy loading with Loader
    Loader {
        id: settingsDialogLoader
        active: false
        source: active ? "qrc:/QML/dialogs/SettingsDialog.qml" : ""

        onLoaded: {
            if (item) {
                item.visible = true
            }
        }

        function open() {
            active = true
        }

        function close() {
            if (item) {
                item.visible = false
            }
            active = false  // Unload to save memory
        }
    }

    // Button to open dialog
    Button {
        text: "Settings"
        onClicked: settingsDialogLoader.open()
    }
}
```

**Benefits:**
- Faster application startup (dialogs not loaded initially)
- Lower memory usage (only load when needed)
- Can unload after close to free memory

---

### Pattern 3: Quick Selection Buttons

**Use case:** Provide preset configurations for multiple checkboxes.

**Implementation:**

```qml
Column {
    // Quick selection buttons
    Row {
        spacing: 10

        Button {
            text: "Critical Only"
            onClicked: {
                // Enable critical types (1-4)
                for (var i = 1; i <= 4; i++) {
                    radioController.setAnnouncementTypeEnabled(i, true)
                }
                // Disable non-critical types (5-11)
                for (var i = 5; i <= 11; i++) {
                    radioController.setAnnouncementTypeEnabled(i, false)
                }
            }
        }

        Button {
            text: "All Types"
            onClicked: {
                for (var i = 1; i <= 11; i++) {
                    radioController.setAnnouncementTypeEnabled(i, true)
                }
            }
        }

        Button {
            text: "Clear All"
            onClicked: {
                for (var i = 1; i <= 11; i++) {
                    radioController.setAnnouncementTypeEnabled(i, false)
                }
            }
        }
    }

    // Checkboxes that respond to buttons
    Repeater {
        model: announcementTypes

        CheckBox {
            id: typeCheckbox
            text: modelData.name
            checked: radioController.isAnnouncementTypeEnabled(modelData.type)

            onClicked: {
                radioController.setAnnouncementTypeEnabled(modelData.type, checked)
            }

            // ✅ CORRECT - Update when backend changes
            Connections {
                target: radioController
                function onAnnouncementTypesChanged() {
                    typeCheckbox.checked = radioController.isAnnouncementTypeEnabled(modelData.type)
                }
            }
        }
    }
}
```

---

### Pattern 4: TextMetrics for Dynamic Width

**Use case:** Size ComboBox to fit longest item in model.

**Implementation:**

```qml
ComboBox {
    id: combo
    property bool sizeToContents: true
    property int modelWidth: 0

    model: ["5A", "10A", "12B", "13E"]

    // ✅ CORRECT - Measure model items
    TextMetrics {
        id: textMetrics
        font: combo.font
    }

    function updateWidth() {
        var maxWidth = 0
        for (var i = 0; i < model.length; i++) {
            textMetrics.text = model[i]
            maxWidth = Math.max(maxWidth, textMetrics.width)
        }
        modelWidth = maxWidth
    }

    Component.onCompleted: updateWidth()
    onModelChanged: updateWidth()

    width: sizeToContents ? modelWidth + indicatorSpace + contentPadding + 6 : implicitWidth
}
```

---

## Anti-Patterns to Avoid

### Anti-Pattern 1: Direct Property Access Without Null Check

```qml
// ❌ BAD
Loader {
    id: loader
    height: item.implicitHeight  // CRASH if item is null!
}

// ✅ GOOD
Loader {
    id: loader
    height: (status === Loader.Ready && item) ? item.implicitHeight : undefined
}
```

---

### Anti-Pattern 2: Polling for State Changes

```qml
// ❌ BAD - Wastes CPU
Timer {
    interval: 100
    repeat: true
    running: true
    onTriggered: {
        checkbox.checked = radioController.getSetting()
    }
}

// ✅ GOOD - Event-driven
Connections {
    target: radioController
    function onSettingChanged() {
        checkbox.checked = radioController.getSetting()
    }
}
```

---

### Anti-Pattern 3: Emitting Signals Unconditionally

```cpp
// ❌ BAD - Can cause infinite loops
void RadioController::setSetting(bool value) {
    setting_ = value;
    emit settingChanged();  // Emits even if unchanged!
}

// ✅ GOOD - Guard with condition
void RadioController::setSetting(bool value) {
    if (setting_ != value) {
        setting_ = value;
        emit settingChanged();
    }
}
```

---

### Anti-Pattern 4: Hardcoded Magic Numbers

```qml
// ❌ BAD
width: modelWidth + 60  // What is 60?

// ✅ GOOD
readonly property int indicatorSpace: indicator.width + spacing
readonly property int contentPadding: leftPadding + rightPadding
readonly property int widthMargin: 6
width: modelWidth + indicatorSpace + contentPadding + widthMargin
```

---

### Anti-Pattern 5: Missing Error Handling

```qml
// ❌ BAD
function addWidget(path) {
    var component = Qt.createComponent(path)
    var object = component.createObject(parent)
    // No error checking!
}

// ✅ GOOD
function addWidget(path) {
    var component = Qt.createComponent(path)
    if (component.status === Component.Error) {
        console.error("Failed to create component:", component.errorString())
        return
    }
    var object = component.createObject(parent)
    if (!object) {
        console.error("Failed to create object")
        return
    }
}
```

---

## Code Examples

### Complete Example: Custom Settings Dialog

```qml
// AnnouncementSettings.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.settings 1.0

Dialog {
    id: dialog
    title: qsTr("Announcement Settings")
    width: 500
    height: 600
    modal: true

    // Persistent settings
    Settings {
        id: settings
        category: "AnnouncementSettings"
        property alias dialogX: dialog.x
        property alias dialogY: dialog.y
    }

    // Model data
    property var announcementTypes: [
        { type: 1, name: qsTr("Alarm") },
        { type: 2, name: qsTr("Road Traffic") },
        { type: 3, name: qsTr("Transport Flash") },
        { type: 4, name: qsTr("Warning/Service") },
        { type: 5, name: qsTr("News Flash") },
        { type: 6, name: qsTr("Area Weather Flash") },
        { type: 7, name: qsTr("Event Announcement") },
        { type: 8, name: qsTr("Special Event") },
        { type: 9, name: qsTr("Programme Information") },
        { type: 10, name: qsTr("Sport Report") },
        { type: 11, name: qsTr("Financial Report") }
    ]

    contentItem: ColumnLayout {
        spacing: 10

        Label {
            text: qsTr("Select announcement types to monitor:")
            font.bold: true
        }

        // Quick selection buttons
        Row {
            spacing: 10
            Layout.fillWidth: true

            Button {
                text: qsTr("Critical Only")
                onClicked: {
                    for (var i = 1; i <= 4; i++) {
                        radioController.setAnnouncementTypeEnabled(i, true)
                    }
                    for (var i = 5; i <= 11; i++) {
                        radioController.setAnnouncementTypeEnabled(i, false)
                    }
                }
            }

            Button {
                text: qsTr("All Types")
                onClicked: {
                    for (var i = 1; i <= 11; i++) {
                        radioController.setAnnouncementTypeEnabled(i, true)
                    }
                }
            }

            Button {
                text: qsTr("Clear All")
                onClicked: {
                    for (var i = 1; i <= 11; i++) {
                        radioController.setAnnouncementTypeEnabled(i, false)
                    }
                }
            }
        }

        // Scrollable list of checkboxes
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                model: announcementTypes

                delegate: CheckBox {
                    id: typeCheckbox
                    text: modelData.name
                    checked: radioController.isAnnouncementTypeEnabled(modelData.type)

                    onClicked: {
                        radioController.setAnnouncementTypeEnabled(modelData.type, checked)
                    }

                    Connections {
                        target: radioController
                        function onAnnouncementTypesChanged() {
                            typeCheckbox.checked = radioController.isAnnouncementTypeEnabled(modelData.type)
                        }
                    }
                }
            }
        }
    }

    standardButtons: Dialog.Close
}
```

---

## Debugging Techniques

### Technique 1: Comprehensive Component Loading Debug

```qml
function addComponent(path, row, column) {
    console.debug("=== addComponent called with path:", path, "row:", row, "column:", column)
    console.debug("=== Current children count:", children.length)

    // Check duplicates
    for (var i = 0; i < children.length; i++) {
        if (children[i].objectName === path) {
            console.debug("=== Component already exists at index", i, "- skipping")
            return
        }
    }

    // Calculate position
    var rowIndex = (row >= 0) ? row : Math.floor(children.length / 2)
    var columnIndex = (column >= 0) ? column : children.length % 2
    console.debug("=== Creating component at row:", rowIndex, "column:", columnIndex)

    // Create component
    console.debug("=== Creating component:", path)
    var component = Qt.createComponent(path)

    if (component.status === Component.Error) {
        console.debug("=== ERROR creating component:", component.errorString())
        return
    }

    if (component.status !== Component.Ready) {
        console.debug("=== Component not ready, status:", component.status)
        return
    }

    console.debug("=== Component ready, creating object...")
    var object = component.createObject(componentGrid, {
        "objectName": path,
        "Layout.row": Qt.binding(function() { return rowIndex }),
        "Layout.column": Qt.binding(function() { return columnIndex })
    })

    if (object === null) {
        console.debug("=== ERROR: createObject returned null")
        return
    }

    console.debug("=== Object created successfully, type:", object.toString())
    console.debug("=== Assigned to Layout.row:", rowIndex, "Layout.column:", columnIndex)
    console.debug("=== Object visibility:", object.visible, "width:", object.width, "height:", object.height)
    console.debug("=== Component added successfully, new children count:", children.length)
}
```

---

### Technique 2: Signal Emission Tracing

```cpp
// radio_controller.cpp
void RadioController::setAnnouncementTypeEnabled(uint16_t type, bool enabled) {
    qDebug() << "=== setAnnouncementTypeEnabled called:" << type << enabled;

    bool wasEnabled = announcementManager_->isAnnouncementTypeEnabled(type);
    qDebug() << "=== Previous state:" << wasEnabled;

    announcementManager_->setAnnouncementTypeEnabled(type, enabled);

    if (wasEnabled != enabled) {
        qDebug() << "=== State changed, emitting announcementTypesChanged()";
        emit announcementTypesChanged();
    } else {
        qDebug() << "=== State unchanged, NOT emitting signal";
    }
}
```

---

### Technique 3: QML Profiler

```bash
# Run with QML profiler enabled
QML_PROFILER_OUTPUT=profile.qtd ./welle-io

# Analyze with Qt Creator
# Tools → QML Profiler → Load QML Trace → profile.qtd
```

---

### Technique 4: Memory Leak Detection

```bash
# Valgrind on Linux
valgrind --leak-check=full --show-leak-kinds=all ./welle-io

# Check for Qt-specific leaks
QT_LOGGING_RULES="qt.*.debug=true" valgrind ./welle-io
```

---

## Contributing Guidelines

### Before Submitting PR

1. **Test all affected features**
   - Run manual test cases from TESTING-GUIDE.md
   - Check console for errors

2. **Run linters**
   ```bash
   qmllint src/welle-gui/QML/**/*.qml
   qmlformat -i src/welle-gui/QML/**/*.qml
   ```

3. **Update documentation**
   - Add entry to CHANGELOG-gui-fixes.md
   - Update CLAUDE.md if adding new patterns
   - Include code comments

4. **Verify Qt6 compatibility**
   - Build with Qt 6.2, 6.5, and 6.8
   - Test on Linux, Windows, macOS

### Code Review Checklist

- [ ] All Loader components have null checks
- [ ] All QML files using Settings import Qt.labs.settings
- [ ] C++ signals emitted with guard conditions
- [ ] No magic numbers (use named properties)
- [ ] Error handling for dynamic component creation
- [ ] Console debug messages for troubleshooting
- [ ] QML resources registered in resources.qrc
- [ ] Version-less imports (Qt6 style)

---

## References

### Official Qt Documentation
- [Loader QML Type](https://doc.qt.io/qt-6/qml-qtquick-loader.html)
- [Qt.labs.settings](https://doc.qt.io/qt-6/qml-qt-labs-settings-settings.html)
- [Signals and Handlers](https://doc.qt.io/qt-6/qtqml-syntax-signals.html)
- [Dynamic Object Management](https://doc.qt.io/qt-6/qtqml-javascript-dynamicobjectcreation.html)

### welle.io Documentation
- [CHANGELOG-gui-fixes.md](CHANGELOG-gui-fixes.md) - Complete changelog
- [TESTING-GUIDE.md](TESTING-GUIDE.md) - Testing procedures
- [RELEASE-NOTES.md](RELEASE-NOTES.md) - User-facing notes
- [CLAUDE.md](../../../CLAUDE.md) - Project conventions

---

**End of Developer Guide**
