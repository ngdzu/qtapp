---
title: "System Components Architecture"
doc_id: DOC-ARCH-016
version: 1.0
category: Architecture
phase: 6D
status: Draft
created: 2025-12-01
author: migration-bot
related:
  - DOC-ARCH-001_system_architecture.md
---

# System Components Architecture

This document provides a complete, authoritative list of all system components in the Z Monitor application and their interactions. It also defines how the application applies Domain-Driven Design (DDD) principles.

**Key Concepts:**
- **DDD Layers:** Domain, Application, Infrastructure, Interface
- **Bounded Contexts:** Monitoring, Admission/ADT, Provisioning, Security
- **Total Components:** 120+ across all layers
- **Module Architecture:** Components grouped by thread/execution context

## 1. Domain-Driven Design (DDD) Overview

### 1.1 DDD Guiding Principles

1. **Explicit Ubiquitous Language** – Terms like *Patient*, *DeviceSession*, *TelemetryBatch*, *Alarm* modeled as domain concepts
2. **Domain First** – Business rules live in domain entities/value objects
3. **Isolation** – Each bounded context has its own aggregates, repositories, events
4. **Record Classes** – Immutable `struct`/`class` types for value objects
5. **Layered Architecture** – Code organized into `domain/`, `application/`, `infrastructure/`, `interface/`

### 1.2 Bounded Contexts

| Context           | Description                                      | Key Aggregates                                                            |
| ----------------- | ------------------------------------------------ | ------------------------------------------------------------------------- |
| **Monitoring**    | Real-time vitals, alarms, telemetry transmission | `PatientAggregate`, `DeviceAggregate`, `TelemetryBatch`, `AlarmAggregate` |
| **Admission/ADT** | Patient admission, discharge, transfer workflow  | `AdmissionAggregate`, `PatientIdentity`, `BedAssignment`                  |
| **Provisioning**  | Device pairing, certificate management           | `ProvisioningSession`, `CredentialBundle`                                 |
| **Security**      | Authentication, authorization, audit logging     | `UserSession`, `PinCredential`, `AuditTrailEntry`                         |

### 1.3 Layer Responsibilities

- **Domain Layer** (`domain/`) – Pure business logic, aggregates, value objects, domain events, repository interfaces. No Qt or SQL includes.
- **Application Layer** (`application/`) – Use cases (admit patient, push telemetry). Coordinates domain objects and repositories.
- **Infrastructure Layer** (`infrastructure/`) – Implementations of repositories, network adapters, persistence, crypto.
- **Interface Layer** (`interface/`) – QML views and QObject controllers binding data to UI.

> **📋 Detailed Structure:** For complete directory structure, see **[DOC-ARCH-015: Project Structure](./DOC-ARCH-015_project_structure.md)**.

---

## 2. Domain Layer Components

### 2.1 Aggregates

| Aggregate             | Context       | Responsibility                            | Key Methods                                       |
| --------------------- | ------------- | ----------------------------------------- | ------------------------------------------------- |
| `PatientAggregate`    | Monitoring    | Patient admission lifecycle, vitals state | `admit()`, `discharge()`, `updateVitals()`        |
| `DeviceAggregate`     | Provisioning  | Device provisioning state, credentials    | `applyProvisioningPayload()`, `markProvisioned()` |
| `TelemetryBatch`      | Monitoring    | Telemetry data collection, signing        | `addVital()`, `sign()`, `validate()`              |
| `AlarmAggregate`      | Monitoring    | Alarm lifecycle, state transitions        | `raise()`, `acknowledge()`, `silence()`           |
| `AdmissionAggregate`  | Admission/ADT | Admission/discharge/transfer workflow     | `admitPatient()`, `dischargePatient()`            |
| `ProvisioningSession` | Provisioning  | Pairing workflow, QR code lifecycle       | `generatePairingCode()`, `validateCode()`         |
| `UserSession`         | Security      | Authentication, session management        | `authenticate()`, `refreshSession()`              |
| `AuditTrailEntry`     | Security      | Security event auditing                   | `logEvent()`, `getEventHistory()`                 |

### 2.2 Value Objects

| Value Object      | Description                             | Immutable |
| ----------------- | --------------------------------------- | --------- |
| `PatientIdentity` | MRN, Name, DOB, Sex                     | Yes       |
| `DeviceSnapshot`  | DeviceId, Label, Firmware, Status       | Yes       |
| `VitalRecord`     | Single vital measurement (HR, SpO2, RR) | Yes       |
| `WaveformSample`  | Single waveform sample                  | Yes       |
| `AlarmSnapshot`   | Alarm state at point in time            | Yes       |
| `BedLocation`     | Bed/unit/facility identifier            | Yes       |
| `PinCredential`   | Hashed PIN with salt                    | Yes       |

