# Interface Module: Class Designs

**Document ID:** DESIGN-009a  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides detailed class designs for the **Interface Module**, which handles all user interface and visualization components on the Main/UI Thread.

> **📋 Related Documents:**
> - [Class Designs Overview (09_CLASS_DESIGNS_OVERVIEW.md)](./09_CLASS_DESIGNS_OVERVIEW.md) - High-level module architecture
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Thread architecture
> - [Waveform Display (41_WAVEFORM_DISPLAY_IMPLEMENTATION.md)](./41_WAVEFORM_DISPLAY_IMPLEMENTATION.md) - Waveform rendering guide

---

## 1. Module Overview

**Thread:** Main/UI Thread  
**Priority:** Default (QoS User Interactive on macOS)  
**Component Count:** 30 components (11 controllers + 7 views + 12 components)

**Purpose:**
- Expose data to QML via Q_PROPERTY bindings
- Handle user interactions (button clicks, form submissions)
- Render visualizations (waveforms at 60 FPS, trends, vitals)
- Navigate between views

**Visualization Strategy:**
- **QML-Based Rendering:** All visualization is handled declaratively in QML
- **No Separate Visualization Service:** Rendering logic resides in QML components
- **Performance:** Critical visualizations (waveforms at 60 FPS) use QML Canvas API

---

## 2. Module Diagram

[View Interface Module Diagram (Mermaid)](./09a_INTERFACE_MODULE.mmd)  
[View Interface Module Diagram (SVG)](./09a_INTERFACE_MODULE.svg)

---

## 3. QML Controllers (C++ QObject Bridges)

Controllers bridge data from C++ application services to QML views and components. They run on the Main/UI Thread and expose properties/signals to QML.

### 3.1. DashboardController

**Responsibility:** Exposes real-time vital signs and device status to the Dashboard View.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `heartRate`: `int` - Current heart rate (BPM) for UI binding
- `spo2`: `int` - Current oxygen saturation (%)
- `batteryLevel`: `int` - Current battery level (%)
- `respirationRate`: `int` - Current respiration rate (breaths/min)
- `perfusionIndex`: `double` - Current perfusion index
- `infusionRate`: `double` - Current medication infusion rate (mL/hr)
- `drugName`: `QString` - Currently infusing drug
- `stSegment`: `double` - ST-Segment analysis value (mm)
- `pvcCount`: `int` - Premature ventricular contraction count

**Q_INVOKABLE Methods:**
- None (read-only display)

**Slots:**
- `onNewData(const PatientData& data)`: Receives new data from `MonitoringService` via queued connection

**Signals:**
- `vitalsUpdated()`: Emitted when vitals change (triggers QML binding updates)

**Dependencies:**
- `MonitoringService`: Source of vitals data (via queued signals)
- `VitalsCache`: Indirect (via MonitoringService)

**Data Flow:**
```
MonitoringService (RT Thread)
    ↓ Qt::QueuedConnection (signal)
DashboardController::onNewData() (Main/UI Thread)
    ↓ Q_PROPERTY update
QML Binding (DashboardView.qml)
    ↓ Property binding
StatCard.qml (displays value)
```

---

### 3.2. AlarmController

**Responsibility:** Exposes alarm state and history to QML.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `activeAlarms`: `QVariantList` - List of active alarms (for QML ListView)
- `alarmHistory`: `QVariantList` - Historical alarm records
- `hasCriticalAlarm`: `bool` - Boolean flag for critical alarm presence (for UI priority)
- `alarmCount`: `int` - Total number of active alarms

**Q_INVOKABLE Methods:**
- `acknowledgeAlarm(const QString& alarmId)`: Acknowledges an alarm from UI
  - Validates alarm exists
  - Delegates to `MonitoringService` (via queued call)
  - Updates UI when acknowledgment completes
- `silenceAlarm(const QString& alarmId, int durationSeconds)`: Silences an alarm
  - Validates duration (max 600 seconds)
  - Delegates to `MonitoringService` (via queued call)
