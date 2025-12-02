---
title: "Class Designs Overview"
doc_id: DOC-ARCH-019
version: 1.0
category: Architecture
phase: 6E
status: Draft
created: 2025-12-01
author: migration-bot
related:
  - DOC-ARCH-001_system_architecture.md
---

# Class Designs Overview

This document provides a high-level overview of the module-based class architecture for the Z Monitor. Classes are organized into **modules** (groups of services/components that execute on the same OS thread) for efficiency and maintainability.

**Key Concepts:**
- **Module:** Logical grouping of components sharing same thread and event loop
- **6 Primary Modules:** Interface, Real-Time, App Services, Database, Network, Background
- **87 Thread-Assigned Components:** Organized by execution context
- **DDD Integration:** Modules align with DDD layers (Domain, Application, Infrastructure, Interface)

## 1. Module Architecture Overview

### 1.1 What is a Module?

A **module** is a logical grouping of multiple services/components that:
- Execute on the **same OS thread**
- Share the **same Qt event loop**
- Can communicate **directly** (same thread, no marshaling)
- Have a **unified lifecycle** (start/stop as a unit)

**Rationale:**
- **Reduced Context Switching:** Fewer threads = less OS overhead
- **Shared State:** Services in same module can share memory without synchronization
- **Simplified Lifecycle:** Module starts/stops as a unit
- **Resource Efficiency:** Conserves thread resources on constrained hardware

### 1.2 Module Organization

The Z Monitor is organized into **6 primary modules**:

| Module                          | Thread              | Priority     | Component Count |
| ------------------------------- | ------------------- | ------------ | --------------- |
| **Interface Module**            | Main/UI Thread      | Default      | 30              |
| **Real-Time Processing Module** | RT Thread           | High/RT      | 12              |
| **Application Services Module** | App Services Thread | Normal       | 12              |
| **Database Module**             | Database I/O Thread | I/O Priority | 13              |
| **Network Module**              | Network I/O Thread  | Normal       | 11              |
| **Background Tasks Module**     | Background Thread   | Low          | 9               |

**Total:** 87 components organized into 6 modules (thread-assigned components only)

> **Note:** This count represents components assigned to specific threads. Additional components (interfaces, DTOs, domain events) are thread-agnostic. See **[DOC-ARCH-016: System Components](./DOC-ARCH-016_system_components.md)** for complete inventory (120+ total).

---

## 2. Module Responsibilities

### 2.1 Interface Module (Main/UI Thread)

**Purpose:** User interface and visualization

**Key Components:**
- QML Controllers (11): `DashboardController`, `AlarmController`, `WaveformController`, `PatientController`, etc.
- QML Views (7): `DashboardView`, `TrendsView`, `AlarmsView`, etc.
- QML Components (12): `WaveformChart`, `TrendChart`, `StatCard`, etc.

**Responsibilities:**
- Expose data to QML via Q_PROPERTY bindings
- Handle user interactions (button clicks, form submissions)
- Render visualizations (waveforms at 60 FPS, trends, vitals)
- Navigate between views

---

### 2.2 Real-Time Processing Module (RT Thread)

**Purpose:** Critical path for sensor data processing and alarm detection

**Key Components:**
- `SharedMemorySensorDataSource` - Sensor data input
- `MonitoringService` - Coordinates vitals ingestion
- `VitalsCache` - In-memory cache (3-day capacity)
- `WaveformCache` - Circular buffer (30 seconds)
- Domain Aggregates: `PatientAggregate`, `TelemetryBatch`, `AlarmAggregate`
- Value Objects: `VitalRecord`, `WaveformSample`, `AlarmSnapshot`

**Responsibilities:**
- Receive sensor data at high frequency (250 Hz waveforms, 5 Hz vitals)
- Cache vitals in-memory for fast alarm detection
- Evaluate alarm conditions (< 50ms latency target)
- Build telemetry batches for transmission

**Critical Path:** Sensor → Cache → Alarm Detection → UI (< 50ms)

---

### 2.3 Application Services Module (App Services Thread)

**Purpose:** Business logic orchestration for use cases

