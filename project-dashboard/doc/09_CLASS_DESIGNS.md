# C++ Class Designs

This document provides detailed class diagrams and descriptions for the key C++ classes in the `project-dashboard` application.

## 1. Class Diagram Overview

[View the full Class Diagram (interactive)](./09_CLASS_DESIGNS.mmd)

This diagram provides a comprehensive overview of all major C++ classes and their relationships. Use a Mermaid-compatible viewer to zoom and pan.


## 2. DDD Layering Summary

### 2.1. Domain Layer (new classes to introduce)
- **PatientAggregate** – Admission lifecycle, vitals state, emits `PatientAdmitted`/`PatientDischarged` events.
- **DeviceAggregate** – Provisioning state, credentials, firmware metadata.
- **TelemetryBatch** – Aggregates `VitalRecord`/`AlarmSnapshot`, enforces signing/nonce requirements.
- **Value Objects:** `PatientIdentity`, `DeviceSnapshot`, `MeasurementUnit`, `AlarmThreshold`, `BedLocation`.
- **Domain Events:** `TelemetryQueued`, `AlarmRaised`, `ProvisioningCompleted`, `PatientTransferred`.
- **Repositories:** `IPatientRepository`, `ITelemetryRepository`, `IAlarmRepository`, `IProvisioningRepository`.

> These classes live in `z-monitor/src/domain/**` and must have zero dependencies on Qt or infrastructure code.

### 2.2. Application Layer
- **MonitoringService** – Coordinates vitals ingestion, batching, signing, handoff to infrastructure.
- **AdmissionService** – Executes admit/discharge/transfer use cases against `PatientAggregate`.
- **ProvisioningService** – Handles QR pairing flows, applies credential bundles.
- **SecurityService** – Authenticates users, enforces PIN policies, issues `UserSession`.

> Application services reside in `z-monitor/src/application/**` and depend on domain interfaces + repositories.

### 2.3. Infrastructure Layer
Existing classes in this document (`DeviceSimulator`, `AlarmManager`, `NetworkManager`, `DatabaseManager`, etc.) now map to infrastructure adapters implementing domain contracts. They should be moved under `z-monitor/src/infrastructure/**` (subfolders for persistence, network, provisioning, qt).

### 2.4. Interface Layer
Controllers (`DashboardController`, `PatientController`, etc.) belong in `z-monitor/src/interface/controllers/**`. They must depend only on application services, not on infrastructure classes directly.

For detailed DDD guidance see `doc/28_DOMAIN_DRIVEN_DESIGN.md`.

## 3. Core Service Classes

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
- `sendTelemetry(const TelemetryData& data)`: Transmits data to server via `ITelemetryServer` with digital signature. **CRITICAL:** Automatically includes current patient MRN from `PatientManager` if patient is admitted. Validates that `patientMrn` is present before sending patient data. **TIMING:** Records timing milestones at every stage (batch creation, signing, queuing, transmission, server receipt) to `telemetry_metrics` table for latency analysis.
- `sendSensorData(const SensorData& data)`: Transmits sensor data to server with digital signature. **CRITICAL:** Automatically includes current patient MRN from `PatientManager` if patient is admitted. Validates that `patientMrn` is present before sending patient sensor data. **TIMING:** Records timing metrics similar to `sendTelemetry()`.
- `configure(const QSslConfiguration& sslConfig)`: Configures mTLS settings
- `setServerUrl(const QString& url)`: Updates server URL and reconnects if needed
- `getServerUrl()`: Returns current server URL
- `validateCertificates()`: Validates client certificate (expiration, revocation, device ID match)
- `loadCertificates()`: Loads certificates from secure storage
- `signPayload(const QByteArray& data)`: Creates digital signature for data integrity. **TIMING:** Records signing timestamp for latency analysis.
- `checkCertificateRevocation()`: Checks certificate revocation list (CRL)
- `recordTelemetryMetrics(const TelemetryMetrics& metrics)`: Records telemetry timing metrics to `telemetry_metrics` table for benchmarking and diagnostics

**Patient Data Association:**
- **Automatic Patient MRN Inclusion:** When sending telemetry, `NetworkManager` automatically retrieves current patient MRN from `PatientManager` and includes it in the payload
- **Validation:** Before sending patient data, validates that a patient is admitted (`patientMrn` is not empty)
- **Standby State:** If no patient is admitted, only device health/status data may be sent (with `patientMrn` set to empty/null, clearly marked as non-patient data)
- **Data Integrity:** All vitals, alarms, and sensor data include `patientMrn` to ensure proper patient association on the server

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

