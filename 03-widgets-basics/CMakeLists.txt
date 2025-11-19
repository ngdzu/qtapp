cmake_minimum_required(VERSION 3.16)
project(widgets-basics VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Widgets module
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Enable automatic MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)

# Create executable
add_executable(widgets-basics main.cpp)

# Link Qt6 Widgets library
target_link_libraries(widgets-basics PRIVATE Qt6::Widgets)

# Installation
install(TARGETS widgets-basics
    RUNTIME DESTINATION /opt/lesson03
)
