# System Components Reference

This document provides a complete, authoritative list of all system components in the Z Monitor application and their interactions. This serves as the single source of truth for component inventory.

---

## 1. Overview

The Z Monitor application is organized according to Domain-Driven Design (DDD) principles with the following layers:

- **Domain Layer**: Pure business logic (aggregates, value objects, domain events, repository interfaces)
- **Application Layer**: Use case orchestration (services, DTOs)
- **Infrastructure Layer**: Technical implementations (persistence, networking, Qt adapters)
- **Interface Layer**: UI integration (QML controllers, QML components)

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
| `IAlarmRepository` | AlarmAggregate | `save()`, `getActive()`, `getHistory()` |
| `IProvisioningRepository` | ProvisioningSession | `save()`, `findByDeviceId()`, `getHistory()` |
| `IUserRepository` | UserSession | `findByUserId()`, `save()`, `updateLastLogin()` |
| `IAuditRepository` | AuditTrailEntry | `save()`, `query()`, `archive()` |

---

## 3. Application Layer Components

### 3.1 Application Services

| Service | Responsibility | Dependencies (Repositories) | Dependencies (Services) |
|---------|----------------|----------------------------|------------------------|
| `MonitoringService` | Coordinates vitals ingestion, telemetry batching, transmission | `ITelemetryRepository`, `IPatientRepository`, `IAlarmRepository` | `SecurityService` (for signing) |
| `AdmissionService` | Executes admit/discharge/transfer use cases | `IPatientRepository`, `IAuditRepository` | `SecurityService` (for audit) |
| `ProvisioningService` | Handles QR pairing, certificate installation, validation | `IProvisioningRepository` | `SecurityService` (for audit) |
| `SecurityService` | Authentication, PIN policy, session lifecycle | `IUserRepository`, `IAuditRepository` | None |
| `DataArchiveService` | Archives old data per retention policies | `ITelemetryRepository`, `IAlarmRepository` | None |
| `FirmwareUpdateService` | Manages firmware updates | None | `SecurityService` (for signature validation) |
| `BackupService` | Database backup and restore | All repositories | `SecurityService` (for encryption) |

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
| `SQLiteAlarmRepository` | `IAlarmRepository` | SQLite + SQLCipher |
| `SQLiteProvisioningRepository` | `IProvisioningRepository` | SQLite + SQLCipher |
| `SQLiteUserRepository` | `IUserRepository` | SQLite + SQLCipher |
| `SQLiteAuditRepository` | `IAuditRepository` | SQLite + SQLCipher |

### 4.2 Network Adapters

| Component | Interface/Base | Technology | Responsibility |
|-----------|---------------|------------|----------------|
| `NetworkTelemetryServer` | `ITelemetryServer` | Qt Network (HTTPS/mTLS) | Production telemetry transmission |
| `MockTelemetryServer` | `ITelemetryServer` | In-memory | Testing/development telemetry endpoint |
| `HISPatientLookupAdapter` | `IPatientLookupService` | HTTPS/REST | Real HIS/EHR integration |
| `MockPatientLookupService` | `IPatientLookupService` | In-memory | Testing/development patient lookup |
| `CentralStationClient` | None | HTTPS/REST | Provisioning payload receiver |

### 4.3 Qt Adapters

| Component | Technology | Responsibility |
|-----------|------------|----------------|
| `DeviceSimulator` | Qt Timers | Generates simulated vitals/waveforms |
| `DatabaseManager` | Qt SQL + SQLCipher | SQLite connection management, migrations |
| `SettingsManager` | Qt Settings | Persistent configuration storage |
| `LogService` | Qt Logging | Centralized logging mechanism |

### 4.4 Security & Provisioning Adapters

| Component | Responsibility | Technology |
|-----------|----------------|------------|
| `CertificateManager` | Certificate loading, validation, renewal | OpenSSL via Qt |
| `QRCodeGenerator` | QR code generation | QR code library |
| `EncryptionService` | Payload encryption/decryption | OpenSSL via Qt |
| `SignatureService` | Data signing/verification | OpenSSL via Qt |
| `SecureStorage` | Secure key storage | Platform keychain (Keychain/Keyring) |

### 4.5 Device & Health Monitoring

| Component | Responsibility | Technology |
|-----------|----------------|------------|
| `HealthMonitor` | System health monitoring (CPU, memory, disk) | Qt System Info |
| `ClockSyncService` | NTP time synchronization | NTP client |
| `FirmwareManager` | Firmware update management | File I/O, signature verification |
| `WatchdogService` | Application crash detection and recovery | OS watchdog APIs |

