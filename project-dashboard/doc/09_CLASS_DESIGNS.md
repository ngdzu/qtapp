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
**Responsibility:** Manages secure network connectivity to the central server using mTLS. Integrates with `ITelemetryServer` interface for server communication. Implements comprehensive security measures including certificate validation, data signing, and audit logging.

**Key Properties:**
- `currentStatus`: Current connection status (Connected, Connecting, Disconnected)
- `sslConfig`: SSL/TLS configuration including client certificates
- `serverUrl`: Configurable server URL (from SettingsManager)
- `clientCertificate`: Device client certificate for mTLS
- `clientPrivateKey`: Device private key for mTLS
- `caCertificate`: CA certificate for server verification

**Key Methods:**
- `connectToServer()`: Initiates connection to central server using configured server URL and mTLS
- `sendTelemetry(const TelemetryData& data)`: Transmits data to server via `ITelemetryServer` with digital signature
- `sendSensorData(const SensorData& data)`: Transmits sensor data to server with digital signature
- `configure(const QSslConfiguration& sslConfig)`: Configures mTLS settings
- `setServerUrl(const QString& url)`: Updates server URL and reconnects if needed
- `getServerUrl()`: Returns current server URL
- `validateCertificates()`: Validates client certificate (expiration, revocation, device ID match)
- `loadCertificates()`: Loads certificates from secure storage
- `signPayload(const QByteArray& data)`: Creates digital signature for data integrity
- `checkCertificateRevocation()`: Checks certificate revocation list (CRL)

**Security Features:**
- **mTLS:** Mutual TLS with client and server certificate validation
- **Certificate Management:** Validates certificates on startup, checks expiration and revocation
- **Data Signing:** Digital signatures on all telemetry payloads (ECDSA or RSA)
- **Replay Prevention:** Timestamp validation with nonce
- **Rate Limiting:** Client-side rate limiting (60 requests/minute)
- **Circuit Breaker:** Prevents resource exhaustion on repeated failures
- **Audit Logging:** Logs all security events to `security_audit_log` table

**Dependencies:**
- `ITelemetryServer`: Interface for server communication (NetworkTelemetryServer, MockTelemetryServer, etc.)
- `SettingsManager`: For retrieving and storing server URL configuration
- `DatabaseManager`: For certificate tracking and security audit logging
- `LogService`: For security event logging

**Signals:**
- `connectionStatusChanged(ConnectionStatus status)`: Emitted when connection status changes
- `telemetrySent(const TelemetryData& data, const ServerResponse& response)`: Emitted when telemetry is successfully sent
- `telemetrySendFailed(const TelemetryData& data, const QString& error)`: Emitted when telemetry send fails
- `certificateExpiring(const QSslCertificate& cert, int daysRemaining)`: Emitted when certificate is expiring soon
- `certificateValidationFailed(const QString& reason)`: Emitted when certificate validation fails

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
- `loadPatient(const QString& id)`: Loads patient profile from local database
- `loadPatientById(const QString& id)`: Looks up patient by ID, first checking local database, then using `IPatientLookupService` if not found locally
- `setCurrentPatient(const PatientInfo& info)`: Sets the current patient context
- `getCurrentPatient()`: Returns current patient data
- `clearCurrentPatient()`: Clears the current patient context

**Dependencies:**
- `IPatientLookupService`: Interface for looking up patient information from external systems (HIS/EHR)
- `DatabaseManager`: For local patient data storage and retrieval

**Signals:**
- `patientChanged(const Patient& patient)`: Emitted when patient context changes
- `patientLookupStarted(const QString& patientId)`: Emitted when patient lookup begins
- `patientLookupCompleted(const PatientInfo& info)`: Emitted when patient lookup succeeds
- `patientLookupFailed(const QString& patientId, const QString& error)`: Emitted when patient lookup fails

### 2.6. SettingsManager
**Responsibility:** Manages device configuration settings and user preferences.

**Key Properties:**
- `settings`: Map of all configuration settings

**Key Methods:**
- `loadSettings()`: Loads settings from persistent storage
- `saveSettings()`: Persists settings to storage
- `getValue(const QString& key)`: Retrieves a setting value
- `setValue(const QString& key, const QVariant& value)`: Updates a setting

**Configuration Settings:**
- `deviceId`: Unique identifier for the monitoring device (QString)
- `bedId`: Identifier for the bed/room location (QString)
- `measurementUnit`: System preference for metric or imperial units (QString: "metric" or "imperial")
- `serverUrl`: Central server URL for telemetry transmission (QString, default: "https://localhost:8443")
- `useMockServer`: Boolean flag to use mock server for testing/development (bool, default: false)

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
**Responsibility:** Exposes patient information to the Patient Banner and related UI, and provides patient lookup functionality.

**Properties (Q_PROPERTY):**
- `patientId`: Patient identifier
- `patientName`: Patient name
- `patientAge`: Patient age
- `allergies`: List of allergies
- `isLookingUp`: Boolean indicating if a patient lookup is in progress
- `lookupError`: Last error message from patient lookup (empty if no error)

**Q_INVOKABLE Methods:**
- `lookupPatientById(const QString& patientId)`: Initiates patient lookup by ID (asynchronous)
- `clearPatient()`: Clears the current patient context

**Slots:**
- `onPatientChanged(const Patient& patient)`: Updates UI when patient changes
- `onPatientLookupStarted(const QString& patientId)`: Updates UI when lookup begins
- `onPatientLookupCompleted(const PatientInfo& info)`: Updates UI when lookup succeeds
- `onPatientLookupFailed(const QString& patientId, const QString& error)`: Updates UI when lookup fails

### 3.5. SettingsController
**Responsibility:** Exposes device settings to the Settings View.

**Properties (Q_PROPERTY):**
- `allSettings`: Map of all configurable settings
- `deviceId`: Device identifier (QString, read/write)
- `bedId`: Bed/room location identifier (QString, read/write)
- `measurementUnit`: Measurement unit preference (QString: "metric" or "imperial", read/write)
- `serverUrl`: Central server URL (QString, read/write)
- `useMockServer`: Use mock server for testing (bool, read/write)

**Q_INVOKABLE Methods:**
- `updateSetting(const QString& key, const QVariant& value)`: Updates a setting from UI
- `resetToDefaults()`: Resets all settings to factory defaults
- `testServerConnection()`: Tests connection to configured server URL

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
