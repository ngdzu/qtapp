# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 COMPONENTS Core Widgets Network REQUIRED)

set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(MyProject 
    src/main.cpp 
    src/mainwindow.cpp 
    src/serialize_example.cpp
)



target_link_libraries(MyProject Qt6::Core Qt6::Widgets Qt6::Network)

