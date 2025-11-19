cmake_minimum_required(VERSION 3.16)
project(qt-setup VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Widgets module
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Create executable
add_executable(qt-setup main.cpp)

# Link Qt6 Widgets
target_link_libraries(qt-setup PRIVATE Qt6::Widgets)

# Enable automoc for Qt meta-object compiler
set_target_properties(qt-setup PROPERTIES
    AUTOMOC ON
)
