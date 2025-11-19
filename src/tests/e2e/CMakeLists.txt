# CMake file for welle.io e2e GUI tests
#
# This test suite provides end-to-end testing for the Qt/QML GUI components
# with comprehensive console log capture and validation.

cmake_minimum_required(VERSION 3.2)

message(STATUS "Configuring e2e GUI test suite")

# Option to build e2e tests
option(BUILD_E2E_TESTS "Build end-to-end GUI tests" ON)

if(BUILD_E2E_TESTS)
    # Find required Qt components for testing
    find_package(Qt6 COMPONENTS Test Qml Quick Core Widgets Charts REQUIRED)

    message(STATUS "e2e GUI tests enabled")

    # Enable automatic test discovery
    enable_testing()

    # Create test framework library
    add_library(e2e_test_framework STATIC
        test_framework.cpp
        test_framework.h
    )

    # Include directories for test framework
    target_include_directories(e2e_test_framework PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/backend
    )

    # Link Qt components to test framework
    target_link_libraries(e2e_test_framework PUBLIC
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
        Qt6::Widgets
    )

    # Enable C++14 standard
    target_compile_features(e2e_test_framework PUBLIC cxx_std_14)

    # Enable QML debugging for console log capture
    target_compile_definitions(e2e_test_framework PUBLIC
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
    )

    # Compiler flags for comprehensive warnings
    target_compile_options(e2e_test_framework PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # ========================================================================
    # GeneralView E2E Tests
    # ========================================================================

    add_executable(test_general_view
        test_general_view.cpp
    )

    target_link_libraries(test_general_view PRIVATE
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
    )

    target_include_directories(test_general_view PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/backend
    )

    target_compile_features(test_general_view PRIVATE cxx_std_14)

    target_compile_definitions(test_general_view PRIVATE
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    target_compile_options(test_general_view PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test to CTest
    add_test(
        NAME test_general_view
        COMMAND test_general_view
    )

    set_tests_properties(test_general_view PROPERTIES
        TIMEOUT 180
        LABELS "e2e;gui;qml;general_view;main_views"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
    )

    # Install test executable
    install(TARGETS test_general_view
        RUNTIME DESTINATION bin/tests
    )

    # ========================================================================
    # RadioView E2E Tests
    # ========================================================================

    add_executable(test_radio_view
        test_radio_view.cpp
    )

    target_link_libraries(test_radio_view PRIVATE
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
    )

    target_include_directories(test_radio_view PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/backend
    )

    target_compile_features(test_radio_view PRIVATE cxx_std_14)

    target_compile_definitions(test_radio_view PRIVATE
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    target_compile_options(test_radio_view PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test to CTest
    add_test(
        NAME test_radio_view
        COMMAND test_radio_view
    )

    set_tests_properties(test_radio_view PROPERTIES
        TIMEOUT 180
        LABELS "e2e;gui;qml;radio_view;main_views"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
    )

    # Install test executable
    install(TARGETS test_radio_view
        RUNTIME DESTINATION bin/tests
    )

    # ========================================================================
    # MainView E2E Tests
    # ========================================================================

    add_executable(test_main_view
        test_main_view.cpp
    )

    target_link_libraries(test_main_view PRIVATE
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
        Qt6::Widgets
    )

    target_include_directories(test_main_view PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/backend
    )

    target_compile_features(test_main_view PRIVATE cxx_std_14)

    target_compile_definitions(test_main_view PRIVATE
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    target_compile_options(test_main_view PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test to CTest
    add_test(
        NAME test_main_view
        COMMAND test_main_view
    )

    set_tests_properties(test_main_view PROPERTIES
        TIMEOUT 180
        LABELS "e2e;gui;qml;main_view;main_views"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
    )

    # Install test executable
    install(TARGETS test_main_view
        RUNTIME DESTINATION bin/tests
    )

    # ========================================================================
    # InfoPage E2E Tests
    # ========================================================================

    add_executable(test_info_page
        test_info_page.cpp
    )

    target_link_libraries(test_info_page PRIVATE
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
    )

    target_include_directories(test_info_page PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/backend
    )

    target_compile_features(test_info_page PRIVATE cxx_std_14)

    target_compile_definitions(test_info_page PRIVATE
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    target_compile_options(test_info_page PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test to CTest
    add_test(
        NAME test_info_page
        COMMAND test_info_page
    )

    set_tests_properties(test_info_page PROPERTIES
        TIMEOUT 180
        LABELS "e2e;gui;qml;info_page;main_views"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
    )

    # Install test executable
    install(TARGETS test_info_page
        RUNTIME DESTINATION bin/tests
    )

    # ========================================================================
    # Expert View E2E Tests
    # ========================================================================

    add_executable(test_expert_view
        test_expert_view.cpp
    )

    target_link_libraries(test_expert_view PRIVATE
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
        Qt6::Widgets
        Qt6::Charts
    )

    target_include_directories(test_expert_view PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/backend
    )

    target_compile_features(test_expert_view PRIVATE cxx_std_14)

    target_compile_definitions(test_expert_view PRIVATE
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    target_compile_options(test_expert_view PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test to CTest
    add_test(
        NAME test_expert_view
        COMMAND test_expert_view
    )

    set_tests_properties(test_expert_view PROPERTIES
        TIMEOUT 180
        LABELS "e2e;gui;qml;expert_view;integration"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
    )

    # Install test executable
    install(TARGETS test_expert_view
        RUNTIME DESTINATION bin/tests
    )

    # ========================================================================
    # MOT View E2E Tests
    # ========================================================================

    add_executable(test_mot_view
        test_mot_view.cpp
    )

    target_link_libraries(test_mot_view PRIVATE
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
        Qt6::Widgets
    )

    target_include_directories(test_mot_view PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/backend
    )

    target_compile_features(test_mot_view PRIVATE cxx_std_14)

    target_compile_definitions(test_mot_view PRIVATE
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    target_compile_options(test_mot_view PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test to CTest
    add_test(
        NAME test_mot_view
        COMMAND test_mot_view
    )

    set_tests_properties(test_mot_view PROPERTIES
        TIMEOUT 180
        LABELS "e2e;gui;qml;mot_view;integration"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
    )

    # Install test executable
    install(TARGETS test_mot_view
        RUNTIME DESTINATION bin/tests
    )

    # ========================================================================
    # Reusable Components E2E Tests
    # ========================================================================

    add_executable(test_components
        test_components.cpp
    )

    target_link_libraries(test_components PRIVATE
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
        Qt6::Widgets
    )

    target_include_directories(test_components PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/welle-gui/QML
        ${CMAKE_SOURCE_DIR}/src/backend
    )

    target_compile_features(test_components PRIVATE cxx_std_14)

    target_compile_definitions(test_components PRIVATE
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    target_compile_options(test_components PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test to CTest
    add_test(
        NAME test_components
        COMMAND test_components
    )

    set_tests_properties(test_components PROPERTIES
        TIMEOUT 180
        LABELS "e2e;gui;qml;components;reusable"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
    )

    # Install test executable
    install(TARGETS test_components
        RUNTIME DESTINATION bin/tests
    )

    # ========================================================================
    # Thai Components E2E Tests
    # ========================================================================

    add_executable(test_thai_components
        test_thai_components.cpp
    )

    target_link_libraries(test_thai_components PRIVATE
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::Core
        Qt6::Widgets
    )

    target_include_directories(test_thai_components PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/welle-gui
        ${CMAKE_SOURCE_DIR}/src/welle-gui/QML
        ${CMAKE_SOURCE_DIR}/src/backend
        ${CMAKE_SOURCE_DIR}/src/backend/thailand-compliance
    )

    target_compile_features(test_thai_components PRIVATE cxx_std_14)

    target_compile_definitions(test_thai_components PRIVATE
        QT_QML_DEBUG
        QT_MESSAGELOGCONTEXT
        E2E_TESTING_MODE=1
        THAILAND_DAB_SUPPORT=1
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

    target_compile_options(test_thai_components PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test to CTest
    add_test(
        NAME test_thai_components
        COMMAND test_thai_components
    )

    set_tests_properties(test_thai_components PROPERTIES
        TIMEOUT 180
        LABELS "e2e;gui;qml;thai;components;unicode"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
    )

    # Install test executable
    install(TARGETS test_thai_components
        RUNTIME DESTINATION bin/tests
    )

    # ========================================================================
    # Settings Pages E2E Tests (existing)
    # ========================================================================

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_settings_pages.cpp")
        add_executable(test_settings_pages
            test_settings_pages.cpp
        )

        target_link_libraries(test_settings_pages PRIVATE
            e2e_test_framework
            Qt6::Test
            Qt6::Qml
            Qt6::Quick
            Qt6::Core
            Qt6::Widgets
        )

        target_include_directories(test_settings_pages PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/welle-gui
            ${CMAKE_SOURCE_DIR}/src/backend
        )

        target_compile_features(test_settings_pages PRIVATE cxx_std_14)

        target_compile_definitions(test_settings_pages PRIVATE
            QT_QML_DEBUG
            QT_MESSAGELOGCONTEXT
            E2E_TESTING_MODE=1
        )

        target_compile_options(test_settings_pages PRIVATE
            -Wall
            -Wextra
            -pedantic
            -g
        )

        # Add test to CTest
        add_test(
            NAME test_settings_pages
            COMMAND test_settings_pages
        )

        set_tests_properties(test_settings_pages PROPERTIES
            TIMEOUT 180
            LABELS "e2e;gui;qml;settings;integration"
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
        )

        # Install test executable
        install(TARGETS test_settings_pages
            RUNTIME DESTINATION bin/tests
        )
    endif()

    # ========================================================================
    # Main Test Runner Executable (if needed)
    # ========================================================================

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main_test_runner.cpp")
        add_executable(e2e_gui_tests
            main_test_runner.cpp
        )

        # Link test framework
        target_link_libraries(e2e_gui_tests PRIVATE
            e2e_test_framework
            Qt6::Test
            Qt6::Qml
            Qt6::Quick
            Qt6::Core
            Qt6::Widgets
        )

        # Include directories for test executable
        target_include_directories(e2e_gui_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/welle-gui
            ${CMAKE_SOURCE_DIR}/src/backend
        )

        # Enable C++14 standard
        target_compile_features(e2e_gui_tests PRIVATE cxx_std_14)

        # Enable QML debugging
        target_compile_definitions(e2e_gui_tests PRIVATE
            QT_QML_DEBUG
            QT_MESSAGELOGCONTEXT
            E2E_TESTING_MODE=1
        )

        # Compiler flags
        target_compile_options(e2e_gui_tests PRIVATE
            -Wall
            -Wextra
            -pedantic
            -g
        )

        # Add to CTest for automatic test discovery
        add_test(
            NAME e2e_gui_tests
            COMMAND e2e_gui_tests
        )

        # Set test properties
        set_tests_properties(e2e_gui_tests PROPERTIES
            TIMEOUT 120
            LABELS "e2e;gui;qml;integration"
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=true"
        )

        # Install test executable
        install(TARGETS e2e_gui_tests
            RUNTIME DESTINATION bin/tests
        )
    endif()

    # ========================================================================
    # Optional: Sanitizers for e2e tests
    # ========================================================================

    if(ENABLE_SANITIZERS)
        message(STATUS "Enabling sanitizers for e2e tests")

        set(SANITIZER_FLAGS
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
        )

        # Apply to main view tests
        target_compile_options(test_general_view PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_general_view PRIVATE ${SANITIZER_FLAGS})

        target_compile_options(test_radio_view PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_radio_view PRIVATE ${SANITIZER_FLAGS})

        target_compile_options(test_main_view PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_main_view PRIVATE ${SANITIZER_FLAGS})

        target_compile_options(test_info_page PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_info_page PRIVATE ${SANITIZER_FLAGS})

        # Apply to expert view test
        target_compile_options(test_expert_view PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_expert_view PRIVATE ${SANITIZER_FLAGS})

        # Apply to MOT view test
        target_compile_options(test_mot_view PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_mot_view PRIVATE ${SANITIZER_FLAGS})

        # Apply to component tests
        target_compile_options(test_components PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_components PRIVATE ${SANITIZER_FLAGS})

        # Apply to Thai component tests
        target_compile_options(test_thai_components PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_thai_components PRIVATE ${SANITIZER_FLAGS})

        # Apply to settings pages test if it exists
        if(TARGET test_settings_pages)
            target_compile_options(test_settings_pages PRIVATE ${SANITIZER_FLAGS})
            target_link_libraries(test_settings_pages PRIVATE ${SANITIZER_FLAGS})
        endif()

        # Apply to main test runner if it exists
        if(TARGET e2e_gui_tests)
            target_compile_options(e2e_gui_tests PRIVATE ${SANITIZER_FLAGS})
            target_link_libraries(e2e_gui_tests PRIVATE ${SANITIZER_FLAGS})
        endif()
    endif()

    # ========================================================================
    # Optional: Code Coverage for e2e tests
    # ========================================================================

    if(ENABLE_COVERAGE)
        message(STATUS "Enabling code coverage for e2e tests")

        # Apply to main view tests
        target_compile_options(test_general_view PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_general_view PRIVATE
            gcov
        )

        target_compile_options(test_radio_view PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_radio_view PRIVATE
            gcov
        )

        target_compile_options(test_main_view PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_main_view PRIVATE
            gcov
        )

        target_compile_options(test_info_page PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_info_page PRIVATE
            gcov
        )

        # Apply to expert view test
        target_compile_options(test_expert_view PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_expert_view PRIVATE
            gcov
        )

        # Apply to MOT view test
        target_compile_options(test_mot_view PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_mot_view PRIVATE
            gcov
        )

        # Apply to component tests
        target_compile_options(test_components PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_components PRIVATE
            gcov
        )

        # Apply to Thai component tests
        target_compile_options(test_thai_components PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_thai_components PRIVATE
            gcov
        )

        # Apply to settings pages test if it exists
        if(TARGET test_settings_pages)
            target_compile_options(test_settings_pages PRIVATE
                --coverage
                -fprofile-arcs
                -ftest-coverage
            )
            target_link_libraries(test_settings_pages PRIVATE
                gcov
            )
        endif()

        # Apply to main test runner if it exists
        if(TARGET e2e_gui_tests)
            target_compile_options(e2e_gui_tests PRIVATE
                --coverage
                -fprofile-arcs
                -ftest-coverage
            )
            target_link_libraries(e2e_gui_tests PRIVATE
                gcov
            )
        endif()
    endif()

    # ========================================================================
    # Summary
    # ========================================================================

    message(STATUS "")
    message(STATUS "========================================")
    message(STATUS "e2e GUI Test Suite Configuration:")
    message(STATUS "========================================")
    message(STATUS "e2e tests: ON")
    message(STATUS "QML debugging: ON")
    message(STATUS "Console log capture: ON")
    message(STATUS "Headless mode: Supported (QT_QPA_PLATFORM=offscreen)")
    message(STATUS "")
    message(STATUS "Test executables:")
    message(STATUS "  - test_general_view (GeneralView.qml - GridLayout)")
    message(STATUS "    • Component loading and initialization")
    message(STATUS "    • Dynamic component management")
    message(STATUS "    • Layout and positioning")
    message(STATUS "    • Expert mode switching")
    message(STATUS "    • Zero console errors verification")
    message(STATUS "  - test_radio_view (RadioView.qml - Service Overview)")
    message(STATUS "    • Station display and text")
    message(STATUS "    • Signal strength indicators")
    message(STATUS "    • Antenna status visualization")
    message(STATUS "    • Radio info display")
    message(STATUS "    • Zero console errors verification")
    message(STATUS "  - test_main_view (MainView.qml - Main Application Window)")
    message(STATUS "    • ApplicationWindow initialization")
    message(STATUS "    • Drawer and navigation")
    message(STATUS "    • Menu items and dialogs")
    message(STATUS "    • State transitions (fullscreen, expert)")
    message(STATUS "    • Volume controls")
    message(STATUS "    • Zero console errors verification")
    message(STATUS "  - test_info_page (InfoPage.qml - About/Info Dialog)")
    message(STATUS "    • Tab navigation")
    message(STATUS "    • Content display")
    message(STATUS "    • License information")
    message(STATUS "    • Interactive elements")
    message(STATUS "    • Zero console errors verification")
    message(STATUS "  - test_expert_view (Expert View + all expert subviews)")
    message(STATUS "    • SpectrumGraph.qml")
    message(STATUS "    • ConstellationGraph.qml")
    message(STATUS "    • ImpulseResponseGraph.qml")
    message(STATUS "    • NullSymbolGraph.qml")
    message(STATUS "    • ServiceDetails.qml")
    message(STATUS "    • TextOutputView.qml")
    message(STATUS "    • RawRecorder.qml")
    message(STATUS "  - test_mot_view (MOT Slideshow)")
    message(STATUS "    • Image loading and display")
    message(STATUS "    • Category management")
    message(STATUS "    • Navigation controls")
    message(STATUS "    • Signal handling")
    message(STATUS "  - test_components (Reusable QML Components - 37 tests)")
    message(STATUS "    • WButton, WComboBox, WSwitch, WDialog, WMenu")
    message(STATUS "    • WSpectrum, WToolTip, WTumbler")
    message(STATUS "    • MessagePopup, AnnouncementIndicator, AnnouncementHistory")
    message(STATUS "    • StationDelegate")
    message(STATUS "    • Zero console error/warning verification")
    message(STATUS "  - test_thai_components (Thai Language Components - 20 tests)")
    message(STATUS "    • ThaiServiceList (bilingual service display)")
    message(STATUS "    • ThaiEPGViewer (Thai EPG display)")
    message(STATUS "    • ThaiDateFormatter (Buddhist Era calendar)")
    message(STATUS "    • Thai font rendering (Noto Sans Thai)")
    message(STATUS "    • Unicode handling and validation")
    if(TARGET test_settings_pages)
        message(STATUS "  - test_settings_pages (comprehensive settings pages tests)")
    endif()
    if(TARGET e2e_gui_tests)
        message(STATUS "  - e2e_gui_tests (main test runner)")
    endif()
    message(STATUS "")
    message(STATUS "To run main view tests:")
    message(STATUS "  cd build && ./src/tests/e2e/test_general_view")
    message(STATUS "  cd build && ./src/tests/e2e/test_radio_view")
    message(STATUS "  cd build && ./src/tests/e2e/test_main_view")
    message(STATUS "  cd build && ./src/tests/e2e/test_info_page")
    message(STATUS "")
    message(STATUS "To run other tests:")
    message(STATUS "  cd build && ./src/tests/e2e/test_expert_view")
    message(STATUS "  cd build && ./src/tests/e2e/test_mot_view")
    message(STATUS "  cd build && ./src/tests/e2e/test_components")
    message(STATUS "  cd build && ./src/tests/e2e/test_thai_components")
    message(STATUS "")
    message(STATUS "To run in headless mode:")
    message(STATUS "  QT_QPA_PLATFORM=offscreen ./build/src/tests/e2e/test_general_view")
    message(STATUS "  QT_QPA_PLATFORM=offscreen ./build/src/tests/e2e/test_main_view")
    message(STATUS "")
    message(STATUS "To run with CTest:")
    message(STATUS "  cd build && ctest -L e2e --output-on-failure  # Run all e2e tests")
    message(STATUS "  cd build && ctest -L main_views --output-on-failure  # Run main views only")
    message(STATUS "  cd build && ctest -L components --output-on-failure  # Run component tests only")
    message(STATUS "  cd build && ctest -L thai --output-on-failure  # Run Thai tests only")
    message(STATUS "  cd build && ctest -R test_general_view --output-on-failure")
    message(STATUS "  cd build && ctest -R test_main_view --output-on-failure")
    message(STATUS "========================================")
    message(STATUS "")

else()
    message(STATUS "e2e GUI tests disabled")
endif()
