# System Components Reference & DDD Strategy

**Document ID:** DESIGN-029  
**Version:** 3.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides a complete, authoritative list of all system components in the Z Monitor application and their interactions. It also defines how the application applies Domain-Driven Design (DDD) principles. This serves as the single source of truth for component inventory and DDD guidelines.

> **Related Documentation:**  
> **Class Designs:** [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md) - Module-based class architecture ⭐  
> **Thread Model:** [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Thread and module architecture ⭐  
> **Code Organization:** [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md) - Detailed directory structure ⭐  
> **Data Caching:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) - Data caching architecture and strategy  
> **Database Access:** [30_DATABASE_ACCESS_STRATEGY.md](./30_DATABASE_ACCESS_STRATEGY.md) - Database access and ORM strategy

---

## 1. Domain-Driven Design (DDD) Overview

The Z Monitor application is organized according to Domain-Driven Design (DDD) principles with the following layers:

- **Domain Layer**: Pure business logic (aggregates, value objects, domain events, repository interfaces, external service interfaces)
- **Application Layer**: Use case orchestration (services, DTOs)
- **Infrastructure Layer**: Technical implementations (persistence, networking, Qt adapters, caching)
- **Interface Layer**: UI integration (QML controllers, QML components)

### 1.1 DDD Guiding Principles

1. **Explicit Ubiquitous Language** – Terms such as *Patient*, *DeviceSession*, *TelemetryBatch*, *Alarm*, *ProvisioningState* must be modeled as domain concepts, not incidental data structures.
2. **Domain First** – Business rules live in domain entities/value objects. Application services orchestrate use cases; infrastructure concerns (Qt, SQL, HTTP) are delegated to adapters.
3. **Isolation** – Each bounded context (Monitoring, Provisioning, Admission/ADT) has its own aggregates, repositories, and events.
4. **Record Classes** – Use immutable `struct`/`class` types (e.g., `VitalRecord`, `AlarmSnapshot`, `PatientIdentity`) to model value objects and domain events.
5. **Layered Architecture** – Code is organized into `domain/`, `application/`, `infrastructure/`, and `interface/` layers under `z-monitor/src/`.

### 1.2 Bounded Contexts

| Context | Description | Key Aggregates |
| --- | --- | --- |
| **Monitoring** | Real-time vitals, alarms, telemetry transmission. | `PatientAggregate`, `DeviceAggregate`, `TelemetryBatch`, `AlarmAggregate`. |
| **Admission/ADT** | Patient admission, discharge, transfer workflow. | `AdmissionAggregate`, `PatientIdentity`, `BedAssignment`. |
| **Provisioning** | Device pairing, certificate management. | `ProvisioningSession`, `CredentialBundle`. |
| **Security** | Authentication, authorization, audit logging. | `UserSession`, `PinCredential`, `AuditTrailEntry`. |

Each bounded context can expose domain services and repositories scoped to its aggregate roots.

### 1.3 Layer Responsibilities

- **Domain Layer** (`domain/`) – Pure business logic, aggregates, value objects, domain events, repository interfaces. No Qt or SQL includes. Organized by bounded contexts (monitoring, admission, provisioning, security).
- **Application Layer** (`application/`) – Use cases (e.g., admit patient, push telemetry). Coordinates domain objects and repositories. Emits domain events for interface layer.
- **Infrastructure Layer** (`infrastructure/`) – Implementations of repositories, network adapters, persistence, crypto, OS-specific utilities. Must depend on domain interfaces, not vice versa.
- **Interface Layer** (`interface/`) – QML views and QObject controllers binding data to UI. Controllers depend on application services rather than infrastructure directly.

> **📋 Detailed Structure:** For the complete directory structure with all files and subdirectories, see **[Code Organization (22_CODE_ORGANIZATION.md)](./22_CODE_ORGANIZATION.md)** Section 2.2.  
> **📋 Workspace Overview:** For workspace-level project structure, see **[Project Structure (27_PROJECT_STRUCTURE.md)](./27_PROJECT_STRUCTURE.md)**.

### 1.4 Domain Model Guidelines

**Aggregates:** Domain entities that enforce business invariants. Each aggregate has a root entity and encapsulates related value objects and entities.

