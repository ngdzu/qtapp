# Unit tests for infrastructure persistence layer
# These are TRUE unit tests - no database connections, no file I/O

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

# SQLiteVitalsRepository unit tests (integration tests with in-memory SQLite)
add_executable(test_sqlite_vitals_repository
    SQLiteVitalsRepositoryTest.cpp
)

target_link_libraries(test_sqlite_vitals_repository
    PRIVATE
    test_fixtures
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
    GTest::gtest_main
)

target_include_directories(test_sqlite_vitals_repository
    PRIVATE
    ${Z_MONITOR_ROOT}/src
    ${Z_MONITOR_ROOT}/tests # For DatabaseTestFixture.h
)

add_test(NAME SQLiteVitalsRepositoryTest COMMAND test_sqlite_vitals_repository)

# SQLiteAuditRepository unit tests
add_executable(test_sqlite_audit_repository
    SQLiteAuditRepositoryTest.cpp
)

target_link_libraries(test_sqlite_audit_repository
    PRIVATE
    test_fixtures
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
    GTest::gtest_main
    GTest::gmock
)

target_include_directories(test_sqlite_audit_repository
    PRIVATE
    ${Z_MONITOR_ROOT}/src
    ${Z_MONITOR_ROOT}/tests
)

add_test(NAME SQLiteAuditRepositoryTest COMMAND test_sqlite_audit_repository)
