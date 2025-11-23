# Project: Medical Device Dashboard

## Overview
Create a modern, real-time dashboard for a medical device using **Qt 6** and **QML**. The design should replicate the visual style and layout of the reference "VibeLink Remote Builder" project, but adapted for a medical context.

The target hardware is an embedded device with an **8-inch touch screen** and a resolution of **1280x800**.

## Requirements

### 1. Visual Design & Layout (QML)
Replicate the "Cyberpunk/Modern" aesthetic of the reference project:
- **Theme:** Dark background (`#09090b` / Zinc-950), dark borders (`#27272a` / Zinc-800).
- **Typography:** Clean, sans-serif font (e.g., Roboto or Inter).
- **Screen Constraints:** Fixed resolution of **1280x800**.
- **Layout Structure:**
  - **Sidebar (Left):** Collapsible navigation bar.
    - **Expanded:** Icon + Label (width ~200px).
    - **Collapsed:** Icon only (width ~64px).
    - **Items:** Dashboard, System Settings, Logs/Diagnostics.
    - **Footer:** "Power/Disconnect" button.
  - **Header (Top):** Status bar (height ~60px).
    - **Left:** Dynamic Connection Status (e.g., "ONLINE", "CONNECTING...", "OFFLINE").
    - **Right:** Notification Bell (icon + count), System Version / Clock.
  - **Main Content (Center):** Dynamic view based on selected navigation item.
  - **Global Overlay:** A top-level item capable of flashing a glowing red border over the entire screen for critical alerts.

### 2. Dashboard View (Home)
A grid layout displaying critical patient data and device status.
- **Grid:** 2x2 Layout (optimized for 1280x800).
- **Stat Cards:**
  - **Card 1: Hemodynamics (ECG)**
    - **Primary Metric:** Heart Rate (BPM).
    - **Secondary Metrics:** ST-Segment Analysis (mm), PVC Count (Premature Ventricular Contractions).
    - **Visualization:** Real-time ECG Lead II waveform (Green, rolling update).
    - **Sophistication:** Simulate occasional "Arrhythmia" events or signal noise.
  - **Card 2: Perfusion & Respiratory**
    - **Primary Metric:** SpO2 (Oxygen Saturation %).
    - **Secondary Metrics:** Respiration Rate (RR - breaths/min), Perfusion Index (PI).
    - **Visualization:** Plethysmograph waveform (Blue, filled area).
    - **Sophistication:** Simulate "Motion Artifact" or "Low Perfusion" states.
  - **Card 3: Infusion Pump Status (TCI)**
    - **Primary Metric:** Flow Rate (mL/hr).
    - **Secondary Metrics:** Drug Name (e.g., "Propofol"), Total Volume Infused (VI), Time Remaining.
    - **Visualization:** Circular progress gauge for syringe volume + Pressure occlusion bar (mmHg).
    - **Sophistication:** Display dynamic "Occlusion Pressure" levels; warn if pressure approaches threshold.
  - **Card 4: Device Health & Telemetry**
    - **Primary Metric:** Battery Runtime Remaining (Hours:Minutes).
    - **Secondary Metrics:** CPU Temp, Network Latency (ms) to Central Station, Active Alerts.
    - **Visualization:** Multi-bar chart for system resources or status list.

### 3. Diagnostics View (Logs)
A view to simulate system logs and remote commands, similar to the "Remote Builder" terminal.
- **Layout:** Split view.
  - **Left/Top:** Command input or "Simulation Control" (e.g., sliders to adjust Heart Rate manually).
  - **Right/Bottom:** Scrolling log terminal showing system events (e.g., "Pump activated", "Sensor calibration started").
- **Style:** Monospace font, dark background (`#1e1e1e`), colored log levels (Info=White, Warn=Yellow, Error=Red).

