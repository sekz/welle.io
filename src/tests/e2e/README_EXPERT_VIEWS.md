# Expert Views E2E GUI Tests

Comprehensive end-to-end GUI tests for welle.io Expert View components.

## Overview

This test suite provides thorough testing of all QML expert view components with:
- **Zero console error validation** - Every test verifies no QML errors or warnings
- **Component initialization testing** - Ensures all objects created successfully
- **Visual rendering verification** - Confirms components render without errors
- **Signal/property testing** - Validates data bindings and signal connections
- **Integration testing** - Tests all expert views working together

## Test Files

### 1. `test_expert_view.cpp`

Comprehensive tests for **ExpertView.qml** and all expert subviews:

#### Components Tested

- **ExpertView.qml** - Main expert view container
  - GridLayout initialization
  - Component loading infrastructure
  - Dynamic component creation

- **SpectrumGraph.qml** - RF spectrum analyzer
  - WSpectrum component initialization
  - Spectrum axis configuration
  - Signal connections (setSpectrumAxis)
  - Refresh timer functionality
  - Visual rendering

- **ConstellationGraph.qml** - DQPSK constellation diagram
  - ChartView initialization
  - Scatter series creation
  - X/Y axis setup (subcarrier, angle)
  - Signal connections (setConstellationAxis)
  - Refresh timer functionality

- **ImpulseResponseGraph.qml** - Impulse response display
  - WSpectrum component (with custom axis)
  - MouseArea hover interaction
  - Sample/time/distance conversions
  - Signal connections (setImpulseResponseAxis)
  - Refresh timer functionality

- **NullSymbolGraph.qml** - Null symbol visualization
  - WSpectrum component initialization
  - Signal connections (setNullSymbolAxis)
  - Refresh timer functionality

- **ServiceDetails.qml** - DAB service information display
  - RadioController property bindings
  - Status indicators (sync, FIC CRC, errors)
  - Color-coded status rectangles
  - Text display for all DAB parameters

- **TextOutputView.qml** - Console output view
  - TextArea initialization
  - Read-only text field
  - Debug output signal handling
  - Auto-scrolling functionality
  - Line limiting (100 lines max)

- **RawRecorder.qml** - I/Q raw data recorder
  - WTumbler component (ring buffer size)
  - Button interactions (init/save)
  - Property management (isStart, ringeBufferSize)
  - RadioController method calls

#### Test Categories

**Loading Tests** - Verify each component loads without errors:
```cpp
test_SpectrumGraph_Load()
test_ConstellationGraph_Load()
test_ImpulseResponseGraph_Load()
// ... and more
```

**Property Tests** - Validate component properties:
```cpp
test_SpectrumGraph_Properties()
test_ServiceDetails_RadioControllerProperties()
test_RawRecorder_Properties()
```

**Component Tests** - Verify child objects created:
```cpp
test_SpectrumGraph_WSpectrumComponent()
test_ConstellationGraph_ChartView()
test_ImpulseResponseGraph_WSpectrumComponent()
```

**Signal Tests** - Test signal connections:
```cpp
test_SpectrumGraph_SignalConnections()
test_NullSymbolGraph_SignalConnections()
```

**Rendering Tests** - Verify visual rendering:
```cpp
test_SpectrumGraph_Rendering()
test_ConstellationGraph_Rendering()
// ... all components
```

**Integration Test** - All expert views together:
```cpp
test_AllExpertViews_NoErrors()
```

### 2. `test_mot_view.cpp`

Comprehensive tests for **MotView.qml** (MOT Slideshow):

#### Components Tested

- **MotView.qml** - MOT slideshow container
  - ViewBaseFrame inheritance
  - Property initialization (slideList, currentCategoryId, etc.)
  - FileDialog integration
  - Category list model
  - Image component
  - Navigation buttons
  - ListView for categories

#### Test Categories

**Initialization Tests**:
```cpp
test_MotView_Load()
test_MotView_Initialization()
test_MotView_ViewBaseFrame()
```

**FileDialog Tests**:
```cpp
test_MotView_FileDialog()
test_MotView_SaveAllImagesEntry()
```

**Category Management Tests**:
```cpp
test_MotView_CategoryListModel()
test_MotView_InitialCategory()
test_MotView_CategorySelection()
test_MotView_MultipleCategories()
```

**Image Display Tests**:
```cpp
test_MotView_MotImageComponent()
test_MotView_ImageSourceUpdate()
```

**Signal Handling Tests**:
```cpp
test_MotView_MotChangedSignal()
test_MotView_MotChangedSignalWithCategory()
test_MotView_MotResetedSignal()
```

**Navigation Tests**:
```cpp
test_MotView_NavigationButtons()
test_MotView_NavigationButtonVisibility()
```

**Slideshow Tests**:
```cpp
test_MotView_MultipleSlidesInCategory()
test_MotView_SlideReplacement()
```

