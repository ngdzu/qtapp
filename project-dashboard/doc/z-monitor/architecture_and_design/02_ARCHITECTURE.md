# Software Architecture

**Document ID:** DESIGN-002  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document details the software architecture of the Z Monitor, which is based on a clean separation between the C++ backend and the QML frontend.

## 1. High-Level Architecture Diagram

[View the full Architecture Diagram (interactive)](./02_ARCHITECTURE.mmd)

This diagram provides a comprehensive overview of all major software components and their relationships. Use a Mermaid-compatible viewer to zoom and pan.

> **📋 Complete Component Reference:** For an authoritative list of ALL system components (120 total) including domain aggregates, application services, infrastructure adapters, controllers, and QML components, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)**.

## 2. Component Breakdown (DDD Perspective)

### 2.1. Layered Overview

```
Interface Layer (QML + QObject controllers)
        ↓
Application Layer (use-case orchestration)
        ↓
Domain Layer (aggregates, value objects, domain events)
        ↓
Infrastructure Layer (Qt/SQL/network adapters)
```

### 2.2. Domain Layer (Entities & Value Objects)

-   `PatientAggregate`: Admission state, vitals history, bed assignment; raises events like `PatientAdmitted`, `PatientDischarged`.
-   `DeviceAggregate`: Provisioning state, credential lifecycle, firmware metadata.
-   `TelemetryBatch`: Holds `VitalRecord` and `AlarmSnapshot` collections, enforces signing/timestamping rules.
-   Value Objects: `PatientIdentity`, `DeviceSnapshot`, `MeasurementUnit`, `AlarmThreshold`, `BedLocation`.
-   Domain Events: `TelemetryQueued`, `AlarmRaised`, `ProvisioningCompleted`.

### 2.3. Application Layer (Services & Repositories)

-   `MonitoringService`: Coordinates data acquisition, persists telemetry via repositories, queues batches for transmission.
-   `AdmissionService`: Executes admit/discharge/transfer use cases, logs admission events.
-   `ProvisioningService`: Handles QR pairing, applies configuration payloads to `DeviceAggregate`.
-   `SecurityService`: Authenticates users, manages PIN policies, issues `UserSession`.
-   Repository Interfaces: `IPatientRepository`, `ITelemetryRepository`, `IAlarmRepository`, `IProvisioningRepository`.

### 2.4. Infrastructure Layer (Adapters)

**Sensor Data Sources:**
-   `ISensorDataSource` interface: Abstraction for sensor data acquisition
-   `WebSocketSensorDataSource`: WebSocket connection to sensor simulator (`ws://localhost:9002`)
-   `DeviceSimulator`: Legacy fallback (internal simulator, deprecated in favor of external simulator)

**Network & Communication:**
-   `NetworkManager` + `ITelemetryServer` implementations: HTTPS/mTLS transport for telemetry
-   `IUserManagementService` adapters: Hospital user authentication (LDAP/REST API)
  - `HospitalUserManagementAdapter`: Production implementation (connects to hospital server)
  - `MockUserManagementService`: Development/testing (hardcoded test users)
-   `IPatientLookupService` adapters: Remote HIS/EHR integration
-   `IProvisioningService` adapters: Device provisioning and configuration

**Data Persistence:**
-   `DatabaseManager`: SQLite/SQLCipher persistence backing repositories
-   `SQLiteVitalsRepository`: Vitals data persistence (periodic, non-critical)
-   `SQLiteActionLogRepository`: Action log persistence (user actions)
-   `SQLiteAuditRepository`: Security audit log persistence

**Caching & Data Management:**
-   `VitalsCache`: In-memory cache for 3-day vitals data (critical path, < 50ms latency)
-   `WaveformCache`: In-memory circular buffer for 30-second waveform display
-   `PersistenceScheduler`: Periodic persistence from cache to database (non-critical, scheduled)
-   `DataCleanupService`: Daily cleanup of expired data per retention policies

**System Services:**
-   `SettingsManager`: Device configuration and settings
-   `LogService`: Application logging (file-based, human-readable or JSON)
-   `DataArchiver`: Data archival and retention management
-   `BackupManager`: Database backup and recovery
-   `FirmwareManager`: Firmware update management
-   `HealthMonitor`: System health monitoring and watchdog
-   `ClockSyncService`: NTP time synchronization with central server
-   `DeviceRegistrationService`: Device registration with central server

### 2.5. Interface Layer (Controllers)

`QObject` controllers remain the bridge to QML. They depend on application services rather than infrastructure:

-   `DashboardController`: Real-time vital signs display, patient banner
-   `AlarmController`: Alarm management, acknowledgment, escalation
-   `TrendsController`: Historical vital signs trends and visualization
-   `SystemController`: System diagnostics, health monitoring
-   `PatientController`: Patient admission/discharge/transfer workflow
-   `SettingsController`: Device settings and configuration
-   `ProvisioningController`: Device provisioning and pairing
-   `NotificationController`: Notification center and alerts
-   `AuthenticationController`: User login/logout, session management

