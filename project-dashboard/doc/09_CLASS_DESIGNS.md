# C++ Class Designs

This document provides detailed class diagrams and descriptions for the key C++ classes in the `project-dashboard` application.

## 1. Class Diagram Overview

[View the full Class Diagram (interactive)](./09_CLASS_DESIGNS.mmd)

This diagram provides a comprehensive overview of all major C++ classes and their relationships. Use a Mermaid-compatible viewer to zoom and pan.


## 2. Core Service Classes

### 2.1. DeviceSimulator
**Responsibility:** Generates realistic simulated patient vital signs and device operational data.

**Key Properties:**
- `currentHeartRate`: Current simulated heart rate (BPM)
- `currentSpO2`: Current oxygen saturation (%)
- `currentBatteryLevel`: Current battery level (%)
- `currentRespirationRate`: Current respiration rate (breaths/min)
- `currentPerfusionIndex`: Current perfusion index
- `currentInfusionRate`: Current medication infusion rate (mL/hr)

**Key Methods:**
- `start()`: Begins the simulation timer
- `stop()`: Stops the simulation timer

**Signals:**
- `newData(const PatientData& data)`: Emitted when new simulated data is available

### 2.2. AlarmManager
**Responsibility:** Monitors patient data, detects alarm conditions, manages alarm states, and maintains alarm history.

**Key Properties:**
- `activeAlarms`: List of currently active alarms
- `alarmHistory`: Historical record of all alarms

**Key Methods:**
- `processData(const PatientData& data)`: Analyzes incoming data for alarm conditions
- `acknowledgeAlarm(int alarmId)`: Marks an alarm as acknowledged
- `silenceAlarm(int alarmId, int duration)`: Temporarily silences an alarm

**Signals:**
- `alarmStateChanged(const QList<Alarm>& activeAlarms)`: Emitted when alarm state changes

### 2.3. NetworkManager
**Responsibility:** Manages secure network connectivity to the central server using mTLS.

**Key Properties:**
- `currentStatus`: Current connection status (Connected, Connecting, Disconnected)
- `sslConfig`: SSL/TLS configuration including client certificates

**Key Methods:**
- `connectToServer()`: Initiates connection to central server
- `sendTelemetry(const TelemetryData& data)`: Transmits data to server
- `configure(const QSslConfiguration& sslConfig)`: Configures mTLS settings

**Signals:**
- `connectionStatusChanged(ConnectionStatus status)`: Emitted when connection status changes

### 2.4. DatabaseManager
**Responsibility:** Manages encrypted SQLite database for local data persistence.

**Key Properties:**
- `database`: QSqlDatabase instance with SQLCipher encryption

**Key Methods:**
- `open(const QString& path, const QString& password)`: Opens encrypted database
- `saveData(const PatientData& data)`: Stores patient data
- `getHistoricalData(QDateTime start, QDateTime end)`: Retrieves historical data for trends
- `cleanupOldData(int retentionDays)`: Removes data older than retention policy

### 2.5. PatientManager
**Responsibility:** Manages patient context, including identification and safety information.

**Key Properties:**
- `patientId`: Unique patient identifier
- `patientName`: Patient full name
- `patientAge`: Patient age
- `allergies`: List of known patient allergies

**Key Methods:**
- `loadPatient(const QString& id)`: Loads patient profile from database
- `getCurrentPatient()`: Returns current patient data

**Signals:**
- `patientChanged(const Patient& patient)`: Emitted when patient context changes

### 2.6. SettingsManager
**Responsibility:** Manages device configuration settings and user preferences.

**Key Properties:**
- `settings`: Map of all configuration settings

**Key Methods:**
- `loadSettings()`: Loads settings from persistent storage
- `saveSettings()`: Persists settings to storage
- `getValue(const QString& key)`: Retrieves a setting value
- `setValue(const QString& key, const QVariant& value)`: Updates a setting

**Signals:**
- `settingsChanged()`: Emitted when settings are modified

### 2.7. AuthenticationService
**Responsibility:** Handles user authentication and role-based access control.

**Key Properties:**
- `currentUser`: Currently logged-in user
- `currentRole`: Current user role (Clinician, Technician)

**Key Methods:**
- `login(const QString& pin)`: Authenticates user with PIN
- `logout()`: Logs out current user
- `getCurrentRole()`: Returns current user's role

