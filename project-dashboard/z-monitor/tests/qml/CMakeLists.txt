# QML Component Tests using Qt Quick Test

find_package(Qt6 COMPONENTS QuickTest QUIET)

if(NOT Qt6QuickTest_FOUND)
    message(WARNING "Qt6::QuickTest not found. Skipping QML component tests.")
    return()
endif()

# WaveformDisplay component test
qt_add_executable(WaveformDisplayTest
    test_main.cpp
)

target_link_libraries(WaveformDisplayTest
    PRIVATE
    Qt6::Quick
    Qt6::QuickTest
    Qt6::Test
)

# Set the QML test import path to find test files
set_target_properties(WaveformDisplayTest PROPERTIES
    QML_IMPORT_PATH "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Add test to CTest with proper working directory
add_test(NAME QML_WaveformDisplayTest
    COMMAND WaveformDisplayTest -import ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set test properties
set_tests_properties(QML_WaveformDisplayTest PROPERTIES
    TIMEOUT 30
    LABELS "qml;ui;waveform"
)
