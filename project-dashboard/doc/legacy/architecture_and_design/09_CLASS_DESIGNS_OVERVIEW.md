# Class Designs Overview: Module Architecture

**Document ID:** DESIGN-009-OVERVIEW  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides a high-level overview of the module-based class architecture for the Z Monitor. Classes are organized into **modules** (groups of services/components that execute on the same OS thread) for efficiency and maintainability.

> **📋 Related Documents:**
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Complete thread and module architecture
> - [System Components (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md) - Complete component inventory
> - [Domain-Driven Design (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md) - DDD layering strategy and component inventory

---

## 1. Module Architecture Overview

### 1.1 What is a Module?

A **module** is a logical grouping of multiple services/components that:
- Execute on the **same OS thread**
- Share the **same Qt event loop**
- Can communicate **directly** (same thread, no marshaling)
- Have a **unified lifecycle** (start/stop as a unit)

**Rationale:**
- **Reduced Context Switching:** Fewer threads = less OS overhead
- **Shared State:** Services in the same module can share memory without synchronization
- **Simplified Lifecycle:** Module starts/stops as a unit
- **Resource Efficiency:** Conserves thread resources on constrained hardware

### 1.2 Module Organization

The Z Monitor is organized into **6 primary modules**:

| Module | Thread | Priority | Component Count | Documentation |
|--------|--------|----------|----------------|---------------|
| **Interface Module** | Main/UI Thread | Default | 30 | [09a_INTERFACE_MODULE.md](./09a_INTERFACE_MODULE.md) |
| **Real-Time Processing Module** | RT Thread | High/RT | 12 | [09b_REALTIME_MODULE.md](./09b_REALTIME_MODULE.md) |
| **Application Services Module** | App Services Thread | Normal | 12 | [09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md) |
| **Database Module** | Database I/O Thread | I/O Priority | 13 | [09d_DATABASE_MODULE.md](./09d_DATABASE_MODULE.md) |
| **Network Module** | Network I/O Thread | Normal | 11 | [09e_NETWORK_MODULE.md](./09e_NETWORK_MODULE.md) |
| **Background Tasks Module** | Background Thread | Low | 9 | [09f_BACKGROUND_MODULE.md](./09f_BACKGROUND_MODULE.md) |

**Total:** 87 components organized into 6 modules (thread-assigned components only)

> **Note:** This count (86) represents components that are assigned to specific threads. Additional components (interfaces, DTOs, domain events) are thread-agnostic and not included in this count. See [29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md) for the complete component inventory (120 total).

---

## 2. High-Level Module Interaction

[View Module Interaction Diagram (Mermaid)](./09_CLASS_DESIGNS_OVERVIEW.mmd)  
[View Module Interaction Diagram (SVG)](./09_CLASS_DESIGNS_OVERVIEW.svg)

### 2.1 Data Flow Between Modules

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
│                 │  │                                        │    │
│ Sensor Data ────┼──┼────────────────────────────────┐ │    │
│ VitalsCache     │  │ Telemetry Transmission          │ │    │
│ WaveformCache   │  │ Certificate Management           │ │    │
│ Alarm Detection │  │ Patient Lookup (HIS/EHR)         │ │    │
└──────┬──────────┘  └─────────────────────────────────┘ │    │
       │                                                  │    │
       │ MPSC Queue (Non-Critical)                       │    │
       │                                                  │    │
┌──────▼──────────────────────────────────────────────────▼────┐
│                    Database Module                            │
│  (Database I/O Thread - Persistence, Logging, Archival)      │
└──────────────────────────────────────────────────────────────┘
                       │
                       │ Scheduled Tasks
                       │
┌──────────────────────▼──────────────────────────────────────┐
│              Background Tasks Module                         │
│  (Background Thread - Health, Backup, Firmware, Settings)    │
└──────────────────────────────────────────────────────────────┘
```

### 2.2 Communication Patterns

**Between Modules (Cross-Thread):**
- **Qt::QueuedConnection:** Signals/slots for asynchronous communication
- **MPSC Queues:** Multiple producers → single consumer (e.g., RT → Database)
- **Lock-Free Queues:** High-frequency data (waveform samples)
- **Domain Events:** Application-level events (PatientAdmitted, AlarmRaised)

**Within Module (Same Thread):**
- **Direct Method Calls:** No marshaling overhead
- **Direct Qt::DirectConnection:** Immediate execution
- **Shared State:** Direct memory access (no synchronization needed)

---

## 3. Module Responsibilities

### 3.1 Interface Module (Main/UI Thread)

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

**See:** [09a_INTERFACE_MODULE.md](./09a_INTERFACE_MODULE.md)

---

### 3.2 Real-Time Processing Module (RT Thread)

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

**See:** [09b_REALTIME_MODULE.md](./09b_REALTIME_MODULE.md)

---

### 3.3 Application Services Module (App Services Thread)

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

**See:** [09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md)

---

### 3.4 Database Module (Database I/O Thread)

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
- Schema migrations and management

**Non-Critical Path:** Database operations don't block alarm detection

**See:** [09d_DATABASE_MODULE.md](./09d_DATABASE_MODULE.md)

---

### 3.5 Network Module (Network I/O Thread)

**Purpose:** Secure network communication

**Key Components:**
- `NetworkTelemetryServer` - Telemetry transmission (HTTPS/mTLS)
- `CertificateManager` - Certificate lifecycle management
- `EncryptionService` - Payload encryption/decryption
- `SignatureService` - Data signing/verification
- `HISPatientLookupAdapter` - Patient lookup from HIS/EHR
- `CentralStationClient` - Provisioning payload receiver
- Domain Aggregates: `DeviceAggregate`

**Responsibilities:**
- Transmit telemetry data to central server (mTLS)
- Manage certificates (loading, validation, renewal)
- Encrypt and sign payloads
- Lookup patients from external systems (HIS/EHR)
- Handle provisioning configuration from Central Station

**See:** [09e_NETWORK_MODULE.md](./09e_NETWORK_MODULE.md)

---

### 3.6 Background Tasks Module (Background Thread)

**Purpose:** Low-priority system maintenance tasks

**Key Components:**
- `FirmwareUpdateService` - Firmware update management
- `BackupService` - Database backup and restore
- `SettingsManager` - Configuration storage
- `QRCodeGenerator` - QR code generation
- `SecureStorage` - Secure key storage
- `HealthMonitor` - System health monitoring
- `ClockSyncService` - NTP time synchronization
- `WatchdogService` - Crash detection and recovery

**Responsibilities:**
- System health monitoring (CPU, memory, disk)
- Scheduled backups
- Firmware updates
- Time synchronization
- Watchdog monitoring

**Low Priority:** Yields to RT/UI threads

**See:** [09f_BACKGROUND_MODULE.md](./09f_BACKGROUND_MODULE.md)

---

## 4. Module Interaction Patterns

### 4.1 Critical Path (Real-Time)

```
Sensor Simulator
    ↓ Shared Memory Frames
SharedMemorySensorDataSource (RT Module)
    ↓ Qt Signal
MonitoringService (RT Module)
    ↓ Direct Call (same thread)
VitalsCache (RT Module)
    ↓ Direct Call (same thread)
AlarmAggregate (RT Module)
    ↓ Qt::QueuedConnection
AlarmController (Interface Module)
    ↓ QML Binding
AlarmIndicator.qml (Interface Module)
```

**Latency Target:** < 50ms end-to-end

### 4.2 Non-Critical Path (Background Persistence)

```
VitalsCache (RT Module)
    ↓ MPSC Queue (non-blocking)
PersistenceScheduler (Database Module)
    ↓ Direct Call (same thread)
SQLiteVitalsRepository (Database Module)
    ↓ Direct Call (same thread)
DatabaseManager (Database Module)
```

**Latency Target:** < 5 seconds (background, periodic)

### 4.3 User Action Flow

```
User clicks "Admit Patient" (Interface Module)
    ↓ Q_INVOKABLE
PatientController (Interface Module)
    ↓ Qt::QueuedConnection
AdmissionService (Application Services Module)
    ↓ Direct Call (same thread)
AdmissionAggregate (Application Services Module)
    ↓ Qt::QueuedConnection
IPatientRepository (Database Module)
    ↓ MPSC Queue
SQLitePatientRepository (Database Module)
    ↓ Domain Event
PatientController (Interface Module)
    ↓ QML Binding
PatientBanner.qml (Interface Module)
```

---

## 5. Module Lifecycle

### 5.1 Startup Sequence

1. **Main Thread:** Application starts, QML engine initializes
2. **Interface Module:** Controllers created, QML views loaded
3. **Database Module:** Database connection opened, migrations run
4. **Network Module:** Certificates loaded, TLS context initialized
5. **Real-Time Module:** Sensor data source started, caches initialized
6. **Application Services Module:** Services initialized
7. **Background Module:** Health monitoring started, timers scheduled

### 5.2 Shutdown Sequence

1. **Background Module:** Timers stopped, pending tasks completed
2. **Real-Time Module:** Sensor data source stopped, caches flushed
3. **Network Module:** Connections closed, certificates saved
4. **Application Services Module:** Sessions terminated, state saved
5. **Database Module:** Final writes committed, connection closed
6. **Interface Module:** UI cleanup, controllers destroyed

---

## 6. Module Design Principles

### 6.1 Single Responsibility

Each module has a **single, well-defined responsibility**:
- Interface Module: UI and visualization
- Real-Time Module: Critical path processing
- Application Services Module: Business logic orchestration
- Database Module: Data persistence
- Network Module: Network communication
- Background Module: System maintenance

### 6.2 Minimize Cross-Module Dependencies

- Use **interfaces** (repository interfaces, service interfaces) to decouple modules
- Communicate via **queued signals** (asynchronous) or **queues** (non-blocking)
- Avoid **direct dependencies** between modules (use dependency injection)

### 6.3 Shared State Within Module

- Services in the same module can **share state directly** (same thread)
- No synchronization needed for same-thread access
- Use **Qt::DirectConnection** for same-thread communication

### 6.4 Error Handling

- Module-level error handling (one service failure doesn't crash others)
- Use **try-catch** in event loop slots
- Emit **error signals** for cross-module error propagation

---

## 7. Detailed Module Documentation

For detailed class designs within each module, see:

- **[09a_INTERFACE_MODULE.md](./09a_INTERFACE_MODULE.md)** - Interface Module (Controllers, QML Components)
- **[09b_REALTIME_MODULE.md](./09b_REALTIME_MODULE.md)** - Real-Time Processing Module (Sensor Data, Caching, Alarms)
- **[09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md)** - Application Services Module (Admission, Provisioning, Security)
- **[09d_DATABASE_MODULE.md](./09d_DATABASE_MODULE.md)** - Database Module (Persistence, Repositories, Logging)
- **[09e_NETWORK_MODULE.md](./09e_NETWORK_MODULE.md)** - Network Module (Telemetry, Certificates, Encryption)
- **[09f_BACKGROUND_MODULE.md](./09f_BACKGROUND_MODULE.md)** - Background Tasks Module (Health, Backup, Firmware)

---

## 8. Related Documents

- **[12_THREAD_MODEL.md](./12_THREAD_MODEL.md)** - Complete thread and module architecture
- **[29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md)** - Complete component inventory
- **[02_ARCHITECTURE.md](./02_ARCHITECTURE.md)** - High-level architecture
- **[29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md)** - DDD layering strategy and component inventory

---

*This document provides the high-level module architecture. For detailed class designs, see the module-specific documents listed above.*

