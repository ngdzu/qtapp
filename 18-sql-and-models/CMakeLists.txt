cmake_minimum_required(VERSION 3.16)
project(lesson18 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets Sql REQUIRED)
add_executable(lesson18 main.cpp)
target_link_libraries(lesson18 PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)
install(TARGETS lesson18 RUNTIME DESTINATION /opt/lesson18)