**Expert Mode Tests**:
```cpp
test_MotView_ExpertViewFlag()
test_MotView_SlideCountLabel()
```

**Visual Tests**:
```cpp
test_MotView_Rendering()
test_MotView_RenderingWithImages()
test_MotView_Layout()
```

**Integration Tests**:
```cpp
test_MotView_CompleteWorkflow()  // End-to-end MOT workflow
test_MotView_StressTest()        // Many images and categories
```

## Test Framework

### `test_framework.h`

Provides base class **QMLComponentTestBase** with utilities:

#### Console Capture
```cpp
// Automatic console message capture
setupConsoleCapture();  // Start capturing
verifyZeroErrors();     // Verify no errors
verifyZeroWarnings();   // Verify no warnings
teardownConsoleCapture(); // Stop capturing
```

#### Component Loading
```cpp
// Load QML from file
bool loadComponent(const QString& qmlPath);

// Load QML from string
bool loadComponentFromString(const QString& qmlCode, const QString& name);
```

#### Property Testing
```cpp
bool hasProperty(const QString& propertyName);
QVariant getProperty(const QString& propertyName);
bool setProperty(const QString& propertyName, const QVariant& value);
```

#### Signal Testing
```cpp
bool hasSignal(const QString& signalName);
```

#### Visual Testing
```cpp
bool testVisualRendering();  // Create window and render component
```

#### Interaction Testing
```cpp
bool simulateClick(int x = -1, int y = -1);  // Simulate mouse click
```

#### Utility Macros
```cpp
VERIFY_ZERO_CONSOLE_ERRORS();    // Assert no errors
VERIFY_ZERO_CONSOLE_WARNINGS();  // Assert no warnings
VERIFY_COMPONENT_LOADED();       // Assert component loaded
VERIFY_PROPERTY_EXISTS(prop);    // Assert property exists
VERIFY_SIGNAL_EXISTS(signal);    // Assert signal exists
```

### Mock Objects

Tests use mock C++ objects for QML context:

- **MockGuiHelper** - Mock guiHelper singleton
- **MockRadioController** - Mock radioController
- **MockMainWindow** - Mock mainWindow

## Building the Tests

### Prerequisites

- Qt 6 with components: Core, Qml, Quick, Test, Charts
- CMake 3.2+
- C++14 compiler

### Build Configuration

```bash
cd welle.io
mkdir build
cd build

# Configure with tests enabled
cmake .. -DBUILD_E2E_TESTS=ON

# Build all tests
cmake --build . --target test_expert_view
cmake --build . --target test_mot_view

# Optional: Enable sanitizers
cmake .. -DBUILD_E2E_TESTS=ON -DENABLE_SANITIZERS=ON

# Optional: Enable code coverage
cmake .. -DBUILD_E2E_TESTS=ON -DENABLE_COVERAGE=ON
```

### CMake Options

- `BUILD_E2E_TESTS` - Enable E2E GUI tests (default: ON)
- `ENABLE_SANITIZERS` - Enable address/undefined sanitizers (default: OFF)
- `ENABLE_COVERAGE` - Enable code coverage (default: OFF)

## Running the Tests

### Run All E2E Tests (CTest)

```bash
cd build
ctest -L e2e --output-on-failure
```

### Run Specific Test Suites

```bash
# Expert View tests
./build/src/tests/e2e/test_expert_view

# MOT View tests
./build/src/tests/e2e/test_mot_view

# With CTest
ctest -R test_expert_view --output-on-failure
ctest -R test_mot_view --output-on-failure
```

### Run in Headless Mode

For CI/CD or headless environments:

```bash
QT_QPA_PLATFORM=offscreen ./build/src/tests/e2e/test_expert_view
QT_QPA_PLATFORM=offscreen ./build/src/tests/e2e/test_mot_view
```

### Run with Verbose Output

```bash
./build/src/tests/e2e/test_expert_view -v2
./build/src/tests/e2e/test_mot_view -v2
```

### Run Specific Test Functions

```bash
# Run only SpectrumGraph tests
./build/src/tests/e2e/test_expert_view test_SpectrumGraph_Load

# Run only MOT workflow test
./build/src/tests/e2e/test_mot_view test_MotView_CompleteWorkflow
```

## Test Coverage

### Expert View Tests

| Component | Load | Properties | Rendering | Signals | Integration |
|-----------|------|------------|-----------|---------|-------------|
| ExpertView | ✓ | ✓ | - | - | ✓ |
| SpectrumGraph | ✓ | ✓ | ✓ | ✓ | ✓ |
| ConstellationGraph | ✓ | ✓ | ✓ | ✓ | ✓ |
| ImpulseResponseGraph | ✓ | ✓ | ✓ | ✓ | ✓ |
| NullSymbolGraph | ✓ | ✓ | ✓ | ✓ | ✓ |
| ServiceDetails | ✓ | ✓ | ✓ | - | ✓ |
| TextOutputView | ✓ | ✓ | ✓ | ✓ | ✓ |
| RawRecorder | ✓ | ✓ | ✓ | - | ✓ |