**Signals:**
- `userLoggedIn(const QString& user, UserRole role)`: Emitted on successful login
- `userLoggedOut()`: Emitted when user logs out

### 2.8. LogService
**Responsibility:** Provides centralized logging mechanism for the application.

**Key Properties:**
- `logs`: Collection of log entries

**Key Methods:**
- `logInfo(const QString& message)`: Logs informational message
- `logWarning(const QString& message)`: Logs warning message
- `logError(const QString& message)`: Logs error message

**Signals:**
- `newLogEntry(const LogEntry& entry)`: Emitted when new log entry is created

### 2.9. DataArchiver
**Responsibility:** Handles archival of historical data beyond retention period.

**Key Methods:**
- `archiveData(const QList<PatientData>& data)`: Archives old data

**Signals:**
- `dataArchived(int recordCount)`: Emitted when archival completes

## 3. UI Controller Classes

### 3.1. DashboardController
**Responsibility:** Exposes real-time vital signs and device status to the Dashboard View.

**Properties (Q_PROPERTY):**
- `heartRate`: Current heart rate for UI binding
- `spo2`: Current oxygen saturation
- `batteryLevel`: Current battery level
- `respirationRate`: Current respiration rate
- `perfusionIndex`: Current perfusion index
- `infusionRate`: Current medication infusion rate
- `drugName`: Currently infusing drug
- `stSegment`: ST-Segment analysis value
- `pvcCount`: Premature ventricular contraction count

**Slots:**
- `onNewData(const PatientData& data)`: Receives new data from DeviceSimulator

### 3.2. AlarmController
**Responsibility:** Exposes alarm state and history to QML.

**Properties (Q_PROPERTY):**
- `activeAlarms`: List of active alarms
- `alarmHistory`: Historical alarm records
- `hasCriticalAlarm`: Boolean flag for critical alarm presence

**Q_INVOKABLE Methods:**
- `acknowledge(int alarmId)`: Acknowledges an alarm from UI
- `silence(int alarmId, int duration)`: Silences an alarm

**Slots:**
- `onAlarmStateChanged(const QList<Alarm>& alarms)`: Updates UI when alarms change

### 3.3. SystemController
**Responsibility:** Manages system-wide state, navigation, and global controls.

**Properties (Q_PROPERTY):**
- `connectionStatusString`: Human-readable connection status
- `currentView`: Currently active view name

**Q_INVOKABLE Methods:**
- `navigateTo(const QString& viewName)`: Changes active view
- `powerOff()`: Initiates device shutdown

**Slots:**
- `onConnectionStatusChanged(NetworkManager::ConnectionStatus status)`: Updates connection status

### 3.4. PatientController
**Responsibility:** Exposes patient information to the Patient Banner and related UI.

**Properties (Q_PROPERTY):**
- `patientId`: Patient identifier
- `patientName`: Patient name
- `patientAge`: Patient age
- `allergies`: List of allergies

**Slots:**
- `onPatientChanged(const Patient& patient)`: Updates UI when patient changes

### 3.5. SettingsController
**Responsibility:** Exposes device settings to the Settings View.

**Properties (Q_PROPERTY):**
- `allSettings`: Map of all configurable settings

**Q_INVOKABLE Methods:**
- `updateSetting(const QString& key, const QVariant& value)`: Updates a setting from UI
- `resetToDefaults()`: Resets all settings to factory defaults

**Slots:**
- `onSettingsChanged()`: Refreshes UI when settings change

### 3.6. TrendsController
**Responsibility:** Provides historical data for trend visualization.

**Properties (Q_PROPERTY):**
- `trendData`: Historical data points for plotting

**Q_INVOKABLE Methods:**
- `loadTrends(const QString& parameter, QDateTime start, QDateTime end)`: Loads trend data for specified parameter and time range

### 3.7. NotificationController
**Responsibility:** Manages non-critical informational messages and warnings.

**Properties (Q_PROPERTY):**
- `notifications`: List of notification messages
- `unreadCount`: Count of unread notifications

**Methods:**
- `addNotification(const QString& message, const QString& level)`: Adds a new notification

**Q_INVOKABLE Methods:**
- `dismissNotification(int id)`: Dismisses a specific notification
- `clearAll()`: Clears all notifications