**Value Objects:** Immutable objects defined by their attributes (e.g., `PatientIdentity`, `VitalRecord`). Use `struct`/`class` with const members or getters to enforce immutability.

**Domain Events:** Plain structs that represent something that happened in the domain (e.g., `PatientAdmitted`, `AlarmRaised`). Consumed by application services or logging/persistence adapters.

**Repository Interfaces:** Defined in domain layer; implementations live in infrastructure layer (e.g., `SQLitePatientRepository`, `MemoryRepository` for tests).

**External Service Interfaces:** Domain layer defines interfaces for external services (e.g., `ISensorDataSource`, `ITelemetryServer`); implementations in infrastructure layer.

### 1.5 Application Service Guidelines

Application services should:
- Validate commands (DTOs) before invoking domain logic.
- Map domain entities to presentation models for controllers.
- Publish domain events for logging/AuditService.
- Coordinate between domain aggregates and infrastructure adapters.

### 1.6 Infrastructure Mapping Principles

**Key Principle:** Infrastructure classes must depend on domain interfaces, not vice versa. All infrastructure implementations are adapters that implement domain-defined interfaces.

**Infrastructure Categories:**
- **Persistence:** SQLite/SQLCipher via repositories
- **Networking:** HTTPS/mTLS adapters implementing `ITelemetryServer`, `IPatientLookupService`, `IUserManagementService`
- **Sensors:** Adapters implementing `ISensorDataSource`
- **Caching:** In-memory caches for performance
- **Security:** Certificate management, encryption, signing
- **Qt Adapters:** Settings, logging
- **System Services:** Health monitoring, firmware, time sync
- **Utilities:** Object pools, lock-free queues, utility functions

### 1.7 Interface Layer Pattern

QObject controllers remain in `z-monitor/src/interface/controllers/` and should depend on application services.

**Example Flow:**
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

QML interacts with controllers via properties/signals; controllers call application services rather than infrastructure directly.

---

## 2. Domain Layer Components

### 2.1 Aggregates

| Aggregate | Bounded Context | Responsibility | Key Methods |
|-----------|----------------|----------------|-------------|
| `PatientAggregate` | Monitoring | Patient admission lifecycle, vitals state, bed assignment | `admit()`, `discharge()`, `transfer()`, `updateVitals()` |
| `DeviceAggregate` | Provisioning | Device provisioning state, credential lifecycle | `applyProvisioningPayload()`, `markProvisioned()`, `rotateCredentials()` |
| `TelemetryBatch` | Monitoring | Telemetry data collection, signing, validation | `addVital()`, `addAlarm()`, `sign()`, `validate()` |
| `AlarmAggregate` | Monitoring | Alarm lifecycle, state transitions, history | `raise()`, `acknowledge()`, `silence()`, `escalate()` |
| `AdmissionAggregate` | Admission/ADT | Admission/discharge/transfer workflow | `admitPatient()`, `dischargePatient()`, `transferPatient()` |
| `ProvisioningSession` | Provisioning | Pairing workflow, QR code lifecycle | `generatePairingCode()`, `validateCode()`, `applyConfiguration()` |
| `UserSession` | Security | Authentication, session management | `authenticate()`, `refreshSession()`, `terminate()` |
| `AuditTrailEntry` | Security | Security event auditing | `logEvent()`, `getEventHistory()` |

### 2.2 Value Objects

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

### 2.3 Domain Events

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

### 2.4 Repository Interfaces (Domain)

| Repository Interface | Aggregate | Key Methods |
|---------------------|-----------|-------------|
| `IPatientRepository` | PatientAggregate | `findByMrn()`, `save()`, `getAdmissionHistory()` |
| `ITelemetryRepository` | TelemetryBatch | `save()`, `getHistorical()`, `archive()` |
| `IVitalsRepository` | VitalRecord | `save()`, `saveBatch()`, `getRange()`, `getUnsynced()` |
| `IAlarmRepository` | AlarmAggregate | `save()`, `getActive()`, `getHistory()` |
| `IProvisioningRepository` | ProvisioningSession | `save()`, `findByDeviceId()`, `getHistory()` |
| `IUserRepository` | UserSession | `findByUserId()`, `save()`, `updateLastLogin()` |
| `IAuditRepository` | AuditTrailEntry | `save()`, `query()`, `archive()` |

