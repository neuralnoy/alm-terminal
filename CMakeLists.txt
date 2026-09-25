cmake_minimum_required(VERSION 3.24)
project(ALM_Workstation VERSION 0.1.0 LANGUAGES CXX)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 6 Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# ---------------------------------------------------------
# Core ALM Domain Engine (No UI dependencies allowed here)
# ---------------------------------------------------------
add_library(alm_domain)
target_include_directories(alm_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add all the source files here (example using file globbing for convenience, 
# though explicit lists are better in production)
file(GLOB_RECURSE ALM_SOURCES "src/*.cpp")
if(ALM_SOURCES)
    target_sources(alm_domain PRIVATE ${ALM_SOURCES})
endif()

# ---------------------------------------------------------
# Qt UI Components Library
# ---------------------------------------------------------
add_library(alm_ui)
target_include_directories(alm_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ui)

# Link the domain engine and Qt to the UI library
target_link_libraries(alm_ui 
    PUBLIC 
        alm_domain 
        Qt6::Core 
        Qt6::Gui 
        Qt6::Widgets
)

# ---------------------------------------------------------
# Subdirectories for Applications and Tests
# ---------------------------------------------------------
add_subdirectory(apps/alm-desktop)

# Testing (Catch2 or GTest goes here)
enable_testing()
add_subdirectory(tests)
