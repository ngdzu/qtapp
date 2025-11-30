# CMakeLists.txt for interface/controllers unit tests

# DashboardController test executable
add_executable(test_dashboard_controller
    DashboardControllerTest.cpp
    ${CMAKE_SOURCE_DIR}/src/interface/controllers/DashboardController.cpp
)

target_link_libraries(test_dashboard_controller
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Test
    z_monitor_infrastructure
)

target_include_directories(test_dashboard_controller
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/mocks
)

# Add test to CTest
add_test(NAME test_dashboard_controller COMMAND test_dashboard_controller)

# SettingsController test executable
add_executable(SettingsControllerTest
    ${CMAKE_SOURCE_DIR}/tests/unit/interface/SettingsControllerTest.cpp
    ${CMAKE_SOURCE_DIR}/src/interface/controllers/SettingsController.cpp
)

target_link_libraries(SettingsControllerTest
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Test
    z_monitor_infrastructure
)

target_include_directories(SettingsControllerTest
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/mocks
)

add_test(NAME SettingsControllerTest COMMAND SettingsControllerTest)

# TrendsController test executable
add_executable(TrendsControllerTest
    ${CMAKE_SOURCE_DIR}/tests/unit/interface/TrendsControllerTest.cpp
    ${CMAKE_SOURCE_DIR}/src/interface/controllers/TrendsController.cpp
)

target_link_libraries(TrendsControllerTest
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Test
    z_monitor_infrastructure
)

target_include_directories(TrendsControllerTest
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/mocks
)

add_test(NAME TrendsControllerTest COMMAND TrendsControllerTest)
