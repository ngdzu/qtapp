# Software Architecture

This document details the software architecture of the Medical Device Dashboard, which is based on a clean separation between the C++ backend and the QML frontend.

## 1. High-Level Architecture Diagram

[View the full Architecture Diagram (interactive)](./02_ARCHITECTURE.mmd)

This diagram provides a comprehensive overview of all major software components and their relationships. Use a Mermaid-compatible viewer to zoom and pan.

## 2. Component Breakdown

### 2.1. C++ Backend

The backend is responsible for all business logic, data processing, and communication. It is organized into two main layers:

-   **Core Services:** These classes manage the application's state and perform key tasks. They have no knowledge of the UI.
    -   `DeviceSimulator`: Generates realistic, simulated patient vital signs and device data.
    -   `AlarmManager`: Monitors data from the simulator, determines if alarm conditions are met, and manages the state of each alarm (e.g., Active, Silenced).
    -   `NetworkManager`: Manages the secure connection (mTLS) to the central server and handles the transmission of telemetry data.
    -   `DatabaseManager`: Manages the encrypted SQLite database for storing historical trend data and logs.
    -   `PatientManager`: Manages patient context, including loading/saving patient profiles and current patient data.
    -   `SettingsManager`: Handles device configuration settings and user preferences.
    -   `AuthenticationService`: Manages user login, session, and role-based access control.
    -   `LogService`: Provides a centralized logging mechanism for the application, emitting messages that can be displayed in the Diagnostics View.
    -   `DataArchiver`: Handles the archival of old data from the local database.

-   **UI Controllers:** These `QObject`-based classes act as a bridge between the Core Services and the QML frontend. They expose data and functionality to QML via properties, signals, and slots.
    -   `DashboardController`: Exposes real-time vital signs and device status for the Dashboard View.
    -   `AlarmController`: Exposes the list of active alarms, their priorities, and alarm history to the QML UI.
    -   `TrendsController`: Provides historical data queried from the `DatabaseManager` for plotting in the Trends View.
    -   `SystemController`: Exposes system-wide state like connection status, navigation state, and global alerts.
    -   `PatientController`: Exposes current patient data to the Patient Banner and other patient-related UI elements.
    -   `SettingsController`: Exposes configurable settings to the Settings View and handles updates.
    -   `NotificationController`: Exposes informational and warning messages to the QML notification system.

### 2.2. QML Frontend

The frontend is responsible for all rendering and user interaction. It is purely declarative and reacts to data changes exposed by the C++ controllers.

-   **`Main.qml`**: The application's entry point. It assembles the main UI structure, including the sidebar, header, and the view loader. It also contains the global alarm overlay.
-   **Views**: Self-contained QML files representing a full screen of content (e.g., `DashboardView.qml`, `SettingsView.qml`, `TrendsView.qml`, `DiagnosticsView.qml`, `LoginView.qml`). A `Loader` in `Main.qml` dynamically loads the active view based on navigation state.
-   **Components**: Reusable, smaller QML files that act as building blocks for the UI (e.g., `StatCard.qml`, `PatientBanner.qml`, `NotificationBell.qml`, `Sidebar.qml`, `TopBar.qml`, `AlarmIndicator.qml`). They are designed to be modular and stateless where possible.

### 2.3. Simulated Central Server

A standalone Python application that simulates a hospital's central monitoring station.
-   **API:** Exposes a single REST API endpoint (`/api/telemetry`) over HTTPS.
-   **Security:** It is configured to require mTLS, validating the client certificate of any connecting device to ensure only authorized devices can send data.

## 3. Data Flow

1.  **Data Generation:** The `DeviceSimulator` generates new data points on a timer.
2.  **Processing:** The data is passed to the `AlarmManager` to check for alarm conditions and to the relevant UI controllers.
3.  **UI Update:** The UI controllers emit signals (`dataChanged`) or update their properties. The QML engine detects these changes and automatically updates the bindings in the UI, causing elements like graphs and text fields to re-render with the new data.
4.  **Data Persistence:** The `DatabaseManager` periodically writes the data to the local encrypted SQLite database.
5.  **Remote Transmission:** The `NetworkManager` periodically sends the latest data to the central server over a secure HTTPS connection.