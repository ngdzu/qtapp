cmake_minimum_required(VERSION 3.16)
project(lesson13 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Quick QuickControls2 REQUIRED)

add_executable(lesson13
    main.cpp
    qml.qrc
)

target_link_libraries(lesson13 PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
)

install(TARGETS lesson13 RUNTIME DESTINATION /opt/lesson13)