- `unsilenceAlarm(const QString& alarmId)`: Manually unsilences an alarm
  - Delegates to `MonitoringService` (via queued call)

**Slots:**
- `onAlarmStateChanged(const QList<AlarmEvent>& alarms)`: Updates UI when alarms change (from MonitoringService)

**Signals:**
- `alarmAcknowledged(const QString& alarmId)`: Emitted when acknowledgment completes
- `alarmSilenced(const QString& alarmId)`: Emitted when alarm silenced
- `criticalAlarmActiveChanged(bool active)`: Emitted when critical alarm state changes

**Dependencies:**
- `MonitoringService`: Alarm state source (via queued signals)
- `SecurityService`: User authentication for alarm actions (via queued calls)

---

### 3.3. WaveformController

**Responsibility:** Bridges waveform data from C++ to QML for real-time waveform visualization.

**Thread:** Main/UI Thread (required for QML property bindings)

**Properties (Q_PROPERTY):**
- `samples`: `QVariantList` - Array of waveform sample values exposed to QML (read-only)
- `sampleRate`: `int` - Samples per second (250 Hz for ECG, 125 Hz for Pleth) (read-only)
- `channel`: `QString` - Waveform channel identifier (e.g., "ECG_LEAD_II", "PLETH") (read-only)
- `isActive`: `bool` - Whether waveform data is actively being received (read-only)

**Q_INVOKABLE Methods:**
- `updateSamples(const QVariantList& newSamples)`: Updates waveform samples from `WaveformCache` (called by `MonitoringService` via queued connection)
- `clearSamples()`: Clears waveform buffer (useful when switching patients or resetting display)
- `setChannel(const QString& channel)`: Sets waveform channel identifier

**Signals:**
- `samplesChanged()`: Emitted when samples property changes (triggers QML binding updates)
- `channelChanged(const QString& channel)`: Emitted when channel changes

**Dependencies:**
- `WaveformCache`: Source of waveform data (accessed via `MonitoringService`)
- `MonitoringService`: Provides waveform samples via signals

**Data Flow:**
1. `MonitoringService` (RT Thread) receives waveform samples from `SharedMemorySensorDataSource`
2. `MonitoringService` stores samples in `WaveformCache` (30-second circular buffer)
3. `MonitoringService` emits signal with window of samples (e.g., last 2,500 samples for 10-second display)
4. `WaveformController::updateSamples()` called (via queued connection from RT thread)
5. `samples` Q_PROPERTY updated, triggering QML binding
6. `WaveformChart.qml` receives data via property binding
7. Canvas renders waveform at 60 FPS

**Performance:**
- Property updates: < 1ms (simple QVariantList assignment)
- QML binding updates: Automatic (Qt engine handles efficiently)
- Memory: ~10 MB for 30-second buffer (managed by `WaveformCache`)

**See:** [41_WAVEFORM_DISPLAY_IMPLEMENTATION.md](./41_WAVEFORM_DISPLAY_IMPLEMENTATION.md) for complete waveform rendering implementation guide.

---

### 3.4. PatientController

**Responsibility:** Exposes patient information to the Patient Banner and related UI, and provides patient admission/discharge functionality.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `mrn`: `QString` - Medical Record Number (read-only)
- `patientName`: `QString` - Patient name (read-only)
- `patientAge`: `int` - Patient age (read-only)
- `dateOfBirth`: `QDate` - Patient date of birth (read-only)
- `sex`: `QString` - Patient sex (read-only)
- `allergies`: `QStringList` - List of allergies (read-only)
- `bedLocation`: `QString` - Current patient's bed location (read-only)
- `admissionState`: `QString` - Current admission state (enum: "NotAdmitted", "Admitted", "Discharged")
- `isAdmitted`: `bool` - Whether patient is currently admitted (read-only)
- `admittedAt`: `QDateTime` - Timestamp of admission (read-only)
- `isLookingUp`: `bool` - Boolean indicating if a patient lookup is in progress
- `lookupError`: `QString` - Last error message from patient lookup (empty if no error)

