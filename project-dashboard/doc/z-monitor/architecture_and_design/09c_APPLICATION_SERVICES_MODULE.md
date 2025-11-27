# Application Services Module: Class Designs

**Document ID:** DESIGN-009c  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides detailed class designs for the **Application Services Module**, which handles business logic orchestration for use cases on the Application Services Thread.

> **📋 Related Documents:**
> - [Class Designs Overview (09_CLASS_DESIGNS_OVERVIEW.md)](./09_CLASS_DESIGNS_OVERVIEW.md) - High-level module architecture
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Thread architecture (Section 4.3: Application Services Thread)
> - [Authentication Workflow (38_AUTHENTICATION_WORKFLOW.md)](./38_AUTHENTICATION_WORKFLOW.md) - Authentication design
> - [ADT Workflow (19_ADT_WORKFLOW.md)](./19_ADT_WORKFLOW.md) - Patient admission/discharge workflow

---

## 1. Module Overview

**Thread:** Application Services Thread (or co-located with RT Thread)  
**Priority:** Normal to High (depending on co-location)  
**Component Count:** 11 components

**Purpose:**
- Execute use cases (admit patient, provision device, authenticate user)
- Coordinate between domain aggregates and repositories
- Emit domain events for UI updates
- Enforce business rules and validation

---

## 2. Module Diagram

[View Application Services Module Diagram (Mermaid)](./09c_APPLICATION_SERVICES_MODULE.mmd)  
[View Application Services Module Diagram (SVG)](./09c_APPLICATION_SERVICES_MODULE.svg)

---

## 3. Application Services

### 3.1. AdmissionService

**Responsibility:** Executes admit/discharge/transfer use cases against `PatientAggregate`.

**Thread:** Application Services Thread

**Note:** `PatientManager` (if it exists as a separate service) would coordinate patient context management. It may be the same as or coordinate with `AdmissionService`. For detailed `PatientManager` class design, see the legacy content in [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) section 2.5.

**Key Methods:**
- `admitPatient(const AdmitPatientCommand& command)`: Admits patient to device
  - Validates user permissions (via `SecurityService`)
  - Queries HIS/EHR via `IPatientLookupService` (or uses local cache)
  - Creates `PatientAggregate` via `AdmissionAggregate`
  - Logs admission event to `IAuditRepository`
  - Emits `PatientAdmitted` domain event
- `dischargePatient(const DischargePatientCommand& command)`: Discharges patient
  - Validates user permissions
  - Updates `PatientAggregate` via `AdmissionAggregate`
  - Logs discharge event
  - Emits `PatientDischarged` domain event
- `transferPatient(const TransferPatientCommand& command)`: Transfers patient to another device
  - Validates user permissions
  - Updates `PatientAggregate`
  - Logs transfer event
  - Emits `PatientTransferred` domain event

**Dependencies:**
- `AdmissionAggregate` - Admission/discharge/transfer workflow
- `IPatientRepository` - Patient data persistence
- `IPatientLookupService` - Patient lookup from HIS/EHR
- `SecurityService` - User authentication and permissions
- `IAuditRepository` - Audit logging

**Domain Events:**
- `PatientAdmitted` - Emitted when patient admitted
- `PatientDischarged` - Emitted when patient discharged
- `PatientTransferred` - Emitted when patient transferred

---

### 3.2. ProvisioningService

**Responsibility:** Handles QR pairing flows, applies credential bundles.

**Thread:** Application Services Thread

**Key Methods:**
- `enterProvisioningMode()`: Enters provisioning mode, generates initial pairing code
- `generatePairingCode()`: Generates new cryptographically secure pairing code
- `generateQRCode()`: Generates QR code with device information and pairing code
- `receiveConfiguration(const QByteArray& encryptedPayload)`: Receives and decrypts configuration payload
- `validateConfiguration(const ProvisioningConfig& config)`: Validates configuration signature and structure
- `applyConfiguration(const ProvisioningConfig& config)`: Applies configuration to device (certificates, server URL)
- `testConnection()`: Tests connection with newly provisioned configuration

**Dependencies:**
- `ProvisioningSession` - Pairing workflow aggregate
- `IProvisioningRepository` - Provisioning state persistence
- `CertificateManager` - Certificate installation (Network Module)
- `SettingsManager` - Configuration storage (Background Module)
- `NetworkManager` - Connection testing (Network Module)

**Domain Events:**
- `ProvisioningCompleted` - Emitted when provisioning completes
- `ProvisioningFailed` - Emitted when provisioning fails

---

### 3.3. SecurityService

**Responsibility:** Authentication, session management, RBAC enforcement, permission checking.

**Thread:** Application Services Thread

**Key Methods:**
- `authenticate(const QString& userId, const QString& secretCode)`: Authenticates user
  - Delegates to `IUserManagementService` (hospital server or mock)
  - Creates `UserSession` aggregate
  - Logs authentication event to `IAuditRepository`
  - Emits `UserLoggedIn` domain event
- `validateSession(const QString& sessionToken)`: Validates session token
  - Checks session expiration
  - Refreshes session if valid
- `logout()`: Logs out current user
  - Terminates `UserSession`
  - Logs logout event
  - Emits `UserLoggedOut` domain event
- `checkPermission(const QString& action, const QString& resource)`: Checks user permission
  - Delegates to `IUserManagementService`
  - Returns `bool` (true if permitted)
- `getPermissions()`: Returns user's permissions
  - Delegates to `IUserManagementService`
  - Returns `QStringList` of permission strings

