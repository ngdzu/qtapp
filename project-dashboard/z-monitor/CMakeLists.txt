cmake_minimum_required(VERSION 3.16)

project(z-monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(z-monitor
    src/main.cpp
    src/core/MockDeviceDataService.cpp
    src/core/MockDeviceDataService.h
    src/core/IDeviceDataService.h
    src/ui/DashboardController.cpp
    src/ui/DashboardController.h
    resources.qrc
)

# Add include directories
target_include_directories(z-monitor PRIVATE src)

target_link_libraries(z-monitor
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2
)

install(TARGETS z-monitor
    RUNTIME DESTINATION /opt/z-monitor
)