**Key Components:**
- `AdmissionService` - Patient admission/discharge/transfer
- `ProvisioningService` - Device provisioning and pairing
- `SecurityService` - Authentication and authorization
- `PermissionRegistry` - Enum-based role → permission mapping
- Domain Aggregates: `AdmissionAggregate`, `ProvisioningSession`, `UserSession`
- Value Objects: `PatientIdentity`, `BedLocation`, `PinCredential`

**Responsibilities:**
- Execute use cases (admit patient, provision device, authenticate user)
- Coordinate between domain aggregates and repositories
- Emit domain events for UI updates
- Enforce business rules and validation

---

### 2.4 Database Module (Database I/O Thread)

**Purpose:** Data persistence and archival

**Key Components:**
- `DatabaseManager` - SQLite connection and schema management
- Repository Implementations (7): `SQLitePatientRepository`, `SQLiteVitalsRepository`, `SQLiteAlarmRepository`, etc.
- `PersistenceScheduler` - Periodic persistence from cache
- `DataCleanupService` - Daily data cleanup
- `LogService` - Application logging
- `DataArchiveService` - Data archival

**Responsibilities:**
- Persist data to encrypted SQLite database (SQLCipher)
- Batch writes for performance (non-critical path)
- Periodic persistence from in-memory cache (every 10 minutes)
- Daily cleanup per retention policies

---

### 2.5 Network Module (Network I/O Thread)

**Purpose:** External communication and telemetry transmission

**Key Components:**
- `NetworkTelemetryServer` - Telemetry transmission via HTTPS/mTLS
- `HISPatientLookupAdapter` - Patient lookup from HIS/EHR
- `CentralStationClient` - Provisioning payload receiver
- `CertificateManager` - Certificate loading and validation
- `EncryptionService` - Payload encryption/decryption
- `SignatureService` - Data signing (HMAC-SHA256)

**Responsibilities:**
- Transmit telemetry batches to central server
- Fetch patient demographics from HIS/EHR
- Receive provisioning payloads from Central Station
- Manage TLS certificates and cryptographic operations

---

### 2.6 Background Tasks Module (Background Thread)

**Purpose:** Non-critical background operations

**Key Components:**
- `HealthMonitor` - System health monitoring (CPU, memory, disk)
- `ClockSyncService` - NTP time synchronization
- `FirmwareManager` - Firmware update management
- `BackupManager` - Database backup and recovery
- `DataArchiver` - Data archival to files
- `WatchdogService` - Crash detection and recovery
- `SettingsManager` - Configuration persistence

**Responsibilities:**
- Monitor system health and resources
- Synchronize system clock with NTP
- Apply firmware updates
- Create encrypted database backups
- Archive old data per retention policies

---

## 3. Data Flow Between Modules

```
┌─────────────────────────────────────────────────────────────┐
│                    Interface Module                         │
│  (Main/UI Thread - Controllers, QML Views/Components)      │
└──────────────────────┬──────────────────────────────────────┘
                       │ Qt::QueuedConnection (signals)
                       │
┌──────────────────────▼──────────────────────────────────────┐
│            Application Services Module                      │
│  (App Services Thread - Admission, Provisioning, Security)  │
└──────┬──────────────────────┬───────────────────────────────┘
       │                      │
       │ Queued Calls         │ Domain Events
       │                      │
┌──────▼──────────┐  ┌────────▼──────────────────────────────┐
│ Real-Time       │  │ Network Module                        │
│ Processing      │  │ (Network I/O Thread - Telemetry,       │
│ Module          │  │  Certificates, Encryption)            │
│ (RT Thread)     │  │                                        │
│                 │  │                                        │
│ Sensor Data     │  │ Telemetry Transmission                │
│ VitalsCache     │  │ Certificate Management                │
│ Alarm Detection │  │ Patient Lookup (HIS/EHR)              │
└──────┬──────────┘  └───────────────────────────────────────┘
       │                                                  
       │ MPSC Queue (Non-Critical)                       
       │                                                  
┌──────▼──────────────────────────────────────────────────────┐
│                    Database Module                          │
│  (Database I/O Thread - Persistence, Logging, Archival)    │
└──────────────────────┬─────────────────────────────────────┘
                       │
                       │ Scheduled Tasks
                       │
┌──────────────────────▼─────────────────────────────────────┐
│              Background Tasks Module                        │
│  (Background Thread - Health, Backup, Firmware, Settings)  │
└────────────────────────────────────────────────────────────┘
```

