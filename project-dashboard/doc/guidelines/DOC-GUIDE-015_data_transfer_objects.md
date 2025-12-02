---
doc_id: DOC-GUIDE-015
title: Data Transfer Objects (DTOs) Design and Usage
version: 1.0
category: Guidelines
status: Approved
related_docs:
  - DOC-ARCH-001  # Software Architecture
  - DOC-GUIDE-014 # Database Access Strategy
  - DOC-ARCH-011  # Thread Model
tags:
  - dto
  - data-transfer
  - immutability
  - validation
  - serialization
source:
  original_id: DESIGN-031
  file: 31_DATA_TRANSFER_OBJECTS.md
  migrated_date: 2025-12-01
---

# Data Transfer Objects (DTOs) Design and Usage

## Purpose

This document defines the Data Transfer Objects (DTOs) used in the Z Monitor application, their purpose, structure, validation rules, and usage patterns.

**Data Transfer Objects** are simple, immutable data structures used to carry data between layers and threads in the application. They have no business logic and serve as pure data carriers.

## Purpose in Z Monitor

1. **Layer Communication:** Transfer data between Interface → Application → Infrastructure layers
2. **Thread Safety:** Pass data between threads without shared mutable state
3. **Validation Boundary:** Validate input at the edge (UI/API) before reaching domain
4. **Decoupling:** Prevent UI/controllers from depending on domain aggregates directly

## DTO vs. Value Object vs. Aggregate

| Concept          | Purpose                               | Layer       | Mutable   | Business Logic     |
| ---------------- | ------------------------------------- | ----------- | --------- | ------------------ |
| **DTO**          | Data transfer between layers/threads  | Application | Immutable | No                 |
| **Value Object** | Domain concept with identity by value | Domain      | Immutable | Maybe (validation) |
| **Aggregate**    | Domain entity with lifecycle          | Domain      | Mutable   | Yes                |

**Example:**
```
UI (QML) sends:
    AdmitPatientCommand (DTO)
        ↓
Application Service receives DTO, validates, creates:
    PatientAggregate (Domain) with PatientIdentity (Value Object)
        ↓
Infrastructure persists:
    SQL rows in patients table
```

## DTO Design Principles

### 1. Immutability

```cpp
struct AdmitPatientCommand {
    const QString mrn;
    const QString name;
    const QString admissionSource;
    const QDateTime admittedAt;
    
    // No setters, construct once
};
```

### 2. No Business Logic

```cpp
// ❌ BAD: Business logic in DTO
struct AdmitPatientCommand {
    bool isValidForAdmission() const;  // NO!
};

// ✅ GOOD: Validation in Application Service
class AdmissionService {
    bool validateCommand(const AdmitPatientCommand& cmd);
};
```

### 3. POD (Plain Old Data) Where Possible

```cpp
struct TelemetrySubmission {
    QString batchId;
    QString deviceId;
    QString patientMrn;
    QList<VitalRecord> vitals;  // Value objects
    QList<AlarmSnapshot> alarms;
    QDateTime timestamp;
    
    // No virtual methods, no complex hierarchy
};
```

### 4. Validation at Boundary

```cpp
// UI Controller validates DTO before passing to service
void PatientController::admitPatient(const QString& mrn, const QString& source) {
    // Validate at UI boundary
    if (mrn.isEmpty()) {
        emit error("MRN cannot be empty");
        return;
    }
    
    // Create DTO
    AdmitPatientCommand cmd {
        .mrn = mrn,
        .name = m_name,
        .admissionSource = source,
        .admittedAt = QDateTime::currentDateTime()
    };
    
    // Pass to service
    m_admissionService->admitPatient(cmd);
}
```

## Complete DTO Catalog

### Patient Management DTOs

#### AdmitPatientCommand

**Purpose:** Carries patient admission request from UI to `AdmissionService`

```cpp
struct AdmitPatientCommand {
    QString mrn;                // Medical Record Number (required)
    QString name;               // Patient name (required)
    QDate dateOfBirth;          // Date of birth (optional)
    QString sex;                // "M", "F", "Other" (optional)
    QString bedLocation;        // Bed location (e.g., "ICU-4B") (required)
    QString admissionSource;    // "manual", "barcode", "central_station" (required)
    QDateTime admittedAt;       // Admission timestamp (required)
    QString userId;             // User who performed admission (optional)
    
    bool isValid() const {
        return !mrn.isEmpty() 
            && !name.isEmpty() 
            && !bedLocation.isEmpty()
            && !admissionSource.isEmpty();
    }
};
```

**Usage:**
- **From:** `PatientController` (UI)
- **To:** `AdmissionService` (Application)
- **Thread:** Main/UI Thread → Application Services Thread

