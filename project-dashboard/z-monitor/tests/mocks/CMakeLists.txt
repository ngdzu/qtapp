# Mock objects for testing

# Infrastructure mocks
add_library(zmon_test_mocks_infrastructure STATIC
    infrastructure/MockTelemetryServer.cpp
    infrastructure/MockSensorDataSource.cpp
    infrastructure/MockPatientLookupService.cpp
)

target_include_directories(zmon_test_mocks_infrastructure
    PUBLIC
    ${Z_MONITOR_ROOT} # z-monitor root (for src/ includes)
    ${CMAKE_CURRENT_SOURCE_DIR}/.. # tests root (for mocks/ includes)
)

target_link_libraries(zmon_test_mocks_infrastructure
    PUBLIC
    z_monitor_infrastructure
    z_monitor_domain # MockPatientLookupService needs domain interface
    Qt6::Core
    Qt6::Network
)

# Domain mocks
add_library(zmon_test_mocks_domain STATIC
    domain/MockPatientRepository.cpp
)

target_include_directories(zmon_test_mocks_domain
    PUBLIC
    ${Z_MONITOR_ROOT} # z-monitor root (for src/ includes)
    ${CMAKE_CURRENT_SOURCE_DIR}/.. # tests root (for mocks/ includes)
)

# Link to domain library to get interface definitions
target_link_libraries(zmon_test_mocks_domain
    PUBLIC
    z_monitor_domain # Provides IPatientRepository interface and PatientAggregate
    zmon_domain_common # Provides Result type
)

# Combined mocks library (for convenience)
add_library(zmon_test_mocks INTERFACE)

target_link_libraries(zmon_test_mocks
    INTERFACE
    zmon_test_mocks_infrastructure
    zmon_test_mocks_domain
)
