# Domain layer sources (no Qt dependencies)
set(DOMAIN_SOURCES
    domain/monitoring/VitalRecord.h
    domain/monitoring/WaveformSample.h
    domain/monitoring/AlarmThreshold.h
    domain/monitoring/AlarmSnapshot.h
    domain/monitoring/PatientAggregate.h
    domain/monitoring/PatientAggregate.cpp
    domain/monitoring/TelemetryBatch.h
    domain/monitoring/TelemetryBatch.cpp
    domain/monitoring/AlarmAggregate.h
    domain/monitoring/AlarmAggregate.cpp
    domain/admission/PatientIdentity.h
    domain/admission/BedLocation.h
    domain/provisioning/DeviceAggregate.h
    domain/provisioning/DeviceAggregate.cpp
    domain/monitoring/events/PatientAdmitted.h
    domain/monitoring/events/PatientDischarged.h
    domain/monitoring/events/TelemetryQueued.h
    domain/monitoring/events/AlarmRaised.h
    domain/repositories/IPatientRepository.h
    domain/repositories/ITelemetryRepository.h
    domain/repositories/IVitalsRepository.h
    domain/repositories/IAlarmRepository.h
)

# Application layer sources (Qt Core only for signals/slots)
set(APPLICATION_SOURCES
    application/services/MonitoringService.h
    application/services/MonitoringService.cpp
    application/services/ISensorDataSource.h
)

# Infrastructure layer sources (Qt dependencies)
set(INFRASTRUCTURE_SOURCES
    infrastructure/logging/LogEntry.h
    infrastructure/logging/ILogBackend.h
    infrastructure/logging/utils/LogFormatter.h
    infrastructure/logging/utils/LogFormatter.cpp
    infrastructure/logging/backends/CustomBackend.h
    infrastructure/logging/backends/CustomBackend.cpp
)

# Interface layer sources (Qt Quick dependencies)
set(INTERFACE_SOURCES
    # Will be added as we implement controllers
)

set(Z_MONITOR_SOURCES
    main.cpp
    ${DOMAIN_SOURCES}
    ${APPLICATION_SOURCES}
    ${INFRASTRUCTURE_SOURCES}
    ${INTERFACE_SOURCES}
)

add_executable(z-monitor
    ${Z_MONITOR_SOURCES}
)

target_link_libraries(z-monitor
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)

target_include_directories(z-monitor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Domain layer target (for testing - no Qt dependencies)
add_library(z_monitor_domain STATIC
    domain/monitoring/PatientAggregate.cpp
    domain/monitoring/TelemetryBatch.cpp
    domain/monitoring/AlarmAggregate.cpp
    domain/provisioning/DeviceAggregate.cpp
)

target_include_directories(z_monitor_domain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Domain layer should not link to Qt
# (enforced by not linking Qt libraries)
