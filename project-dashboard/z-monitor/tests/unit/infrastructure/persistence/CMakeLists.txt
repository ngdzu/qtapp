# Unit tests for infrastructure persistence layer

add_executable(test_schema_generation
    test_schema_generation.cpp
)

target_link_libraries(test_schema_generation
    PRIVATE
    GTest::gtest_main
)

target_include_directories(test_schema_generation
    PRIVATE
    ${Z_MONITOR_ROOT}/src # For generated/SchemaInfo.h and infrastructure includes
)

add_test(NAME SchemaGenerationTest COMMAND test_schema_generation)

# SQLitePatientRepository tests
add_executable(test_sqlite_patient_repository
    test_sqlite_patient_repository.cpp
)

target_link_libraries(test_sqlite_patient_repository
    PRIVATE
    GTest::gtest_main
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
)

target_include_directories(test_sqlite_patient_repository
    PRIVATE
    ${Z_MONITOR_ROOT} # For generated/SchemaInfo.h
)

# Link QxOrm if enabled
if(USE_QXORM)
    target_link_libraries(test_sqlite_patient_repository
        PRIVATE
        QxOrm
    )
    target_compile_definitions(test_sqlite_patient_repository
        PRIVATE
        USE_QXORM
    )
endif()

add_test(NAME SQLitePatientRepositoryTest COMMAND test_sqlite_patient_repository)

# SQLiteVitalsRepository tests
add_executable(test_sqlite_vitals_repository
    test_sqlite_vitals_repository.cpp
    ${Z_MONITOR_ROOT}/tests/mocks/infrastructure/MockDatabaseManager.cpp
)

target_link_libraries(test_sqlite_vitals_repository
    PRIVATE
    Qt6::Test
    z_monitor_infrastructure
    z_monitor_domain
    Qt6::Core
    Qt6::Sql
)

target_include_directories(test_sqlite_vitals_repository
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME SQLiteVitalsRepositoryTest COMMAND test_sqlite_vitals_repository)

# SQLiteTelemetryRepository tests
add_executable(test_sqlite_telemetry_repository
    test_sqlite_telemetry_repository.cpp
    ${Z_MONITOR_ROOT}/tests/mocks/infrastructure/MockDatabaseManager.cpp
)

target_link_libraries(test_sqlite_telemetry_repository
    PRIVATE
    Qt6::Test
    z_monitor_infrastructure
    z_monitor_domain
    Qt6::Core
    Qt6::Sql
)

target_include_directories(test_sqlite_telemetry_repository
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME SQLiteTelemetryRepositoryTest COMMAND test_sqlite_telemetry_repository)

# SQLiteAlarmRepository tests
add_executable(test_sqlite_alarm_repository
    SQLiteAlarmRepositoryTest.cpp
)

target_link_libraries(test_sqlite_alarm_repository
    PRIVATE
    GTest::gmock_main
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
)

target_include_directories(test_sqlite_alarm_repository
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME SQLiteAlarmRepositoryTest COMMAND test_sqlite_alarm_repository)

add_executable(test_query_registry
    test_query_registry.cpp
)

target_link_libraries(test_query_registry
    PRIVATE
    GTest::gtest_main
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
)

target_include_directories(test_query_registry
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME QueryRegistryTest COMMAND test_query_registry)

# Migration tests
add_executable(test_migrations
    test_migrations.cpp
)

target_link_libraries(test_migrations
    PRIVATE
    GTest::gtest_main
    Qt6::Core
    Qt6::Sql
)

target_include_directories(test_migrations
    PRIVATE
    ${Z_MONITOR_ROOT}
)

add_test(NAME MigrationTest COMMAND test_migrations)