### 2.3 Domain Events

| Event                   | Context       | Triggered When                   | Consumed By                      |
| ----------------------- | ------------- | -------------------------------- | -------------------------------- |
| `PatientAdmitted`       | Admission/ADT | Patient admitted to device       | UI controllers, audit, telemetry |
| `PatientDischarged`     | Admission/ADT | Patient discharged               | UI controllers, audit            |
| `TelemetryQueued`       | Monitoring    | Telemetry ready for transmission | Network manager                  |
| `AlarmRaised`           | Monitoring    | New alarm triggered              | UI controllers, alarm manager    |
| `ProvisioningCompleted` | Provisioning  | Device successfully provisioned  | UI controllers, settings         |

### 2.4 Repository Interfaces (Domain)

| Repository Interface   | Aggregate        | Key Methods                                      |
| ---------------------- | ---------------- | ------------------------------------------------ |
| `IPatientRepository`   | PatientAggregate | `findByMrn()`, `save()`, `getAdmissionHistory()` |
| `ITelemetryRepository` | TelemetryBatch   | `save()`, `getHistorical()`, `archive()`         |
| `IVitalsRepository`    | VitalRecord      | `save()`, `getRange()`, `getUnsynced()`          |
| `IAlarmRepository`     | AlarmAggregate   | `save()`, `getActive()`, `getHistory()`          |
| `IUserRepository`      | UserSession      | `findByUserId()`, `updateLastLogin()`            |

### 2.5 External Service Interfaces (Domain)

| Interface                | Purpose                       | Key Methods                           |
| ------------------------ | ----------------------------- | ------------------------------------- |
| `ISensorDataSource`      | Abstracts sensor data input   | `start()`, `stop()`, `isActive()`     |
| `IPatientLookupService`  | Patient lookup from HIS/EHR   | `lookupPatient()`, `searchByName()`   |
| `ITelemetryServer`       | Secure telemetry transmission | `sendBatch()`, `sendAlarm()`          |
| `IProvisioningService`   | Device provisioning           | `requestProvisioning()`               |
| `IUserManagementService` | Hospital user authentication  | `authenticate()`, `checkPermission()` |

---

## 3. Application Layer Components

### 3.1 Application Services

| Service                 | Responsibility                           | Repository Dependencies                                         |
| ----------------------- | ---------------------------------------- | --------------------------------------------------------------- |
| `MonitoringService`     | Vitals ingestion, telemetry batching     | `ITelemetryRepository`, `IVitalsRepository`, `IAlarmRepository` |
| `AdmissionService`      | Admit/discharge/transfer use cases       | `IPatientRepository`, `IAuditRepository`                        |
| `ProvisioningService`   | QR pairing, certificate installation     | `IProvisioningRepository`                                       |
| `SecurityService`       | Authentication, session management, RBAC | `IUserRepository`, `IAuditRepository`                           |
| `DataArchiveService`    | Data archival workflow orchestration     | All repositories                                                |
| `FirmwareUpdateService` | Firmware updates                         | None (coordinates infrastructure)                               |

### 3.2 DTOs (Data Transfer Objects)

| DTO                   | Purpose                            | Used By                                       |
| --------------------- | ---------------------------------- | --------------------------------------------- |
| `AdmitPatientCommand` | Patient admission request          | `AdmissionService`, `PatientController`       |
| `TelemetrySubmission` | Telemetry data for transmission    | `MonitoringService`, `NetworkManager`         |
| `ProvisioningPayload` | Configuration from Central Station | `ProvisioningService`                         |
| `LoginRequest`        | Authentication request             | `SecurityService`, `AuthenticationController` |

---

## 4. Infrastructure Layer Components

### 4.1 Persistence (Repository Implementations)

| Implementation            | Interface            | Technology         |
| ------------------------- | -------------------- | ------------------ |
| `SQLitePatientRepository` | `IPatientRepository` | SQLite + SQLCipher |
| `SQLiteVitalsRepository`  | `IVitalsRepository`  | SQLite + SQLCipher |
| `SQLiteAlarmRepository`   | `IAlarmRepository`   | SQLite + SQLCipher |
| `SQLiteUserRepository`    | `IUserRepository`    | SQLite + SQLCipher |

### 4.2 Data Caching Components

| Component              | Purpose                       | Technology           | Capacity             |
| ---------------------- | ----------------------------- | -------------------- | -------------------- |
| `VitalsCache`          | In-memory vitals cache        | `std::deque` + locks | 3-day (~39 MB)       |
| `WaveformCache`        | Circular buffer for waveforms | `std::deque`         | 30 seconds (~0.1 MB) |
| `PersistenceScheduler` | Periodic DB persistence       | `QTimer`             | Every 10 min         |
| `DataCleanupService`   | Daily data cleanup            | `QTimer`             | Runs 3 AM            |