### 2.5 External Service Interfaces (Domain)

| Interface | Purpose | Key Methods | Documentation |
|-----------|---------|-------------|---------------|
| `ISensorDataSource` | Abstracts sensor data input (simulator, hardware, mock) | `start()`, `stop()`, `isActive()` | [ISensorDataSource.md](./interfaces/ISensorDataSource.md) |
| `IPatientLookupService` | Patient demographic lookup from HIS/EHR | `lookupPatient()`, `searchByName()` | [IPatientLookupService.md](./interfaces/IPatientLookupService.md) |
| `ITelemetryServer` | Secure telemetry transmission to central server | `sendBatch()`, `sendAlarm()`, `registerDevice()` | [ITelemetryServer.md](./interfaces/ITelemetryServer.md) |
| `IProvisioningService` | Device provisioning and configuration | `requestProvisioning()`, `applyConfiguration()` | [IProvisioningService.md](./interfaces/IProvisioningService.md) |
| `IUserManagementService` | **NEW:** Hospital user authentication and authorization (nurses, physicians, technicians, administrators) | `authenticate()`, `validateSession()`, `logout()`, `checkPermission()` | [IUserManagementService.md](./interfaces/IUserManagementService.md) |

---

## 3. Application Layer Components

### 3.1 Application Services

| Service | Responsibility | Dependencies (Repositories) | Dependencies (Services) |
|---------|----------------|----------------------------|------------------------|
| `MonitoringService` | Coordinates vitals ingestion, telemetry batching, transmission | `ITelemetryRepository`, `IPatientRepository`, `IAlarmRepository`, `IVitalsRepository` | `SecurityService` (for signing), `ISensorDataSource` (for data input) |
| `AdmissionService` | Executes admit/discharge/transfer use cases | `IPatientRepository`, `IAuditRepository` | `SecurityService` (for audit), `IPatientLookupService` (for lookup) |
| `ProvisioningService` | Handles QR pairing, certificate installation, validation | `IProvisioningRepository` | `SecurityService` (for audit), `IProvisioningService` (external) |
| `SecurityService` | **UPDATED:** Authentication, session management, RBAC enforcement, permission checking | `IUserRepository`, `IAuditRepository` | `IUserManagementService` (hospital server authentication), **PermissionRegistry** (role defaults) |
| `PermissionRegistry` | Enum-based role → permission mapping, permission string/label helpers | None | `SecurityService`, `UserProfile` (seeding), UI controllers (display labels) |
| `DataArchiveService` | Coordinates data archival workflow (orchestrates `DataArchiver`) | `ITelemetryRepository`, `IAlarmRepository`, `IVitalsRepository` | `DataArchiver` (infrastructure) |
| `FirmwareUpdateService` | Manages firmware updates | None | `SecurityService` (for signature validation), `FirmwareManager` (infrastructure) |
| `BackupService` | Coordinates database backup and restore workflow (orchestrates `BackupManager`) | All repositories | `SecurityService` (for encryption), `BackupManager` (infrastructure) |

### 3.2 DTOs (Data Transfer Objects)

| DTO | Purpose | Used By |
|-----|---------|---------|
| `AdmitPatientCommand` | Patient admission request | `AdmissionService`, `PatientController` |
| `DischargePatientCommand` | Patient discharge request | `AdmissionService`, `PatientController` |
| `TransferPatientCommand` | Patient transfer request | `AdmissionService`, `PatientController` |
| `TelemetrySubmission` | Telemetry data for transmission | `MonitoringService`, `NetworkManager` |
| `ProvisioningPayload` | Configuration from Central Station | `ProvisioningService`, `ProvisioningController` |
| `LoginRequest` | Authentication request | `SecurityService`, `AuthenticationController` |

---

## 4. Infrastructure Layer Components

### 4.1 Persistence (Repository Implementations)