Controllers expose properties/signals to QML and issue commands to application services (`MonitoringService`, `AdmissionService`, `SecurityService`, etc.).

### 2.6. QML Frontend and Visualization

The frontend is responsible for all rendering and user interaction. It is purely declarative and reacts to data changes exposed by the C++ controllers.

**Visualization Strategy:**
- **QML-Based Rendering:** All visualization is handled declaratively in QML using Qt Quick components and Canvas API
- **No Separate Visualization Service:** Rendering logic resides in QML components, not in C++ services (following Qt/QML best practices)
- **Data Flow:** C++ Controllers (QObject) → Q_PROPERTY bindings → QML Components → Canvas/Graphics rendering
- **Performance:** Critical visualizations (waveforms at 60 FPS) use QML Canvas API for efficient 2D rendering
- **See:** [41_WAVEFORM_DISPLAY_IMPLEMENTATION.md](./41_WAVEFORM_DISPLAY_IMPLEMENTATION.md) for detailed waveform rendering guide

**Visualization Components:**
- **Waveforms:** `WaveformChart.qml` (Canvas-based, 60 FPS) - ECG and plethysmogram rendering
- **Trends:** `TrendChart.qml` (Line chart) - Historical vital signs visualization
- **Vitals:** `StatCard.qml` - Real-time numeric vital signs display
- **Alarms:** `AlarmIndicator.qml` - Visual alarm indicators with priority colors

**UI Structure:**
-   **`Main.qml`**: The application's entry point. It assembles the main UI structure, including the sidebar, header, and the view loader. It also contains the global alarm overlay.
-   **Views**: Self-contained QML files representing a full screen of content (e.g., `DashboardView.qml`, `SettingsView.qml`, `TrendsView.qml`, `DiagnosticsView.qml`, `LoginView.qml`). A `Loader` in `Main.qml` dynamically loads the active view based on navigation state.
-   **Components**: Reusable, smaller QML files that act as building blocks for the UI (e.g., `StatCard.qml`, `PatientBanner.qml`, `NotificationBell.qml`, `Sidebar.qml`, `TopBar.qml`, `AlarmIndicator.qml`, `WaveformChart.qml`, `TrendChart.qml`). They are designed to be modular and stateless where possible.

### 2.7. External Systems

**Central Telemetry Server:**
A standalone Python application that simulates a hospital's central monitoring station.
-   **API:** Exposes REST API endpoints:
    -   `POST /api/v1/telemetry/vitals`: Receives telemetry data from devices (HTTPS, mTLS)
    -   `POST /api/v1/alarms`: Receives alarm notifications
    -   `POST /api/v1/devices/register`: Device registration
    -   `GET /api/v1/patients/{mrn}`: Patient lookup endpoint (optional, for `IPatientLookupService` integration)
-   **Security:** Requires mTLS, validating client certificates to ensure only authorized devices can send data
-   **Configuration:** Server URL configurable through `SettingsManager` (stored in `settings` table). Default is `https://localhost:8443` for local development
-   **Mock Server:** `MockTelemetryServer` implementation available for testing (swallows data without network)

**Sensor Simulator:**
External Qt application (`project-dashboard/sensor-simulator/`) that generates realistic vital signs data.
-   **Protocol:** WebSocket server on `ws://localhost:9002`
-   **Data Rate:** Vitals at 5 Hz (200ms), waveforms at 250 Hz
-   **Format:** JSON messages with vitals and waveform data
-   **Purpose:** Development and testing without actual medical sensors

**Hospital User Management Server:**
Hospital's centralized user authentication and authorization system.
-   **Protocol:** REST API or LDAP/Active Directory
-   **Purpose:** Validates healthcare worker credentials (nurses, physicians, technicians, administrators)
-   **Mock:** `MockUserManagementService` available for development/testing (hardcoded test users)
-   **Integration:** Via `IUserManagementService` interface

**NTP Server:**
Network Time Protocol server for clock synchronization.
-   **Purpose:** Ensures accurate timestamps across all devices
-   **Frequency:** Syncs every 1 hour
-   **Fallback:** Uses central server timestamp if NTP unavailable

## 3. Data Flow

### 3.1. Critical Path: Sensor → Alarm Detection (< 50ms)

1.  **Sensor Data Acquisition:** 
    - `WebSocketSensorDataSource` receives vital signs from sensor simulator via WebSocket (`ws://localhost:9002`)
    - Parses JSON messages and emits `vitalSignsReceived()` signal
    - **Alternative:** `DeviceSimulator` (legacy fallback) generates data internally

