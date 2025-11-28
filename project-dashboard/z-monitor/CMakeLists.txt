cmake_minimum_required(VERSION 3.16)
project(z-monitor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)

# Optional: spdlog library for high-performance logging
# Can be enabled with: cmake -DZ_MONITOR_USE_SPDLOG=ON ..
option(Z_MONITOR_USE_SPDLOG "Enable spdlog backend for high-performance logging" OFF)

if(Z_MONITOR_USE_SPDLOG)
    # Try to find spdlog using FetchContent (recommended for header-only)
    include(FetchContent)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    FetchContent_MakeAvailable(spdlog)
    
    # Alternative: Use find_package if spdlog is installed system-wide
    # find_package(spdlog REQUIRED)
    
    message(STATUS "spdlog backend enabled")
    add_compile_definitions(Z_MONITOR_USE_SPDLOG)
else()
    message(STATUS "spdlog backend disabled (using CustomBackend)")
endif()

# Add source code subdirectory (handles all DDD layers)
add_subdirectory(src)

# Install main executable
install(TARGETS z-monitor RUNTIME DESTINATION /opt/z-monitor/)


