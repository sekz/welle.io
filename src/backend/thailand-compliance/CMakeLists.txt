# CMake file for Thailand DAB+ Compliance Components
#
# This file is part of the welle.io Thailand DAB+ compliance implementation.

cmake_minimum_required(VERSION 3.2)

# Thailand compliance library
add_library(thailand_compliance STATIC
    nbtc_compliance_checker.cpp
    nbtc_compliance_checker.h
    thai_service_parser.cpp
    thai_service_parser.h
)

# Include directories
target_include_directories(thailand_compliance PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/backend
    ${CMAKE_SOURCE_DIR}/src/various
)

# Link with existing welle.io backend components
target_link_libraries(thailand_compliance
    backend_common
)

# Compiler features
target_compile_features(thailand_compliance PRIVATE cxx_std_14)

# Add Thailand compliance definitions
target_compile_definitions(thailand_compliance PRIVATE
    THAILAND_DAB_SUPPORT=1
    NBTC_COMPLIANCE=1
    THAI_CHARACTER_SET=1
)

# Install Thailand compliance library
install(TARGETS thailand_compliance
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(FILES
    nbtc_compliance_checker.h
    thai_service_parser.h
    DESTINATION include/welle-io/thailand-compliance
)
