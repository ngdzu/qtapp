# Interface Layer - UI integration (Qt Quick dependencies)

# Interface layer sources (will be added as controllers are implemented)
# These sources are compiled into the main executable, not a separate library
# Set in parent scope so src/CMakeLists.txt can use it
set(INTERFACE_SOURCES
    # Controllers
    controllers/SettingsController.h
    controllers/SettingsController.cpp
    # Example:
    # controllers/DashboardController.h
    # controllers/DashboardController.cpp
    # controllers/AlarmController.h
    # controllers/AlarmController.cpp
    # ...
)

# Export to parent scope for use in src/CMakeLists.txt
set(INTERFACE_SOURCES ${INTERFACE_SOURCES} PARENT_SCOPE)

# Note: Interface layer sources are compiled into the main executable in src/CMakeLists.txt,
# not a separate library, as they are the UI entry point

