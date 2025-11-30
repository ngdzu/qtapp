# Source code build - orchestrates DDD layers and creates main executable

# Add subdirectories for each DDD layer
add_subdirectory(domain)
add_subdirectory(application)
add_subdirectory(infrastructure)
add_subdirectory(interface)

# Interface layer sources are set by interface/CMakeLists.txt via PARENT_SCOPE
# INTERFACE_SOURCES variable is now available after add_subdirectory(interface)

# QML Resources
set(QML_RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/qml/qml.qrc
)

# Main executable sources
set(MAIN_SOURCES
    main.cpp
    ${INTERFACE_SOURCES}
    ${QML_RESOURCES}
)

# Create main executable
add_executable(z-monitor
    ${MAIN_SOURCES}
)

# Link all layer libraries to main executable
# Note: Infrastructure layer transitively provides domain and application layers
# via PUBLIC linking. Only infrastructure and Qt GUI libraries need to be linked directly.
target_link_libraries(z-monitor
    PRIVATE
    z_monitor_infrastructure # Infrastructure layer (provides domain + application transitively)
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

# Set include directories for main executable (project-relative paths)
target_include_directories(z-monitor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Ensure proto and schema are generated before building main executable
add_dependencies(z-monitor z_monitor_proto generate_schema)
