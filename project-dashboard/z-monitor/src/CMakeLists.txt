# Source code build - orchestrates DDD layers and creates main executable

# Add subdirectories for each DDD layer
add_subdirectory(domain)
add_subdirectory(application)
add_subdirectory(infrastructure)
add_subdirectory(interface)

# Interface layer sources are set by interface/CMakeLists.txt via PARENT_SCOPE
# INTERFACE_SOURCES variable is now available after add_subdirectory(interface)

# Main executable sources
set(MAIN_SOURCES
    main.cpp
    ${INTERFACE_SOURCES}
)

# Create main executable
add_executable(z-monitor
    ${MAIN_SOURCES}
)

# Link all layer libraries to main executable
# Note: Since layer libraries use PUBLIC linking, we could technically just link
# to z_monitor_infrastructure and get everything transitively. However, explicitly
# listing all layers makes the DDD architecture clear and enforces layer boundaries.
target_link_libraries(z-monitor
    PRIVATE
        z_monitor_domain          # Domain layer (pure C++, no Qt)
        z_monitor_application     # Application layer (Qt Core only)
        z_monitor_infrastructure   # Infrastructure layer (full Qt)
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
