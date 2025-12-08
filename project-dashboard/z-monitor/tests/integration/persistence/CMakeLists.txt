# Integration tests for infrastructure persistence layer
# These tests connect to a real database (in-memory SQLite) and test complete data flow

# SQLitePatientRepository integration tests
add_executable(integration_test_sqlite_patient_repository
    test_sqlite_patient_repository.cpp
)

target_link_libraries(integration_test_sqlite_patient_repository
    PRIVATE
    GTest::gtest_main
    test_fixtures
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
)

target_include_directories(integration_test_sqlite_patient_repository
    PRIVATE
    ${Z_MONITOR_ROOT} # For generated/SchemaInfo.h
)

# Link QxOrm if enabled
if(USE_QXORM)
    target_link_libraries(integration_test_sqlite_patient_repository
        PRIVATE
        QxOrm
    )
    target_compile_definitions(integration_test_sqlite_patient_repository
        PRIVATE
        USE_QXORM
    )
endif()

add_test(NAME Integration_SQLitePatientRepositoryTest COMMAND integration_test_sqlite_patient_repository)

# SQLiteVitalsRepository integration tests
add_executable(integration_test_sqlite_vitals_repository
    test_sqlite_vitals_repository.cpp
    ${Z_MONITOR_ROOT}/tests/mocks/infrastructure/MockDatabaseManager.cpp
)

target_link_libraries(integration_test_sqlite_vitals_repository
    PRIVATE
    Qt6::Test
    test_fixtures
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
)

target_include_directories(integration_test_sqlite_vitals_repository
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME Integration_SQLiteVitalsRepositoryTest COMMAND integration_test_sqlite_vitals_repository)

# SQLiteTelemetryRepository integration tests
add_executable(integration_test_sqlite_telemetry_repository
    test_sqlite_telemetry_repository.cpp
    ${Z_MONITOR_ROOT}/tests/mocks/infrastructure/MockDatabaseManager.cpp
)

target_link_libraries(integration_test_sqlite_telemetry_repository
    PRIVATE
    Qt6::Test
    test_fixtures
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
)

target_include_directories(integration_test_sqlite_telemetry_repository
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME Integration_SQLiteTelemetryRepositoryTest COMMAND integration_test_sqlite_telemetry_repository)

# SQLiteAlarmRepository integration tests
add_executable(integration_test_sqlite_alarm_repository
    test_sqlite_alarm_repository.cpp
)

target_link_libraries(integration_test_sqlite_alarm_repository
    PRIVATE
    test_fixtures
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
    GTest::gtest_main
)

target_include_directories(integration_test_sqlite_alarm_repository
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME Integration_SQLiteAlarmRepositoryTest COMMAND integration_test_sqlite_alarm_repository)

#
# add_test(NAME Integration_SQLiteAlarmRepositoryTest COMMAND integration_test_sqlite_alarm_repository)

# QueryRegistry integration tests
add_executable(integration_test_query_registry
    test_query_registry.cpp
)

target_link_libraries(integration_test_query_registry
    PRIVATE
    GTest::gtest_main
    test_fixtures
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
)

target_include_directories(integration_test_query_registry
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME Integration_QueryRegistryTest COMMAND integration_test_query_registry)

# Database migration integration tests
add_executable(integration_test_migrations
    test_migrations.cpp
)

target_link_libraries(integration_test_migrations
    PRIVATE
    GTest::gtest_main
    Qt6::Core
    Qt6::Sql
    test_fixtures
)

target_include_directories(integration_test_migrations
    PRIVATE
    ${Z_MONITOR_ROOT}
)

target_compile_definitions(integration_test_migrations
    PRIVATE
    Z_MONITOR_SOURCE_DIR="${Z_MONITOR_ROOT}"
)

add_test(NAME Integration_MigrationTest COMMAND integration_test_migrations)
