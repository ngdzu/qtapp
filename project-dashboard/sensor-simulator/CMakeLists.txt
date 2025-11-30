cmake_minimum_required(VERSION 3.16)
project(sensor-simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Quick QuickControls2 REQUIRED)
find_package(Qt6 COMPONENTS WebSockets QUIET)

set(SIMULATOR_SOURCES
    main.cpp
    Simulator.cpp
    Simulator.h
    src/core/SharedMemoryWriter.cpp
    src/core/SharedMemoryWriter.h
    src/core/ControlServer.cpp
    src/core/ControlServer.h
    qml.qrc
)

# Add WebSocket support conditionally
if(Qt6WebSockets_FOUND)
    message(STATUS "Qt6WebSockets found - enabling WebSocket support")
    list(APPEND SIMULATOR_SOURCES)
    set(WEBSOCKETS_ENABLED ON)
else()
    message(WARNING "Qt6WebSockets not found - WebSocket support disabled (shared memory only)")
    set(WEBSOCKETS_ENABLED OFF)
endif()

qt_add_executable(sensor_simulator ${SIMULATOR_SOURCES})

target_link_libraries(sensor_simulator PRIVATE Qt6::Quick Qt6::QuickControls2)

if(Qt6WebSockets_FOUND)
    target_link_libraries(sensor_simulator PRIVATE Qt6::WebSockets)
    target_compile_definitions(sensor_simulator PRIVATE WEBSOCKETS_ENABLED)
endif()

# Integration test (basic structure compatibility check)
find_package(Qt6 COMPONENTS Test REQUIRED)
qt_add_executable(integration_test
    tests/integration_test.cpp
)
target_link_libraries(integration_test PRIVATE Qt6::Test Qt6::Core)
target_include_directories(integration_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS sensor_simulator RUNTIME DESTINATION /opt/sensor-simulator)
