# Integration tests

# DatabaseManager smoke test
add_executable(db_smoke_test
    db_smoke_test.cpp
)

target_link_libraries(db_smoke_test
    PRIVATE
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
    GTest::gtest_main
)

target_include_directories(db_smoke_test
    PRIVATE
    ${Z_MONITOR_ROOT}
)

# Ensure schema is generated before building test
add_dependencies(db_smoke_test generate_schema)

add_test(NAME DatabaseManagerSmokeTest COMMAND db_smoke_test)

# Logging integration tests
add_subdirectory(logging)

# Persistence integration tests (database repositories, migrations, queries)
add_subdirectory(persistence)

# MonitoringService + SharedMemorySensorDataSource integration test
add_executable(monitoring_service_sensor_integration_test
    monitoring_service_sensor_integration_test.cpp
)

target_link_libraries(monitoring_service_sensor_integration_test
    PRIVATE
    z_monitor_infrastructure
    Qt6::Core
    GTest::gtest_main
)

target_include_directories(monitoring_service_sensor_integration_test
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME MonitoringServiceSensorIntegrationTest COMMAND monitoring_service_sensor_integration_test)