**Total: 35+ individual test functions**

### MOT View Tests

| Category | Test Count | Coverage |
|----------|------------|----------|
| Initialization | 3 | ✓ |
| FileDialog | 2 | ✓ |
| Category Management | 4 | ✓ |
| Image Display | 2 | ✓ |
| Signal Handling | 3 | ✓ |
| Navigation | 2 | ✓ |
| Slideshow | 2 | ✓ |
| Expert Mode | 2 | ✓ |
| Visual Rendering | 3 | ✓ |
| Integration | 2 | ✓ |

**Total: 25+ individual test functions**

## Expected Output

### Successful Test Run

```
********* Start testing of TestExpertView *********
Config: Using QtTest library 6.x.x
PASS   : TestExpertView::initTestCase()
PASS   : TestExpertView::test_ExpertView_Load()
PASS   : TestExpertView::test_SpectrumGraph_Load()
PASS   : TestExpertView::test_ConstellationGraph_Load()
...
PASS   : TestExpertView::test_AllExpertViews_NoErrors()
PASS   : TestExpertView::cleanupTestCase()
Totals: 35 passed, 0 failed, 0 skipped, 0 blacklisted, 12.3s
********* Finished testing of TestExpertView *********
```

### Console Error Detection

If a component has errors:

```
WARNING: Console errors detected:
  ERROR: file:///path/to/Component.qml:42: ReferenceError: obj is not defined
FAIL!  : TestExpertView::test_Component_Load()
   Console errors detected - see output above
```

## Continuous Integration

### GitHub Actions Example

```yaml
- name: Run Expert View E2E Tests
  run: |
    cd build
    QT_QPA_PLATFORM=offscreen ctest -R test_expert_view --output-on-failure

- name: Run MOT View E2E Tests
  run: |
    cd build
    QT_QPA_PLATFORM=offscreen ctest -R test_mot_view --output-on-failure
```

### Timeout Configuration

Tests are configured with 180s timeout (CTest properties):
- Adequate for headless rendering
- Can be adjusted in CMakeLists.txt if needed

## Debugging Failed Tests

### Enable QML Debugging

All tests are built with `QT_QML_DEBUG` enabled.

### Verbose Console Output

```bash
QT_LOGGING_RULES="*.debug=true" ./build/src/tests/e2e/test_expert_view
```

### Check Console Messages

Tests automatically capture and report:
- Debug messages (`console.log`)
- Info messages (`console.info`)
- Warnings (`console.warn`)
- Errors (`console.error`)

### Print Test Report

Each test includes a detailed report:
```
=== Test Report: SpectrumGraph ===
Console Messages: 5
Console Warnings: 0
Console Errors: 0
===========================================
```

## Maintenance

### Adding New Expert View Tests

1. Add component to `test_expert_view.cpp`
2. Create test functions following naming convention:
   ```cpp
   void test_ComponentName_Load()
   void test_ComponentName_Properties()
   void test_ComponentName_Rendering()
   ```
3. Use macros for consistent validation:
   ```cpp
   VERIFY_COMPONENT_LOADED();
   VERIFY_ZERO_CONSOLE_ERRORS();
   ```

### Updating Mock Objects

If QML components require new C++ objects:
1. Add properties/methods to mock classes
2. Register in `setupMockContext()`
3. Ensure signal emission works correctly

## Known Limitations

1. **WSpectrum Component**: Tests verify component exists but cannot validate actual spectrum data rendering (requires backend)
2. **ChartView Series**: Chart data validation requires Qt Charts to be fully initialized
3. **Image Provider**: MOT tests use mock image paths; actual image rendering not validated
4. **File Operations**: FileDialog and RawRecorder file I/O not tested (UI only)

## Future Enhancements

- [ ] Add screenshot comparison tests
- [ ] Test component animations
- [ ] Validate chart data points
- [ ] Test keyboard navigation
- [ ] Add performance benchmarks
- [ ] Test memory leak detection
- [ ] Add theme switching tests

## References

- Qt Test documentation: https://doc.qt.io/qt-6/qtest-overview.html
- Qt Quick Test: https://doc.qt.io/qt-6/qtquicktest-index.html
- welle.io repository: https://github.com/AlbrechtL/welle.io

## Support

For issues or questions:
1. Check test output for specific error messages
2. Enable verbose logging: `QT_LOGGING_RULES="*.debug=true"`
3. Review QML component source in `src/welle-gui/QML/expertviews/`
4. Check CI test results for baseline behavior

## License

Tests inherit welle.io GPL v2 license.
Copyright (C) 2025 welle.io contributors.
