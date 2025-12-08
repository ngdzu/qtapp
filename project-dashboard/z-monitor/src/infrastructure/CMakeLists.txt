# Infrastructure Layer - Technical implementations (full Qt dependencies)

# Infrastructure layer sources
set(INFRASTRUCTURE_SOURCES

    # Logging infrastructure
    logging/ILogBackend.h
    logging/LogEntry.h
    logging/LogService.h
    logging/LogService.cpp
    logging/utils/LogFormatter.h
    logging/utils/LogFormatter.cpp
    logging/backends/CustomBackend.h
    logging/backends/CustomBackend.cpp
    logging/backends/SpdlogBackend.h
    logging/backends/SpdlogBackend.cpp

    # Platform adapters (Qt-based)
    adapters/SettingsManager.h
    adapters/SettingsManager.cpp

    # Infrastructure interfaces
    interfaces/IPatientLookupService.h
    interfaces/ITelemetryServer.h
    interfaces/ISensorDataSource.h
    interfaces/IArchiver.h

    # Authentication adapters
    authentication/MockUserManagementService.h
    authentication/MockUserManagementService.cpp
    authentication/HospitalUserManagementAdapter.h
    authentication/HospitalUserManagementAdapter.cpp

    # Sensor adapters
    sensors/SharedMemoryRingBuffer.h
    sensors/SharedMemoryRingBuffer.cpp
    sensors/SharedMemoryControlChannel.h
    sensors/SharedMemoryControlChannel.cpp
    sensors/SharedMemorySensorDataSource.h
    sensors/SharedMemorySensorDataSource.cpp
    sensors/InMemorySensorDataSource.h
    sensors/InMemorySensorDataSource.cpp

    # Caching
    caching/VitalsCache.h
    caching/VitalsCache.cpp
    caching/WaveformCache.h
    caching/WaveformCache.cpp

    # Network adapters
    network/MockNetworkManager.h
    network/MockNetworkManager.cpp
    network/RetryPolicy.h
    network/CircuitBreaker.h

    # Persistence adapters
    persistence/DatabaseManager.h
    persistence/DatabaseManager.cpp
    persistence/QueryRegistry.h
    persistence/QueryCatalog.cpp
    persistence/SqlUtils.h
    persistence/SqlUtils.cpp
    persistence/SQLiteCertificateRepository.h
    persistence/SQLiteCertificateRepository.cpp
    persistence/SQLiteActionLogRepository.h
    persistence/SQLiteActionLogRepository.cpp
    persistence/SQLitePatientRepository.h
    persistence/SQLitePatientRepository.cpp
    persistence/SQLiteVitalsRepository.h
    persistence/SQLiteVitalsRepository.cpp
    persistence/SQLiteTelemetryRepository.h
    persistence/SQLiteTelemetryRepository.cpp
    persistence/SQLiteAlarmRepository.h
    persistence/SQLiteAlarmRepository.cpp
    persistence/SQLiteAuditRepository.h
    persistence/SQLiteAuditRepository.cpp

    # ORM mappings (optional, only if USE_QXORM is enabled)
    persistence/orm/OrmRegistry.h
    persistence/orm/OrmRegistry.cpp
    persistence/orm/PatientEntity.h

    # Security
    security/CertificateManager.h
    security/CertificateManager.cpp
)

# Create infrastructure layer static library
add_library(z_monitor_infrastructure STATIC
    ${INFRASTRUCTURE_SOURCES}
)

# Ensure schema and proto are generated before building infrastructure
add_dependencies(z_monitor_infrastructure generate_schema z_monitor_proto)

# Set include directories (project-relative paths)
target_include_directories(z_monitor_infrastructure
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Infrastructure layer depends on domain and application layers
target_link_libraries(z_monitor_infrastructure
    PUBLIC
    z_monitor_application
    z_monitor_proto # Protocol Buffers generated classes for telemetry
)

# Infrastructure layer uses full Qt dependencies
target_link_libraries(z_monitor_infrastructure
    PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::Network
)

# Link spdlog if enabled
if(Z_MONITOR_USE_SPDLOG)
    target_link_libraries(z_monitor_infrastructure
        PUBLIC
        spdlog::spdlog
    )
endif()

# Link QxOrm if enabled
if(USE_QXORM)
    target_link_libraries(z_monitor_infrastructure
        PUBLIC
        QxOrm # Library target created by QxOrm's CMakeLists.txt
        Qt6::Sql
    )

    # Ensure QxOrm headers are available to infrastructure sources
    target_include_directories(z_monitor_infrastructure
        PUBLIC
        ${qxorm_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/_deps/qxorm-src/include
    )

    target_compile_definitions(z_monitor_infrastructure
        PUBLIC
        USE_QXORM
    )
endif()