**Q_INVOKABLE Methods:**
- `openAdmissionModal()`: Opens admission modal for admitting a patient
- `admitPatient(const QString& mrn, const QString& admissionSource)`: Admits patient
  - `admissionSource`: "manual", "barcode", or "central_station"
  - Delegates to `AdmissionService` (via queued call)
  - Updates UI when admission completes
- `dischargePatient()`: Discharges current patient (requires confirmation)
  - Delegates to `AdmissionService` (via queued call)
- `scanBarcode()`: Initiates barcode scanning for admission
  - Opens barcode scanner UI
  - On scan success, calls `admitPatient()` with scanned MRN
- `lookupPatientById(const QString& patientId)`: Initiates patient lookup by ID (asynchronous)
  - Delegates to `AdmissionService` (via queued call)
  - Updates `isLookingUp` property during lookup
  - Updates `lookupError` if lookup fails

**Slots:**
- `onPatientChanged(const Patient& patient)`: Updates UI when patient changes (from AdmissionService)
- `onPatientAdmitted(const Patient& patient, const QString& admissionSource)`: Updates UI when patient is admitted
- `onPatientDischarged(const QString& mrn)`: Updates UI when patient is discharged
- `onAdmissionStateChanged(AdmissionState state)`: Updates UI when admission state changes
- `onPatientLookupStarted(const QString& patientId)`: Updates UI when lookup begins
- `onPatientLookupCompleted(const PatientInfo& info)`: Updates UI when lookup succeeds
- `onPatientLookupFailed(const QString& patientId, const QString& error)`: Updates UI when lookup fails

**Dependencies:**
- `AdmissionService`: Patient admission/discharge logic (via queued calls)
- `IPatientLookupService`: Patient lookup from HIS/EHR (via AdmissionService)

---

### 3.5. SettingsController

**Responsibility:** Exposes device settings to the Settings View (excluding network/provisioning settings).

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `allSettings`: `QVariantMap` - Map of all configurable settings
- `deviceId`: `QString` - Device identifier for telemetry transmission (read/write)
- `deviceLabel`: `QString` - Static device identifier/asset tag (read-only for most users, read/write for Technician role)
- `measurementUnit`: `QString` - Measurement unit preference ("metric" or "imperial", read/write)

**Note:** `bedId` property has been removed. Bed location is now managed through Patient object and ADT workflow.

**Q_INVOKABLE Methods:**
- `updateSetting(const QString& key, const QVariant& value)`: Updates a setting from UI
  - Validates setting key and value
  - Delegates to `SettingsManager` (via queued call)
  - Updates UI when setting saved
- `resetToDefaults()`: Resets all settings to factory defaults
  - Delegates to `SettingsManager` (via queued call)
  - Requires confirmation dialog

**Slots:**
- `onSettingsChanged()`: Refreshes UI when settings change (from SettingsManager)

**Dependencies:**
- `SettingsManager`: Configuration storage (via queued calls)

---

### 3.6. ProvisioningController

**Responsibility:** Exposes device provisioning state and controls to the Network Settings View. Manages QR code-based pairing workflow.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `provisioningState`: `QString` - Current provisioning state (enum: "NotProvisioned", "ReadyToPair", "Pairing", "Configuring", "Validating", "Provisioned", "Error")
- `pairingCode`: `QString` - Current pairing code (e.g., "ABC-123-XYZ")
- `qrCodeData`: `QByteArray` - QR code image data for display
- `expirationTime`: `int` - Time remaining until pairing code expires in seconds
- `errorMessage`: `QString` - Error message if provisioning failed (empty if no error)
- `isProvisioned`: `bool` - Whether device is successfully provisioned (read-only)
- `serverUrl`: `QString` - Current server URL (read-only, empty if not provisioned)
- `certificateStatus`: `QString` - Certificate status information (read-only)
- `connectionStatus`: `QString` - Current connection status (enum: "Connected", "Connecting", "Disconnected")
- `lastConnected`: `QDateTime` - Timestamp of last successful connection (read-only)

