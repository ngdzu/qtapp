# Domain aggregates unit tests

add_executable(PatientAggregateTest
    PatientAggregateTest.cpp
)

target_link_libraries(PatientAggregateTest
    PRIVATE
    z_monitor_domain
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(PatientAggregateTest
    PRIVATE
    ${Z_MONITOR_SOURCE_DIR}
    ${Z_MONITOR_ROOT}/tests/mocks
)

# Add to CTest
add_test(NAME PatientAggregateTest COMMAND PatientAggregateTest)

# Set test properties
set_tests_properties(PatientAggregateTest PROPERTIES
    LABELS "unit;domain;aggregate"
    TIMEOUT 30
)

# Alarm aggregate unit tests
add_executable(AlarmAggregateTest
    ${Z_MONITOR_ROOT}/tests/unit/domain/monitoring/AlarmAggregateTest.cpp
)

target_link_libraries(AlarmAggregateTest
    PRIVATE
    z_monitor_domain
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(AlarmAggregateTest
    PRIVATE
    ${Z_MONITOR_SOURCE_DIR}
    ${Z_MONITOR_ROOT}/tests/mocks
)

add_test(NAME AlarmAggregateTest COMMAND AlarmAggregateTest)

set_tests_properties(AlarmAggregateTest PROPERTIES
    LABELS "unit;domain;aggregate;alarm"
    TIMEOUT 30
)
