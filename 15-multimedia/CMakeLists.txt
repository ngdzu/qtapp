cmake_minimum_required(VERSION 3.16)
project(lesson15 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets Multimedia MultimediaWidgets REQUIRED)

add_executable(lesson15 main.cpp)
target_link_libraries(lesson15 PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)

install(TARGETS lesson15 RUNTIME DESTINATION /opt/lesson15)
