# Executable for the desktop application
add_executable(alm-desktop
    main.cpp
)

# Link both the domain engine and the Qt UI library
target_link_libraries(alm-desktop
    PRIVATE
        alm_domain
        alm_ui
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

# Set Windows application subsystem if compiling on MSVC/MinGW
if(WIN32)
    set_property(TARGET alm-desktop PROPERTY WIN32_EXECUTABLE ON)
endif()