**Q_INVOKABLE Methods:**
- `enterProvisioningMode()`: Enters provisioning mode (requires Technician role)
  - Validates user permissions via `SecurityService`
  - Delegates to `ProvisioningService` (via queued call)
- `exitProvisioningMode()`: Exits provisioning mode
  - Delegates to `ProvisioningService` (via queued call)
- `regenerateQRCode()`: Generates new QR code and pairing code
  - Delegates to `ProvisioningService` (via queued call)
- `simulateConfiguration()`: Simulates configuration push from Central Station (development/testing only)
  - Generates mock configuration payload
  - Delegates to `ProvisioningService` (via queued call)
- `reprovisionDevice()`: Starts re-provisioning process (requires Technician role and confirmation)
  - Shows confirmation dialog
  - Delegates to `ProvisioningService` (via queued call)
- `cancelProvisioning()`: Cancels current provisioning operation
  - Delegates to `ProvisioningService` (via queued call)

**Dependencies:**
- `ProvisioningService`: Core provisioning logic and state management (via queued calls)
- `SecurityService`: For role-based access control (Technician role required) (via queued calls)
- `NetworkManager`: For connection testing after provisioning (via queued calls)
- `SettingsManager`: For storing provisioned configuration (via queued calls)

**Signals:**
- `provisioningStateChanged(ProvisioningState state)`: Emitted when provisioning state changes
- `pairingCodeGenerated(const QString& code)`: Emitted when new pairing code is generated
- `provisioningCompleted()`: Emitted when provisioning completes successfully
- `provisioningFailed(const QString& error)`: Emitted when provisioning fails
- `connectionStatusChanged(ConnectionStatus status)`: Emitted when connection status changes

---

### 3.7. TrendsController

**Responsibility:** Provides historical data for trend visualization.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `trendData`: `QVariantList` - Historical data points for plotting
- `isLoading`: `bool` - Whether trend data is being loaded
- `timeRange`: `QString` - Current time range ("1h", "8h", "24h")

**Q_INVOKABLE Methods:**
- `loadTrends(const QString& parameter, const QString& timeRange)`: Loads trend data for specified parameter and time range
  - `parameter`: "HR", "SPO2", "RR", etc.
  - `timeRange`: "1h", "8h", "24h"
  - Delegates to `IVitalsRepository` (via queued call)
  - Updates `trendData` property when data loaded
- `setTimeRange(const QString& timeRange)`: Changes time range and reloads data

**Slots:**
- `onTrendDataLoaded(const QList<VitalRecord>& data)`: Updates UI when trend data loaded (from repository)

**Dependencies:**
- `IVitalsRepository`: Historical vitals data (via queued calls)
- `MonitoringService`: Indirect (via repository)

---

### 3.8. SystemController

**Responsibility:** Manages system-wide state, navigation, and global controls.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `connectionStatusString`: `QString` - Human-readable connection status ("Online", "Offline", "Connecting")
- `currentView`: `QString` - Currently active view name
- `systemHealth`: `QVariantMap` - System health metrics (CPU, memory, disk)
- `firmwareVersion`: `QString` - Current firmware version

**Q_INVOKABLE Methods:**
- `navigateTo(const QString& viewName)`: Changes active view
  - Updates `currentView` property
  - Triggers QML Loader to load new view
- `powerOff()`: Initiates device shutdown
  - Shows confirmation dialog
  - Delegates to system shutdown (via queued call)
- `restart()`: Restarts the application
  - Shows confirmation dialog
  - Delegates to application restart