**Timing and Performance Tracking:**

The `NetworkManager` tracks comprehensive timing metrics for every telemetry transmission:

```cpp
struct TelemetryMetrics {
    QString batchId;                    // UUID for batch correlation
    QString deviceId;
    QString patientMrn;                 // NULL if no patient admitted
    
    // Timing milestones (Unix milliseconds)
    qint64 dataCreatedAt;               // First data point creation time
    qint64 batchCreatedAt;              // Batch object creation time
    qint64 signedAt;                    // Batch signed time
    qint64 queuedForTxAt;               // Queued for transmission time
    qint64 transmittedAt;               // Sent over network time
    qint64 serverReceivedAt;            // Server receipt time (from response)
    qint64 serverProcessedAt;           // Server processing complete (from response)
    qint64 serverAckAt;                 // Server ACK sent time (from response)
    
    // Batch statistics
    int recordCount;                    // Number of records in batch
    int batchSizeBytes;                 // Payload size in bytes
    int compressedSizeBytes;            // Compressed size (if applicable)
    
    // Status
    QString status;                     // "success", "failed", "timeout", "retrying"
    QString errorMessage;               // Error details if failed
    int retryCount;                     // Number of retry attempts
};
```

**Recording Flow:**
1. **Batch Creation**: Record `dataCreatedAt` (oldest data in batch) and `batchCreatedAt` (QDateTime::currentMSecsSinceEpoch())
2. **Signing**: Record `signedAt` after `signPayload()` completes
3. **Queueing**: Record `queuedForTxAt` when batch is enqueued to network thread
4. **Transmission**: Record `transmittedAt` when `QNetworkReply` sends data
5. **Server Response**: Parse `serverReceivedAt`, `serverProcessedAt`, `serverAckAt` from server JSON response
6. **Persist Metrics**: Call `recordTelemetryMetrics()` to save to `telemetry_metrics` table

**Server Response Format:**
```json
{
    "status": "success",
    "batchId": "550e8400-e29b-41d4-a716-446655440000",
    "receivedAt": 1701385200123,      // Unix ms when server received
    "processedAt": 1701385200145,     // Unix ms when processing complete
    "ackAt": 1701385200150,           // Unix ms when ACK sent
    "recordsProcessed": 42
}
```

**Latency Calculation (Automatic):**
- `batch_creation_latency_ms = batchCreatedAt - dataCreatedAt`
- `signing_latency_ms = signedAt - batchCreatedAt`
- `queue_wait_latency_ms = transmittedAt - queuedForTxAt`
- `network_latency_ms = serverReceivedAt - transmittedAt`
- `server_processing_latency_ms = serverProcessedAt - serverReceivedAt`
- `end_to_end_latency_ms = serverAckAt - dataCreatedAt`

**Use Cases:**
1. **Alarm Latency Compliance**: Verify P95 end-to-end latency < 100ms
2. **Bottleneck Detection**: Identify if delays are in signing, network, or server
3. **Performance Monitoring**: Real-time dashboard showing current latencies
4. **Capacity Planning**: Analyze batch sizes and transmission rates
5. **Diagnostics**: Troubleshoot slow transmissions with detailed timing breakdown

**Signals:**
- `connectionStatusChanged(ConnectionStatus status)`: Emitted when connection status changes
- `telemetrySent(const TelemetryData& data, const ServerResponse& response)`: Emitted when telemetry is successfully sent. **Response includes server timing information.**
- `telemetrySendFailed(const TelemetryData& data, const QString& error)`: Emitted when telemetry send fails
- `certificateExpiring(const QSslCertificate& cert, int daysRemaining)`: Emitted when certificate is expiring soon
- `certificateValidationFailed(const QString& reason)`: Emitted when certificate validation fails
- `telemetryMetricsRecorded(const TelemetryMetrics& metrics)`: Emitted when timing metrics are recorded (for diagnostics UI)

### 2.4. DatabaseManager
**Responsibility:** Manages encrypted SQLite database for local data persistence, backup, and restore operations.

**Key Properties:**
- `database`: QSqlDatabase instance with SQLCipher encryption
- `encryptionKey`: Database encryption key (loaded from secure storage)
- `backupLocation`: Location for database backups