#### DischargePatientCommand

```cpp
struct DischargePatientCommand {
    QString mrn;                // Medical Record Number (required)
    QDateTime dischargedAt;     // Discharge timestamp (required)
    QString userId;             // User who performed discharge (optional)
    QString reason;             // Discharge reason (optional)
    
    bool isValid() const {
        return !mrn.isEmpty();
    }
};
```

#### TransferPatientCommand

```cpp
struct TransferPatientCommand {
    QString mrn;                // Medical Record Number (required)
    QString targetDeviceLabel;  // Target device (e.g., "ICU-MON-05") (required)
    QString targetBedLocation;  // Target bed location (required)
    QDateTime transferredAt;    // Transfer timestamp (required)
    QString userId;             // User who performed transfer (optional)
    
    bool isValid() const {
        return !mrn.isEmpty() 
            && !targetDeviceLabel.isEmpty()
            && !targetBedLocation.isEmpty();
    }
};
```

### Telemetry DTOs

#### TelemetrySubmission

**Purpose:** Carries telemetry batch from RT thread to Network thread for transmission

```cpp
struct TelemetrySubmission {
    QString batchId;            // UUID for batch (required)
    QString deviceId;           // Device identifier (required)
    QString deviceLabel;        // Device asset tag (required)
    QString patientMrn;         // Patient MRN (NULL if no patient) (nullable)
    
    // Data
    QList<VitalRecord> vitals;  // Vital sign records
    QList<AlarmSnapshot> alarms;  // Alarm snapshots
    QList<InfusionEvent> infusionEvents;  // Infusion events (optional)
    
    // Timing (for metrics)
    qint64 dataCreatedAt;       // First data point timestamp (Unix ms)
    qint64 batchCreatedAt;      // Batch creation timestamp (Unix ms)
    
    // Metadata
    int recordCount;            // Total records in batch
    
    bool isValid() const {
        return !batchId.isEmpty() 
            && !deviceId.isEmpty()
            && recordCount > 0;
    }
};
```

**Usage:**
- **From:** `MonitoringService` (RT Thread)
- **To:** `NetworkManager` (Network Thread)
- **Thread:** RT Processing Thread → Network I/O Thread

### Authentication DTOs

#### LoginRequest

**Purpose:** Carries login credentials from UI to `SecurityService`

```cpp
struct LoginRequest {
    QString userId;             // User ID (required)
    QString pinHash;            // SHA-256(pin + salt) (required)
    QString salt;               // Per-user salt (required)
    QString deviceId;           // Device ID (for audit) (required)
    QDateTime timestamp;        // Request timestamp (required)
    
    bool isValid() const {
        return !userId.isEmpty() 
            && !pinHash.isEmpty()
            && pinHash.length() == 64;  // SHA-256 hash length
    }
};
```

**Security Note:** 
- PIN should be hashed on client before creating DTO
- Never log PIN in plain text
- DTO contains `pinHash`, not raw `pin`

#### SessionRefreshRequest

```cpp
struct SessionRefreshRequest {
    QString sessionId;          // Current session ID (required)
    QString userId;             // User ID (required)
    QDateTime lastActivityAt;   // Last activity timestamp (required)
    
    bool isValid() const {
        return !sessionId.isEmpty() && !userId.isEmpty();
    }
};
```

### Provisioning DTOs

#### ProvisioningPayload

**Purpose:** Carries device configuration from Central Station to device

```cpp
struct ProvisioningPayload {
    QString deviceId;           // Device ID (required)
    QString pairingCode;        // Pairing code for validation (required)
    
    // Configuration
    QString serverUrl;          // Central server URL (required)
    QByteArray clientCertificate;  // Device client certificate (required)
    QByteArray clientPrivateKey;   // Device private key (required)
    QByteArray caCertificate;      // CA certificate (required)
    
    // Metadata
    QDateTime issuedAt;         // Timestamp when config was generated (required)
    QDateTime expiresAt;        // Configuration expiry (required)
    
    // Security
    QByteArray signature;       // Signature from Central Station (required)
    
    bool isValid() const {
        return !deviceId.isEmpty()
            && !pairingCode.isEmpty()
            && !serverUrl.isEmpty()
            && !clientCertificate.isEmpty()
            && !signature.isEmpty();
    }
    
    bool isExpired() const {
        return QDateTime::currentDateTime() > expiresAt;
    }
};
```

**Usage:**
- **From:** `CentralStationClient` (Network Thread)
- **To:** `ProvisioningService` (Application Services)
- **Thread:** Network I/O Thread → Application Services Thread

### Settings DTOs

#### UpdateSettingCommand

