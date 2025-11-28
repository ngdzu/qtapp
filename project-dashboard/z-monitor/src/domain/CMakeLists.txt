# Domain Layer - Pure business logic (no Qt dependencies)

# Domain layer sources
set(DOMAIN_SOURCES
    # Monitoring bounded context
    monitoring/PatientAggregate.h
    monitoring/PatientAggregate.cpp
    monitoring/TelemetryBatch.h
    monitoring/TelemetryBatch.cpp
    monitoring/AlarmAggregate.h
    monitoring/AlarmAggregate.cpp
    monitoring/VitalRecord.h
    monitoring/WaveformSample.h
    monitoring/AlarmThreshold.h
    monitoring/AlarmSnapshot.h
    
    # Monitoring events
    monitoring/events/PatientAdmitted.h
    monitoring/events/PatientDischarged.h
    monitoring/events/TelemetryQueued.h
    monitoring/events/AlarmRaised.h
    
    # Admission bounded context
    admission/PatientIdentity.h
    admission/BedLocation.h
    
    # Provisioning bounded context
    provisioning/DeviceAggregate.h
    provisioning/DeviceAggregate.cpp
    
    # Repository interfaces
    repositories/IPatientRepository.h
    repositories/ITelemetryRepository.h
    repositories/IVitalsRepository.h
    repositories/IAlarmRepository.h
)

# Create domain layer static library
add_library(z_monitor_domain STATIC
    ${DOMAIN_SOURCES}
)

# Set include directories (project-relative paths)
target_include_directories(z_monitor_domain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Domain layer should NOT link to Qt (enforced by not linking Qt libraries)
# Domain layer is pure C++ with standard library only