2.  **In-Memory Caching (Critical Path):**
    - `MonitoringService` receives vitals via signals
    - `VitalsCache` stores data in-memory (3-day capacity, ~400 MB)
    - **Latency Target:** < 50ms from sensor to cache (REQ-NFR-PERF-100)

3.  **Alarm Detection:**
    - `AlarmManager` checks thresholds against cached vitals
    - **Critical:** No database access in alarm detection path (pre-allocated buffers only)
    - Alarms triggered within 50ms of threshold violation
    - Alarm events logged to `SQLiteAuditRepository`

4.  **UI Update:**
    - Controllers (`DashboardController`, `AlarmController`) receive signals
    - QML engine detects property changes and updates UI bindings
    - Real-time display updates at 1 Hz (vitals) and 60 FPS (waveforms)

### 3.2. Non-Critical Path: Persistence & Transmission

5.  **Periodic Persistence (Non-Critical):**
    - `PersistenceScheduler` periodically (every 5-10 minutes) persists cached vitals to database
    - `SQLiteVitalsRepository` writes to encrypted SQLite database (SQLCipher)
    - **Non-blocking:** Database writes do not block critical path
    - Batch writes (10 records per transaction) for efficiency

6.  **Remote Transmission:**
    - `NetworkManager` syncs data to central server via `ITelemetryServer` interface
    - Sync frequency: Every 10 seconds (configurable)
    - **CRITICAL:** All telemetry data includes current patient MRN from `PatientManager`
    - If no patient admitted: Device in STANDBY state, no telemetry sent
    - Uses mTLS (mutual TLS) for secure authentication and encryption

7.  **Data Cleanup:**
    - `DataCleanupService` runs daily at 3 AM
    - Removes data older than retention policies (7 days vitals, 90 days alarms)
    - Monitors database size (500 MB limit) and triggers emergency cleanup if needed

### 3.3. Patient Management Flow

8.  **Patient Admission:**
    - User (nurse/physician) initiates admission via `PatientController`
    - `AdmissionService` orchestrates workflow:
      - Validates user permissions (via `SecurityService` + `IUserManagementService`)
      - Queries HIS/EHR via `IPatientLookupService` (or uses local cache)
      - Creates `PatientAggregate` and logs admission event
    - Patient MRN associated with all subsequent vitals and alarms

9.  **Action Logging:**
    - All user actions logged via `SQLiteActionLogRepository`
    - Includes: login, logout, patient admission/discharge, settings changes
    - Hash chain for tamper detection

### 3.4. Authentication Flow

10. **User Authentication:**
    - User enters User ID and secret code (PIN) via `LoginView`
    - `AuthenticationController` delegates to `SecurityService`
    - `SecurityService` uses `IUserManagementService` to validate credentials:
      - Production: `HospitalUserManagementAdapter` (REST API or LDAP)
      - Development: `MockUserManagementService` (hardcoded test users)
    - Session created with role and permissions
    - All authentication events logged to `SQLiteAuditRepository`

## 4. Key Architectural Principles

### 4.1. Critical Path Isolation
- **Alarm detection path** (sensor → cache → alarm) runs on high-priority real-time thread
- **No database access** in critical path (pre-allocated buffers only)
- **No memory allocations** in critical path (pre-allocated data structures)

### 4.2. Dependency Inversion
- All external dependencies abstracted via interfaces:
  - `ISensorDataSource` (sensor data)
  - `ITelemetryServer` (server communication)
  - `IPatientLookupService` (patient lookup)
  - `IUserManagementService` (user authentication)
  - `IProvisioningService` (device provisioning)
- Enables testing with mocks and swapping implementations

### 4.3. Data Caching Strategy
- **In-memory cache:** 3-day vitals data (~400 MB) for critical path
- **Database storage:** 7-day vitals data for historical trends
- **Periodic persistence:** Decouples critical path from database operations
- **Automatic cleanup:** Manages storage within 500 MB limit

### 4.4. Security Architecture
- **Encryption at rest:** SQLCipher (AES-256) for local database
- **Encryption in transit:** mTLS (TLS 1.2+) for all network communication
- **Authentication:** Hospital user management server integration
- **Audit logging:** Complete audit trail for compliance (HIPAA, IEC 62304)

## 5. Related Documents

- **System Components:** [29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md) - Complete component inventory (120 components)
- **Thread Model:** [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Threading architecture and latency targets
- **Data Caching:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) - In-memory caching strategy
- **Sensor Integration:** [37_SENSOR_INTEGRATION.md](./37_SENSOR_INTEGRATION.md) - Sensor data source interface
- **Authentication:** [38_AUTHENTICATION_WORKFLOW.md](./38_AUTHENTICATION_WORKFLOW.md) - User authentication workflow
- **Database Design:** [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) - Database schema and retention policies
- **Security:** [06_SECURITY.md](./06_SECURITY.md) - Security architecture and encryption