### 4.3 Network Adapters

| Component                 | Interface               | Technology              | Use Case              |
| ------------------------- | ----------------------- | ----------------------- | --------------------- |
| `NetworkTelemetryServer`  | `ITelemetryServer`      | Qt Network (HTTPS/mTLS) | Production telemetry  |
| `MockTelemetryServer`     | `ITelemetryServer`      | In-memory               | Testing/development   |
| `HISPatientLookupAdapter` | `IPatientLookupService` | HTTPS/REST              | Real HIS integration  |
| `CentralStationClient`    | None                    | HTTPS/REST              | Provisioning receiver |

### 4.4 Sensor Data Source Adapters

| Component                      | Interface           | Technology          | Use Case              |
| ------------------------------ | ------------------- | ------------------- | --------------------- |
| `SharedMemorySensorDataSource` | `ISensorDataSource` | POSIX shared memory | Production simulator  |
| `SimulatorDataSource`          | `ISensorDataSource` | Qt Timers           | Internal fallback     |
| `MockSensorDataSource`         | `ISensorDataSource` | In-memory           | Deterministic testing |
| `HardwareSensorAdapter`        | `ISensorDataSource` | Serial/USB          | Future real hardware  |

### 4.5 Platform Adapters (Qt-based)

| Component         | Technology         | Responsibility                |
| ----------------- | ------------------ | ----------------------------- |
| `DatabaseManager` | Qt SQL + SQLCipher | SQLite connection, migrations |
| `SettingsManager` | Qt Settings        | Persistent configuration      |
| `LogService`      | Qt Logging         | Centralized logging           |

### 4.6 Security & Provisioning Adapters

| Component            | Responsibility                   | Technology       |
| -------------------- | -------------------------------- | ---------------- |
| `CertificateManager` | Certificate loading, validation  | OpenSSL via Qt   |
| `KeyManager`         | Encryption key storage, rotation | HSM/TPM/Keychain |
| `EncryptionService`  | Payload encryption/decryption    | OpenSSL          |
| `SignatureService`   | Data signing (HMAC-SHA256)       | OpenSSL          |

### 4.7 Device & Health Monitoring

| Component          | Responsibility                    | Technology            |
| ------------------ | --------------------------------- | --------------------- |
| `HealthMonitor`    | System health (CPU, memory, disk) | Qt System Info        |
| `ClockSyncService` | NTP time synchronization          | NTP client            |
| `FirmwareManager`  | Firmware update management        | File I/O, signatures  |
| `BackupManager`    | Database backup and recovery      | File I/O, encryption  |
| `DataArchiver`     | Data archival (exports old data)  | File I/O, compression |

---

## 5. Interface Layer Components

### 5.1 QML Controllers (QObject Bridges)

| Controller                 | Responsibility                  | Service Dependencies  |
| -------------------------- | ------------------------------- | --------------------- |
| `DashboardController`      | Exposes real-time vitals to QML | `MonitoringService`   |
| `AlarmController`          | Alarm state and history         | `MonitoringService`   |
| `PatientController`        | Patient admission/discharge UI  | `AdmissionService`    |
| `SettingsController`       | Device settings UI              | `SettingsManager`     |
| `ProvisioningController`   | Provisioning/pairing UI         | `ProvisioningService` |
| `TrendsController`         | Historical data visualization   | `MonitoringService`   |
| `SystemController`         | System-wide state, navigation   | `SecurityService`     |
| `AuthenticationController` | Login/logout UI                 | `SecurityService`     |

### 5.2 QML UI Components (Reusable)

| Component         | File                  | Responsibility                 | Properties                             |
| ----------------- | --------------------- | ------------------------------ | -------------------------------------- |
| `StatCard`        | `StatCard.qml`        | Displays single vital sign     | `label`, `value`, `unit`, `color`      |
| `PatientBanner`   | `PatientBanner.qml`   | Patient info header (tappable) | `patientName`, `mrn`, `admissionState` |
| `AlarmIndicator`  | `AlarmIndicator.qml`  | Visual alarm indicator         | `priority`, `message`, `isActive`      |
| `TrendChart`      | `TrendChart.qml`      | Line chart for historical data | `dataPoints`, `xAxis`, `yAxis`         |
| `WaveformDisplay` | `WaveformDisplay.qml` | Real-time waveform (ECG, SpO2) | `waveformData`, `channel`              |

### 5.3 QML Views (Full Screens)