### 4. Enhanced Clinical Features & Safety
*   **Comprehensive Alarm System:**
    *   **Requirement:** Implement a prioritized alarm system (Critical, High, Medium, Low priority).
    *   **UI:**
        *   **Critical:** Screen-wide, glowing red flash animation.
        *   **High/Medium:** Flashing UI elements on specific cards, audible alerts.
        *   **All:** A dedicated "Alarm History" panel.
    *   **C++ Backend:** An `AlarmManager` class to handle alarm conditions, silencing, and history logging.
*   **Patient Context & Banner:**
    *   **Requirement:** Add a persistent "Patient Banner" at the top of the screen.
    *   **UI:** Display critical info like **Patient ID, Name, Age, and known Allergies**.
*   **Historical Trends View:**
    *   **Requirement:** A new "Trends" view accessible from the sidebar.
    *   **UI:** Should allow plotting all major vital signs (from the SQLite database) over selectable time ranges (e.g., 1 hour, 8 hours, 24 hours).

### 5. Advanced UI/UX & Interactivity
*   **Notification System:**
    *   **Requirement:** Display non-critical informational messages and warnings.
    *   **UI:** A bell icon in the header with a badge showing the number of unread messages. Clicking the bell opens a dropdown/panel with a list of notifications.
*   **System Settings View:**
    *   **Requirement:** Create a `SettingsView.qml`.
    *   **UI:** Allow configuration of:
        *   **Alarm Limits:** Set upper/lower thresholds for Heart Rate, SpO2, etc.
        *   **Display:** Screen brightness, Day/Night theme.
        *   **Sound:** Alarm volume.
*   **User Authentication & Roles:**
    *   **Requirement:** Implement a simple PIN-based login screen.
    - **C++ Backend:** Introduce user roles (e.g., `Clinician`, `Technician`). A `Technician` could access a new "Device Calibration" screen, while a `Clinician` cannot.
*   **Modal Dialogs for Critical Actions:**
    *   **Requirement:** Use modal pop-ups to confirm critical actions like shutting down the device, acknowledging a high-priority alarm, or starting a new patient monitoring session.

### 6. Architectural & Technical Sophistication
*   **Central Server Communication:**
    *   **Requirement:** The device must periodically send its telemetry data (vitals, alarms, device status) to a central server.
    *   **Simulated Server:** A separate, lightweight server application (e.g., Python/Flask or Node.js/Express) will be created to simulate the central monitoring station. It should expose a simple REST API endpoint (e.g., `POST /api/telemetry`).
    *   **Connection Handling:** The device application must gracefully handle connection failures (server down, network issues) and attempt to reconnect. The UI must reflect the current connection status.
*   **Internationalization (i18n):**
    *   **Requirement:** All user-facing strings in QML and C++ must be prepared for translation using `qsTr()`. Include example translation files for English and Spanish (`en_US.ts`, `es_ES.ts`).
*   **Predictive Analytics (Simulated):**
    *   **Requirement:** Add a "Predictive Alert" feature.
    *   **C++ Backend:** The `DeviceSimulator` can generate a "Sepsis Risk" or "Arrhythmia Probability" score based on a combination of vitals.
    *   **UI:** Display this risk score on a dedicated dashboard card with a clear "suggestion" if the risk exceeds a threshold (e.g., "Recommend blood gas analysis").

### 7. Security Architecture
*   **Encryption in Transit:**
    *   **Requirement:** All communication between the device and the central server must be encrypted using **TLS 1.2** or higher. All API endpoints must be accessed via `https://`.
*   **Authentication (Mutual TLS):**
    *   **Requirement:** The device and server must authenticate each other using **Mutual TLS (mTLS)**. Unauthenticated connections must be rejected.
    *   **Device Side:** Each device must be provisioned with a unique client certificate and a corresponding private key. The `NetworkManager` will use these to authenticate itself to the server.
    *   **Server Side:** The server will be configured with a list of trusted client certificate authorities (CAs). It will only allow connections from devices presenting a valid certificate signed by one of these CAs.
    *   **Provisioning:** Certificates are assumed to be securely installed on the device during manufacturing/provisioning.
