# Interface Layer - UI integration (Qt Quick dependencies)

# Interface layer sources (will be added as controllers are implemented)
# These sources are compiled into the main executable, not a separate library
# Set in parent scope so src/CMakeLists.txt can use it
# Use absolute paths relative to interface directory
set(INTERFACE_SOURCES

    # Controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/SettingsController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/SettingsController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/AuthenticationController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/AuthenticationController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/DashboardController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/DashboardController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/AlarmController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/AlarmController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/PatientController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/PatientController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/SystemController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/SystemController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/TrendsController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/TrendsController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/NotificationController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/NotificationController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/ProvisioningController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/ProvisioningController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/DiagnosticsController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/DiagnosticsController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/WaveformController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/WaveformController.cpp
)

# Export to parent scope for use in src/CMakeLists.txt
set(INTERFACE_SOURCES ${INTERFACE_SOURCES} PARENT_SCOPE)

# Note: Interface layer sources are compiled into the main executable in src/CMakeLists.txt,
# not a separate library, as they are the UI entry point
