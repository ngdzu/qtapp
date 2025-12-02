# Domain Layer - Pure business logic (no Qt dependencies)

# Common utilities (pure C++ helpers shared across layers)
add_subdirectory(common)

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
    monitoring/events/PatientTransferred.h
    monitoring/events/TelemetryQueued.h
    monitoring/events/AlarmRaised.h

    # Domain events infrastructure
    events/IDomainEvent.h
    events/DomainEventDispatcher.h
    events/DomainEventDispatcher.cpp

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
    repositories/IProvisioningRepository.h
    repositories/IUserRepository.h
    repositories/IAuditRepository.h
    repositories/IActionLogRepository.h

    # External service interfaces (uses Qt for async communication)
    interfaces/IUserManagementService.h

    # Security (RBAC)
    security/Permission.h
    security/UserRole.h
    security/PermissionRegistry.h
    security/PermissionRegistry.cpp

    # Constants
    constants/ActionTypes.h
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
# EXCEPTION: IUserManagementService uses Qt types (QObject, QString) for async communication
# This is an exception for external service interfaces that require Qt signals/slots
# All other domain code must remain Qt-free

# Link Qt Core for IUserManagementService interface (exception to no-Qt rule)
target_link_libraries(z_monitor_domain
    PUBLIC
    Qt6::Core
)
