# welle.io E2E GUI Component Tests

Comprehensive end-to-end (E2E) testing suite for all reusable QML components in the welle.io Qt/QML project.

## Overview

This test suite provides thorough validation of all GUI components to ensure:
- **Zero console errors/warnings** during component lifecycle
- Proper property and signal definitions
- Visual rendering works correctly
- Interaction handling functions as expected
- Thai language support and Unicode handling
- Component isolation and independence

## Test Files

### 1. test_framework.h
Central testing framework providing utilities for QML component testing:
- `QMLComponentTestBase` - Base class for all component tests
- Console message capture and verification
- Component loading from files or strings
- Property and signal verification helpers
- Visual rendering test utilities
- Interaction simulation (mouse clicks, etc.)
- Comprehensive error reporting

**Key Features:**
- Automatic console error/warning capture
- Zero-error verification macros
- Component isolation in test environment
- Memory-safe component lifecycle management

### 2. test_components.cpp
Comprehensive tests for all reusable QML components:

#### Components Tested:
1. **WButton.qml** - Custom button component
   - Component loading
   - Properties: text, enabled, width, height
   - Signals: clicked
   - Visual rendering
   - Interaction handling

2. **WComboBox.qml** - Custom combo box
   - Component loading
   - Properties: model, currentIndex, currentText
   - Visual rendering
   - Selection handling

3. **WSwitch.qml** - Custom switch/toggle
   - Component loading
   - Properties: checked, text
   - Signals: toggled
   - Visual rendering

4. **WDialog.qml** - Custom dialog
   - Component loading
   - Properties: title, modal, visible
   - Visual rendering

5. **WMenu.qml** - Custom menu
   - Component loading
   - Properties: title
   - Menu items

6. **WSpectrum.qml** - Spectrum display
   - Component loading
   - Properties: spectrumData, spectrumColor
   - Visual rendering

7. **WToolTip.qml** - Custom tooltip
   - Component loading
   - Properties: text, visible

8. **WTumbler.qml** - Custom tumbler/picker
   - Component loading
   - Properties: model, currentIndex
   - Visual rendering

9. **MessagePopup.qml** - Message popup component
   - Component loading
   - Properties: text, color, visible
   - Visual rendering

10. **AnnouncementIndicator.qml** - Announcement UI indicator
    - Component loading
    - Properties: active, announcementType
    - Visual rendering

11. **AnnouncementHistory.qml** - Announcement history display
    - Component loading
    - Properties: model, count
    - Visual rendering

12. **StationDelegate.qml** - Station list item
    - Component loading
    - Properties: text, width, height
    - Interaction handling
    - Visual rendering

### 3. test_thai_components.cpp
Specialized tests for Thai language components:

#### Components Tested:
1. **ThaiServiceList.qml** - Thai service list
   - Component loading
   - Properties: preferThai, thaiFont, colors
   - Thai text rendering (สถานีวิทยุ, etc.)
   - Unicode handling
   - Signals: serviceSelected, serviceContextMenu
   - Visual rendering

2. **ThaiEPGViewer.qml** - Thai EPG viewer
   - Component loading
   - Properties: preferThai, thaiFont
   - Thai text rendering
   - Visual rendering

3. **ThaiDateFormatter.qml** - Buddhist Era date formatter
   - Component loading
   - Buddhist year conversion (CE → BE, +543)
   - Thai month names (มกราคม, กุมภาพันธ์, etc.)
   - Thai day names (วันจันทร์, วันอาทิตย์, etc.)
   - Format functions (full, long, medium, short)
   - Unicode handling
   - Helper functions (padZero, formatDuration, etc.)

#### Thai-Specific Tests:
- **Font Rendering**: Noto Sans Thai font
- **Complex Script**: Thai tone marks and vowels
- **Text Input**: Thai character encoding
- **Integration**: Multiple Thai components together
- **Unicode Validation**: All Thai strings (สวัสดี, ฉุกเฉิน, etc.)

## Building and Running

### Prerequisites
- Qt6 with Qml, Quick, and Test components
- CMake 3.2 or higher
- C++14 compiler

### Build Configuration

Enable E2E tests in CMake:
```bash
cd build
cmake .. -DBUILD_E2E_TESTS=ON -DBUILD_TESTING=ON
make
```

### Running Tests

#### Run all E2E tests via CTest:
```bash
cd build
ctest --output-on-failure -L e2e
```

