cmake_minimum_required(VERSION 3.16)
project(lesson17 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets Concurrent REQUIRED)
add_executable(lesson17 main.cpp)
target_link_libraries(lesson17 PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent)
install(TARGETS lesson17 RUNTIME DESTINATION /opt/lesson17)
