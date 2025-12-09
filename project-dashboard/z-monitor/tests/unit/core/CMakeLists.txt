# Core unit tests

# Test: Alarm Manager (example demonstrating mock usage)
add_executable(test_alarm_manager
    test_alarm_manager.cpp
)

target_link_libraries(test_alarm_manager
    PRIVATE
    z_monitor_domain
    z_monitor_application
    z_monitor_infrastructure
    zmon_test_mocks
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
)

add_test(NAME test_alarm_manager COMMAND test_alarm_manager)
