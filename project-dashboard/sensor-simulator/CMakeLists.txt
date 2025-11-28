cmake_minimum_required(VERSION 3.16)
project(sensor-simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Quick WebSockets QuickControls2 REQUIRED)

qt_add_executable(sensor_simulator
    main.cpp
    Simulator.cpp
    Simulator.h
    src/core/SharedMemoryWriter.cpp
    src/core/SharedMemoryWriter.h
    src/core/ControlServer.cpp
    src/core/ControlServer.h
    qml.qrc
)

target_link_libraries(sensor_simulator PRIVATE Qt6::Quick Qt6::WebSockets Qt6::QuickControls2)

# Integration test (basic structure compatibility check)
find_package(Qt6 COMPONENTS Test REQUIRED)
qt_add_executable(integration_test
    tests/integration_test.cpp
)
target_link_libraries(integration_test PRIVATE Qt6::Test Qt6::Core)

install(TARGETS sensor_simulator RUNTIME DESTINATION /opt/sensor-simulator)
