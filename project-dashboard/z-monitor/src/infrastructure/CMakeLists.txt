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
)

# Create infrastructure layer static library
add_library(z_monitor_infrastructure STATIC
    ${INFRASTRUCTURE_SOURCES}
)

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
)

# Link spdlog if enabled
if(Z_MONITOR_USE_SPDLOG)
    target_link_libraries(z_monitor_infrastructure
        PUBLIC
            spdlog::spdlog
    )
endif()