**Key Methods:**
- `open(const QString& path, const QString& password)`: Opens encrypted database
- `saveData(const PatientData& data)`: Stores patient data
- `getHistoricalData(QDateTime start, QDateTime end)`: Retrieves historical data for trends
- `cleanupOldData(int retentionDays)`: Removes data older than retention policy
- `backupDatabase(const QString& backupPath)`: Creates encrypted backup of database
- `restoreDatabase(const QString& backupPath, const QString& password)`: Restores database from backup
- `rotateEncryptionKey(const QString& newKey)`: Rotates database encryption key (re-encrypts all data)
- `verifyDataIntegrity()`: Verifies database integrity using checksums
- `getDatabaseSize()`: Returns current database size in bytes
- `getBackupList()`: Returns list of available backups with timestamps

### 2.5. PatientManager
**Responsibility:** Manages patient context, admission/discharge workflow, and patient lifecycle. Implements ADT (Admission, Discharge, Transfer) workflow aligned with hospital information systems.

**Key Properties:**
- `currentPatient`: Current patient object (includes MRN, name, DOB, sex, allergies, bedLocation)
- `admissionState`: Current admission state (NotAdmitted, Admitted, Discharged)
- `admittedAt`: Timestamp of current admission (QDateTime, NULL if not admitted)
- `deviceLabel`: Static device identifier/asset tag (e.g., "ICU-MON-04")

**Key Methods:**
- `loadPatient(const QString& id)`: Loads patient profile from local database
- `loadPatientById(const QString& id)`: Looks up patient by ID, first checking local database, then using `IPatientLookupService` if not found locally
- `admitPatient(const PatientInfo& info, const QString& admissionSource)`: Admits patient to device (admissionSource: "manual", "barcode", "central_station")
- `dischargePatient(const QString& mrn)`: Discharges patient from device
- `transferPatient(const QString& mrn, const QString& targetDevice)`: Transfers patient to another device
- `getCurrentAdmission()`: Returns current admission information
- `isPatientAdmitted()`: Returns whether a patient is currently admitted
- `getAdmissionHistory(const QString& mrn)`: Returns admission/discharge history for patient
- `setCurrentPatient(const PatientInfo& info)`: Sets the current patient context (deprecated, use admitPatient)
- `getCurrentPatient()`: Returns current patient data
- `clearCurrentPatient()`: Clears the current patient context (deprecated, use dischargePatient)

**Dependencies:**
- `IPatientLookupService`: Interface for looking up patient information from external systems (HIS/EHR)
- `DatabaseManager`: For local patient data storage and retrieval, admission events logging

**Signals:**
- `patientChanged(const Patient& patient)`: Emitted when patient context changes
- `patientAdmitted(const Patient& patient, const QString& admissionSource)`: Emitted when patient is admitted
- `patientDischarged(const QString& mrn)`: Emitted when patient is discharged
- `patientTransferred(const QString& mrn, const QString& targetDevice)`: Emitted when patient is transferred
- `admissionStateChanged(AdmissionState state)`: Emitted when admission state changes
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
- `deviceId`: Unique identifier for the monitoring device used for telemetry transmission (QString)
- `deviceLabel`: Static device identifier/asset tag (e.g., "ICU-MON-04") - fixed technical identifier, separate from patient assignment (QString)
- `measurementUnit`: System preference for metric or imperial units (QString: "metric" or "imperial")
- `serverUrl`: Central server URL for telemetry transmission (QString, default: "https://localhost:8443")
- `useMockServer`: Boolean flag to use mock server for testing/development (bool, default: false)

**Note:** `bedId` setting has been removed. Bed location is now part of the Patient object and managed through the ADT workflow. See `doc/19_ADT_WORKFLOW.md` for details.

**Signals:**
- `settingsChanged()`: Emitted when settings are modified

### 2.7. AuthenticationService
**Responsibility:** Handles user authentication, role-based access control, and session management with brute force protection.

**Key Properties:**
- `currentUser`: Currently logged-in user
- `currentRole`: Current user role (Clinician, Technician)
- `sessionTimeout`: Session timeout in minutes (default: 30, configurable)
- `lastActivityTime`: Timestamp of last user activity for session timeout
- `failedAttempts`: Map of user ID to failed login attempt count
- `lockoutUntil`: Map of user ID to lockout expiration timestamp

