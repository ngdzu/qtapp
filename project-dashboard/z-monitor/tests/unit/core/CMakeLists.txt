# Core unit tests

# Example test: Alarm Manager (or similar service test)
add_executable(test_alarm_manager
    test_alarm_manager.cpp
)

target_link_libraries(test_alarm_manager
    PRIVATE
    GTest::gtest_main
    zmon_test_mocks
    Qt6::Core
    Qt6::Test
)

target_include_directories(test_alarm_manager
    PRIVATE
    ${Z_MONITOR_ROOT}/src # z-monitor src directory (for domain/ includes)
)

add_test(NAME AlarmManagerTest COMMAND test_alarm_manager)
