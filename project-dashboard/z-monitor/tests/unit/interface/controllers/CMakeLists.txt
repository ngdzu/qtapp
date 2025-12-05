# CMakeLists.txt for interface/controllers unit tests

# DashboardController test executable
add_executable(test_dashboard_controller
    DashboardControllerTest.cpp
    ${Z_MONITOR_SOURCE_DIR}/interface/controllers/DashboardController.cpp
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
    ${Z_MONITOR_SOURCE_DIR}
    ${Z_MONITOR_ROOT}/tests/mocks
)

# Add test to CTest
add_test(NAME test_dashboard_controller COMMAND test_dashboard_controller)

# SystemController test executable
add_executable(test_system_controller
    SystemControllerTest.cpp
    ${Z_MONITOR_SOURCE_DIR}/interface/controllers/SystemController.cpp
)

target_link_libraries(test_system_controller
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Test
)

target_include_directories(test_system_controller
    PRIVATE
    ${Z_MONITOR_SOURCE_DIR}
    ${Z_MONITOR_ROOT}/tests/mocks
)

# Add test to CTest
add_test(NAME test_system_controller COMMAND test_system_controller)

# SettingsController test executable - TODO: Create SettingsControllerTest.cpp
# add_executable(SettingsControllerTest
# ${CMAKE_SOURCE_DIR}/tests/unit/interface/SettingsControllerTest.cpp
# ${CMAKE_SOURCE_DIR}/src/interface/controllers/SettingsController.cpp
# )
#
# target_link_libraries(SettingsControllerTest
# PRIVATE
# GTest::gtest
# GTest::gtest_main
# Qt6::Core
# Qt6::Test
# z_monitor_infrastructure
# )
#
# target_include_directories(SettingsControllerTest
# PRIVATE
# ${CMAKE_SOURCE_DIR}/src
# ${CMAKE_SOURCE_DIR}/tests/mocks
# )
#
# add_test(NAME SettingsControllerTest COMMAND SettingsControllerTest)

# TrendsController test executable - TODO: Create TrendsControllerTest.cpp
# add_executable(TrendsControllerTest
# ${CMAKE_SOURCE_DIR}/tests/unit/interface/TrendsControllerTest.cpp
# ${CMAKE_SOURCE_DIR}/src/interface/controllers/TrendsController.cpp
# )
#
# target_link_libraries(TrendsControllerTest
# PRIVATE
# GTest::gtest
# GTest::gtest_main
# Qt6::Core
# Qt6::Test
# z_monitor_infrastructure
# )
#
# target_include_directories(TrendsControllerTest
# PRIVATE
# ${CMAKE_SOURCE_DIR}/src
# ${CMAKE_SOURCE_DIR}/tests/mocks
# )
#
# add_test(NAME TrendsControllerTest COMMAND TrendsControllerTest)
