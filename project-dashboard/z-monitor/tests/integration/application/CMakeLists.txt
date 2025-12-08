# Integration tests for application layer

add_executable(MonitoringWorkflowTest
    MonitoringWorkflowTest.cpp
    ${Z_MONITOR_ROOT}/tests/mocks/infrastructure/MockSensorDataSource.h
    ${Z_MONITOR_ROOT}/tests/mocks/infrastructure/MockSensorDataSource.cpp
)

target_link_libraries(MonitoringWorkflowTest
    PRIVATE
    z_monitor_application
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Test
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)

# Provide source dir macro for locating DDL files
target_compile_definitions(MonitoringWorkflowTest
    PRIVATE
    Z_MONITOR_SOURCE_DIR="${Z_MONITOR_ROOT}"
)

target_include_directories(MonitoringWorkflowTest PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME MonitoringWorkflowTest COMMAND MonitoringWorkflowTest)

add_executable(TelemetryWorkflowTest
    TelemetryWorkflowTest.cpp
)

target_link_libraries(TelemetryWorkflowTest
    PRIVATE
    z_monitor_application
    z_monitor_domain
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Test
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(TelemetryWorkflowTest PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME TelemetryWorkflowTest COMMAND TelemetryWorkflowTest)
