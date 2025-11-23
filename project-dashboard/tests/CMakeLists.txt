cmake_minimum_required(VERSION 3.16)
project(dashboard-tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Test REQUIRED)

add_executable(tst_datagenerator tst_datagenerator.cpp ../src/core/MockDeviceDataService.cpp)

target_include_directories(tst_datagenerator PRIVATE ../src)

target_link_libraries(tst_datagenerator PRIVATE Qt6::Core Qt6::Test)
