# Naming Conventions

## Overview

This document defines naming conventions for classes, files, and components in the Z Monitor project to ensure consistency and clarity.

## Mock Classes and Test Doubles

### Rule: Mock Classes Must Use "Mock" Prefix

**All mock implementations, test doubles, and fake objects MUST use the "Mock" prefix in their class name.**

### Examples

✅ **Correct:**
- `MockNetworkManager` - Mock implementation of network manager
- `MockTelemetryServer` - Mock implementation of telemetry server
- `MockPatientRepository` - Mock implementation of patient repository
- `MockUserManagementService` - Mock implementation of user management service

❌ **Incorrect:**
- `NetworkManager` - If it's a mock, it should be `MockNetworkManager`
- `TestNetworkManager` - Use "Mock" prefix, not "Test"
- `FakeNetworkManager` - Use "Mock" prefix for consistency

### Rationale

1. **Clarity:** The "Mock" prefix immediately indicates this is a test double, not a production implementation
2. **Consistency:** All mocks follow the same naming pattern, making them easy to identify
3. **Searchability:** Easy to find all mocks by searching for "Mock" prefix
4. **Documentation:** Clear distinction between production code and test code

### Location

- Mock classes should be placed in `tests/mocks/` directory structure
- If a mock is temporarily in `src/` for development, it MUST still use "Mock" prefix
- Production implementations should NOT use "Mock" prefix

### When to Use Mock Prefix

Use "Mock" prefix when:
- ✅ Implementing a test double for an interface
- ✅ Creating a fake implementation for testing
- ✅ Building a temporary mock implementation before real implementation
- ✅ Creating a development-only implementation

Do NOT use "Mock" prefix when:
- ❌ Implementing the actual production class
- ❌ Creating a real implementation (even if simplified)

## File Naming

### Rule: File Names Match Class Names

**File names MUST match the class name exactly (case-sensitive).**

### Examples

✅ **Correct:**
- Class: `MockNetworkManager` → Files: `MockNetworkManager.h`, `MockNetworkManager.cpp`
- Class: `NetworkManager` → Files: `NetworkManager.h`, `NetworkManager.cpp`

❌ **Incorrect:**
- Class: `MockNetworkManager` → Files: `NetworkManager.h`, `NetworkManager.cpp`
- Class: `MockNetworkManager` → Files: `mock_network_manager.h`, `mock_network_manager.cpp`

## Domain Names and DDD Naming Conventions

### Rule: Domain Names Must Reflect Ubiquitous Language

**All domain classes, entities, value objects, and aggregates MUST use names from the domain's ubiquitous language.**

### Domain Layer Components

#### Entities
Entity names should represent business concepts with identity:

✅ **Correct:**
- `Patient` - A patient in the medical monitoring system
- `VitalSignReading` - A specific vital sign measurement
- `MonitoringSession` - A patient monitoring session
- `MedicalDevice` - A medical device in the system
- `User` - A user account with identity

❌ **Incorrect:**
- `PatientData` - Too generic, use `Patient` for entity
- `ReadingModel` - Don't use "Model" suffix for domain entities
- `SessionObject` - Don't use technical suffixes

#### Value Objects
Value objects should describe attributes or measurements without identity:

✅ **Correct:**
- `BloodPressure` - Blood pressure measurement (systolic/diastolic)
- `HeartRate` - Heart rate measurement with unit
- `Temperature` - Temperature measurement with unit
- `TimeRange` - A time range value
- `MeasurementUnit` - A unit of measurement
- `ContactInformation` - Contact details value object

❌ **Incorrect:**
- `BloodPressureVO` - Don't use "VO" suffix
- `HeartRateValue` - Don't use "Value" suffix unless necessary for disambiguation
- `TempData` - Use full business term

#### Aggregates
Aggregate root names should represent the main entity controlling the aggregate boundary:

✅ **Correct:**
- `Patient` (root) controlling `VitalSignReading`, `MedicalHistory`
- `MonitoringSession` (root) controlling `DeviceConnection`, `AlertConfiguration`
- `MedicalDevice` (root) controlling `Calibration`, `MaintenanceRecord`

#### Domain Services
Service names should clearly indicate the domain operation or capability:

✅ **Correct:**
- `VitalSignAnalyzer` - Analyzes vital sign data
- `AlertingService` - Handles alert generation and management
- `PatientMonitoringService` - Coordinates patient monitoring
- `DeviceCalibrationService` - Manages device calibration

❌ **Incorrect:**
- `VitalSignService` - Too vague, what does it do?
- `DataProcessor` - Too technical, not domain language
- `Manager` - Avoid generic "Manager" suffix

#### Repositories
Repository names follow pattern: `{EntityName}Repository`

✅ **Correct:**
- `PatientRepository` - Repository for Patient entities
- `VitalSignReadingRepository` - Repository for readings
- `MonitoringSessionRepository` - Repository for sessions

❌ **Incorrect:**
- `PatientRepo` - Use full "Repository" suffix
- `PatientDataRepository` - Don't add redundant "Data"

### Application Layer Components

#### Application Services
Application service names should describe use cases or application workflows:

✅ **Correct:**
- `PatientRegistrationService` - Handles patient registration workflow
- `MonitoringSessionManagementService` - Manages monitoring sessions
- `AlertNotificationService` - Sends alert notifications
- `ReportGenerationService` - Generates reports

#### DTOs (Data Transfer Objects)
DTO names should indicate their purpose and direction:

✅ **Correct:**
- `RegisterPatientRequest` - Request to register a patient
- `PatientDetailsResponse` - Response with patient details
- `VitalSignDto` - DTO for vital sign data transfer
- `MonitoringSessionDto` - DTO for session data

❌ **Incorrect:**
- `PatientModel` - Use domain entity `Patient` instead
- `PatientData` - Too generic
- `PatientInfo` - Use more specific name like `PatientDetailsResponse`

### Infrastructure Layer Components

#### Implementations
Infrastructure implementations should clearly indicate technology or mechanism:

✅ **Correct:**
- `PostgreSqlPatientRepository` - PostgreSQL implementation
- `FileSystemReportStorage` - File system storage
- `RestApiTelemetryClient` - REST API client
- `InMemoryPatientRepository` - In-memory implementation

❌ **Incorrect:**
- `PatientRepositoryImpl` - Use specific technology name
- `DbPatientRepository` - Use specific database type

### Interface Layer Components

#### Controllers/Presenters
Names should indicate the interface type and responsibility:

✅ **Correct:**
- `PatientRegistrationController` - REST controller
- `MonitoringDashboardPresenter` - UI presenter
- `VitalSignDisplayPresenter` - UI presenter

#### View Models
View model names follow pattern: `{Feature}ViewModel`

✅ **Correct:**
- `PatientListViewModel` - View model for patient list
- `MonitoringDashboardViewModel` - View model for dashboard
- `AlertPanelViewModel` - View model for alert panel

### Common Naming Patterns

#### Avoid Generic Suffixes
❌ Avoid:
- `Manager` - Too generic, be specific
- `Helper` - What kind of help?
- `Handler` - What does it handle?
- `Processor` - What does it process?
- `Util` / `Utils` - Extract specific, well-named classes

✅ Use instead:
- `VitalSignAnalyzer` instead of `VitalSignProcessor`
- `AlertGenerator` instead of `AlertManager`
- `DeviceConnector` instead of `DeviceHelper`

#### Use Domain-Specific Verbs
✅ **Correct:**
- `VitalSignAnalyzer` - Analyzes vital signs
- `AlertGenerator` - Generates alerts
- `PatientRegistrar` - Registers patients
- `DeviceCalibrator` - Calibrates devices

#### Collections and Pluralization
✅ **Correct:**
- `PatientList` or `Patients` - Collection of patients
- `VitalSignReadings` - Collection of readings
- Avoid: `PatientCollection`, `PatientArray`

### Namespace Alignment

Domain names should align with namespace structure:

```cpp
// Domain layer
namespace ZMonitor::Domain {
    class Patient;                    // Entity
    class VitalSignReading;          // Entity
    class BloodPressure;             // Value Object
}

namespace ZMonitor::Domain::Services {
    class VitalSignAnalyzer;         // Domain Service
}

namespace ZMonitor::Domain::Repositories {
    class IPatientRepository;        // Repository Interface
}

// Application layer
namespace ZMonitor::Application {
    class PatientRegistrationService; // Application Service
}

namespace ZMonitor::Application::Dtos {
    class RegisterPatientRequest;     // Request DTO
    class PatientDetailsResponse;     // Response DTO
}

// Infrastructure layer
namespace ZMonitor::Infrastructure::Persistence {
    class PostgreSqlPatientRepository; // Repository Implementation
}
```

### Consistency with Ubiquitous Language

**All names MUST come from the team's ubiquitous language:**

✅ **Use business terms:**
- `Patient`, `VitalSign`, `Alert`, `MonitoringSession`
- `HeartRate`, `BloodPressure`, `Temperature`
- `Threshold`, `Calibration`, `Measurement`

❌ **Avoid technical jargon in domain:**
- `DataModel`, `Record`, `Entry` (unless part of domain language)
- `Object`, `Entity` (as suffixes in domain layer)

### Documentation Requirements

All domain classes MUST have Doxygen comments explaining:
1. **Business meaning** - What does this represent in the domain?
2. **Responsibilities** - What is this class responsible for?
3. **Relationships** - How does it relate to other domain concepts?
4. **Invariants** - What rules must always be true?

Example:
```cpp
/**
 * @brief Represents a patient in the medical monitoring system
 * 
 * Patient is an aggregate root that controls all patient-related information
 * including vital sign readings, medical history, and monitoring configurations.
 * 
 * **Domain Invariants:**
 * - Patient ID must be unique and immutable
 * - Patient must have a valid name and date of birth
 * - Patient status must always be a valid state
 * 
 * @see VitalSignReading for patient measurements
 * @see MonitoringSession for active monitoring
 */
class Patient {
    // ...
};
```

## Related Guidelines

- See `cpp_guidelines.mdc` for general C++ naming conventions
- See `api_documentation.mdc` for documentation requirements
- See `ztodo_verification.mdc` for testing requirements
- See `namespace_guidelines.md` for namespace organization