| Implementation | Interface | Technology |
|----------------|-----------|------------|
| `SQLitePatientRepository` | `IPatientRepository` | SQLite + SQLCipher |
| `SQLiteTelemetryRepository` | `ITelemetryRepository` | SQLite + SQLCipher |
| `SQLiteVitalsRepository` | `IVitalsRepository` | SQLite + SQLCipher |
| `SQLiteAlarmRepository` | `IAlarmRepository` | SQLite + SQLCipher |
| `SQLiteProvisioningRepository` | `IProvisioningRepository` | SQLite + SQLCipher |
| `SQLiteUserRepository` | `IUserRepository` | SQLite + SQLCipher |
| `SQLiteAuditRepository` | `IAuditRepository` | SQLite + SQLCipher |

### 4.2 Data Caching Components

| Component | Purpose | Technology | Thread | Documentation |
|-----------|---------|------------|--------|---------------|
| `VitalsCache` | In-memory cache for vitals (3-day capacity, ~39 MB) | `std::deque` + `QReadWriteLock` | Real-Time Processing | [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) |
| `WaveformCache` | Circular buffer for waveforms (30 seconds, ~0.1 MB) | `std::deque` | Real-Time Processing | [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) |
| `PersistenceScheduler` | Periodic database persistence (every 10 min) | `QTimer` | Database I/O | [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) |
| `DataCleanupService` | Daily data cleanup (7-day retention, runs at 3 AM) | `QTimer` | Database I/O | [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) |

### 4.3 Network Adapters

| Component | Interface/Base | Technology | Responsibility | Documentation |
|-----------|---------------|------------|----------------|---------------|
| `NetworkTelemetryServer` | `ITelemetryServer` | Qt Network (HTTPS/mTLS) | Production telemetry transmission | [ITelemetryServer.md](./interfaces/ITelemetryServer.md) |
| `MockTelemetryServer` | `ITelemetryServer` | In-memory | Testing/development telemetry endpoint | [ITelemetryServer.md](./interfaces/ITelemetryServer.md) |
| `HISPatientLookupAdapter` | `IPatientLookupService` | HTTPS/REST | Real HIS/EHR integration | [IPatientLookupService.md](./interfaces/IPatientLookupService.md) |
| `MockPatientLookupService` | `IPatientLookupService` | In-memory | Testing/development patient lookup | [IPatientLookupService.md](./interfaces/IPatientLookupService.md) |
| `CentralStationClient` | None | HTTPS/REST | Provisioning payload receiver | [IProvisioningService.md](./interfaces/IProvisioningService.md) |

### 4.4 Sensor Data Source Adapters

| Component | Interface | Technology | Responsibility | Documentation |
|-----------|-----------|------------|----------------|---------------|
| `SharedMemorySensorDataSource` | `ISensorDataSource` | POSIX shared memory + memfd | Maps shared ring buffer exposed by local simulator | [ISensorDataSource.md](./interfaces/ISensorDataSource.md) |
| `SimulatorDataSource` | `ISensorDataSource` | Qt Timers | Internal fallback simulator (no external process) | [ISensorDataSource.md](./interfaces/ISensorDataSource.md) |
| `MockSensorDataSource` | `ISensorDataSource` | In-memory | Testing with deterministic data | [ISensorDataSource.md](./interfaces/ISensorDataSource.md) |
| `HardwareSensorAdapter` | `ISensorDataSource` | Serial/USB | Future: Real hardware sensors | [ISensorDataSource.md](./interfaces/ISensorDataSource.md) |
| `ReplayDataSource` | `ISensorDataSource` | File I/O | Development: Replay recorded data | [ISensorDataSource.md](./interfaces/ISensorDataSource.md) |

### 4.4b User Management Service Adapters (NEW)

| Component | Interface | Technology | Responsibility | Documentation |
|-----------|-----------|------------|----------------|---------------|
| `MockUserManagementService` | `IUserManagementService` | In-memory | **Development/Testing:** Hardcoded test users (nurses, physicians, technicians, admin), no network required | [IUserManagementService.md](./interfaces/IUserManagementService.md) |
| `HospitalUserManagementAdapter` | `IUserManagementService` | Qt Network (HTTPS) or LDAP | **Production:** Hospital server integration for user authentication, role retrieval, permission checks | [IUserManagementService.md](./interfaces/IUserManagementService.md) |

### 4.5 Qt Adapters

| Component | Technology | Responsibility |
|-----------|------------|----------------|
| `DatabaseManager` | Qt SQL + SQLCipher | SQLite connection management, migrations, query registry |
| `SettingsManager` | Qt Settings | Persistent configuration storage |
| `LogService` | Qt Logging | Centralized logging mechanism |