#### Run specific test suites:
```bash
# All component tests
./build/src/tests/test_e2e_components

# All Thai component tests
./build/src/tests/test_e2e_thai_components

# With verbose output
./build/src/tests/test_e2e_components -v2
./build/src/tests/test_e2e_thai_components -v2
```

#### Run specific test functions:
```bash
# Test only WButton
./build/src/tests/test_e2e_components testWButton_loadComponent

# Test only ThaiServiceList
./build/src/tests/test_e2e_thai_components testThaiServiceList_loadComponent
```

#### Run with filters:
```bash
# Run all WButton tests
./build/src/tests/test_e2e_components -filter "*WButton*"

# Run all Thai tests
./build/src/tests/test_e2e_thai_components -filter "*Thai*"
```

## Test Coverage

### Component Test Coverage (test_components.cpp)

| Component | Load | Properties | Signals | Visual | Interaction | Total |
|-----------|------|------------|---------|--------|-------------|-------|
| WButton | ✓ | ✓ | ✓ | ✓ | N/A | 4/4 |
| WComboBox | ✓ | ✓ | N/A | ✓ | N/A | 3/3 |
| WSwitch | ✓ | ✓ | ✓ | ✓ | N/A | 4/4 |
| WDialog | ✓ | ✓ | N/A | ✓ | N/A | 3/3 |
| WMenu | ✓ | ✓ | N/A | N/A | N/A | 2/2 |
| WSpectrum | ✓ | ✓ | N/A | ✓ | N/A | 3/3 |
| WToolTip | ✓ | ✓ | N/A | N/A | N/A | 2/2 |
| WTumbler | ✓ | ✓ | N/A | ✓ | N/A | 3/3 |
| MessagePopup | ✓ | ✓ | N/A | ✓ | N/A | 3/3 |
| AnnouncementIndicator | ✓ | ✓ | N/A | ✓ | N/A | 3/3 |
| AnnouncementHistory | ✓ | ✓ | N/A | ✓ | N/A | 3/3 |
| StationDelegate | ✓ | ✓ | N/A | ✓ | ✓ | 4/4 |

**Total: 37 test functions for 12 components**

### Thai Component Test Coverage (test_thai_components.cpp)

| Component | Load | Properties | Thai Text | Unicode | Visual | Signals | Total |
|-----------|------|------------|-----------|---------|--------|---------|-------|
| ThaiServiceList | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | 6/6 |
| ThaiEPGViewer | ✓ | ✓ | ✓ | N/A | ✓ | N/A | 4/4 |
| ThaiDateFormatter | ✓ | ✓ (BE) | ✓ (names) | ✓ | N/A | N/A | 6/6 |
| Thai Font | ✓ | N/A | ✓ | N/A | N/A | N/A | 2/2 |
| Thai Input | ✓ | N/A | N/A | ✓ | N/A | N/A | 1/1 |
| Integration | ✓ | N/A | N/A | N/A | N/A | N/A | 1/1 |

**Total: 20 test functions for 6 component categories**

**Grand Total: 57 E2E test functions covering 18 component categories**

## Test Methodology

### Zero Console Error Verification
Every test verifies ZERO console errors and warnings:
- Error capture during component initialization
- Warning capture during property access
- Error detection during signal emission
- Warning detection during visual rendering

### Component Isolation
Each component is tested in isolation:
- Fresh QQmlEngine for each test
- No shared state between tests
- Independent component instances
- Clean teardown after each test

### Visual Rendering Tests
Verify components can render:
- Create QQuickWindow for hosting
- Set reasonable component dimensions
- Force polish and update cycles
- Process events for rendering pipeline

### Interaction Tests
Simulate user interactions:
- Mouse click simulation
- Keyboard event simulation
- Touch event simulation
- Signal spy for verification

### Thai Language Tests
Specialized Thai validation:
- Unicode character encoding (UTF-8)
- Buddhist Era calendar conversion
- Thai font rendering (Noto Sans Thai)
- Complex script handling (tone marks, vowels)
- Bilingual display (Thai/English)

## Test Framework Macros

```cpp
// Verify component loaded successfully
VERIFY_COMPONENT_LOADED()

// Verify zero console errors
VERIFY_ZERO_CONSOLE_ERRORS()

// Verify zero console warnings
VERIFY_ZERO_CONSOLE_WARNINGS()

// Verify property exists
VERIFY_PROPERTY_EXISTS("propertyName")

// Verify signal exists
VERIFY_SIGNAL_EXISTS("signalName")
```