| View            | File                | Controller                 | Responsibility           |
| --------------- | ------------------- | -------------------------- | ------------------------ |
| `LoginView`     | `LoginView.qml`     | `AuthenticationController` | User login screen        |
| `DashboardView` | `DashboardView.qml` | `DashboardController`      | Main monitoring screen   |
| `TrendsView`    | `TrendsView.qml`    | `TrendsController`         | Historical data viz      |
| `AlarmsView`    | `AlarmsView.qml`    | `AlarmController`          | Alarm history management |
| `SettingsView`  | `SettingsView.qml`  | `SettingsController`       | Device configuration     |

---

## 6. Component Interaction Diagram

**Flow:** QML UI → Controllers → Application Services → Domain Aggregates → Repositories → Infrastructure

**Example:**
```
DashboardView.qml
  ↓ (property binding)
DashboardController (interface)
  ↓ (Qt signals)
MonitoringService (application)
  ↓ (domain logic)
PatientAggregate / TelemetryBatch (domain)
  ↓ (persistence)
SQLiteTelemetryRepository (infrastructure)
```

---

## 7. External Systems

| System               | Interface                   | Purpose                                   |
| -------------------- | --------------------------- | ----------------------------------------- |
| **Central Server**   | REST API (HTTPS/mTLS)       | Receives telemetry, provides provisioning |
| **HIS/EHR**          | REST API (HTTPS)            | Patient lookup service                    |
| **Sensor Simulator** | Shared memory + Unix socket | External sensor data (development)        |
| **NTP Server**       | NTP protocol                | Time synchronization                      |

---

## 8. Component Count Summary

| Layer              | Component Type              | Count    |
| ------------------ | --------------------------- | -------- |
| **Domain**         | Aggregates                  | 8        |
| **Domain**         | Value Objects               | 10       |
| **Domain**         | Domain Events               | 12       |
| **Domain**         | Repository Interfaces       | 7        |
| **Domain**         | External Service Interfaces | 5        |
| **Application**    | Services                    | 7        |
| **Application**    | DTOs                        | 6        |
| **Infrastructure** | Repository Implementations  | 7        |
| **Infrastructure** | Caching Components          | 4        |
| **Infrastructure** | Network Adapters            | 5        |
| **Infrastructure** | Sensor Adapters             | 5        |
| **Infrastructure** | Platform Adapters           | 3        |
| **Infrastructure** | Security Adapters           | 6        |
| **Infrastructure** | Device/Health               | 7        |
| **Interface**      | QML Controllers             | 10       |
| **Interface**      | QML Components              | 12       |
| **Interface**      | QML Views                   | 7        |
| **Total**          |                             | **120+** |

---

## 9. Module Architecture

Components are organized into **6 modules** (thread-assigned groups):

| Module                   | Thread       | Component Count | Documentation                |
| ------------------------ | ------------ | --------------- | ---------------------------- |
| **Interface Module**     | Main/UI      | 30              | Controllers, QML views       |
| **Real-Time Processing** | RT Thread    | 12              | Sensor data, alarm detection |
| **Application Services** | App Services | 12              | Use case orchestration       |
| **Database Module**      | Database I/O | 13              | Persistence, logging         |
| **Network Module**       | Network I/O  | 11              | Telemetry, certificates      |
| **Background Tasks**     | Background   | 9               | Health, backup, firmware     |

> **📋 Detailed Module Design:** See legacy `z-monitor/architecture_and_design/09_CLASS_DESIGNS_OVERVIEW.md` for complete module architecture.

---

## 10. Maintenance Guidelines

### 10.1 When Adding New Components

1. Add component to appropriate DDD layer section
2. Update component count summary
3. Update interaction diagram if needed
4. Update related documents (DOC-ARCH-001, DOC-ARCH-015)
5. Add implementation tasks to ZTODO.md

### 10.2 When Removing Components

1. Remove from this document
2. Update component count
3. Update references in other documentation
4. Mark as "removed" in ZTODO.md

---

## 11. Related Documents

- **[DOC-ARCH-001: Architecture Overview](./DOC-ARCH-001_architecture_overview.md)** - High-level architecture
- **[DOC-ARCH-013: Dependency Injection](./DOC-ARCH-013_dependency_injection.md)** - DI patterns and AppContainer
- **[DOC-ARCH-015: Project Structure](./DOC-ARCH-015_project_structure.md)** - File system organization
- **[DOC-ARCH-017: Database Design](./DOC-ARCH-017_database_design.md)** - Database schema and access
- **Legacy:** `z-monitor/architecture_and_design/29_SYSTEM_COMPONENTS.md` - Complete component inventory (being migrated)
- **Legacy:** `z-monitor/architecture_and_design/09_CLASS_DESIGNS_OVERVIEW.md` - Module-based class architecture

---
**Status:** ✅ Migrated from legacy 29_SYSTEM_COMPONENTS.md (condensed for architecture overview)
