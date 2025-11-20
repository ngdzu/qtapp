cmake_minimum_required(VERSION 3.16)
project(layouts-and-containers VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Widgets module
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Enable automatic MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)

# Create executable
add_executable(layouts-and-containers main.cpp)

# Link Qt6 Widgets library
target_link_libraries(layouts-and-containers PRIVATE Qt6::Widgets)

# Installation
install(TARGETS layouts-and-containers
    RUNTIME DESTINATION /opt/lesson04
)