**Key Methods:**
- `login(const QString& userId, const QString& pin)`: Authenticates user with PIN, implements brute force protection
- `logout()`: Logs out current user and clears session
- `getCurrentRole()`: Returns current user's role
- `refreshSession()`: Refreshes session timeout on user activity
- `checkSessionTimeout()`: Checks if session has expired, logs out if expired
- `isAccountLocked(const QString& userId)`: Checks if account is locked due to failed attempts
- `getRemainingLockoutTime(const QString& userId)`: Returns remaining lockout time in seconds
- `unlockAccount(const QString& userId)`: Unlocks account (requires administrator role)
- `changePin(const QString& userId, const QString& oldPin, const QString& newPin)`: Changes user PIN
- `validatePinComplexity(const QString& pin)`: Validates PIN meets complexity requirements

**Security Features:**
- **PIN Hashing:** SHA-256 with per-user salt
- **Brute Force Protection:** 5 failed attempts → 15-minute lockout, exponential backoff
- **Session Management:** 30-minute timeout, automatic refresh on activity
- **Audit Logging:** All authentication events logged to `security_audit_log`

**Signals:**
- `userLoggedIn(const QString& user, UserRole role)`: Emitted on successful login
- `userLoggedOut()`: Emitted when user logs out
- `sessionExpired()`: Emitted when session times out
- `accountLocked(const QString& userId)`: Emitted when account is locked
- `loginFailed(const QString& userId, int remainingAttempts)`: Emitted on failed login

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
**Responsibility:** Exposes patient information to the Patient Banner and related UI, and provides patient admission/discharge functionality.

**Properties (Q_PROPERTY):**
- `mrn`: Medical Record Number (QString, read-only)
- `patientName`: Patient name (QString, read-only)
- `patientAge`: Patient age (int, read-only)
- `dateOfBirth`: Patient date of birth (QDate, read-only)
- `sex`: Patient sex (QString, read-only)
- `allergies`: List of allergies (QStringList, read-only)
- `bedLocation`: Current patient's bed location (QString, read-only)
- `admissionState`: Current admission state (enum: NotAdmitted, Admitted, Discharged)
- `isAdmitted`: Whether patient is currently admitted (bool, read-only)
- `admittedAt`: Timestamp of admission (QDateTime, read-only)
- `isLookingUp`: Boolean indicating if a patient lookup is in progress
- `lookupError`: Last error message from patient lookup (empty if no error)

**Q_INVOKABLE Methods:**
- `openAdmissionModal()`: Opens admission modal for admitting a patient
- `admitPatient(const QString& mrn, const QString& admissionSource)`: Admits patient (admissionSource: "manual", "barcode", "central_station")
- `dischargePatient()`: Discharges current patient (requires confirmation)
- `scanBarcode()`: Initiates barcode scanning for admission
- `lookupPatientById(const QString& patientId)`: Initiates patient lookup by ID (asynchronous, used by admission workflow)
- `clearPatient()`: Clears the current patient context (deprecated, use dischargePatient)

**Slots:**
- `onPatientChanged(const Patient& patient)`: Updates UI when patient changes
- `onPatientAdmitted(const Patient& patient, const QString& admissionSource)`: Updates UI when patient is admitted
- `onPatientDischarged(const QString& mrn)`: Updates UI when patient is discharged
- `onAdmissionStateChanged(AdmissionState state)`: Updates UI when admission state changes
- `onPatientLookupStarted(const QString& patientId)`: Updates UI when lookup begins
- `onPatientLookupCompleted(const PatientInfo& info)`: Updates UI when lookup succeeds
- `onPatientLookupFailed(const QString& patientId, const QString& error)`: Updates UI when lookup fails

### 3.5. SettingsController
**Responsibility:** Exposes device settings to the Settings View (excluding network/provisioning settings, which are handled by ProvisioningController).

**Properties (Q_PROPERTY):**
- `allSettings`: Map of all configurable settings
- `deviceId`: Device identifier for telemetry transmission (QString, read/write)
- `deviceLabel`: Static device identifier/asset tag (QString, read-only for most users, read/write for Technician role)
- `measurementUnit`: Measurement unit preference (QString: "metric" or "imperial", read/write)

**Note:** `bedId` property has been removed. Bed location is now managed through Patient object and ADT workflow.

**Q_INVOKABLE Methods:**
- `updateSetting(const QString& key, const QVariant& value)`: Updates a setting from UI
- `resetToDefaults()`: Resets all settings to factory defaults

**Slots:**
- `onSettingsChanged()`: Refreshes UI when settings change

