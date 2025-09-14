# CMake file for Thai font resources

cmake_minimum_required(VERSION 3.2)

# Option to bundle Thai fonts
option(BUNDLE_THAI_FONTS "Bundle Thai fonts in executable" ON)

if(BUNDLE_THAI_FONTS)
    message(STATUS "Thai font bundling enabled")

    # Check for Thai font files
    set(THAI_FONT_FILES
        "NotoSansThai-Regular.ttf"
        "NotoSansThai-Bold.ttf"
    )

    set(FOUND_FONTS "")

    foreach(FONT_FILE ${THAI_FONT_FILES})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${FONT_FILE}")
            list(APPEND FOUND_FONTS ${FONT_FILE})
            message(STATUS "Found Thai font: ${FONT_FILE}")
        else()
            message(WARNING "Thai font not found: ${FONT_FILE}")
            message(WARNING "Please download from https://fonts.google.com/noto/specimen/Noto+Sans+Thai")
        endif()
    endforeach()

    # Install found font files
    if(FOUND_FONTS)
        install(FILES ${FOUND_FONTS}
            DESTINATION share/welle-io/fonts
        )

        # Add to Qt resources if Qt is available
        if(Qt5Core_FOUND OR Qt6Core_FOUND)
            # Create Qt resource file for fonts
            configure_file(
                ${CMAKE_CURRENT_SOURCE_DIR}/thai_fonts.qrc.in
                ${CMAKE_CURRENT_BINARY_DIR}/thai_fonts.qrc
                @ONLY
            )

            # Add fonts to Qt resources
            if(Qt6Core_FOUND)
                qt6_add_resources(THAI_FONT_RESOURCES
                    ${CMAKE_CURRENT_BINARY_DIR}/thai_fonts.qrc
                )
            elseif(Qt5Core_FOUND)
                qt5_add_resources(THAI_FONT_RESOURCES
                    ${CMAKE_CURRENT_BINARY_DIR}/thai_fonts.qrc
                )
            endif()

            # Make font resources available to parent scope
            set(THAI_FONT_RESOURCES ${THAI_FONT_RESOURCES} PARENT_SCOPE)
        endif()

        # Register font loading in application
        add_compile_definitions(THAI_FONTS_AVAILABLE=1)

        message(STATUS "Thai fonts will be bundled: ${FOUND_FONTS}")
    else()
        message(WARNING "No Thai fonts found - Thai text may not render properly")
        message(WARNING "Download Noto Sans Thai fonts and place them in resources/fonts/")
    endif()

else()
    message(STATUS "Thai font bundling disabled")
endif()
