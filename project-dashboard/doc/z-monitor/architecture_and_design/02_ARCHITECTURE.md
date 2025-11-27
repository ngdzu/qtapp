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

> **📋 Complete Component Reference:** For an authoritative list of ALL system components (98 total) including domain aggregates, application services, infrastructure adapters, controllers, and QML components, see **[System Components Reference (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md)**.

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

-   `DeviceSimulator`: Source of vitals/waveforms (implements domain-facing simulator adapter).
-   `NetworkManager` + `ITelemetryServer` implementations: HTTPS/mTLS transport for telemetry.
-   `DatabaseManager`: SQLite/SQLCipher persistence backing repositories.
-   `IPatientLookupService` adapters: Remote HIS/EHR integration.
-   `SettingsManager`, `AuthenticationService`, `LogService`, `DataArchiver`, `BackupManager`, `FirmwareManager`, `HealthMonitor`, `ClockSyncService`, `DeviceRegistrationService`, `ProvisioningService` (infrastructure responsibilities that fulfill domain/application contracts).

### 2.5. Interface Layer (Controllers)

`QObject` controllers remain the bridge to QML. They depend on application services rather than infrastructure:

-   `DashboardController`, `AlarmController`, `TrendsController`, `SystemController`, `PatientController`, `SettingsController`, `ProvisioningController`, `NotificationController`.
-   Controllers expose properties/signals to QML and issue commands to application services (`MonitoringService`, `AdmissionService`, etc.).

### 2.2. QML Frontend

The frontend is responsible for all rendering and user interaction. It is purely declarative and reacts to data changes exposed by the C++ controllers.

-   **`Main.qml`**: The application's entry point. It assembles the main UI structure, including the sidebar, header, and the view loader. It also contains the global alarm overlay.
-   **Views**: Self-contained QML files representing a full screen of content (e.g., `DashboardView.qml`, `SettingsView.qml`, `TrendsView.qml`, `DiagnosticsView.qml`, `LoginView.qml`). A `Loader` in `Main.qml` dynamically loads the active view based on navigation state.
-   **Components**: Reusable, smaller QML files that act as building blocks for the UI (e.g., `StatCard.qml`, `PatientBanner.qml`, `NotificationBell.qml`, `Sidebar.qml`, `TopBar.qml`, `AlarmIndicator.qml`). They are designed to be modular and stateless where possible.

### 2.3. Simulated Central Server

A standalone Python application that simulates a hospital's central monitoring station.
-   **API:** Exposes REST API endpoints:
    -   `POST /api/telemetry`: Receives telemetry data from devices (HTTPS, mTLS)
    -   `GET /api/patients/{patientId}`: Patient lookup endpoint for retrieving patient information (optional, for `IPatientLookupService` integration)
-   **Security:** It is configured to require mTLS, validating the client certificate of any connecting device to ensure only authorized devices can send data.
-   **Configuration:** The server URL is configurable through `SettingsManager` (stored in `settings` table). Default is `https://localhost:8443` for local development.
-   **Mock Server:** For testing and development, a `MockTelemetryServer` implementation is available that swallows all data without sending to a real server, enabling testing without server infrastructure.

## 3. Data Flow

1.  **Data Generation:** The `DeviceSimulator` generates new data points on a timer.
2.  **Processing:** The data is passed to the `AlarmManager` to check for alarm conditions and to the relevant UI controllers.
3.  **UI Update:** The UI controllers emit signals (`dataChanged`) or update their properties. The QML engine detects these changes and automatically updates the bindings in the UI, causing elements like graphs and text fields to re-render with the new data.
4.  **Data Persistence:** The `DatabaseManager` periodically writes the data to the local encrypted SQLite database.
5.  **Remote Transmission:** The `NetworkManager` periodically sends the latest data to the central server over a secure HTTPS connection. **CRITICAL:** All telemetry data automatically includes the current patient MRN from `PatientManager` to ensure proper patient data association. If no patient is admitted, patient telemetry data is not sent (device in STANDBY state).