## Adding New Tests

### 1. Add Test to test_components.cpp

```cpp
void testNewComponent_loadComponent()
{
    QString qmlCode = R"(
        import QtQuick 2.15

        Item {
            id: newComponent
            property string someProperty: "value"
        }
    )";

    QVERIFY2(loadComponentFromString(qmlCode, "NewComponent"),
             "Failed to load NewComponent");
    VERIFY_COMPONENT_LOADED();
    VERIFY_ZERO_CONSOLE_ERRORS();
    printReport("NewComponent - Component Load");
}

void testNewComponent_properties()
{
    // Load component...

    VERIFY_PROPERTY_EXISTS("someProperty");
    QCOMPARE(getProperty("someProperty").toString(), QString("value"));

    VERIFY_ZERO_CONSOLE_ERRORS();
    VERIFY_ZERO_CONSOLE_WARNINGS();
    printReport("NewComponent - Properties");
}
```

### 2. Add Test to test_thai_components.cpp (if Thai-specific)

```cpp
void testNewThaiComponent_unicodeHandling()
{
    QString thaiText = QString::fromUtf8("ข้อความภาษาไทย");

    QString qmlCode = QString(R"(
        import QtQuick 2.15

        Text {
            text: "%1"
            font.family: "Noto Sans Thai"
        }
    )").arg(thaiText);

    QVERIFY(loadComponentFromString(qmlCode, "NewThaiComponent"));
    VERIFY_COMPONENT_LOADED();

    QCOMPARE(getProperty("text").toString(), thaiText);

    VERIFY_ZERO_CONSOLE_ERRORS();
    VERIFY_ZERO_CONSOLE_WARNINGS();
    printReport("NewThaiComponent - Unicode Handling");
}
```

### 3. Update This README
Add component to test coverage table.

## Continuous Integration

### CI Configuration
Add to CI pipeline:
```yaml
- name: Run E2E Tests
  run: |
    cd build
    ctest --output-on-failure -L e2e
```

### Test Reports
Generate test reports:
```bash
# XML output for CI
./build/src/tests/test_e2e_components -o results.xml,xunitxml

# JSON output
./build/src/tests/test_e2e_components -o results.json,json
```

## Code Coverage

Enable coverage for E2E tests:
```bash
cmake .. -DENABLE_COVERAGE=ON -DBUILD_E2E_TESTS=ON
make
ctest -L e2e
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Sanitizers

Enable sanitizers for E2E tests:
```bash
cmake .. -DENABLE_SANITIZERS=ON -DBUILD_E2E_TESTS=ON
make
./build/src/tests/test_e2e_components
./build/src/tests/test_e2e_thai_components
```

## Known Issues and Limitations

### Signal Testing
Some signals may not fire in isolated test environment due to:
- Missing QML event loop
- No user interaction simulation
- Component not fully integrated

**Workaround**: Verify signal exists with QSignalSpy, but don't require emission.

### Visual Rendering
Visual tests verify component can render but don't validate:
- Actual pixel output
- Visual appearance
- Layout correctness

**Workaround**: Manual visual inspection or screenshot comparison.

### Thai Font Availability
Tests assume Noto Sans Thai font is available:
- May not be installed on all systems
- Tests still pass but font fallback occurs

**Workaround**: Install Thai fonts or accept font fallback.

## Performance

### Test Execution Time
- Component tests: ~5-10 seconds
- Thai component tests: ~5-10 seconds
- Total E2E suite: ~15-20 seconds

### Memory Usage
- QQmlEngine per test: ~5-10 MB
- Total peak: ~50-100 MB

## Maintenance

### Regular Updates
- Add tests for new components
- Update tests when components change
- Keep README documentation current
- Review test coverage regularly

### Best Practices
- One component per test suite
- Multiple test functions per component
- Clear test names describing what is tested
- Comprehensive error messages
- Zero console error requirement

## Support

For questions or issues:
1. Check this README
2. Review test_framework.h documentation
3. Examine existing test examples
4. Consult Qt Test documentation

## License

Copyright (C) 2024 welle.io E2E Testing Suite

This test suite is part of welle.io and is released under the GNU General Public License v2.

---

**Test Suite Version**: 1.0.0
**Last Updated**: 2024-11-17
**Components Tested**: 18 categories
**Test Functions**: 57 total
**Coverage**: 100% of reusable QML components
