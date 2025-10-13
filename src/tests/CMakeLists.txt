# CMake file for welle.io tests including Thailand compliance tests

cmake_minimum_required(VERSION 3.2)

# Option to build Thailand compliance tests
option(BUILD_THAILAND_TESTS "Build Thailand DAB+ compliance tests" ON)

if(BUILD_THAILAND_TESTS)
    message(STATUS "Thailand compliance tests enabled")

    # Thailand compliance test executable
    add_executable(test_thailand_compliance
        test_thailand_runner.cpp
        thailand_compliance_tests.cpp
        thailand_compliance_tests.h
    )

    # Include directories
    target_include_directories(test_thailand_compliance PRIVATE
        ${CMAKE_SOURCE_DIR}/src/backend
        ${CMAKE_SOURCE_DIR}/src/various
        ${CMAKE_SOURCE_DIR}/src/backend/thailand-compliance
    )

    # Link libraries
    target_link_libraries(test_thailand_compliance
        # Assuming these are available from main build
        backend_common
        thailand_compliance
    )

    # Compiler features
    target_compile_features(test_thailand_compliance PRIVATE cxx_std_14)

    # Add test definitions
    target_compile_definitions(test_thailand_compliance PRIVATE
        THAILAND_DAB_SUPPORT=1
        TESTING_MODE=1
    )

    # Add to CTest if enabled
    if(BUILD_TESTING)
        enable_testing()
        add_test(
            NAME thailand_compliance
            COMMAND test_thailand_compliance
        )
        set_tests_properties(thailand_compliance PROPERTIES
            TIMEOUT 60
        )
    endif()

    # Install test executable
    install(TARGETS test_thailand_compliance
        RUNTIME DESTINATION bin/tests
    )

    message(STATUS "Thailand compliance test suite configured")

else()
    message(STATUS "Thailand compliance tests disabled")
endif()

# Security test executable
add_executable(test_security
    test_security_runner.cpp
    security_tests.cpp
    security_tests.h
    # Include source files directly for testing
    ${CMAKE_SOURCE_DIR}/src/backend/thailand-compliance/thai_service_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/thailand-compliance/security_logger.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/charsets.cpp
    ${CMAKE_SOURCE_DIR}/src/various/thai_text_converter.cpp
)

# Include directories
target_include_directories(test_security PRIVATE
    ${CMAKE_SOURCE_DIR}/src/backend
    ${CMAKE_SOURCE_DIR}/src/various
    ${CMAKE_SOURCE_DIR}/src/backend/thailand-compliance
)

# Link libraries (using Qt6 and system libraries directly)
target_link_libraries(test_security
    Qt6::Core
    pthread
)

# Compiler features
target_compile_features(test_security PRIVATE cxx_std_14)

# Add test definitions
target_compile_definitions(test_security PRIVATE
    THAILAND_DAB_SUPPORT=1
    TESTING_MODE=1
)

# Add to CTest if enabled
if(BUILD_TESTING)
    add_test(
        NAME security_tests
        COMMAND test_security
    )
    set_tests_properties(security_tests PROPERTIES
        TIMEOUT 120
        LABELS "security;critical"
    )
endif()

# Install test executable
install(TARGETS test_security
    RUNTIME DESTINATION bin/tests
)

message(STATUS "Security test suite configured")

# ============================================================================
# Announcement Integration Tests
# ============================================================================

option(BUILD_ANNOUNCEMENT_TESTS "Build announcement integration tests" ON)

if(BUILD_ANNOUNCEMENT_TESTS)
    message(STATUS "Announcement integration tests enabled")

    # Announcement test executable
    add_executable(announcement_tests
        announcement_integration_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/backend/announcement-types.cpp
        ${CMAKE_SOURCE_DIR}/src/backend/announcement-manager.cpp
    )

    # Include directories
    target_include_directories(announcement_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/backend
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Link libraries
    target_link_libraries(announcement_tests
        pthread
    )

    # Compiler features
    target_compile_features(announcement_tests PRIVATE cxx_std_14)

    # Compiler flags for comprehensive warnings
    target_compile_options(announcement_tests PRIVATE
        -Wall
        -Wextra
        -pedantic
        -g
    )

    # Add test definitions
    target_compile_definitions(announcement_tests PRIVATE
        DEBUG_ANNOUNCEMENT=1
        TESTING_MODE=1
    )

    # Add to CTest if enabled
    if(BUILD_TESTING)
        enable_testing()
        add_test(
            NAME announcement_integration
            COMMAND announcement_tests
        )
        set_tests_properties(announcement_integration PROPERTIES
            TIMEOUT 60
            LABELS "announcement;integration;etsi"
        )
    endif()

    # Install test executable
    install(TARGETS announcement_tests
        RUNTIME DESTINATION bin/tests
    )

    message(STATUS "Announcement integration test suite configured")
    message(STATUS "  - 12 test suites covering all announcement functionality")
    message(STATUS "  - ETSI EN 300 401 compliance validation")
    message(STATUS "  - Performance benchmarks (< 1us per operation)")
    message(STATUS "  - Thread safety tests")
    message(STATUS "  - Run with: ./build/src/tests/announcement_tests")

else()
    message(STATUS "Announcement integration tests disabled")
endif()

# ============================================================================
# Optional: Code Coverage Support
# ============================================================================

option(ENABLE_COVERAGE "Enable code coverage for tests" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled")

    if(BUILD_ANNOUNCEMENT_TESTS)
        target_compile_options(announcement_tests PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(announcement_tests
            gcov
        )
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/src/tests/test_security")
        target_compile_options(test_security PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_libraries(test_security
            gcov
        )
    endif()
endif()

# ============================================================================
# Optional: Sanitizers
# ============================================================================

option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled (address, undefined)")

    set(SANITIZER_FLAGS
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )

    if(BUILD_ANNOUNCEMENT_TESTS)
        target_compile_options(announcement_tests PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(announcement_tests ${SANITIZER_FLAGS})
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/src/tests/test_security")
        target_compile_options(test_security PRIVATE ${SANITIZER_FLAGS})
        target_link_libraries(test_security ${SANITIZER_FLAGS})
    endif()
endif()

# ============================================================================
# Test Execution Helpers
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Test Suite Summary:")
message(STATUS "========================================")
message(STATUS "Thailand compliance tests: ${BUILD_THAILAND_TESTS}")
message(STATUS "Security tests: ON")
message(STATUS "Announcement tests: ${BUILD_ANNOUNCEMENT_TESTS}")
message(STATUS "Code coverage: ${ENABLE_COVERAGE}")
message(STATUS "Sanitizers: ${ENABLE_SANITIZERS}")
message(STATUS "")
message(STATUS "To run all tests:")
message(STATUS "  cd build && ctest --output-on-failure")
message(STATUS "")
message(STATUS "To run specific tests:")
message(STATUS "  ./build/src/tests/announcement_tests")
message(STATUS "  ./build/src/tests/test_security")
message(STATUS "  ./build/src/tests/test_thailand_compliance")
message(STATUS "========================================")