```cpp
struct UpdateSettingCommand {
    QString key;                // Setting key (e.g., "measurementUnit") (required)
    QVariant value;             // New value (required)
    QString userId;             // User who made change (for audit) (optional)
    QDateTime timestamp;        // Change timestamp (required)
    
    bool isValid() const {
        return !key.isEmpty() && value.isValid();
    }
};
```

### Diagnostics DTOs

#### TelemetryMetrics

**Purpose:** Carries timing metrics from `NetworkManager` to `DatabaseManager`

```cpp
struct TelemetryMetrics {
    QString batchId;            // Batch UUID (required)
    QString deviceId;           // Device ID (required)
    QString patientMrn;         // Patient MRN (nullable)
    
    // Timing milestones (Unix milliseconds)
    qint64 dataCreatedAt;
    qint64 batchCreatedAt;
    qint64 signedAt;
    qint64 queuedForTxAt;
    qint64 transmittedAt;
    qint64 serverReceivedAt;
    qint64 serverProcessedAt;
    qint64 serverAckAt;
    
    // Statistics
    int recordCount;
    int batchSizeBytes;
    int compressedSizeBytes;
    
    // Status
    QString status;             // "success", "failed", "timeout", "retrying"
    QString errorMessage;
    int retryCount;
    
    bool isValid() const {
        return !batchId.isEmpty() && recordCount > 0;
    }
};
```

**Usage:**
- **From:** `NetworkManager` (Network Thread)
- **To:** `DatabaseManager` (DB Thread)
- **Thread:** Network I/O Thread → Database I/O Thread

## DTO Usage Patterns

### Pattern: UI → Application Service

**Scenario:** User admits a patient via UI

```cpp
// 1. UI Controller (PatientController.cpp)
void PatientController::admitPatient(const QString& mrn, const QString& source) {
    // Create DTO from UI inputs
    AdmitPatientCommand cmd {
        .mrn = mrn,
        .name = m_nameInput,
        .dateOfBirth = m_dobInput,
        .sex = m_sexInput,
        .bedLocation = m_bedLocationInput,
        .admissionSource = source,
        .admittedAt = QDateTime::currentDateTime(),
        .userId = m_authService->getCurrentUserId()
    };
    
    // Validate at boundary
    if (!cmd.isValid()) {
        emit error("Invalid admission data");
        return;
    }
    
    // Pass DTO to application service
    auto result = m_admissionService->admitPatient(cmd);
    
    if (result.isSuccess()) {
        emit patientAdmitted();
    } else {
        emit error(result.error().message());
    }
}

// 2. Application Service (AdmissionService.cpp)
Result<void, Error> AdmissionService::admitPatient(const AdmitPatientCommand& cmd) {
    // Additional business validation
    if (m_patientRepository->findByMrn(cmd.mrn).has_value()) {
        return Error("Patient already admitted");
    }
    
    // Create domain aggregate from DTO
    PatientIdentity identity(cmd.mrn, cmd.name, cmd.dateOfBirth, cmd.sex);
    BedLocation bedLocation(cmd.bedLocation);
    PatientAggregate patient(identity, bedLocation);
    
    // Execute domain logic
    patient.admit(cmd.admissionSource, cmd.admittedAt);
    
    // Persist via repository
    if (!m_patientRepository->save(patient)) {
        return Error("Failed to save patient");
    }
    
    // Emit domain event
    emit patientAdmitted(patient.getIdentity().getMrn());
    
    return Success();
}
```

### Pattern: Cross-Thread Communication

**Scenario:** RT thread sends telemetry to Network thread

```cpp
// 1. Monitoring Service (RT Thread)
void MonitoringService::sendBatchToNetwork() {
    // Create DTO from domain objects
    TelemetrySubmission dto {
        .batchId = QUuid::createUuid().toString(),
        .deviceId = m_settingsManager->getDeviceId(),
        .deviceLabel = m_settingsManager->getDeviceLabel(),
        .patientMrn = m_patientAggregate->getIdentity().getMrn(),
        .vitals = m_telemetryBatch->getVitals(),
        .alarms = m_telemetryBatch->getAlarms(),
        .dataCreatedAt = m_telemetryBatch->getOldestTimestamp(),
        .batchCreatedAt = QDateTime::currentMSecsSinceEpoch(),
        .recordCount = m_telemetryBatch->getRecordCount()
    };
    
    // Emit signal to network thread (Qt::QueuedConnection)
    emit telemetryReadyForTransmission(dto);
}

// 2. Network Manager (Network Thread)
void NetworkManager::onTelemetryReady(const TelemetrySubmission& dto) {
    // DTO received safely in network thread
    // Process transmission...
}
```

## DTO Validation

### Validation Strategy

**Validate at the boundary (UI/API), not in domain:**

