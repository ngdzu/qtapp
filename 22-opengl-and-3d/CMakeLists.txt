cmake_minimum_required(VERSION 3.16)
project(lesson22 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets REQUIRED)
add_executable(lesson22 main.cpp)
target_link_libraries(lesson22 PRIVATE Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
install(TARGETS lesson22 RUNTIME DESTINATION /opt/lesson22)
