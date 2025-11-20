cmake_minimum_required(VERSION 3.16)
project(signals-and-slots VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 modules
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# Enable automatic MOC (Meta-Object Compiler) - required for Q_OBJECT
set(CMAKE_AUTOMOC ON)

# Create executable
add_executable(signals-and-slots main.cpp)

# Link Qt6 libraries
target_link_libraries(signals-and-slots PRIVATE Qt6::Core Qt6::Widgets)

# Installation
install(TARGETS signals-and-slots
    RUNTIME DESTINATION /opt/lesson05
)