---

## 5. Interface Layer Components

### 5.1 QML Controllers (QObject Bridges)

| Controller | Responsibility | Application Service Dependencies | Infrastructure Dependencies |
|------------|----------------|--------------------------------|----------------------------|
| `DashboardController` | Exposes real-time vitals to Dashboard View | `MonitoringService` | `DeviceSimulator` (for data) |
| `AlarmController` | Exposes alarm state and history to QML | `MonitoringService` | None |
| `PatientController` | Patient admission/discharge/lookup UI | `AdmissionService` | `IPatientLookupService` |
| `SettingsController` | Device settings UI (excluding network) | None | `SettingsManager` |
| `ProvisioningController` | Provisioning/pairing UI | `ProvisioningService` | None |
| `TrendsController` | Historical data visualization | `MonitoringService` | `ITelemetryRepository` |
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
| `SettingsRow` | `SettingsRow.qml` | Single settings row (label + input) | `label`, `value`, `editable` |
| `ConfirmDialog` | `ConfirmDialog.qml` | Confirmation dialog | `title`, `message`, `onConfirm`, `onCancel` |
| `LoadingSpinner` | `LoadingSpinner.qml` | Loading indicator | `isLoading`, `message` |
| `QRCodeDisplay` | `QRCodeDisplay.qml` | QR code display for provisioning | `qrCodeData`, `pairingCode`, `expiresIn` |

### 5.3 QML Views (Full Screens)

| View | File | Controller | Responsibility |
|------|------|------------|----------------|
| `LoginView` | `LoginView.qml` | `AuthenticationController` | User login screen |
| `DashboardView` | `DashboardView.qml` | `DashboardController` | Main monitoring screen with vitals |
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
| **NTP Server** | NTP protocol | Time synchronization |
| **CRL Server** | HTTP/HTTPS | Certificate revocation list |

---

## 8. Component Count Summary

| Layer | Component Type | Count |
|-------|---------------|-------|
| **Domain** | Aggregates | 8 |
| **Domain** | Value Objects | 8 |
| **Domain** | Domain Events | 11 |
| **Domain** | Repository Interfaces | 6 |
| **Application** | Services | 7 |
| **Application** | DTOs | 6 |
| **Infrastructure** | Repository Implementations | 6 |
| **Infrastructure** | Network Adapters | 5 |
| **Infrastructure** | Qt Adapters | 4 |
| **Infrastructure** | Security Adapters | 5 |
| **Infrastructure** | Device/Health Adapters | 4 |
| **Interface** | QML Controllers | 10 |
| **Interface** | QML Components (Reusable) | 11 |
| **Interface** | QML Views | 7 |
| **Total** | | **98** |

---

## 9. Maintenance Guidelines

### 9.1 When Adding New Components

When adding a new component, update this document:
1. Add the component to the appropriate section (Domain/Application/Infrastructure/Interface)
2. Update the component count summary
3. Update the interaction diagram (`29_SYSTEM_COMPONENTS.mmd`) if the component introduces new interactions
4. Regenerate the SVG: `npx @mermaid-js/mermaid-cli -i doc/29_SYSTEM_COMPONENTS.mmd -o doc/29_SYSTEM_COMPONENTS.svg`
5. Update related documents:
   - `doc/02_ARCHITECTURE.md` – Add to architecture descriptions
   - `doc/09_CLASS_DESIGNS.md` – Add class/interface documentation
   - `doc/28_DOMAIN_DRIVEN_DESIGN.md` – Update if it's a domain component
6. Add implementation tasks to `ZTODO.md`

### 9.2 When Removing Components

1. Remove from this document
2. Update component count summary
3. Update interaction diagram if needed
4. Update references in other documentation
5. Mark as "removed" or "deprecated" in `ZTODO.md`

### 9.3 When Refactoring

If a component changes layers (e.g., moving from infrastructure to domain):
1. Update this document to reflect new layer assignment
2. Update interaction diagram
3. Update `doc/28_DOMAIN_DRIVEN_DESIGN.md` if relevant
4. Update `doc/27_PROJECT_STRUCTURE.md` with new file paths

---

## 10. References

- `doc/02_ARCHITECTURE.md` – High-level architecture and data flow
- `doc/09_CLASS_DESIGNS.md` – Detailed class documentation
- `doc/28_DOMAIN_DRIVEN_DESIGN.md` – DDD strategy and guidelines
- `doc/27_PROJECT_STRUCTURE.md` – File system organization
- `ZTODO.md` – Implementation tasks

---

*This document serves as the authoritative source for component inventory. Keep it synchronized with the codebase.*

