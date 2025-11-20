cmake_minimum_required(VERSION 3.16)
project(lesson06-events VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

add_executable(lesson06-events main.cpp)
target_link_libraries(lesson06-events PRIVATE Qt6::Core Qt6::Widgets)

install(TARGETS lesson06-events RUNTIME DESTINATION /opt/lesson06)