### 4.6 Security & Provisioning Adapters

| Component | Responsibility | Technology |
|-----------|----------------|------------|
| `CertificateManager` | Certificate loading, validation, renewal | OpenSSL via Qt |
| `KeyManager` | Encryption key storage, rotation, lifecycle management | HSM/TPM/Keychain |
| `QRCodeGenerator` | QR code generation | QR code library |
| `EncryptionService` | Payload encryption/decryption | OpenSSL via Qt |
| `SignatureService` | Data signing/verification (HMAC-SHA256) | OpenSSL via Qt |
| `SecureStorage` | Secure key storage | Platform keychain (Keychain/Keyring) |

### 4.7 Device & Health Monitoring

| Component | Responsibility | Technology |
|-----------|----------------|------------|
| `HealthMonitor` | System health monitoring (CPU, memory, disk) | Qt System Info |
| `ClockSyncService` | NTP time synchronization | NTP client |
| `FirmwareManager` | Firmware update management | File I/O, signature verification |
| `WatchdogService` | Application crash detection and recovery | OS watchdog APIs |
| `BackupManager` | Database backup and recovery | File I/O, encryption |
| `DataArchiver` | Data archival and retention management (exports old data to archive files) | File I/O, compression |
| `DeviceRegistrationService` | Device registration with central server | HTTPS/REST |

### 4.8 Infrastructure Utilities

| Component | Purpose | Status | Location | Documentation |
|-----------|---------|--------|----------|---------------|
| `ObjectPool<T>` | Object pooling for high-frequency allocations | ⏳ Not implemented | `infrastructure/utils/` | [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) |
| `LockFreeQueue<T>` | Lock-free queue for inter-thread communication | ⏳ Not implemented | External libraries recommended | [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) |
| `LogBuffer` | Pre-allocated log buffer for high-frequency logging | ⏳ Not implemented | `infrastructure/utils/` | [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) |
| `MemoryPool<T>` | Memory pool allocator for fixed-size objects | ⏳ Not implemented | `infrastructure/utils/` | [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) |
| `CryptoUtils` | Cryptographic utilities (hashing, encoding) | ⏳ Not implemented | `infrastructure/utils/` | - |
| `DateTimeUtils` | Date/time formatting and parsing utilities | ⏳ Not implemented | `infrastructure/utils/` | - |
| `StringUtils` | String manipulation utilities | ⏳ Not implemented | `infrastructure/utils/` | - |
| `ValidationUtils` | Input validation utilities | ⏳ Not implemented | `infrastructure/utils/` | - |

**Note:** Utility classes are planned but not yet implemented. See [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) Section 12 for implementation status and recommendations.

---

## 5. Interface Layer Components

### 5.1 QML Controllers (QObject Bridges)

| Controller | Responsibility | Application Service Dependencies | Infrastructure Dependencies |
|------------|----------------|--------------------------------|----------------------------|
| `DashboardController` | Exposes real-time vitals to Dashboard View | `MonitoringService` | `VitalsCache`, `WaveformCache` (for display) |
| `AlarmController` | Exposes alarm state and history to QML | `MonitoringService` | None |
| `PatientController` | Patient admission/discharge/lookup UI | `AdmissionService` | `IPatientLookupService` |
| `SettingsController` | Device settings UI (excluding network) | None | `SettingsManager` |
| `ProvisioningController` | Provisioning/pairing UI | `ProvisioningService` | `QRCodeGenerator` |
| `TrendsController` | Historical data visualization | `MonitoringService` | `IVitalsRepository` |
| `SystemController` | System-wide state, navigation, power management | `SecurityService` | `HealthMonitor` |
| `NotificationController` | Non-critical messages and warnings | None | `LogService` |
| `DiagnosticsController` | System diagnostics and logs | None | `LogService`, `HealthMonitor` |
| `AuthenticationController` | Login/logout UI | `SecurityService` | None |

### 5.2 QML UI Components (Reusable)

