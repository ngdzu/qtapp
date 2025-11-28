# Domain-Driven Design (DDD) Strategy

**Document ID:** DESIGN-028  
**Version:** 2.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document defines how the Z Monitor application applies Domain-Driven Design (DDD). It clarifies bounded contexts, layered structure, and domain model guidelines so the codebase evolves with a clear ubiquitous language.

---

## 1. Guiding Principles

1. **Explicit Ubiquitous Language** – Terms such as *Patient*, *DeviceSession*, *TelemetryBatch*, *Alarm*, *ProvisioningState* must be modeled as domain concepts, not incidental data structures.
2. **Domain First** – Business rules live in domain entities/value objects. Application services orchestrate use cases; infrastructure concerns (Qt, SQL, HTTP) are delegated to adapters.
3. **Isolation** – Each bounded context (Monitoring, Provisioning, Admission/ADT) has its own aggregates, repositories, and events.
4. **Record Classes** – Use immutable `struct`/`class` types (e.g., `VitalsRecord`, `AlarmSnapshot`, `PatientIdentity`) to model value objects and domain events.
5. **Layered Architecture** – Code is organized into `domain/`, `application/`, `infrastructure/`, and `interface/` layers under `z-monitor/src/`.

---

## 2. Bounded Contexts

| Context | Description | Key Aggregates |
| --- | --- | --- |
| **Monitoring** | Real-time vitals, alarms, telemetry transmission. | `PatientAggregate`, `DeviceAggregate`, `TelemetryBatch`, `AlarmAggregate`. |
| **Admission/ADT** | Patient admission, discharge, transfer workflow. | `AdmissionAggregate`, `PatientIdentity`, `BedAssignment`. |
| **Provisioning** | Device pairing, certificate management. | `ProvisioningSession`, `CredentialBundle`. |
| **Security** | Authentication, authorization, audit logging. | `UserSession`, `PinCredential`, `AuditTrailEntry`. |

Each bounded context can expose domain services and repositories scoped to its aggregate roots.

> **📋 Complete Component Reference:** For an authoritative list of all aggregates, value objects, domain events, and repository interfaces, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)**.

---

## 3. Layered Structure

The Z Monitor codebase is organized into four DDD layers under `z-monitor/src/`:

```
src/
├── domain/          # Domain Layer (pure business logic)
├── application/     # Application Layer (use-case orchestration)
├── infrastructure/  # Infrastructure Layer (adapters)
└── interface/       # Interface Layer (UI integration)
```

**Layer Overview:**
- **Domain Layer** (`domain/`): Aggregates, value objects, domain events, repository interfaces, external service interfaces. Organized by bounded contexts (monitoring, admission, provisioning, security).
- **Application Layer** (`application/`): Application services, DTOs. Orchestrates use cases and coordinates domain objects.
- **Infrastructure Layer** (`infrastructure/`): Repository implementations, network adapters, sensor adapters, caching, security, Qt adapters, system services, utilities.
- **Interface Layer** (`interface/`): QML controllers (QObject bridges) and QML UI files.

> **📋 Detailed Structure:** For the complete directory structure with all files and subdirectories, see **[Code Organization (22_CODE_ORGANIZATION.md)](./22_CODE_ORGANIZATION.md)** Section 2.2.  
> **📋 Workspace Overview:** For workspace-level project structure, see **[Project Structure (27_PROJECT_STRUCTURE.md)](./27_PROJECT_STRUCTURE.md)**.

### 3.1 Layer Responsibilities

- **Domain** – Pure business logic, aggregates, value objects, domain events, repository interfaces. No Qt or SQL includes.
- **Application** – Use cases (e.g., admit patient, push telemetry). Coordinates domain objects and repositories. Emits domain events for interface layer.
- **Infrastructure** – Implementations of repositories, network adapters, persistence, crypto, OS-specific utilities.
- **Interface** – QML views and QObject controllers binding data to UI.

---

## 4. Domain Model Guidelines

### 4.1 Entities & Aggregates