---

## 4. Communication Patterns

### 4.1 Between Modules (Cross-Thread)

- **Qt::QueuedConnection:** Signals/slots for asynchronous communication
- **MPSC Queues:** Multiple producers → single consumer (e.g., RT → Database)
- **Lock-Free Queues:** High-frequency data (waveform samples)
- **Domain Events:** Application-level events (`PatientAdmitted`, `AlarmRaised`)

### 4.2 Within Module (Same Thread)

- **Direct Method Calls:** No marshaling overhead
- **Qt::DirectConnection:** Immediate execution
- **Shared State:** Direct memory access (no synchronization needed)

---

## 5. Class Design Patterns

### 5.1 Dependency Injection

All services use **constructor injection** for dependencies:

```cpp
class MonitoringService {
public:
    explicit MonitoringService(IVitalsRepository* repo, ISettingsManager* settings)
        : repo_(repo), settings_(settings) {}
private:
    IVitalsRepository* repo_;
    ISettingsManager* settings_;
};
```

> **📋 DI Details:** See **[DOC-ARCH-013: Dependency Injection](./DOC-ARCH-013_dependency_injection.md)** for complete DI architecture.

### 5.2 Repository Pattern

All persistence operations use repository interfaces defined in domain layer:

```cpp
// Domain interface
class IPatientRepository {
public:
    virtual ~IPatientRepository() = default;
    virtual Result<Patient, Error> findByMrn(const QString& mrn) = 0;
    virtual Result<void, Error> save(const Patient& patient) = 0;
};

// Infrastructure implementation
class SQLitePatientRepository : public IPatientRepository {
public:
    explicit SQLitePatientRepository(DatabaseManager* db);
    Result<Patient, Error> findByMrn(const QString& mrn) override;
    Result<void, Error> save(const Patient& patient) override;
};
```

### 5.3 Value Objects (Immutable)

Domain value objects are immutable `struct` types:

```cpp
struct VitalRecord {
    QDateTime timestamp;
    float heartRate;
    float spo2;
    float respiratoryRate;
    MeasurementUnit unit;
    
    // Immutable: no setters, only constructor
    VitalRecord(QDateTime ts, float hr, float sp, float rr, MeasurementUnit u)
        : timestamp(ts), heartRate(hr), spo2(sp), respiratoryRate(rr), unit(u) {}
};
```

### 5.4 Aggregates (Encapsulation)

Domain aggregates encapsulate business logic and enforce invariants:

```cpp
class PatientAggregate {
public:
    Result<void, Error> admit(const PatientIdentity& identity) {
        if (isAdmitted_) {
            return Error{"Patient already admitted"};
        }
        identity_ = identity;
        isAdmitted_ = true;
        emit PatientAdmitted{identity};
        return Result<void, Error>::ok();
    }
    
private:
    PatientIdentity identity_;
    bool isAdmitted_ = false;
};
```

---

## 6. Related Documents

- **[DOC-ARCH-001: Architecture Overview](./DOC-ARCH-001_architecture_overview.md)** - High-level system architecture
- **[DOC-ARCH-013: Dependency Injection](./DOC-ARCH-013_dependency_injection.md)** - DI patterns and AppContainer
- **[DOC-ARCH-016: System Components](./DOC-ARCH-016_system_components.md)** - Complete component inventory
- **Legacy:** `z-monitor/architecture_and_design/09_CLASS_DESIGNS_OVERVIEW.md` - Detailed module architecture (being migrated)
- **Legacy:** `z-monitor/architecture_and_design/12_THREAD_MODEL.md` - Complete thread and module architecture

---
**Status:** ✅ Migrated from legacy 09_CLASS_DESIGNS_OVERVIEW.md