| Component | File | Responsibility | Properties |
|-----------|------|----------------|-----------|
| `StatCard` | `StatCard.qml` | Displays single vital sign with label and unit | `label`, `value`, `unit`, `color` |
| `PatientBanner` | `PatientBanner.qml` | Displays patient info header (tappable for admission) | `patientName`, `mrn`, `age`, `bedLocation`, `admissionState` |
| `AlarmIndicator` | `AlarmIndicator.qml` | Visual alarm indicator with priority color | `priority`, `message`, `isActive` |
| `NotificationBell` | `NotificationBell.qml` | Notification badge with count | `unreadCount` |
| `Sidebar` | `Sidebar.qml` | Navigation sidebar | `currentView`, `onNavigate` signal |
| `TopBar` | `TopBar.qml` | Top application bar with logout/settings | `userName`, `connectionStatus` |
| `TrendChart` | `TrendChart.qml` | Line chart for historical data | `dataPoints`, `xAxis`, `yAxis` |
| `WaveformDisplay` | `WaveformDisplay.qml` | Real-time waveform display (ECG, SpO2 pleth) | `waveformData`, `channel`, `sampleRate` |
| `SettingsRow` | `SettingsRow.qml` | Single settings row (label + input) | `label`, `value`, `editable` |
| `ConfirmDialog` | `ConfirmDialog.qml` | Confirmation dialog | `title`, `message`, `onConfirm`, `onCancel` |
| `LoadingSpinner` | `LoadingSpinner.qml` | Loading indicator | `isLoading`, `message` |
| `QRCodeDisplay` | `QRCodeDisplay.qml` | QR code display for provisioning | `qrCodeData`, `pairingCode`, `expiresIn` |

### 5.3 QML Views (Full Screens)

| View | File | Controller | Responsibility |
|------|------|------------|----------------|
| `LoginView` | `LoginView.qml` | `AuthenticationController` | User login screen |
| `DashboardView` | `DashboardView.qml` | `DashboardController` | Main monitoring screen with vitals and waveforms |
| `TrendsView` | `TrendsView.qml` | `TrendsController` | Historical data visualization |
| `AlarmsView` | `AlarmsView.qml` | `AlarmController` | Alarm history and management |
| `SettingsView` | `SettingsView.qml` | `SettingsController`, `ProvisioningController` | Device configuration |
| `DiagnosticsView` | `DiagnosticsView.qml` | `DiagnosticsController` | System logs and diagnostics |
| `PatientAdmissionModal` | `PatientAdmissionModal.qml` | `PatientController` | Patient admission workflow |

---

## 6. Component Interaction Diagram

[View Component Interaction Diagram (Mermaid)](./29_SYSTEM_COMPONENTS.mmd)  
[View Component Interaction Diagram (SVG)](./29_SYSTEM_COMPONENTS.svg)

This diagram shows the complete flow of data and control through all system layers, from QML UI components through controllers, application services, domain aggregates, and infrastructure adapters.

---

## 7. External Systems

| System | Interface | Purpose |
|--------|-----------|---------|
| **Central Server** | REST API (HTTPS/mTLS) | Receives telemetry data, provides provisioning payloads |
| **HIS/EHR** | REST API (HTTPS) | Patient lookup service |
| **Central Station** | REST API (HTTPS) | Provisioning orchestration, patient assignments |
| **Sensor Simulator** | Shared memory ring buffer (`memfd://zmonitor-sim-ring`) + Unix control socket | External sensor data simulator (development) |
| **NTP Server** | NTP protocol | Time synchronization |
| **CRL Server** | HTTP/HTTPS | Certificate revocation list |

---

## 8. Component Count Summary

| Layer | Component Type | Count |
|-------|---------------|-------|
| **Domain** | Aggregates | 8 |
| **Domain** | Value Objects | 10 |
| **Domain** | Domain Events | 12 |
| **Domain** | Repository Interfaces | 7 |
| **Domain** | External Service Interfaces | 5 | ← **Updated:** Added `IUserManagementService` |
| **Application** | Services | 7 |
| **Application** | DTOs | 6 |
| **Infrastructure** | Repository Implementations | 7 |
| **Infrastructure** | Data Caching Components | 4 |
| **Infrastructure** | Network Adapters | 5 |
| **Infrastructure** | Sensor Data Source Adapters | 5 |
| **Infrastructure** | User Management Service Adapters | 2 | ← **NEW:** Mock + Hospital adapters |
| **Infrastructure** | Qt Adapters | 3 |
| **Infrastructure** | Security Adapters | 6 |
| **Infrastructure** | Device/Health Adapters | 7 | ← **Updated:** Added BackupManager, DataArchiver, DeviceRegistrationService |
| **Infrastructure** | Utilities | 8 | ← **NEW:** Infrastructure utility classes (not yet implemented) |
| **Interface** | QML Controllers | 10 |
| **Interface** | QML Components (Reusable) | 12 |
| **Interface** | QML Views | 7 |
| **Total** | | **128** | ← **Updated:** 120 + 3 infrastructure components + 8 utilities (planned) |

