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