**Slots:**
- `onConnectionStatusChanged(NetworkManager::ConnectionStatus status)`: Updates connection status (from NetworkManager)
- `onSystemHealthUpdated(const QVariantMap& health)`: Updates system health (from HealthMonitor)

**Dependencies:**
- `NetworkManager`: Connection status (via queued signals)
- `HealthMonitor`: System health metrics (via queued signals)

---

### 3.9. NotificationController

**Responsibility:** Manages non-critical informational messages and warnings.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `notifications`: `QVariantList` - List of notification messages
- `unreadCount`: `int` - Count of unread notifications

**Q_INVOKABLE Methods:**
- `dismissNotification(int id)`: Dismisses a specific notification
  - Removes notification from list
  - Updates `unreadCount`
- `clearAll()`: Clears all notifications
  - Requires login (delegates to `SecurityService`)
  - Clears notification list
  - Resets `unreadCount` to 0

**Methods:**
- `addNotification(const QString& message, const QString& level)`: Adds a new notification
  - `level`: "info", "warning", "error"
  - Updates `notifications` list
  - Increments `unreadCount`
  - Emits `notificationAdded()` signal

**Dependencies:**
- `LogService`: Indirect (notifications may be logged)

---

### 3.10. DiagnosticsController

**Responsibility:** System diagnostics and logs.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `logEntries`: `QVariantList` - Recent log entries (last 1000)
- `systemInfo`: `QVariantMap` - System information (OS, Qt version, device ID)
- `isLogging`: `bool` - Whether log streaming is active

**Q_INVOKABLE Methods:**
- `refreshLogs()`: Refreshes log entries from `LogService`
- `clearLogs()`: Clears log display (does not delete log files)
- `exportLogs(const QString& filePath)`: Exports logs to file
  - Requires login (delegates to `SecurityService`)
  - Delegates to `LogService` (via queued call)

**Dependencies:**
- `LogService`: Application logs (via queued calls)
- `HealthMonitor`: System health metrics (via queued calls)

---

### 3.11. AuthenticationController

**Responsibility:** Login/logout UI.

**Thread:** Main/UI Thread

**Properties (Q_PROPERTY):**
- `isLoggedIn`: `bool` - Whether user is currently logged in
- `currentUser`: `QString` - Current user ID (empty if not logged in)
- `currentRole`: `QString` - Current user role ("NURSE", "PHYSICIAN", "TECHNICIAN", "ADMINISTRATOR")
- `loginError`: `QString` - Last login error message (empty if no error)
- `remainingLockoutTime`: `int` - Remaining lockout time in seconds (0 if not locked out)

**Q_INVOKABLE Methods:**
- `login(const QString& userId, const QString& secretCode)`: Authenticates user
  - Delegates to `SecurityService` (via queued call)
  - Updates UI when authentication completes
- `logout()`: Logs out current user
  - Delegates to `SecurityService` (via queued call)
  - Clears user session
- `checkSessionTimeout()`: Checks if session has expired
  - Called periodically by timer
  - Delegates to `SecurityService` (via queued call)

**Slots:**
- `onUserLoggedIn(const QString& userId, const QString& role)`: Updates UI when login succeeds (from SecurityService)
- `onUserLoggedOut()`: Updates UI when logout completes (from SecurityService)
- `onSessionExpired()`: Updates UI when session expires (from SecurityService)
- `onLoginFailed(const QString& error)`: Updates UI when login fails (from SecurityService)

**Dependencies:**
- `SecurityService`: Authentication logic (via queued calls)

---

## 4. QML Views (Full Screens)

QML Views are full-screen UI components that compose QML Components and bind to Controllers.

