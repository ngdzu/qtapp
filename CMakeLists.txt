
cmake_minimum_required(VERSION 3.14)
project(d0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 COMPONENTS Core Widgets Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)   # Enable automatic processing of UI files
set(CMAKE_AUTORCC ON)   # Enable automatic processing of resource files
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(d0 
    src/main.cpp 
    src/mainwindow.cpp
    src/customapplication.cpp
    src/chatoverlay.cpp
)



target_link_libraries(d0 Qt6::Core Qt6::Widgets Qt6::Network)

