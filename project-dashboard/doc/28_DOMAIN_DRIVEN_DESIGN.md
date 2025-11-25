# Domain-Driven Design (DDD) Strategy

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

---

## 3. Layered Structure

```
z-monitor/src/
├── domain/
│   ├── monitoring/
│   │   ├── PatientAggregate.h
│   │   ├── TelemetryBatch.h
│   │   ├── VitalRecord.h
│   │   └── events/
│   ├── admission/
│   ├── provisioning/
│   └── security/
├── application/
│   ├── services/
│   │   ├── MonitoringService.h
│   │   ├── AdmissionService.h
│   │   ├── ProvisioningService.h
│   │   └── SecurityService.h
│   └── dto/
├── infrastructure/
│   ├── persistence/      # SQLite, repositories, migrations
│   ├── network/          # ITelemetryServer impl, HTTPS/mTLS
│   ├── qt/               # Qt adapters, QAbstractListModel, etc.
│   └── provisioning/     # Certificate management adapters
└── interface/
    ├── controllers/      # QObject/QML bridges
    └── qml/              # UI resources (existing resources/)
```

### 3.1 Layer Responsibilities

- **Domain** – Pure business logic, aggregates, value objects, domain events, repository interfaces. No Qt or SQL includes.
- **Application** – Use cases (e.g., admit patient, push telemetry). Coordinates domain objects and repositories. Emits domain events for interface layer.
- **Infrastructure** – Implementations of repositories, network adapters, persistence, crypto, OS-specific utilities.
- **Interface** – QML views and QObject controllers binding data to UI.

---

## 4. Domain Model Guidelines

### 4.1 Entities & Aggregates

- **PatientAggregate**
  - Fields: `PatientIdentity`, `AdmissionState`, `BedAssignment`, `CurrentVitals`.
  - Invariants: only one active admission per patient; device label matches assignment.
  - Methods: `admit()`, `discharge()`, `transfer()`, `updateVitals()`.
- **DeviceAggregate**
  - Fields: `DeviceId`, `DeviceLabel`, `ProvisioningState`, `Certificates`.
  - Methods: `applyProvisioningPayload()`, `markProvisioned()`, `rotateCredentials()`.
- **TelemetryBatch**
  - Fields: `PatientIdentity`, `DeviceSnapshot`, `std::vector<VitalRecord>`, `std::vector<AlarmSnapshot>`.
  - Methods: `sign(QByteArray privateKey)`, `validate()`.
- **AlarmAggregate**
  - Fields: `AlarmId`, `Priority`, `State`, `History`.
  - Methods: `raise()`, `acknowledge()`, `silence()`.

### 4.2 Value Objects

- `PatientIdentity` (MRN, Name, DOB, Sex) – immutable.
- `DeviceSnapshot` (DeviceId, DeviceLabel, FirmwareVersion, ProvisioningStatus).
- `MeasurementUnit`, `AlarmThreshold`, `BedLocation`.
- Use `struct`/`class` with const members or getters to enforce immutability.

### 4.3 Domain Events

- `PatientAdmitted`, `PatientDischarged`, `TelemetryQueued`, `TelemetrySent`, `AlarmRaised`, `ProvisioningCompleted`.
- Events are plain structs consumed by application services or logging/persistence adapters.

### 4.4 Repositories

- `IPatientRepository`, `ITelemetryRepository`, `IAlarmRepository`, `IProvisioningRepository`.
- Interfaces in domain layer; implementations live in infrastructure (e.g., SQLiteRepository, MemoryRepository for tests).

### 4.5 Record Classes

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

| Service | Responsibilities | Notes |
| --- | --- | --- |
| `MonitoringService` | Collect vitals, create telemetry batches, orchestrate NetworkManager. | Calls domain aggregates, persists telemetry via repositories. |
| `AdmissionService` | Admit/discharge/transfer patients, coordinate with PatientAggregate. | Emits domain events for UI and audit logs. |
| `ProvisioningService` | Handle QR pairing, certificate installation, validation. | Works with DeviceAggregate + infrastructure provisioning adapter. |
| `SecurityService` | Authentication, PIN policy, session lifecycle. | Coordinates `UserRepository`, `AuditTrail`. |

Application services should:
- Validate commands (DTOs) before invoking domain logic.
- Map domain entities to presentation models for controllers.
- Publish domain events for logging/AuditService.

---

## 6. Infrastructure Mapping

| Concern | Implementation |
| --- | --- |
| Persistence | SQLite/SQLCipher via repositories (`SQLitePatientRepository`, `TelemetryArchiveRepository`). |
| Networking | `NetworkTelemetryServer` (implements `ITelemetryServer`), uses `QSslConfiguration`. |
| Provisioning | QR generator, certificate installer, secure storage. |
| Logging | `LogService` adapters writing to database + QML diagnostics. |
| External Interfaces | `SensorSimulatorAdapter`, `CentralServerClient`. |

Infrastructure classes must depend on domain interfaces, not vice versa.

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

- `doc/02_ARCHITECTURE.md` – Updated with DDD layers and contexts.
- `doc/09_CLASS_DESIGNS.md` – Class definitions categorized by domain/application/infrastructure.
- `doc/27_PROJECT_STRUCTURE.md` – Directory overview including DDD layout.
- `ZTODO.md` – Tasks for restructuring source tree and implementing repositories/aggregates.

Adhering to this DDD strategy keeps the Z Monitor codebase maintainable, testable, and aligned with the medical-domain language used throughout the project documentation.

