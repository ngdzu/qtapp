
cmake_minimum_required(VERSION 3.16)
project(qtapp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(d0
    src/main.cpp
    src/mainwindow.cpp
)

target_link_libraries(d0 Qt6::Core Qt6::Widgets)

add_subdirectory(z-monitor)

