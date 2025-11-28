cmake_minimum_required(VERSION 3.16)
project(z-monitor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)

# Add source code subdirectory (handles all DDD layers)
add_subdirectory(src)

# Install main executable
install(TARGETS z-monitor RUNTIME DESTINATION /opt/z-monitor/)