| View | File | Controller | Responsibility |
|------|------|------------|----------------|
| `LoginView` | `LoginView.qml` | `AuthenticationController` | User login screen |
| `DashboardView` | `DashboardView.qml` | `DashboardController`, `WaveformController` | Main monitoring screen with vitals and waveforms |
| `TrendsView` | `TrendsView.qml` | `TrendsController` | Historical data visualization |
| `AlarmsView` | `AlarmsView.qml` | `AlarmController` | Alarm history and management |
| `SettingsView` | `SettingsView.qml` | `SettingsController`, `ProvisioningController` | Device configuration |
| `DiagnosticsView` | `DiagnosticsView.qml` | `DiagnosticsController` | System logs and diagnostics |
| `PatientAdmissionModal` | `PatientAdmissionModal.qml` | `PatientController` | Patient admission workflow |

**See:** [03_UI_UX_GUIDE.md](./03_UI_UX_GUIDE.md) for detailed UI/UX specifications.

---

## 5. QML Components (Reusable)

QML Components are reusable UI widgets that can be composed into Views.

**Visualization Components:**
- `WaveformChart.qml` - Real-time waveform rendering (Canvas API, 60 FPS)
- `TrendChart.qml` - Historical trends visualization (Line chart)

**Display Components:**
- `StatCard.qml` - Numeric vital signs display
- `PatientBanner.qml` - Patient info header
- `AlarmIndicator.qml` - Visual alarm indicators
- `NotificationBell.qml` - Notification badge

**Navigation Components:**
- `Sidebar.qml` - Navigation sidebar
- `TopBar.qml` - Top application bar

**Form Components:**
- `SettingsRow.qml` - Single settings row
- `ConfirmDialog.qml` - Confirmation dialog
- `LoadingSpinner.qml` - Loading indicator
- `QRCodeDisplay.qml` - QR code display

**See:** [03_UI_UX_GUIDE.md](./03_UI_UX_GUIDE.md) for detailed component specifications.

---

## 6. Module Communication

### 6.1 Inbound (From Other Modules)

**From Real-Time Processing Module:**
- `Qt::QueuedConnection` signals for vitals updates
- `Qt::QueuedConnection` signals for alarm events
- `Qt::QueuedConnection` signals for waveform samples

**From Application Services Module:**
- `Qt::QueuedConnection` signals for patient admission/discharge
- `Qt::QueuedConnection` signals for provisioning state changes
- `Qt::QueuedConnection` signals for authentication events

**From Network Module:**
- `Qt::QueuedConnection` signals for connection status changes

**From Database Module:**
- `Qt::QueuedConnection` signals for trend data loaded

### 6.2 Outbound (To Other Modules)

**To Application Services Module:**
- `Qt::QueuedConnection` method invocations (Q_INVOKABLE)
- User action commands (admit patient, provision device, login)

**To Database Module:**
- `Qt::QueuedConnection` method invocations (load trends, export logs)

**To Network Module:**
- `Qt::QueuedConnection` method invocations (test connection)

---

## 7. Performance Considerations

**Frame Budget:**
- **60 FPS Target:** < 16.67ms per frame
- **30 FPS Minimum:** < 33ms per frame
- Keep controller logic lightweight (delegate heavy work to other modules)

**QML Binding Performance:**
- Q_PROPERTY updates are efficient (automatic binding updates)
- Avoid complex calculations in QML (do in C++ controllers)
- Use `ListView` with `delegate` for large lists (virtualization)

**Memory:**
- Controllers are lightweight (just property holders)
- QML components handle their own rendering memory
- Waveform data managed by `WaveformCache` (RT Thread)

---

## 8. Related Documents

- **[09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md)** - High-level module architecture
- **[12_THREAD_MODEL.md](./12_THREAD_MODEL.md)** - Thread architecture (Section 4.1: Main/UI Thread)
- **[03_UI_UX_GUIDE.md](./03_UI_UX_GUIDE.md)** - UI/UX specifications
- **[41_WAVEFORM_DISPLAY_IMPLEMENTATION.md](./41_WAVEFORM_DISPLAY_IMPLEMENTATION.md)** - Waveform rendering guide

---

*This document provides detailed class designs for the Interface Module. For other modules, see the module-specific documents listed in [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md).*