| Aggregate | Bounded Context | Responsibility | Key Methods |
|-----------|----------------|----------------|-------------|
| **PatientAggregate** | Monitoring | Patient admission lifecycle, vitals state, bed assignment | `admit()`, `discharge()`, `transfer()`, `updateVitals()` |
| **DeviceAggregate** | Provisioning | Device provisioning state, credential lifecycle | `applyProvisioningPayload()`, `markProvisioned()`, `rotateCredentials()` |
| **TelemetryBatch** | Monitoring | Telemetry data collection, signing, validation | `addVital()`, `addAlarm()`, `sign()`, `validate()` |
| **AlarmAggregate** | Monitoring | Alarm lifecycle, state transitions, history | `raise()`, `acknowledge()`, `silence()`, `escalate()` |
| **AdmissionAggregate** | Admission/ADT | Admission/discharge/transfer workflow | `admitPatient()`, `dischargePatient()`, `transferPatient()` |
| **ProvisioningSession** | Provisioning | Pairing workflow, QR code lifecycle | `generatePairingCode()`, `validateCode()`, `applyConfiguration()` |
| **UserSession** | Security | Authentication, session management | `authenticate()`, `refreshSession()`, `terminate()` |
| **AuditTrailEntry** | Security | Security event auditing | `logEvent()`, `getEventHistory()` |

**Detailed Aggregate Specifications:**

- **PatientAggregate**
  - Fields: `PatientIdentity`, `AdmissionState`, `BedAssignment`, `CurrentVitals`.
  - Invariants: only one active admission per patient; device label matches assignment.
  - Methods: `admit()`, `discharge()`, `transfer()`, `updateVitals()`.
- **DeviceAggregate**
  - Fields: `DeviceId`, `DeviceLabel`, `ProvisioningState`, `Certificates`.
  - Methods: `applyProvisioningPayload()`, `markProvisioned()`, `rotateCredentials()`.
- **TelemetryBatch**
  - Fields: `PatientIdentity`, `DeviceSnapshot`, `std::vector<VitalRecord>`, `std::vector<AlarmSnapshot>`.
  - Methods: `addVital()`, `addAlarm()`, `sign()`, `validate()`.
- **AlarmAggregate**
  - Fields: `AlarmId`, `Priority`, `State`, `History`.
  - Methods: `raise()`, `acknowledge()`, `silence()`, `escalate()`.

> **📋 Complete Aggregate Reference:** For detailed specifications of all aggregates, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)** Section 2.1.

### 4.2 Value Objects

| Value Object | Description | Immutability |
|--------------|-------------|--------------|
| `PatientIdentity` | MRN, Name, DOB, Sex | Yes |
| `DeviceSnapshot` | DeviceId, DeviceLabel, FirmwareVersion, ProvisioningStatus | Yes |
| `VitalRecord` | Single vital sign measurement (HR, SpO2, RR, timestamp) | Yes |
| `WaveformSample` | Single waveform sample (channel, value, timestamp) | Yes |
| `AlarmSnapshot` | Alarm state at a point in time | Yes |
| `MeasurementUnit` | Metric or Imperial | Yes |
| `AlarmThreshold` | Min/max values for alarm triggers | Yes |
| `BedLocation` | Bed/unit/facility identifier | Yes |
| `PinCredential` | Hashed PIN with salt | Yes |
| `CredentialBundle` | Certificates, keys, server URL | Yes |

**Implementation:** Use `struct`/`class` with const members or getters to enforce immutability.

> **📋 Complete Value Object Reference:** For detailed specifications of all value objects, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)** Section 2.2.

### 4.3 Domain Events

| Event | Context | Triggered When | Consumed By |
|-------|---------|----------------|-------------|
| `PatientAdmitted` | Admission/ADT | Patient is admitted to device | UI controllers, audit logging, telemetry service |
| `PatientDischarged` | Admission/ADT | Patient is discharged | UI controllers, audit logging |
| `PatientTransferred` | Admission/ADT | Patient transferred to another device | UI controllers, audit logging |
| `TelemetryQueued` | Monitoring | Telemetry batch ready for transmission | Network manager |
| `TelemetrySent` | Monitoring | Telemetry successfully transmitted | UI controllers, audit logging |
| `AlarmRaised` | Monitoring | New alarm triggered | UI controllers, alarm manager |
| `AlarmAcknowledged` | Monitoring | Alarm acknowledged by user | UI controllers, audit logging |
| `ProvisioningCompleted` | Provisioning | Device successfully provisioned | UI controllers, settings manager |
| `ProvisioningFailed` | Provisioning | Provisioning failed | UI controllers |
| `UserLoggedIn` | Security | User successfully authenticated | UI controllers, audit logging |
| `UserLoggedOut` | Security | User logged out | UI controllers |
| `SessionExpired` | Security | Session timeout | UI controllers |

**Implementation:** Events are plain structs consumed by application services or logging/persistence adapters.

> **📋 Complete Domain Events Reference:** For detailed specifications of all domain events, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)** Section 2.3.

