cmake_minimum_required(VERSION 3.16)
project(sensor-simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Quick WebSockets REQUIRED)

qt_add_executable(sensor_simulator
    main.cpp
    Simulator.cpp
    Simulator.h
    qml.qrc
)

target_link_libraries(sensor_simulator PRIVATE Qt6::Quick Qt6::WebSockets)

install(TARGETS sensor_simulator RUNTIME DESTINATION /opt/sensor-simulator)
