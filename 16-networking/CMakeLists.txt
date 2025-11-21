cmake_minimum_required(VERSION 3.16)
project(lesson16 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets Network REQUIRED)

add_executable(lesson16 main.cpp)
target_link_libraries(lesson16 PRIVATE Qt6::Core Qt6::Widgets Qt6::Network)

install(TARGETS lesson16 RUNTIME DESTINATION /opt/lesson16)