```cpp
// UI Controller - First line of defense
void PatientController::admitPatient(const QString& mrn) {
    if (mrn.isEmpty()) {
        emit error("MRN cannot be empty");
        return;  // Reject early
    }
    
    AdmitPatientCommand cmd = createCommand(mrn);
    m_admissionService->admitPatient(cmd);
}

// Application Service - Business validation
Result<void, Error> AdmissionService::admitPatient(const AdmitPatientCommand& cmd) {
    // Business rules validation
    if (cmd.mrn.length() < 3 || cmd.mrn.length() > 20) {
        return Error("Invalid MRN format");
    }
    
    // Check business constraints
    if (isPatientAlreadyAdmitted(cmd.mrn)) {
        return Error("Patient already admitted");
    }
    
    // Proceed with domain logic...
}
```

### Validation Helpers

```cpp
class DTOValidator {
public:
    static Result<void, Error> validate(const AdmitPatientCommand& cmd) {
        if (cmd.mrn.isEmpty()) {
            return Error("MRN is required");
        }
        if (cmd.name.isEmpty()) {
            return Error("Name is required");
        }
        if (cmd.bedLocation.isEmpty()) {
            return Error("Bed location is required");
        }
        return Success();
    }
    
    static Result<void, Error> validate(const LoginRequest& req) {
        if (req.userId.isEmpty()) {
            return Error("User ID is required");
        }
        if (req.pinHash.length() != 64) {  // SHA-256 hash length
            return Error("Invalid PIN hash");
        }
        return Success();
    }
};
```

## DTO Naming Conventions

| Pattern           | Example                | When to Use                         |
| ----------------- | ---------------------- | ----------------------------------- |
| **`*Command`**    | `AdmitPatientCommand`  | User action/request from UI         |
| **`*Request`**    | `LoginRequest`         | API/service request                 |
| **`*Submission`** | `TelemetrySubmission`  | Data submission for processing      |
| **`*Payload`**    | `ProvisioningPayload`  | Configuration/data payload          |
| **`*Metrics`**    | `TelemetryMetrics`     | Diagnostic/performance data         |
| **`*Event`**      | `PatientAdmittedEvent` | Domain event notification (not DTO) |

## DTO Lifecycle

```
┌──────────────┐
│ UI / QML     │  1. User Input
└──────┬───────┘
       │ Creates DTO (Command)
       ▼
┌──────────────┐
│ Controller   │  2. Validation (basic)
└──────┬───────┘
       │ Passes DTO
       ▼
┌──────────────┐
│ Application  │  3. Validation (business)
│ Service      │  4. Maps DTO → Domain Objects
└──────┬───────┘
       │ Domain logic
       ▼
┌──────────────┐
│ Domain       │  5. Execute business logic
│ Aggregate    │
└──────┬───────┘
       │ Persist
       ▼
┌──────────────┐
│ Repository   │  6. Save to database
│ (via DTO)    │
└──────────────┘
```

## Best Practices

### DO:
✅ Keep DTOs simple (POD where possible)  
✅ Make DTOs immutable  
✅ Validate at boundaries (UI, API)  
✅ Use DTOs for cross-thread communication  
✅ Include `isValid()` method for basic validation  
✅ Use clear naming (`*Command`, `*Request`, etc.)

### DON'T:
❌ Put business logic in DTOs  
❌ Make DTOs mutable  
❌ Share DTOs across incompatible versions (use versioning)  
❌ Use DTOs inside domain layer (use Value Objects)  
❌ Inherit from DTOs (prefer composition)  
❌ Store DTOs in database (map to SQL or use aggregates)

## Verification Guidelines

### DTO Design Verification

1. **Immutability Check:** Verify DTOs have no setters, all fields const or immutable
2. **POD Verification:** Verify DTOs are simple structs without virtual methods
3. **Validation Check:** Verify `isValid()` methods present and correct
4. **Naming Check:** Verify DTOs use consistent naming conventions

### DTO Usage Verification

1. **Boundary Validation:** Verify validation happens at UI/API boundary
2. **Thread Safety:** Verify DTOs used for cross-thread communication are immutable
3. **No Business Logic:** Verify DTOs contain no business logic
4. **Serialization:** Verify DTOs can be serialized (JSON, Protocol Buffers) if needed

## Document Metadata

**Original Document ID:** DESIGN-031  
**Migration Date:** 2025-12-01  
**New Document ID:** DOC-GUIDE-015

## Revision History

| Version | Date       | Changes                                                                                                                               |
| ------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------- |
| 1.0     | 2025-12-01 | Initial migration from DESIGN-031 to DOC-GUIDE-015. Complete DTO design patterns, catalog, validation strategies, and usage patterns. |