**Dependencies:**
- `UserSession` - Authentication and session management aggregate
- `IUserManagementService` - Hospital user authentication (Network Module)
- `IAuditRepository` - Security audit logging (Database Module)
- `IActionLogRepository` - Action logging (Database Module)

**Domain Events:**
- `UserLoggedIn` - Emitted when user logs in
- `UserLoggedOut` - Emitted when user logs out
- `SessionExpired` - Emitted when session expires

**See:** [38_AUTHENTICATION_WORKFLOW.md](./38_AUTHENTICATION_WORKFLOW.md) for complete authentication workflow.

---

## 4. Domain Aggregates

### 4.1. AdmissionAggregate

**Responsibility:** Admission/discharge/transfer workflow.

**Thread:** Application Services Thread

**Key Methods:**
- `admitPatient(const PatientIdentity& identity, const BedLocation& bed)`: Admits patient
- `dischargePatient(const QString& mrn)`: Discharges patient
- `transferPatient(const QString& mrn, const QString& targetDevice)`: Transfers patient

**Domain Events:**
- `PatientAdmitted`, `PatientDischarged`, `PatientTransferred`

---

### 4.2. ProvisioningSession

**Responsibility:** Pairing workflow, QR code lifecycle.

**Thread:** Application Services Thread

**Key Methods:**
- `generatePairingCode()`: Generates cryptographically secure pairing code
- `validatePairingCode(const QString& code)`: Validates pairing code (not expired, not used)
- `applyConfiguration(const ProvisioningConfig& config)`: Applies configuration

**Domain Events:**
- `ProvisioningCompleted`, `ProvisioningFailed`

---

### 4.3. UserSession

**Responsibility:** Authentication, session management.

**Thread:** Application Services Thread

**Key Properties:**
- `sessionToken`: `QString` - Session token (hashed for storage)
- `userId`: `QString` - User identifier
- `role`: `UserRole` - User role (NURSE, PHYSICIAN, TECHNICIAN, ADMINISTRATOR)
- `permissions`: `QStringList` - User permissions
- `expiresAt`: `QDateTime` - Session expiration time
- `lastActivityTime`: `QDateTime` - Last activity timestamp

**Key Methods:**
- `authenticate(const QString& userId, const QString& secretCode)`: Authenticates user
- `refreshSession()`: Refreshes session timeout on activity
- `terminate()`: Terminates session

**Domain Events:**
- `UserLoggedIn`, `UserLoggedOut`, `SessionExpired`

---

### 4.4. AuditTrailEntry

**Responsibility:** Security event auditing.

**Thread:** Application Services Thread

**Key Methods:**
- `logEvent(const AuditEntry& entry)`: Logs security event
- `getEventHistory(const AuditFilter& filter)`: Retrieves event history

---

## 5. Value Objects

### 5.1. PatientIdentity

**Responsibility:** Patient identity information (immutable value object).

**Properties:**
- `mrn`: `QString` - Medical Record Number
- `name`: `QString` - Patient name
- `dateOfBirth`: `QDate` - Date of birth
- `sex`: `QString` - Patient sex

---

### 5.2. BedLocation

**Responsibility:** Bed/unit/facility identifier (immutable value object).

**Properties:**
- `bedNumber`: `QString` - Bed number (e.g., "4")
- `unit`: `QString` - Unit identifier (e.g., "ICU")
- `facility`: `QString` - Facility identifier

---

### 5.3. PinCredential

**Responsibility:** Hashed PIN with salt (immutable value object).

**Properties:**
- `hashedPin`: `QByteArray` - SHA-256 hash of PIN
- `salt`: `QByteArray` - Per-user salt
- `createdAt`: `QDateTime` - Credential creation time

---

### 5.4. CredentialBundle

**Responsibility:** Certificates, keys, server URL (immutable value object).

**Properties:**
- `clientCertificate`: `QSslCertificate` - Device client certificate
- `clientPrivateKey`: `QSslKey` - Device private key
- `caCertificate`: `QSslCertificate` - CA certificate
- `serverUrl`: `QString` - Central server URL

---

## 6. Module Communication

### 6.1. Inbound (From Other Modules)

**From Interface Module (UI Thread):**
- `Qt::QueuedConnection` method invocations (Q_INVOKABLE)
- User action commands (admit patient, provision device, login)

### 6.2. Outbound (To Other Modules)

**To Interface Module (UI Thread):**
- `Qt::QueuedConnection` domain events (PatientAdmitted, UserLoggedIn, etc.)

**To Database Module (Database I/O Thread):**
- `MPSC Queue` for repository calls (patient data, provisioning state, audit logs)

**To Network Module (Network I/O Thread):**
- `Qt::QueuedConnection` calls for patient lookup (HIS/EHR)
- `Qt::QueuedConnection` calls for user authentication (hospital server)

---

## 7. Related Documents

- **[09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md)** - High-level module architecture
- **[12_THREAD_MODEL.md](./12_THREAD_MODEL.md)** - Thread architecture (Section 4.3: Application Services Thread)
- **[38_AUTHENTICATION_WORKFLOW.md](./38_AUTHENTICATION_WORKFLOW.md)** - Authentication workflow
- **[19_ADT_WORKFLOW.md](./19_ADT_WORKFLOW.md)** - Patient admission/discharge workflow
- **[17_DEVICE_PROVISIONING.md](./17_DEVICE_PROVISIONING.md)** - Device provisioning workflow

---

*This document provides detailed class designs for the Application Services Module. For other modules, see the module-specific documents listed in [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md).*