*   **Encryption at Rest:**
    *   **Requirement:** All sensitive patient data stored locally on the device must be encrypted.
    *   **Implementation:** The SQLite database file must be encrypted using a standard, robust solution like **SQLCipher**.

### 8. Technical Architecture (C++)
- **Language:** C++17 standard.
- **Build System:** CMake.
- **Backend Logic:**
  - **DeviceSimulator:** A class that generates realistic random data.
  - **LogService:** A system to emit log messages from C++ to QML.
  - **AlarmManager:** Manages alarm states, priorities, and history.
  - **PatientManager:** Handles patient context and profiles.
  - **SettingsManager:** Manages device settings and user preferences.
  - **AuthenticationService:** Handles user login and role-based access.
  - **NetworkManager:** Manages connection and data transmission to the central server. It is responsible for configuring the `QSslConfiguration` with the device's client certificate, private key, and trusted CA to implement mTLS.
  - **Data Persistence:**
    - **Database:** Cache all sensor data to a local, **encrypted** SQLite database (`QSqlDatabase` with SQLCipher driver).
    - **Retention Policy:** Keep data for 7 days.
    - **Archival Service:** Create an abstraction `IDataArchiver` to handle data older than 7 days.
  - **Architecture:** Use `QObject` based controllers to interface between C++ logic and QML UI.
    - `DashboardController`: Exposes properties for stats.
    - `SystemController`: Handles navigation state and system commands.
    - `AlarmController`: Exposes active alarms and history to QML.
    - `PatientController`: Exposes current patient data to QML.
    - `SettingsController`: Exposes settings to QML and handles updates.
    - `TrendsController`: Provides historical data for plotting.
    - `NotificationController`: Exposes informational messages to the QML notification system.

### 9. Project Structure
```text
.
├── project-dashboard/
│   ├── CMakeLists.txt       # Build configuration
│   ├── Dockerfile           # Multi-stage build
│   ├── README.md            # Instructions
│   ├── src/
│   │   ├── main.cpp         # Entry point
│   │   ├── core/
│   │   │   ├── DeviceSimulator.cpp/h
│   │   │   ├── AlarmManager.cpp/h
│   │   │   ├── NetworkManager.cpp/h
│   │   │   ├── DatabaseManager.cpp/h
│   │   │   ├── PatientManager.cpp/h
│   │   │   ├── SettingsManager.cpp/h
│   │   │   ├── AuthenticationService.cpp/h
│   │   │   ├── LogService.cpp/h
│   │   │   └── DataArchiver.cpp/h
│   │   └── ui/
│   │       ├── DashboardController.cpp/h
│   │       ├── AlarmController.cpp/h
│   │       ├── SystemController.cpp/h
│   │       ├── PatientController.cpp/h
│   │       ├── SettingsController.cpp/h
│   │       ├── TrendsController.cpp/h
│   │       └── NotificationController.cpp/h
│   ├── resources/
│   │   ├── qml/
│   │   │   ├── Main.qml
│   │   │   ├── components/
│   │   │   │   ├── Sidebar.qml
│   │   │   │   ├── TopBar.qml
│   │   │   │   ├── StatCard.qml
│   │   │   │   ├── SparkLine.qml
│   │   │   │   ├── PatientBanner.qml
│   │   │   │   ├── AlarmIndicator.qml
│   │   │   │   └── NotificationBell.qml
│   │   │   └── views/
│   │   │       ├── DashboardView.qml
│   │   │       ├── DiagnosticsView.qml
│   │   │       ├── TrendsView.qml
│   │   │       ├── SettingsView.qml
│   │   │       └── LoginView.qml
│   │   ├── assets/          # Icons (SVG/PNG)
│   │   ├── i18n/            # Translation files (.ts)
│   │   └── certs/           # Device client certificate and key
│   ├── tests/               # Unit tests
│   └── doc/                 # Documentation
│       ├── 01_OVERVIEW.md
│       ├── 02_ARCHITECTURE.md
│       ├── 03_UI_UX_GUIDE.md
│       ├── 04_ALARM_SYSTEM.md
│       ├── 05_STATE_MACHINES.md
│       ├── 06_SECURITY.md
│       ├── 07_SETUP_GUIDE.md
│       ├── 08_DEFINITIONS.md
│       └── 09_CLASS_DESIGNS.md
└── central-server-simulator/
    ├── app.py               # Simple Flask/FastAPI server
    ├── requirements.txt     # Python dependencies
    ├── README.md
    └── certs/               # Server certificate and trusted CA
```