### 4.4 Repository Interfaces

| Repository Interface | Aggregate | Key Methods |
|---------------------|-----------|-------------|
| `IPatientRepository` | PatientAggregate | `findByMrn()`, `save()`, `getAdmissionHistory()` |
| `ITelemetryRepository` | TelemetryBatch | `save()`, `getHistorical()`, `archive()` |
| `IVitalsRepository` | VitalRecord | `save()`, `saveBatch()`, `getRange()`, `getUnsynced()` |
| `IAlarmRepository` | AlarmAggregate | `save()`, `getActive()`, `getHistory()` |
| `IProvisioningRepository` | ProvisioningSession | `save()`, `findByDeviceId()`, `getHistory()` |
| `IUserRepository` | UserSession | `findByUserId()`, `save()`, `updateLastLogin()` |
| `IAuditRepository` | AuditTrailEntry | `save()`, `query()`, `archive()` |

**Implementation:** Interfaces defined in domain layer (`src/domain/repositories/`); implementations live in infrastructure layer (e.g., `SQLitePatientRepository`, `SQLiteTelemetryRepository`, `MemoryRepository` for tests).

> **📋 Complete Repository Reference:** For detailed specifications of all repository interfaces, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)** Section 2.4.

### 4.5 External Service Interfaces

Domain layer defines interfaces for external services (implemented in infrastructure layer):

| Interface | Purpose | Key Methods | Documentation |
|-----------|---------|-------------|---------------|
| `ISensorDataSource` | Abstracts sensor data input (simulator, hardware, mock) | `start()`, `stop()`, `isActive()` | [ISensorDataSource.md](./interfaces/ISensorDataSource.md) |
| `IPatientLookupService` | Patient demographic lookup from HIS/EHR | `lookupPatient()`, `searchByName()` | [IPatientLookupService.md](./interfaces/IPatientLookupService.md) |
| `ITelemetryServer` | Secure telemetry transmission to central server | `sendBatch()`, `sendAlarm()`, `registerDevice()` | [ITelemetryServer.md](./interfaces/ITelemetryServer.md) |
| `IProvisioningService` | Device provisioning and configuration | `requestProvisioning()`, `applyConfiguration()` | [IProvisioningService.md](./interfaces/IProvisioningService.md) |
| `IUserManagementService` | Hospital user authentication and authorization | `authenticate()`, `validateSession()`, `logout()`, `checkPermission()` | [IUserManagementService.md](./interfaces/IUserManagementService.md) |

**Implementation:** Interfaces defined in domain layer; implementations in infrastructure layer (e.g., `WebSocketSensorDataSource`, `NetworkTelemetryServer`, `HospitalUserManagementAdapter`).

> **📋 Complete Interface Reference:** For detailed specifications of all external service interfaces, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)** Section 2.5.

### 4.6 Record Classes

Implement record/value types as lightweight `struct`s with semantics:

```cpp
struct VitalRecord {
    PatientIdentity patient;
    QDateTime timestamp;
    int heartRate;
    double spo2;
    double respirationRate;
    QString source;
};

struct AlarmSnapshot {
    QUuid alarmId;
    AlarmPriority priority;
    AlarmState state;
    QDateTime occurredAt;
};
```

Use these records to pass data between domain/application layers without leaking infrastructure details.

---

## 5. Application Services & Use Cases

| Service | Responsibility | Dependencies (Repositories) | Dependencies (Services) |
|---------|----------------|----------------------------|------------------------|
| `MonitoringService` | Coordinates vitals ingestion, telemetry batching, transmission | `ITelemetryRepository`, `IPatientRepository`, `IAlarmRepository`, `IVitalsRepository` | `SecurityService` (for signing), `ISensorDataSource` (for data input) |
| `AdmissionService` | Executes admit/discharge/transfer use cases | `IPatientRepository`, `IAuditRepository` | `SecurityService` (for audit), `IPatientLookupService` (for lookup) |
| `ProvisioningService` | Handles QR pairing, certificate installation, validation | `IProvisioningRepository` | Works with `DeviceAggregate` + infrastructure provisioning adapter |
| `SecurityService` | Authentication, PIN policy, session lifecycle | `IUserRepository`, `IAuditRepository` | Coordinates `UserRepository`, `AuditTrail` |
| `DataArchiveService` | Data archival and retention management | `ITelemetryRepository`, `IVitalsRepository`, `IAlarmRepository` | Coordinates data archival workflows |
| `FirmwareUpdateService` | Firmware update management | N/A | Coordinates firmware update workflows |
| `BackupService` | Database backup and recovery | `DatabaseManager` | Coordinates backup/restore workflows |