### 3.8. ProvisioningController
**Responsibility:** Exposes device provisioning state and controls to the Network Settings View. Manages QR code-based pairing workflow.

**Properties (Q_PROPERTY):**
- `provisioningState`: Current provisioning state (enum: NotProvisioned, ReadyToPair, Pairing, Configuring, Validating, Provisioned, Error)
- `pairingCode`: Current pairing code (QString, e.g., "ABC-123-XYZ")
- `qrCodeData`: QR code image data for display (QByteArray)
- `expirationTime`: Time remaining until pairing code expires in seconds (int)
- `errorMessage`: Error message if provisioning failed (QString, empty if no error)
- `isProvisioned`: Whether device is successfully provisioned (bool, read-only)
- `serverUrl`: Current server URL (QString, read-only, empty if not provisioned)
- `certificateStatus`: Certificate status information (QString, read-only)
- `connectionStatus`: Current connection status (enum: Connected, Connecting, Disconnected)
- `lastConnected`: Timestamp of last successful connection (QDateTime, read-only)

**Q_INVOKABLE Methods:**
- `enterProvisioningMode()`: Enters provisioning mode (requires Technician role)
- `exitProvisioningMode()`: Exits provisioning mode
- `regenerateQRCode()`: Generates new QR code and pairing code
- `simulateConfiguration()`: Simulates configuration push from Central Station (development/testing only)
- `reprovisionDevice()`: Starts re-provisioning process (requires Technician role and confirmation)
- `cancelProvisioning()`: Cancels current provisioning operation

**Dependencies:**
- `ProvisioningService`: Core provisioning logic and state management
- `AuthenticationService`: For role-based access control (Technician role required)
- `NetworkManager`: For connection testing after provisioning
- `SettingsManager`: For storing provisioned configuration

**Signals:**
- `provisioningStateChanged(ProvisioningState state)`: Emitted when provisioning state changes
- `pairingCodeGenerated(const QString& code)`: Emitted when new pairing code is generated
- `provisioningCompleted()`: Emitted when provisioning completes successfully
- `provisioningFailed(const QString& error)`: Emitted when provisioning fails
- `connectionStatusChanged(ConnectionStatus status)`: Emitted when connection status changes

### 3.9. ProvisioningService
**Responsibility:** Manages device provisioning workflow, including pairing code generation, QR code generation, configuration validation, and connection testing.

**Key Properties:**
- `currentState`: Current provisioning state
- `pairingCode`: Current active pairing code
- `pairingCodeExpiration`: Timestamp when pairing code expires
- `provisionedConfig`: Current provisioned configuration (if any)

**Key Methods:**
- `enterProvisioningMode()`: Enters provisioning mode, generates initial pairing code
- `exitProvisioningMode()`: Exits provisioning mode, clears pairing information
- `generatePairingCode()`: Generates new cryptographically secure pairing code
- `generateQRCode()`: Generates QR code with device information and pairing code
- `validatePairingCode(const QString& code)`: Validates pairing code (not expired, not used)
- `receiveConfiguration(const QByteArray& encryptedPayload)`: Receives and decrypts configuration payload
- `validateConfiguration(const ProvisioningConfig& config)`: Validates configuration signature and structure
- `applyConfiguration(const ProvisioningConfig& config)`: Applies configuration to device (certificates, server URL)
- `testConnection()`: Tests connection with newly provisioned configuration
- `clearProvisioning()`: Clears provisioning state (for factory reset or re-provisioning)

**Security Features:**
- Pairing codes expire after 10 minutes (configurable)
- Pairing codes are one-time use (invalidated after successful pairing)
- Configuration payload encrypted with device's public key
- Configuration signed by Central Station (signature validated)
- All provisioning events logged to `security_audit_log`

**Dependencies:**
- `NetworkManager`: For connection testing
- `SettingsManager`: For storing provisioned configuration
- `DatabaseManager`: For logging provisioning events
- `CertificateManager`: For certificate installation and validation

**Signals:**
- `provisioningStateChanged(ProvisioningState state)`: Emitted when state changes
- `pairingCodeGenerated(const QString& code, const QDateTime& expiresAt)`: Emitted when new code generated
- `configurationReceived(const ProvisioningConfig& config)`: Emitted when configuration received
- `configurationApplied()`: Emitted when configuration successfully applied
- `provisioningCompleted()`: Emitted when provisioning completes
- `provisioningFailed(const QString& error)`: Emitted when provisioning fails

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