### 10. Docker & Deployment
- **Base Images:** Use `qtapp-qt-dev-env:latest` (build) and `qtapp-qt-runtime-nano:latest` (runtime).
- **Optimization:** Minimal final image size.
- **Environment:** The app should run in a containerized environment, rendering to a virtual framebuffer or VNC if needed (handled by base image).

### 11. Implementation Steps
1.  **Scaffold:** Create/update the project and folder structures.
2.  **Security Provisioning:** Generate self-signed CA, server, and client certificates for simulation purposes.
3.  **Simulated Server:** Create the Python/Flask server and configure it for mTLS, requiring client certificates.
4.  **Database Encryption:** Integrate SQLCipher support into the build and `DatabaseManager`.
5.  **Backend Core:** Implement all core services in C++.
6.  **mTLS Client:** Implement the mTLS logic in `NetworkManager`, loading certificates and configuring SSL for all requests.
7.  **Controllers:** Implement all C++ controllers to expose data to QML.
8.  **UI:** Build all QML views, components, and the main shell.
9.  **Connect & Test:** Connect all parts, test data flow, connection status, alarms, and security features.
10. **Polish & Finalize:** Add animations and perform final unit testing.

### 12. Documentation
The following documents provide detailed information about the project's design, architecture, and implementation:

*   **[01_OVERVIEW.md](doc/01_OVERVIEW.md)**: High-level introduction to the project, its goals, and core features.
*   **[02_ARCHITECTURE.md](doc/02_ARCHITECTURE.md)**: Detailed software architecture, including C++ backend, QML frontend, and client-server communication, with diagrams.
*   **[03_UI_UX_GUIDE.md](doc/03_UI_UX_GUIDE.md)**: Describes the user interface, layout, color codes, and specific UI behaviors.
*   **[04_ALARM_SYSTEM.md](doc/04_ALARM_SYSTEM.md)**: Comprehensive details on the alarm system, including priorities, visual indicators, and audible patterns.
*   **[05_STATE_MACHINES.md](doc/05_STATE_MACHINES.md)**: State machine diagrams and descriptions for key components and application flows.
*   **[06_SECURITY.md](doc/06_SECURITY.md)**: Detailed documentation of the security architecture, including mTLS, data encryption, and provisioning.
*   **[07_SETUP_GUIDE.md](doc/07_SETUP_GUIDE.md)**: A guide for setting up the development environment, building the project, and configuring the simulated server.
*   **[08_DEFINITIONS.md](doc/08_DEFINITIONS.md)**: A glossary of terms and terminologies used throughout the project.
*   **[09_CLASS_DESIGNS.md](doc/09_CLASS_DESIGNS.md)**: Detailed design of key C++ classes, including their responsibilities, properties, and methods.
*   **[10_DATABASE_DESIGN.md](doc/10_DATABASE_DESIGN.md)**: Detailed design of the local SQLite database schema, including an Entity Relationship Diagram (ERD) and table descriptions.
*   **[11_DATA_FLOW_AND_CACHING.md](doc/11_DATA_FLOW_AND_CACHING.md)**: Describes the data flow from generation to on-device caching and synchronization with the central server, including the server payload definition.
 