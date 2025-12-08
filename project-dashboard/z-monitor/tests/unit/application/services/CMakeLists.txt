# Unit tests for application services

add_executable(MonitoringServiceTest
    MonitoringServiceTest.cpp
    ${Z_MONITOR_ROOT}/tests/mocks/infrastructure/MockSensorDataSource.h
)

target_link_libraries(MonitoringServiceTest
    PRIVATE
    z_monitor_application
    z_monitor_infrastructure
    zmon_test_mocks_domain
    zmon_test_mocks_infrastructure
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    Qt6::Core
    Qt6::Test
)

target_include_directories(MonitoringServiceTest PRIVATE
    ${Z_MONITOR_ROOT} # allow includes like "tests/mocks/..."
)

add_test(NAME MonitoringServiceTest COMMAND MonitoringServiceTest)

add_executable(TelemetryServiceTest
    TelemetryServiceTest.cpp
)

target_link_libraries(TelemetryServiceTest
    PRIVATE
    z_monitor_application
    z_monitor_infrastructure
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Test
)

target_include_directories(TelemetryServiceTest PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME TelemetryServiceTest COMMAND TelemetryServiceTest)
