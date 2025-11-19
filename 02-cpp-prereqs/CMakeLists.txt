cmake_minimum_required(VERSION 3.16)
project(cpp-prereqs VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Core and Widgets modules
find_package(Qt6 COMPONENTS Core REQUIRED)

# Create executable
add_executable(cpp-prereqs main.cpp)

# Link Qt6 libraries
target_link_libraries(cpp-prereqs PRIVATE 
    Qt6::Core
)

# Enable automoc for Qt meta-object compiler
set_target_properties(cpp-prereqs PROPERTIES
    AUTOMOC ON
)
