# CMake file for welle.io resources including Thailand DAB+ support

cmake_minimum_required(VERSION 3.2)

# Option to enable Thailand DAB+ support
option(THAILAND_DAB_SUPPORT "Enable Thailand DAB+ compliance support" ON)

# Install Thailand resource files if enabled
if(THAILAND_DAB_SUPPORT)
    message(STATUS "Thailand DAB+ support enabled")

    # Install Thailand configuration files
    install(DIRECTORY thailand/
        DESTINATION share/welle-io/thailand
        FILES_MATCHING
        PATTERN "*.json"
        PATTERN "*.txt"
    )

    # Add Thailand resources to Qt resource system if Qt is available
    if(Qt5Core_FOUND OR Qt6Core_FOUND)
        # Create Qt resource file for Thailand resources
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/thailand.qrc.in
            ${CMAKE_CURRENT_BINARY_DIR}/thailand.qrc
            @ONLY
        )

        # Add to Qt resources
        if(Qt6Core_FOUND)
            qt6_add_resources(THAILAND_RESOURCES
                ${CMAKE_CURRENT_BINARY_DIR}/thailand.qrc
            )
        elseif(Qt5Core_FOUND)
            qt5_add_resources(THAILAND_RESOURCES
                ${CMAKE_CURRENT_BINARY_DIR}/thailand.qrc
            )
        endif()

        # Make Thailand resources available to parent scope
        set(THAILAND_RESOURCES ${THAILAND_RESOURCES} PARENT_SCOPE)

        message(STATUS "Thailand resources added to Qt resource system")
    endif()

    # Add compiler definitions for Thailand support
    add_compile_definitions(
        THAILAND_DAB_SUPPORT=1
        NBTC_COMPLIANCE=1
    )

else()
    message(STATUS "Thailand DAB+ support disabled")
endif()
