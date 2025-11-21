cmake_minimum_required(VERSION 3.16)
project(lesson10-delegates VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

add_executable(lesson10-delegates main.cpp)
target_link_libraries(lesson10-delegates PRIVATE Qt6::Core Qt6::Widgets)

install(TARGETS lesson10-delegates RUNTIME DESTINATION /opt/lesson10)
