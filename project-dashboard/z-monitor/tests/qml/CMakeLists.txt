# QML Component Tests using Qt Quick Test

find_package(Qt6 COMPONENTS QuickTest Charts QUIET)

if(NOT Qt6QuickTest_FOUND)
    message(WARNING "Qt6::QuickTest not found. Skipping QML component tests.")
    return()
endif()

if(NOT Qt6Charts_FOUND)
    message(WARNING "Qt6::Charts not found. Some QML component tests may fail.")
endif()

# Helper function to create QML test executables
function(add_qml_test TEST_NAME QML_FILE TEST_LABELS)
    # Create a unique main file for this test from template to avoid CMake string escaping issues
    set(MAIN_FILE "${CMAKE_CURRENT_BINARY_DIR}/test_main_${TEST_NAME}.cpp")
    set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_main.in.cpp ${MAIN_FILE} @ONLY)

    qt_add_executable(${TEST_NAME}
        ${MAIN_FILE}
    )

    target_link_libraries(${TEST_NAME}
        PRIVATE
        Qt6::Quick
        Qt6::QuickTest
        Qt6::Test
        Qt6::Widgets
    )

    # Link Qt6::Charts if available (needed for TrendPanel, TrendsView)
    if(Qt6Charts_FOUND)
        target_link_libraries(${TEST_NAME} PRIVATE Qt6::Charts)
    endif()

    # Set the QML test file as a source to run
    set_target_properties(${TEST_NAME} PROPERTIES
        QML_IMPORT_PATH "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # Add test to CTest - specify the exact QML file to run
    add_test(NAME QML_${TEST_NAME}
        COMMAND ${TEST_NAME} -input ${CMAKE_CURRENT_SOURCE_DIR}/${QML_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Set test properties
    set_tests_properties(QML_${TEST_NAME} PROPERTIES
        TIMEOUT 30
        LABELS "${TEST_LABELS}"
    )
endfunction()

# Component Tests
add_qml_test(WaveformDisplayTest "components/tst_WaveformDisplayTest.qml" "qml;ui;component;waveform")
add_qml_test(AlarmPanelTest "components/tst_AlarmPanelTest.qml" "qml;ui;component;alarm")
add_qml_test(VitalTileTest "components/tst_VitalTileTest.qml" "qml;ui;component;vital")
add_qml_test(TrendPanelTest "components/tst_TrendPanelTest.qml" "qml;ui;component;trend")
add_qml_test(PatientBannerTest "components/tst_PatientBannerTest.qml" "qml;ui;component;patient")

# View Tests
add_qml_test(TrendsViewTest "views/tst_TrendsViewTest.qml" "qml;ui;view;trends")

# Custom target to run all QML tests
add_custom_target(qml_tests
    DEPENDS
    WaveformDisplayTest
    AlarmPanelTest
    VitalTileTest
    TrendPanelTest
    PatientBannerTest
    TrendsViewTest
    COMMENT "Building all QML tests"
)
