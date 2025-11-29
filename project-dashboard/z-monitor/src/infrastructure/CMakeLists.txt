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
    
    # Persistence adapters
    persistence/SQLiteActionLogRepository.h
    persistence/SQLiteActionLogRepository.cpp
    persistence/generated/SchemaInfo.h  # Generated from schema/database.yaml
)

# Create infrastructure layer static library
add_library(z_monitor_infrastructure STATIC
    ${INFRASTRUCTURE_SOURCES}
)

# Ensure schema is generated before building infrastructure
add_dependencies(z_monitor_infrastructure generate_schema)

# Set include directories (project-relative paths)
target_include_directories(z_monitor_infrastructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Infrastructure layer depends on domain and application layers
target_link_libraries(z_monitor_infrastructure
    PUBLIC
        z_monitor_domain
        z_monitor_application
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