**Application Service Guidelines:**
- Validate commands (DTOs) before invoking domain logic.
- Map domain entities to presentation models for controllers.
- Publish domain events for logging/AuditService.
- Coordinate between domain aggregates and infrastructure adapters.

> **📋 Complete Application Services Reference:** For detailed specifications of all application services, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)** Section 3.1.

---

## 6. Infrastructure Mapping

| Concern | Implementation | Location |
| --- | --- | --- |
| **Persistence** | SQLite/SQLCipher via repositories (`SQLitePatientRepository`, `SQLiteTelemetryRepository`, `SQLiteVitalsRepository`, `SQLiteAlarmRepository`, `SQLiteProvisioningRepository`, `SQLiteUserRepository`, `SQLiteAuditRepository`) | `infrastructure/persistence/` |
| **Networking** | `NetworkTelemetryServer` (implements `ITelemetryServer`), `HISPatientLookupAdapter` (implements `IPatientLookupService`), `HospitalUserManagementAdapter` (implements `IUserManagementService`), uses `QSslConfiguration` | `infrastructure/network/` |
| **Sensors** | `WebSocketSensorDataSource` (implements `ISensorDataSource`), `SimulatorDataSource`, `MockSensorDataSource`, `HardwareSensorAdapter`, `ReplayDataSource` | `infrastructure/sensors/` |
| **Caching** | `VitalsCache`, `WaveformCache`, `PersistenceScheduler`, `DataCleanupService` | `infrastructure/caching/` |
| **Security** | `CertificateManager`, `KeyManager`, `EncryptionService`, `SignatureService`, `SecureStorage` | `infrastructure/security/` |
| **Qt Adapters** | `SettingsManager`, `LogService` | `infrastructure/qt/` |
| **System Services** | `HealthMonitor`, `ClockSyncService`, `FirmwareManager`, `WatchdogService` | `infrastructure/system/` |
| **Utilities** | `ObjectPool`, `LockFreeQueue`, `LogBuffer`, `MemoryPool`, `CryptoUtils`, `DateTimeUtils`, `StringUtils`, `ValidationUtils` | `infrastructure/utils/` |

**Key Principle:** Infrastructure classes must depend on domain interfaces, not vice versa. All infrastructure implementations are adapters that implement domain-defined interfaces.

> **📋 Complete Infrastructure Reference:** For detailed specifications of all infrastructure components, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)** Section 4.

---

## 7. Interface Layer

- QObject controllers remain in `z-monitor/src/interface/controllers/` (or `controllers/` initially) and should depend on application services.
- QML interacts with controllers via properties/signals; controllers call application services rather than infrastructure directly.

Example flow:
```
DashboardView.qml
  ↓ (signal/property binding)
DashboardController (interface)
  ↓
MonitoringService (application)
  ↓
PatientAggregate / TelemetryBatch (domain) + Repositories
  ↓
SQLiteTelemetryRepository / NetworkTelemetryServer (infrastructure)
```

---

## 8. Migration Plan

1. **Create directories** `z-monitor/src/domain`, `application`, `infrastructure`, `interface`.
2. **Move domain logic** from monolithic classes (e.g., `PatientManager`) into aggregates/value objects.
3. **Extract repositories** interfaces and implementations.
4. **Refactor controllers** to depend on application services.
5. **Update build files** (`CMakeLists.txt`) to reflect new structure.

Tracking tasks for this migration should be added to `ZTODO.md` (see “DDD Refactor” entries).

---

## 9. References

### 9.1 Architecture Documents
- **[02_ARCHITECTURE.md](./02_ARCHITECTURE.md)** – High-level architecture with DDD layers and contexts.
- **[29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md)** – Complete authoritative list of all system components (120 total) ⭐
- **[22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md)** – Detailed code organization and directory structure.
- **[27_PROJECT_STRUCTURE.md](./27_PROJECT_STRUCTURE.md)** – Workspace-level project structure overview.

### 9.2 Design Documents
- **[09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md)** – Class definitions categorized by domain/application/infrastructure.
- **[13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md)** – Dependency injection strategy (no singletons).

### 9.3 Task Tracking
- **ZTODO.md** – Tasks for restructuring source tree and implementing repositories/aggregates.

Adhering to this DDD strategy keeps the Z Monitor codebase maintainable, testable, and aligned with the medical-domain language used throughout the project documentation.