---

## 9. DDD Migration Plan

When implementing the DDD structure:

1. **Create directories** `z-monitor/src/domain`, `application`, `infrastructure`, `interface`.
2. **Move domain logic** from monolithic classes (e.g., `PatientManager`) into aggregates/value objects.
3. **Extract repositories** interfaces and implementations.
4. **Refactor controllers** to depend on application services.
5. **Update build files** (`CMakeLists.txt`) to reflect new structure.

Tracking tasks for this migration should be added to `ZTODO.md` (see "DDD Refactor" entries).

---

## 10. Maintenance Guidelines

### 10.1 When Adding New Components

When adding a new component, update this document:
1. Add the component to the appropriate section (Domain/Application/Infrastructure/Interface)
2. Update the component count summary
3. Update the interaction diagram (`29_SYSTEM_COMPONENTS.mmd`) if the component introduces new interactions
4. Regenerate the SVG: `npx -y @mermaid-js/mermaid-cli -i doc/z-monitor/architecture_and_design/29_SYSTEM_COMPONENTS.mmd -o doc/z-monitor/architecture_and_design/29_SYSTEM_COMPONENTS.svg`
5. Update related documents:
   - `doc/z-monitor/architecture_and_design/02_ARCHITECTURE.md` – Add to architecture descriptions
   - `doc/z-monitor/architecture_and_design/09_CLASS_DESIGNS.md` – Add class/interface documentation
6. Add implementation tasks to `ZTODO.md`

### 10.2 When Removing Components

1. Remove from this document
2. Update component count summary
3. Update interaction diagram if needed
4. Update references in other documentation
5. Mark as "removed" or "deprecated" in `ZTODO.md`

### 10.3 When Refactoring

If a component changes layers (e.g., moving from infrastructure to domain):
1. Update this document to reflect new layer assignment
2. Update interaction diagram
4. Update `doc/z-monitor/architecture_and_design/27_PROJECT_STRUCTURE.md` with new file paths

---

## 11. References

- [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md) – **Module-based class architecture overview** ⭐
  - [09a_INTERFACE_MODULE.md](./09a_INTERFACE_MODULE.md) – Interface Module class designs
  - [09b_REALTIME_MODULE.md](./09b_REALTIME_MODULE.md) – Real-Time Processing Module class designs
  - [09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md) – Application Services Module class designs
  - [09d_DATABASE_MODULE.md](./09d_DATABASE_MODULE.md) – Database Module class designs
  - [09e_NETWORK_MODULE.md](./09e_NETWORK_MODULE.md) – Network Module class designs
  - [09f_BACKGROUND_MODULE.md](./09f_BACKGROUND_MODULE.md) – Background Tasks Module class designs
- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) – Thread and module architecture
- [02_ARCHITECTURE.md](./02_ARCHITECTURE.md) – High-level architecture and data flow
- [27_PROJECT_STRUCTURE.md](./27_PROJECT_STRUCTURE.md) – File system organization
- [30_DATABASE_ACCESS_STRATEGY.md](./30_DATABASE_ACCESS_STRATEGY.md) – Database access and ORM strategy
- [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) – Data caching architecture
- [ZTODO.md](../../../ZTODO.md) – Implementation tasks

---

**Document Version:** 3.0  
**Last Updated:** 2025-11-27  
**Status:** Merged DDD strategy from 28_DOMAIN_DRIVEN_DESIGN.md. Now serves as comprehensive DDD strategy and component inventory.

*This document serves as the authoritative source for component inventory and DDD guidelines. Keep it synchronized with the codebase.*
