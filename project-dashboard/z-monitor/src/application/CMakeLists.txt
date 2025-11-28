# Application Layer - Use-case orchestration (Qt Core only for signals/slots)

# Application layer sources
set(APPLICATION_SOURCES
    services/MonitoringService.h
    services/MonitoringService.cpp
    services/AdmissionService.h
    services/AdmissionService.cpp
)

# Create application layer static library
add_library(z_monitor_application STATIC
    ${APPLICATION_SOURCES}
)

# Set include directories (project-relative paths)
target_include_directories(z_monitor_application
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Application layer depends on domain layer
target_link_libraries(z_monitor_application
    PUBLIC
        z_monitor_domain
)

# Application layer uses Qt Core only (for signals/slots, QObject)
target_link_libraries(z_monitor_application
    PUBLIC
        Qt6::Core
        Qt6::Sql
)

