# C++ Class Designs

**Document ID:** DESIGN-009  
**Version:** 2.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

> **⚠️ This document has been reorganized into module-based documents for better maintainability and clarity.**
> 
> **See:** [Class Designs Overview (09_CLASS_DESIGNS_OVERVIEW.md)](./09_CLASS_DESIGNS_OVERVIEW.md) for the high-level module architecture and links to module-specific documents.

---

## 1. Document Reorganization

This document has been **split into module-specific documents** to improve maintainability and make diagrams smaller and easier to understand. The class designs are now organized by **module** (groups of services/components that execute on the same OS thread).

### 1.1 Module-Based Documentation Structure

| Module | Document | Thread | Component Count |
|--------|----------|--------|----------------|
| **Interface Module** | [09a_INTERFACE_MODULE.md](./09a_INTERFACE_MODULE.md) | Main/UI Thread | 30 |
| **Real-Time Processing Module** | [09b_REALTIME_MODULE.md](./09b_REALTIME_MODULE.md) | RT Thread | 12 |
| **Application Services Module** | [09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md) | App Services Thread | 11 |
| **Database Module** | [09d_DATABASE_MODULE.md](./09d_DATABASE_MODULE.md) | Database I/O Thread | 13 |
| **Network Module** | [09e_NETWORK_MODULE.md](./09e_NETWORK_MODULE.md) | Network I/O Thread | 11 |
| **Background Tasks Module** | [09f_BACKGROUND_MODULE.md](./09f_BACKGROUND_MODULE.md) | Background Thread | 9 |

**Total:** 86 components organized into 6 modules

### 1.2 High-Level Module Interaction

[View Module Interaction Diagram (Mermaid)](./09_CLASS_DESIGNS_OVERVIEW.mmd)  
[View Module Interaction Diagram (SVG)](./09_CLASS_DESIGNS_OVERVIEW.svg)

For detailed module interaction patterns, see **[09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md)**.

---

## 2. Module-Specific Documents

All class designs have been organized into module-specific documents for better maintainability and clarity:

### 2.1. Interface Module
**[09a_INTERFACE_MODULE.md](./09a_INTERFACE_MODULE.md)** - UI Controllers, QML Views, and Components

**Components:**
- 11 QML Controllers (DashboardController, AlarmController, WaveformController, PatientController, etc.)
- 7 QML Views (DashboardView, TrendsView, AlarmsView, etc.)
- 12 QML Components (WaveformChart, TrendChart, StatCard, etc.)

**Thread:** Main/UI Thread

---

### 2.2. Real-Time Processing Module
**[09b_REALTIME_MODULE.md](./09b_REALTIME_MODULE.md)** - Sensor Data, Caching, and Alarm Detection

**Components:**
- `WebSocketSensorDataSource` - Sensor data input
- `MonitoringService` - Vitals ingestion coordination
- `VitalsCache` - In-memory cache (3-day capacity)
- `WaveformCache` - Circular buffer (30 seconds)
- Domain Aggregates: `PatientAggregate`, `TelemetryBatch`, `AlarmAggregate`
- Value Objects: `VitalRecord`, `WaveformSample`, `AlarmSnapshot`, `AlarmThreshold`

**Thread:** Real-Time Processing Thread (High Priority)

**Critical Path:** Sensor → Cache → Alarm Detection → UI (< 50ms)

---

### 2.3. Application Services Module
**[09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md)** - Business Logic Orchestration

**Components:**
- `AdmissionService` - Patient admission/discharge/transfer
- `ProvisioningService` - Device provisioning and pairing
- `SecurityService` - Authentication and authorization
- Domain Aggregates: `AdmissionAggregate`, `ProvisioningSession`, `UserSession`, `AuditTrailEntry`
- Value Objects: `PatientIdentity`, `BedLocation`, `PinCredential`, `CredentialBundle`

**Thread:** Application Services Thread

---

### 2.4. Database Module
**[09d_DATABASE_MODULE.md](./09d_DATABASE_MODULE.md)** - Data Persistence and Archival

**Components:**
- `DatabaseManager` - SQLite connection and schema management
- 7 Repository Implementations (SQLitePatientRepository, SQLiteVitalsRepository, etc.)
- `PersistenceScheduler` - Periodic persistence from cache (every 10 min)
- `DataCleanupService` - Daily data cleanup (7-day retention)
- `LogService` - Application logging (file-based)
- `DataArchiveService` - Data archival

**Thread:** Database I/O Thread (Single Writer)

**Non-Critical Path:** Database operations don't block alarm detection

---

### 2.5. Network Module
**[09e_NETWORK_MODULE.md](./09e_NETWORK_MODULE.md)** - Secure Network Communication

**Components:**
- `NetworkTelemetryServer` - Telemetry transmission (HTTPS/mTLS)
- `CertificateManager` - Certificate lifecycle management
- `EncryptionService` - Payload encryption/decryption
- `SignatureService` - Data signing/verification
- `HISPatientLookupAdapter` - Patient lookup from HIS/EHR
- `CentralStationClient` - Provisioning payload receiver
- Domain Aggregates: `DeviceAggregate`
- Value Objects: `DeviceSnapshot`, `MeasurementUnit`

**Thread:** Network I/O Thread

---

### 2.6. Background Tasks Module
**[09f_BACKGROUND_MODULE.md](./09f_BACKGROUND_MODULE.md)** - System Maintenance Tasks

**Components:**
- `FirmwareUpdateService` - Firmware update management
- `BackupService` - Database backup and restore
- `SettingsManager` - Configuration storage
- `QRCodeGenerator` - QR code generation
- `SecureStorage` - Secure key storage
- `HealthMonitor` - System health monitoring
- `ClockSyncService` - NTP time synchronization
- `FirmwareManager` - Firmware file management
- `WatchdogService` - Thread monitoring and crash detection

**Thread:** Background Thread (Low Priority)

---

## 3. Migration Status

**Status:** Class designs are being migrated to module-specific documents. The original detailed class designs remain in this document for reference during migration.

**Classes Already Migrated:**
- ✅ All QML Controllers → [09a_INTERFACE_MODULE.md](./09a_INTERFACE_MODULE.md)
- ✅ MonitoringService, VitalsCache, WaveformCache, AlarmAggregate → [09b_REALTIME_MODULE.md](./09b_REALTIME_MODULE.md)
- ✅ AdmissionService, ProvisioningService, SecurityService → [09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md)
- ✅ DatabaseManager, Repositories, PersistenceScheduler → [09d_DATABASE_MODULE.md](./09d_DATABASE_MODULE.md)
- ✅ NetworkTelemetryServer, CertificateManager, EncryptionService → [09e_NETWORK_MODULE.md](./09e_NETWORK_MODULE.md)
- ✅ SettingsManager, HealthMonitor, WatchdogService → [09f_BACKGROUND_MODULE.md](./09f_BACKGROUND_MODULE.md)

**Classes Pending Migration:**
- ⚠️ `AlarmManager` - Should be reviewed and integrated into [09b_REALTIME_MODULE.md](./09b_REALTIME_MODULE.md) (alarm detection logic)
- ⚠️ `NetworkManager` - Should be reviewed and integrated into [09e_NETWORK_MODULE.md](./09e_NETWORK_MODULE.md) (network connectivity)
- ⚠️ `PatientManager` - Should be reviewed and integrated into [09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md) (patient context management)
- ⚠️ `KeyManager` - Should be reviewed and integrated into [09f_BACKGROUND_MODULE.md](./09f_BACKGROUND_MODULE.md) (key management)
- ⚠️ `AuthenticationService` - Should be reviewed and integrated into [09c_APPLICATION_SERVICES_MODULE.md](./09c_APPLICATION_SERVICES_MODULE.md) (may be same as SecurityService)

> **Note:** These classes will be migrated to their appropriate module documents in a future update. For now, see the detailed designs in the sections below.

---

## 4. Legacy Content (Deprecated)

The following sections contain detailed class designs that are being migrated to module-specific documents. They are kept here for reference during migration.


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

For detailed DDD guidance see `29_SYSTEM_COMPONENTS.md`.

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
**Responsibility:** Monitors patient data, detects alarm conditions, manages alarm states, enforces alarm escalation rules, and maintains alarm history. Implements IEC 60601-1-8 compliant alarm system with priority-based processing and audio-visual indicators.

**Compliance:** IEC 60601-1-8 (Alarm Systems), IEC 62304 (Medical Device Software)

**Key Properties:**
- `activeAlarms`: `QList<AlarmAggregate*>` - Currently active alarms (real-time list)
- `alarmHistory`: Historical record of all alarms (persisted to database)
- `silencedAlarms`: `QMap<QString, SilenceContext>` - Alarms currently silenced with expiry tracking
- `alarmThresholds`: `QMap<QString, AlarmThreshold>` - Per-patient alarm thresholds (loaded from settings)
- `escalationTimer`: `QTimer*` - Timer for alarm escalation checks
- `audioController`: `AlarmAudioController*` - Controls audio alarm patterns (IEC 60601-1-8 compliant)
- `maxSilenceDuration`: `int` - Maximum silence duration (600 seconds / 10 minutes per IEC 60601-1-8)

**Key Methods:**

*Core Alarm Processing:*
- `processVitalSigns(const VitalRecord& vital)`: Analyzes incoming vitals for alarm conditions
  - Checks each vital sign against configured thresholds
  - Determines alarm priority (HIGH, MEDIUM, LOW) based on severity
  - Creates new alarm or updates existing alarm state
  - Returns list of triggered alarms
  
- `evaluateAlarmCondition(const QString& vitalType, double value, const AlarmThreshold& threshold)`: Evaluates specific alarm condition
  - Compares vital sign value against threshold
  - Implements hysteresis to prevent alarm flutter (±5% tolerance)
  - Returns AlarmCondition (NO_ALARM, LOW_PRIORITY, MEDIUM_PRIORITY, HIGH_PRIORITY)
  
*Alarm State Management:*
- `acknowledgeAlarm(const QString& alarmId, const QString& userId)`: Marks an alarm as acknowledged
  - Validates alarm exists and is active
  - Records acknowledgment timestamp and user ID
  - Pauses audio (for acknowledged alarms)
  - Logs acknowledgment to audit log
  - Emits `alarmAcknowledged(alarmId)` signal
  - **Returns:** `bool` (true if acknowledged successfully)
  
- `silenceAlarm(const QString& alarmId, int durationSeconds, const QString& userId)`: Temporarily silences an alarm
  - **Validation:** Enforces maximum silence duration (600 seconds / 10 minutes per IEC 60601-1-8)
  - **Validation:** Cannot silence HIGH priority alarms for > 2 minutes (safety requirement)
  - Records silence timestamp, duration, and user ID
  - Starts silence expiry timer
  - Logs silence event to audit log with justification
  - **Returns:** `Result<SilenceContext>` (success or error if invalid)
  
- `unsilenceAlarm(const QString& alarmId, const QString& userId)`: Manually unsilences an alarm
  - Removes from silenced alarms list
  - Restores audio if alarm still active
  - Logs unsilence event
  - **Returns:** `bool` (true if unsilenced successfully)
  
- `resolveAlarm(const QString& alarmId)`: Resolves an alarm (condition no longer exists)
  - Marks alarm as RESOLVED
  - Records resolution timestamp
  - Stops audio and visual indicators
  - Moves to alarm history
  - Emits `alarmResolved(alarmId)` signal
  
*Alarm Escalation:*
- `checkEscalation()`: Checks for alarms requiring escalation (called every 30 seconds)
  - **HIGH Priority Alarms:** Escalate after 60 seconds if not acknowledged
  - **MEDIUM Priority Alarms:** Escalate after 120 seconds if not acknowledged
  - **LOW Priority Alarms:** No escalation (advisory only)
  - Escalation actions:
    - Increase audio volume by 20%
    - Increase visual indicator intensity (flash rate doubles)
    - Send escalation notification to central server
    - Log escalation event
  - Emits `alarmEscalated(alarmId, escalationLevel)` signal
  
- `escalateAlarm(const QString& alarmId)`: Escalates a specific alarm
  - Increments escalation level (1 → 2 → 3, max 3 levels)
  - Updates audio/visual indicators
  - Notifies central server (critical alarms)
  - Logs escalation with timestamp and reason
  - **Returns:** `int` (new escalation level)
  
*Alarm History and Persistence:*
- `getAlarmHistory(const QString& patientMrn, const QDateTime& startTime, const QDateTime& endTime)`: Retrieves alarm history
  - Queries `alarms` table for specified patient and time range
  - Returns `QList<AlarmEvent>` sorted by timestamp (descending)
  
- `saveAlarmToHistory(const AlarmAggregate& alarm)`: Persists alarm to database
  - Saves alarm data to `alarms` table
  - Includes all state transitions (triggered, acknowledged, resolved)
  - Includes threshold value that was exceeded
  - **Critical:** Records `threshold` at time of alarm (for audit/compliance)
  
- `clearResolvedAlarms()`: Removes resolved alarms from active list
  - Moves resolved alarms to history
  - Keeps active alarms in memory for fast access
  - Called periodically (every 60 seconds)
  
*Threshold Management:*
- `updateThresholds(const QString& patientMrn, const QMap<QString, AlarmThreshold>& thresholds)`: Updates alarm thresholds for patient
  - Validates threshold values (min/max ranges)
  - Persists to **settings table** (`alarm_thresholds_{patientMrn}` key) - stores *currently configured* thresholds
  - Logs threshold change to audit log (user ID, timestamp)
  - Triggers re-evaluation of active alarms with new thresholds
  - Emits `thresholdsChanged(patientMrn)` signal
  - **Note:** When alarm triggers, threshold value is also saved to **alarms table** (`threshold_value` column) as historical snapshot
  
- `getThresholds(const QString& patientMrn)`: Retrieves current alarm thresholds
  - Loads from **settings table** (`alarm_thresholds_{patientMrn}` key)
  - Returns `QMap<QString, AlarmThreshold>` with all configured thresholds
  - Falls back to default thresholds if none configured
  - **Storage Clarification:** Settings table stores *active configuration*, alarms table stores *historical snapshots*
  
- `loadDefaultThresholds()`: Loads default alarm thresholds
  - Default HR: 60-120 bpm (adult)
  - Default SpO2: 90-100%
  - Default RR: 12-20 rpm (adult)
  - Returns `QMap<QString, AlarmThreshold>`
  
*Audio Control:*
- `playAlarmAudio(const QString& priority)`: Plays IEC 60601-1-8 compliant audio pattern
  - **HIGH:** 3 pulses, 800 Hz, loud volume (IEC C-priority pattern)
  - **MEDIUM:** 2 pulses, 600 Hz, medium volume (IEC B-priority pattern)
  - **LOW:** 1 pulse, 400 Hz, low volume (IEC A-priority pattern)
  - Delegates to `AlarmAudioController`
  
- `stopAlarmAudio(const QString& alarmId)`: Stops audio for specific alarm
  - Stops audio playback
  - Updates alarm audio state
  
- `muteAllAudio(int durationSeconds, const QString& userId)`: Mutes all alarm audio temporarily
  - **Validation:** Maximum mute duration: 120 seconds (2 minutes) for safety
  - **Validation:** Requires administrator role for mute > 60 seconds
  - Logs mute event to audit log
  - Starts unmute timer
  - **WARNING:** Visual indicators remain active (cannot be muted)
  
*Silence Duration Enforcement (IEC 60601-1-8 Compliance):*
- `validateSilenceDuration(const QString& priority, int requestedDuration)`: Validates silence duration
  - **HIGH Priority:** Max 120 seconds (2 minutes)
  - **MEDIUM Priority:** Max 300 seconds (5 minutes)
  - **LOW Priority:** Max 600 seconds (10 minutes)
  - Returns validated duration (capped if exceeds max)
  
- `onSilenceExpired(const QString& alarmId)`: Called when silence timer expires
  - Removes alarm from silenced list
  - Restores audio if alarm still active
  - Logs silence expiration
  - Emits `alarmUnsilenced(alarmId)` signal

**Signals:**
- `alarmTriggered(const AlarmEvent& alarm)`: Emitted when new alarm is detected
- `alarmStateChanged(const QList<AlarmEvent>& activeAlarms)`: Emitted when alarm state changes
- `alarmAcknowledged(const QString& alarmId, const QString& userId)`: Emitted when alarm acknowledged
- `alarmSilenced(const QString& alarmId, int duration, const QString& userId)`: Emitted when alarm silenced
- `alarmUnsilenced(const QString& alarmId)`: Emitted when silence expires or manually unsilenced
- `alarmResolved(const QString& alarmId)`: Emitted when alarm condition resolves
- `alarmEscalated(const QString& alarmId, int escalationLevel)`: Emitted when alarm escalates
- `thresholdsChanged(const QString& patientMrn)`: Emitted when thresholds updated
- `criticalAlarmActive(bool active)`: Emitted when critical alarm state changes (for UI priority)

**Data Structures:**

```cpp
/**
 * @struct AlarmThreshold
 * @brief Threshold configuration for alarm detection.
 */
struct AlarmThreshold {
    QString vitalType;          ///< "HR", "SPO2", "RR", etc.
    double lowLimit;            ///< Low threshold (e.g., 60 for HR)
    double highLimit;           ///< High threshold (e.g., 120 for HR)
    double hysteresis;          ///< Hysteresis range (±5% typical)
    QString priority;           ///< "HIGH", "MEDIUM", "LOW"
    bool enabled;               ///< true if alarm enabled
};

/**
 * @struct SilenceContext
 * @brief Context for silenced alarm.
 */
struct SilenceContext {
    QString alarmId;            ///< Alarm being silenced
    QDateTime silencedAt;       ///< When silenced
    int durationSeconds;        ///< Silence duration (max 600s)
    QDateTime expiresAt;        ///< Silence expiry time
    QString userId;             ///< User who silenced alarm
    QString reason;             ///< Reason for silence (optional)
    QTimer* expiryTimer;        ///< Timer for auto-unsilence
};

/**
 * @struct AlarmEvent
 * @brief Complete alarm event data (for signals and persistence).
 */
struct AlarmEvent {
    QString alarmId;            ///< UUID
    QString deviceId;           ///< Device identifier
    QString patientMrn;         ///< Patient MRN
    QDateTime timestamp;        ///< Trigger time
    QString priority;           ///< HIGH, MEDIUM, LOW
    QString alarmType;          ///< HR_HIGH, SPO2_LOW, etc.
    double value;               ///< Vital sign value
    double threshold;           ///< Threshold exceeded
    QString status;             ///< ACTIVE, ACKNOWLEDGED, SILENCED, RESOLVED
    QString acknowledgedBy;     ///< User ID (if acknowledged)
    QDateTime acknowledgedAt;   ///< Acknowledgment time
    int escalationLevel;        ///< 0, 1, 2, 3
    
    // Serialize to JSON for transmission
    QJsonObject toJson() const;
};
```

**Alarm Processing Flow:**

1. **Detection:** `processVitalSigns()` called with new vital signs
2. **Evaluation:** `evaluateAlarmCondition()` compares value against thresholds
3. **Trigger:** If condition met, create `AlarmEvent` with priority
4. **Audio/Visual:** Play audio pattern and update visual indicators
5. **Persistence:** Save alarm to `alarms` table
6. **Notification:** Emit `alarmTriggered()` signal, send to central server
7. **Escalation Check:** If not acknowledged within time limit, escalate
8. **Resolution:** When condition clears, resolve alarm and stop indicators

**IEC 60601-1-8 Compliance:**

- ✅ **Alarm Priorities:** HIGH (C), MEDIUM (B), LOW (A) audio patterns
- ✅ **Audio Patterns:** Compliant pulse patterns (1-3 pulses at specified frequencies)
- ✅ **Silence Limits:** Maximum 10 minutes (600 seconds) for LOW priority
- ✅ **Visual Indicators:** Cannot be silenced (safety requirement)
- ✅ **Escalation:** Automatic escalation for unacknowledged HIGH alarms
- ✅ **Acknowledgment:** Distinct state (audio paused, visual remains)
- ✅ **Latency:** < 50ms from detection to audio trigger (enforced by high-priority thread)

**Performance:**

- **Alarm Detection Latency:** < 50ms (real-time processing thread)
- **Threshold Lookup:** O(1) (QMap with vital type key)
- **Escalation Check:** Every 30 seconds (low CPU overhead)
- **Active Alarms:** In-memory list (fast access)
- **Alarm History:** Database query (optimized with indices)

**Dependencies:**
- `IAlarmRepository`: Persistence of alarm events and history
- `SettingsManager`: Alarm threshold configuration
- `AudioController`: IEC 60601-1-8 compliant audio playback
- `PatientManager`: Current patient context
- `ITelemetryServer`: Alarm event transmission to central server
- `SecurityService`: User authentication for alarm actions (acknowledge, silence)

**Security:**
- **Audit Logging:** All alarm actions logged (acknowledge, silence, threshold changes)
- **User Authentication:** Alarm actions require active user session
- **Threshold Changes:** Logged with user ID and timestamp (IEC 62304 compliance)

**Testing:**
- **Unit Tests:** Threshold evaluation, escalation logic, silence enforcement
- **Integration Tests:** End-to-end alarm workflow (detect → acknowledge → resolve)
- **Regulatory Tests:** IEC 60601-1-8 compliance verification (audio patterns, timing, limits)

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

*Core Database Operations:*
- `open(const QString& path, const QString& password)`: Opens encrypted database
- `saveData(const PatientData& data)`: Stores patient data
- `getHistoricalData(QDateTime start, QDateTime end)`: Retrieves historical data for trends
- `cleanupOldData(int retentionDays)`: Removes data older than retention policy
- `backupDatabase(const QString& backupPath)`: Creates encrypted backup of database
- `restoreDatabase(const QString& backupPath, const QString& password)`: Restores database from backup
- `rotateEncryptionKey(const QString& newKey)`: Rotates database encryption key (re-encrypts all data)
- `verifyDataIntegrity()`: Verifies database integrity using checksums
- `getBackupList()`: Returns list of available backups with timestamps

*Database Size Monitoring (CON-SW-003):*
- `getDatabaseSize()`: Returns current database size in bytes
  - Queries file size using QFileInfo
  - Returns size in bytes (e.g., 123456789)
  - **Performance:** Cached (refreshed every 60 seconds)
  
- `getDatabaseSizeMB()`: Returns database size in megabytes
  - Convenience method: `getDatabaseSize() / (1024 * 1024)`
  - Returns double (e.g., 117.5 MB)
  
- `checkDatabaseSizeLimit()`: Checks if database exceeds size limits
  - **Limit:** 500 MB (per CON-SW-003)
  - **Warning Threshold:** 400 MB (80% of limit)
  - **Critical Threshold:** 450 MB (90% of limit)
  - Returns `DatabaseSizeStatus` enum (OK, WARNING, CRITICAL, EXCEEDED)
  - Emits `databaseSizeWarning(sizeMB)` signal when threshold exceeded
  
- `getTableSizes()`: Returns size breakdown by table
  - Queries `SELECT name, SUM(pgsize) as size FROM dbstat GROUP BY name`
  - Returns `QMap<QString, qint64>` (table name → size in bytes)
  - Useful for identifying which tables consuming most space
  - Example output: `{"vitals": 350MB, "alarms": 50MB, "security_audit_log": 20MB}`
  
- `estimateDaysUntilFull()`: Estimates days until database reaches limit
  - Calculates average daily growth rate (last 7 days)
  - Projects when database will reach 500 MB limit
  - Returns `int` (days remaining, -1 if not growing)
  - Formula: `(500MB - currentSize) / avgDailyGrowth`
  
- `monitorDatabaseSize()`: Periodic size monitoring (called by timer)
  - Checks size every 60 seconds
  - If size > 400 MB (80%), emits `databaseSizeWarning(sizeMB, daysUntilFull)`
  - If size > 450 MB (90%), emits `databaseSizeCritical(sizeMB, daysUntilFull)`
  - If size > 500 MB (100%), triggers emergency cleanup + emits `databaseSizeExceeded(sizeMB)`
  - Logs size check to debug log (every hour, not every check)

**Signals:**
- `databaseSizeWarning(double sizeMB, int daysUntilFull)`: Emitted when database exceeds 80% of limit (400 MB)
- `databaseSizeCritical(double sizeMB, int daysUntilFull)`: Emitted when database exceeds 90% of limit (450 MB)
- `databaseSizeExceeded(double sizeMB)`: Emitted when database exceeds 100% of limit (500 MB)
- `databaseSizeNormal()`: Emitted when database size returns to normal (< 80%)

**Database Size Management:**

```cpp
/**
 * @enum DatabaseSizeStatus
 * @brief Database size status relative to limits.
 */
enum class DatabaseSizeStatus {
    OK,             ///< < 400 MB (< 80%)
    WARNING,        ///< 400-450 MB (80-90%)
    CRITICAL,       ///< 450-500 MB (90-100%)
    EXCEEDED        ///< > 500 MB (> 100%)
};

/**
 * @brief Check and handle database size limits.
 */
void DatabaseManager::monitorDatabaseSize() {
    double sizeMB = getDatabaseSizeMB();
    DatabaseSizeStatus status = checkDatabaseSizeLimit();
    
    switch (status) {
        case DatabaseSizeStatus::OK:
            // Normal operation
            break;
            
        case DatabaseSizeStatus::WARNING:
            {
                int daysLeft = estimateDaysUntilFull();
                qWarning() << "Database size warning:" << sizeMB << "MB (80% of limit)";
                qWarning() << "Estimated days until full:" << daysLeft;
                emit databaseSizeWarning(sizeMB, daysLeft);
                
                // Suggest cleanup
                showNotification("Database size: " + QString::number(sizeMB, 'f', 1) + " MB. " +
                                "Consider archiving old data.");
            }
            break;
            
        case DatabaseSizeStatus::CRITICAL:
            {
                int daysLeft = estimateDaysUntilFull();
                qCritical() << "Database size critical:" << sizeMB << "MB (90% of limit)";
                qCritical() << "Estimated days until full:" << daysLeft;
                emit databaseSizeCritical(sizeMB, daysLeft);
                
                // Warn user
                showCriticalAlert("Database nearly full: " + QString::number(sizeMB, 'f', 1) + " MB. " +
                                 "Archive data immediately.");
            }
            break;
            
        case DatabaseSizeStatus::EXCEEDED:
            qCritical() << "Database size EXCEEDED:" << sizeMB << "MB (over 500 MB limit)!";
            emit databaseSizeExceeded(sizeMB);
            
            // Emergency cleanup
            performEmergencyCleanup();
            
            // Alert user
            showCriticalAlert("Database full (" + QString::number(sizeMB, 'f', 1) + " MB). " +
                             "Emergency cleanup performed. Archive data now.");
            break;
    }
}

/**
 * @brief Perform emergency cleanup when database exceeds limit.
 */
void DatabaseManager::performEmergencyCleanup() {
    qWarning() << "Performing emergency database cleanup...";
    
    // 1. Delete vitals older than 3 days (instead of 7)
    cleanupOldData(3);
    
    // 2. Archive resolved alarms older than 7 days
    archiveOldAlarms(7);
    
    // 3. Compact database (SQLite VACUUM)
    vacuum();
    
    qInfo() << "Emergency cleanup complete. New size:" << getDatabaseSizeMB() << "MB";
}
```

**Size Monitoring Strategy:**
- **Periodic Check:** Every 60 seconds (low overhead)
- **Cached Size:** File size cached for 60 seconds (avoid excessive file system queries)
- **Growth Rate:** Calculated daily (average of last 7 days)
- **Alert Threshold:** 400 MB (80% of 500 MB limit)
- **Emergency Cleanup:** Triggered automatically at 500 MB

**Performance Impact:**
- **Size Check:** < 1ms (file stat operation, cached)
- **Table Size Breakdown:** ~50ms (dbstat query, not run frequently)
- **Growth Rate Calculation:** ~10ms (query last 7 days of size samples)

### 2.4b. KeyManager
**Responsibility:** Manages encryption keys for database, network communication, and data signing. Implements secure key storage, rotation, and lifecycle management to ensure data confidentiality and integrity.

**Compliance:** REQ-SEC-ENC-004 (Key Management), NIST SP 800-57 (Key Management), IEC 62443 (Cybersecurity)

**Key Properties:**
- `databaseEncryptionKey`: AES-256 key for SQLCipher database encryption (32 bytes)
- `signingKey`: Private key for telemetry data signing (RSA-2048 or ECDSA P-256)
- `keyStore`: Secure storage backend (HSM, TPM, or encrypted keychain)
- `keyRotationSchedule`: Key rotation schedule (database: yearly, signing: 2 years, mTLS cert: 2 years)
- `keyMetadata`: `QMap<QString, KeyMetadata>` - Key creation dates, rotation history, usage counters

**Key Methods:**

*Key Initialization and Loading:*
- `initialize()`: Initializes KeyManager on application startup
  - Loads encryption keys from secure storage
  - Validates key integrity (checksum/hash verification)
  - Checks key expiration and rotation schedule
  - Returns `Result<void>` (success or error)
  
- `loadDatabaseKey()`: Loads database encryption key
  - Retrieves from secure storage (HSM, TPM, or encrypted keychain)
  - Validates key format (32 bytes for AES-256)
  - Decrypts key using master key (platform-specific)
  - Returns `Result<QByteArray>` (key or error)
  
- `loadSigningKey()`: Loads signing key for telemetry data
  - Retrieves private key from secure storage
  - Validates key format (PEM or DER)
  - Returns `Result<QSslKey>` (key or error)
  
*Key Generation:*
- `generateDatabaseKey()`: Generates new database encryption key
  - Uses cryptographically secure random number generator (QRandomGenerator::system())
  - Generates 256-bit AES key (32 bytes)
  - Stores in secure storage with metadata
  - Logs key generation event to audit log
  - Returns `Result<QByteArray>` (key or error)
  
- `generateSigningKeyPair()`: Generates new signing key pair
  - Generates RSA-2048 or ECDSA P-256 key pair
  - Stores private key in secure storage
  - Returns `Result<KeyPair>` (public/private key pair or error)
  
- `generateMasterKey()`: Generates device master key (first-time setup)
  - Generates 256-bit master key
  - Derived from device-specific hardware identifiers (MAC address, serial number, TPM)
  - Used to encrypt other keys in storage
  - **Never leaves device** (device-bound encryption)
  - Returns `Result<QByteArray>` (key or error)
  
*Key Rotation:*
- `rotateDatabaseKey()`: Rotates database encryption key
  - Generates new AES-256 key
  - Re-encrypts entire database with new key (using SQLCipher PRAGMA rekey)
  - Archives old key (for emergency recovery, 30-day retention)
  - Updates key metadata (rotation date, version)
  - Logs rotation event to audit log
  - **Downtime:** ~5-10 seconds for typical database size (< 500 MB)
  - Returns `Result<void>` (success or error)
  
- `rotateSigningKey()`: Rotates signing key pair
  - Generates new key pair
  - Archives old private key (30-day retention for signature verification)
  - Updates all services using signing key
  - Notifies central server of new public key
  - Logs rotation event
  - Returns `Result<KeyPair>` (new key pair or error)
  
- `checkRotationSchedule()`: Checks if keys need rotation
  - Database key: Rotate annually (365 days)
  - Signing key: Rotate every 2 years (730 days)
  - mTLS certificate: Managed by CertificateManager (2 years typical)
  - Returns `QList<QString>` (list of keys needing rotation)
  
*Secure Key Storage:*
- `storeKey(const QString& keyId, const QByteArray& key, const KeyMetadata& metadata)`: Stores key securely
  - Encrypts key with master key (AES-256-GCM)
  - Stores in platform-specific secure storage:
    - **Linux:** Encrypted file with restricted permissions (0600) + TPM if available
    - **macOS:** Keychain Access (Security framework)
    - **Windows:** DPAPI (Data Protection API) or TPM
    - **Embedded:** Hardware Security Module (HSM) or TEE (Trusted Execution Environment)
  - Stores metadata (creation date, rotation schedule, usage count)
  - Returns `Result<void>` (success or error)
  
- `retrieveKey(const QString& keyId)`: Retrieves key from secure storage
  - Loads encrypted key from storage
  - Decrypts with master key
  - Validates integrity (HMAC or checksum)
  - Returns `Result<QByteArray>` (key or error)
  
- `deleteKey(const QString& keyId)`: Securely deletes key
  - Overwrites key data with random bytes (3 passes minimum)
  - Removes from secure storage
  - Logs deletion event
  - Returns `Result<void>` (success or error)
  
*Key Validation and Integrity:*
- `validateKeyIntegrity(const QString& keyId)`: Validates key integrity
  - Computes checksum (SHA-256)
  - Compares with stored checksum in metadata
  - Returns `Result<bool>` (true if valid, false if corrupted)
  
- `checkKeyExpiration(const QString& keyId)`: Checks if key is expired
  - Compares current date with expiration date in metadata
  - Returns expiration status (VALID, EXPIRING_SOON, EXPIRED)
  
*Key Derivation (for multi-key scenarios):*
- `deriveKey(const QString& purpose, const QByteArray& masterKey)`: Derives purpose-specific key
  - Uses HKDF (HMAC-based Key Derivation Function) per NIST SP 800-108
  - Derives keys for specific purposes (e.g., "database", "signing", "backup")
  - Ensures different keys for different purposes (defense in depth)
  - Returns `QByteArray` (derived key)
  
*Emergency Recovery:*
- `recoverFromBackup(const QString& backupPath)`: Recovers keys from backup
  - Loads encrypted key backup
  - Decrypts with recovery passphrase (administrator-provided)
  - Restores keys to secure storage
  - Logs recovery event
  - Returns `Result<void>` (success or error)
  
- `exportKeyBackup(const QString& backupPath, const QString& recoveryPassphrase)`: Exports encrypted key backup
  - Encrypts all keys with recovery passphrase (user-provided, high-entropy)
  - Stores in encrypted backup file
  - **Security:** Backup file uses AES-256-GCM + Argon2 key derivation
  - Returns `Result<void>` (success or error)

**Signals:**
- `keyRotationRequired(const QString& keyId)`: Emitted when key needs rotation
- `keyRotationCompleted(const QString& keyId)`: Emitted when key rotation succeeds
- `keyRotationFailed(const QString& keyId, const QString& error)`: Emitted when rotation fails
- `keyIntegrityCheckFailed(const QString& keyId)`: Emitted when key corruption detected
- `keyExpiring(const QString& keyId, int daysUntilExpiry)`: Emitted when key approaching expiry

**Data Structures:**

```cpp
/**
 * @struct KeyMetadata
 * @brief Metadata for encryption keys.
 */
struct KeyMetadata {
    QString keyId;              ///< Unique key identifier (e.g., "db_key_v2")
    QString purpose;            ///< "database", "signing", "backup", etc.
    QString algorithm;          ///< "AES-256", "RSA-2048", "ECDSA-P256"
    int keySizeBytes;           ///< Key size in bytes (32 for AES-256)
    QDateTime createdAt;        ///< Key creation timestamp
    QDateTime expiresAt;        ///< Key expiration date
    QDateTime lastRotated;      ///< Last rotation timestamp
    int rotationCount;          ///< Number of times key has been rotated
    QString checksum;           ///< SHA-256 checksum for integrity
    int usageCount;             ///< Number of times key has been used (for audit)
};

/**
 * @struct KeyPair
 * @brief Public/private key pair for asymmetric cryptography.
 */
struct KeyPair {
    QSslKey privateKey;         ///< Private key (keep secure!)
    QSslKey publicKey;          ///< Public key (can be shared)
    QString algorithm;          ///< "RSA-2048", "ECDSA-P256"
    QDateTime createdAt;        ///< Creation timestamp
    QDateTime expiresAt;        ///< Expiration date
};

/**
 * @enum KeyExpirationStatus
 * @brief Key expiration status.
 */
enum class KeyExpirationStatus {
    VALID,                      ///< Key is valid
    EXPIRING_SOON,              ///< Key expires within 30 days
    EXPIRED                     ///< Key has expired
};
```

**Key Rotation Schedule:**

| Key Type | Rotation Frequency | Automated? | Trigger |
|----------|-------------------|------------|---------|
| Database Encryption Key | Annually (365 days) | Yes | Scheduled task (nightly check) |
| Signing Key | Every 2 years (730 days) | Yes | Scheduled task (nightly check) |
| mTLS Certificate | Every 2 years (per policy) | No | CertificateManager (manual or provisioning) |
| Master Key | Never (device-bound) | No | Device replacement only |

**Secure Storage Backend:**

The KeyManager supports multiple secure storage backends:

1. **Hardware Security Module (HSM):** Best security, dedicated hardware (medical device recommended)
2. **Trusted Platform Module (TPM):** Good security, available on modern PCs/embedded systems
3. **Platform Keychain:** 
   - macOS: Keychain Access (Security framework)
   - Windows: DPAPI (Data Protection API)
   - Linux: Secret Service API (libsecret) or encrypted file with TPM
4. **Encrypted File (Fallback):** Encrypted with device master key, restricted permissions (0600)

**Key Lifecycle:**

```
1. Generation → 2. Storage → 3. Usage → 4. Rotation → 5. Archival → 6. Deletion
    ↓              ↓            ↓           ↓            ↓             ↓
[generateKey] [storeKey]   [retrieveKey] [rotateKey] [archiveOldKey] [deleteKey]
                                            ↑
                                            └─ Triggered by schedule or manual
```

**Performance:**

- **Key Retrieval:** < 10ms (cached in memory after first load)
- **Key Rotation (Database):** ~5-10 seconds for 500 MB database
- **Key Rotation (Signing):** < 1 second (asymmetric key generation)
- **Key Validation:** < 5ms (checksum verification)

**Security Best Practices:**

- ✅ **Never log keys** - Keys never appear in logs or debug output
- ✅ **Memory protection** - Keys stored in secure memory (mlock/VirtualLock if available)
- ✅ **Zero on exit** - Keys overwritten with zeros before application exit
- ✅ **Minimal exposure** - Keys loaded only when needed, cleared after use
- ✅ **Separation of duties** - Different keys for different purposes
- ✅ **Defense in depth** - Multiple layers (master key → storage key → data)
- ✅ **Audit logging** - All key operations logged (generation, rotation, access)

**Dependencies:**
- `QCryptographicHash`: SHA-256 checksums
- `QRandomGenerator`: Cryptographically secure random numbers
- `QSslKey`: Asymmetric key management
- `Platform-specific APIs`: Secure storage (Keychain, DPAPI, TPM)
- `DatabaseManager`: Database key rotation integration
- `NetworkManager`: Signing key integration
- `LogService`: Audit logging

**Compliance:**
- **NIST SP 800-57:** Key management lifecycle and rotation schedules
- **NIST SP 800-108:** Key derivation functions (HKDF)
- **IEC 62443:** Cybersecurity for industrial automation (key management requirements)
- **FIPS 140-2:** Cryptographic module security (if using HSM)
- **IEC 62304:** Medical device software lifecycle (key management documentation)

**Testing:**
- **Unit Tests:** Key generation, rotation, storage, retrieval, validation
- **Integration Tests:** End-to-end key lifecycle (generate → store → rotate → delete)
- **Security Tests:** Key exposure checks, memory analysis, secure deletion verification
- **Performance Tests:** Rotation performance, retrieval latency

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

**Note:** `bedId` setting has been removed. Bed location is now part of the Patient object and managed through the ADT workflow. See [19_ADT_WORKFLOW.md](./19_ADT_WORKFLOW.md) for details.

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

### 2.8. LogService (Application Logs - File-Based)
**Responsibility:** Provides centralized logging mechanism for application events (errors, warnings, info, debug messages). Writes to rotated text/JSON files.

**Thread:** Background Log Thread (asynchronous, non-blocking)

**Key Properties:**
- `m_logLevel`: Minimum log level (Trace, Debug, Info, Warning, Error, Critical, Fatal)
- `m_logFormat`: Log format ("human" or "json")
- `m_logFilePath`: Path to log file (default: `logs/z-monitor.log`)
- `m_maxLogSize`: Maximum log file size in bytes (default: 10 MB)
- `m_maxLogFiles`: Maximum number of rotated files (default: 7)
- `m_recentLogs`: In-memory buffer for Diagnostics View (last 1000 entries)
- `m_logQueue`: Lock-free queue for asynchronous logging
- `m_logThread`: Background thread for processing log queue

**Key Methods:**
- `log(LogLevel level, const QString& message, const QVariantMap& context = {})`: Logs message with level and context
- `trace(const QString& message, const QVariantMap& context = {})`: Logs trace message
- `debug(const QString& message, const QVariantMap& context = {})`: Logs debug message
- `info(const QString& message, const QVariantMap& context = {})`: Logs info message
- `warning(const QString& message, const QVariantMap& context = {})`: Logs warning message
- `error(const QString& message, const QVariantMap& context = {})`: Logs error message
- `critical(const QString& message, const QVariantMap& context = {})`: Logs critical message
- `fatal(const QString& message, const QVariantMap& context = {})`: Logs fatal message
- `setLogLevel(LogLevel level)`: Sets minimum log level
- `setLogFormat(const QString& format)`: Sets log format ("human" or "json")
- `setOutputFile(const QString& path)`: Sets log file path
- `enableConsoleOutput(bool enable)`: Enables/disables console output
- `setCategoryEnabled(const QString& category, bool enabled)`: Enables/disables category
- `getRecentLogs(int count = 1000)`: Returns recent log entries for Diagnostics View

**Signals:**
- `logEntryAdded(const LogEntry& entry)`: Emitted when new log entry is created (for Diagnostics View)

**Dependencies:**
- `SettingsManager` - For configuration (log level, format, file path)

**See:** [21_LOGGING_STRATEGY.md](./21_LOGGING_STRATEGY.md) for complete logging strategy.

### 2.8a. LogEntry (DTO for Application Logs)
**Purpose:** Data transfer object for application log entries.

**Location:** `z-monitor/src/application/dto/LogEntry.h`

```cpp
/**
 * @struct LogEntry
 * @brief Data transfer object for application log entries.
 * 
 * Represents a single log entry for application logs (file-based).
 * Used by LogService for writing to log files and Diagnostics View.
 * 
 * @note This is a POD (Plain Old Data) struct, no business logic
 */
struct LogEntry {
    QDateTime timestamp;              ///< Timestamp when log entry was created
    LogService::LogLevel level;      ///< Log level (Trace, Debug, Info, Warning, Error, Critical, Fatal)
    QString category;                ///< Component category (e.g., "NetworkManager", "DatabaseManager")
    QString message;                 ///< Log message
    QVariantMap context;             ///< Structured context data (key-value pairs)
    QString threadId;                ///< Thread identifier
    QString file;                    ///< Source file name
    int line;                        ///< Source line number
    QString function;                ///< Source function name
    
    /**
     * @brief Convert to JSON string (for JSON format output).
     */
    QString toJson() const;
    
    /**
     * @brief Convert to human-readable string (for human format output).
     */
    QString toHumanReadable() const;
};
```

### 2.9. IActionLogRepository (Interface for Action Logs - Database-Based)
**Responsibility:** Interface for persisting user actions to `action_log` table for audit and compliance.

**Thread:** Database I/O Thread (asynchronous, non-blocking)

**Location:** `z-monitor/src/domain/interfaces/IActionLogRepository.h`

**Key Methods:**
- `logAction(const ActionLogEntry& entry)`: Logs a single user action (asynchronous)
- `logActions(const QList<ActionLogEntry>& entries)`: Logs multiple actions in batch (asynchronous)
- `queryActions(const ActionLogFilter& filter)`: Queries action log entries (asynchronous)
- `getActionCount(const QString& userId, const QDateTime& startTime, const QDateTime& endTime)`: Gets action count for user in time range

**Signals:**
- `actionLogged(const ActionLogEntry& entry)`: Emitted when action is successfully logged
- `actionLogFailed(const ActionLogEntry& entry, const Error& error)`: Emitted when action logging fails
- `actionsQueried(const QList<ActionLogEntry>& entries)`: Emitted when query completes

**Dependencies:**
- `DatabaseManager` - For database access (via implementation)

**See:** [39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md](./39_LOGIN_WORKFLOW_AND_ACTION_LOGGING.md) for action logging workflow.

### 2.9a. SQLiteActionLogRepository (Implementation)
**Responsibility:** SQLite implementation of `IActionLogRepository`. Persists action log entries to `action_log` table.

**Thread:** Database I/O Thread

**Location:** `z-monitor/src/infrastructure/persistence/SQLiteActionLogRepository.cpp/h`

**Key Properties:**
- `m_dbManager`: DatabaseManager instance (for database access)
- `m_pendingEntries`: Queue of pending entries for batch writes
- `m_flushTimer`: Timer for periodic batch writes (every 10 seconds)
- `m_batchSize`: Maximum batch size before flush (default: 100)

**Key Methods:**
- `logAction(const ActionLogEntry& entry)`: Queues action for batch write
- `logActions(const QList<ActionLogEntry>& entries)`: Queues multiple actions
- `queryActions(const ActionLogFilter& filter)`: Queries database for action log entries
- `flushPendingEntries()`: Flushes pending entries to database (batch write)

**Implementation Details:**
- Uses prepared statements for performance
- Implements hash chain for tamper detection (`previous_hash` column)
- Batch writes for performance (reduces database I/O)
- Asynchronous (non-blocking) operations

**Dependencies:**
- `DatabaseManager` - For database access
- `Schema::Columns::ActionLog` - For column name constants (from schema generation)

### 2.9b. ActionLogEntry (DTO for Action Logs)
**Purpose:** Data transfer object for action log entries persisted to database.

**Location:** `z-monitor/src/domain/dto/ActionLogEntry.h`

```cpp
/**
 * @struct ActionLogEntry
 * @brief Data transfer object for action log entries.
 * 
 * Represents a single user action logged to action_log table.
 * Used by IActionLogRepository for persisting user actions.
 * 
 * @note This is a POD (Plain Old Data) struct, no business logic
 * @note All fields match action_log table schema
 */
struct ActionLogEntry {
    QString userId;              ///< User who performed action (empty if no login required)
    QString userRole;            ///< User role (NURSE, PHYSICIAN, TECHNICIAN, ADMINISTRATOR)
    QString actionType;          ///< Action type (LOGIN, LOGOUT, AUTO_LOGOUT, ADMIT_PATIENT, etc.)
    QString targetType;           ///< Type of target (PATIENT, SETTING, NOTIFICATION, etc.)
    QString targetId;             ///< Target identifier (MRN, setting name, notification ID)
    QJsonObject details;          ///< Additional context (JSON object)
    QString result;               ///< SUCCESS, FAILURE, PARTIAL
    QString errorCode;            ///< Error code if result is FAILURE
    QString errorMessage;         ///< Error message if result is FAILURE
    QString sessionTokenHash;     ///< SHA-256 hash of session token (for audit trail)
    QString ipAddress;            ///< IP address (if available, for network actions)
    
    /**
     * @brief Convert details to JSON string for database storage.
     */
    QString detailsToJsonString() const {
        QJsonDocument doc(details);
        return doc.toJson(QJsonDocument::Compact);
    }
    
    /**
     * @brief Parse details from JSON string (from database).
     */
    void detailsFromJsonString(const QString& jsonString) {
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
        if (doc.isObject()) {
            details = doc.object();
        }
    }
};
```

**Action Types:**
- `LOGIN`, `LOGOUT`, `AUTO_LOGOUT`, `SESSION_EXPIRED`
- `ADMIT_PATIENT`, `DISCHARGE_PATIENT`, `TRANSFER_PATIENT`
- `CHANGE_SETTING`, `ADJUST_ALARM_THRESHOLD`, `RESET_SETTINGS`
- `CLEAR_NOTIFICATIONS`, `DISMISS_NOTIFICATION`
- `VIEW_AUDIT_LOG`, `EXPORT_DATA`, `ACCESS_DIAGNOSTICS`, `PROVISIONING_MODE_ENTERED`

### 2.9c. ActionLogFilter (Query Filter for Action Logs)
**Purpose:** Filter criteria for querying action log entries.

**Location:** `z-monitor/src/domain/dto/ActionLogFilter.h`

```cpp
/**
 * @struct ActionLogFilter
 * @brief Filter criteria for querying action log entries.
 */
struct ActionLogFilter {
    QString userId;                      ///< Filter by user ID (empty = all users)
    QString actionType;                  ///< Filter by action type (empty = all actions)
    QString targetType;                   ///< Filter by target type (empty = all targets)
    QString targetId;                    ///< Filter by target ID (empty = all targets)
    QDateTime startTime;                 ///< Start time (inclusive)
    QDateTime endTime;                   ///< End time (inclusive)
    QString deviceId;                    ///< Filter by device ID (empty = all devices)
    int limit;                           ///< Maximum number of results (0 = no limit)
    int offset;                          ///< Offset for pagination
    
    ActionLogFilter() : limit(0), offset(0) {}
};
```

### 2.10. IAuditRepository (Interface for Security Audit Logs - Database-Based)
**Responsibility:** Interface for persisting security events to `security_audit_log` table for forensics and compliance.

**Thread:** Database I/O Thread (asynchronous, non-blocking)

**Location:** `z-monitor/src/domain/interfaces/IAuditRepository.h`

**Key Methods:**
- `logAuditEvent(const AuditEntry& entry)`: Logs a single security audit event (asynchronous)
- `logAuditEvents(const QList<AuditEntry>& entries)`: Logs multiple events in batch (asynchronous)
- `queryAuditEvents(const AuditFilter& filter)`: Queries security audit log entries (asynchronous)
- `getAuditEventCount(const QString& eventType, const QDateTime& startTime, const QDateTime& endTime)`: Gets event count for type in time range

**Signals:**
- `auditEventLogged(const AuditEntry& entry)`: Emitted when event is successfully logged
- `auditEventLogFailed(const AuditEntry& entry, const Error& error)`: Emitted when event logging fails
- `auditEventsQueried(const QList<AuditEntry>& entries)`: Emitted when query completes

**Dependencies:**
- `DatabaseManager` - For database access (via implementation)

**See:** [06_SECURITY.md](./06_SECURITY.md) for security audit logging requirements.

### 2.10a. SQLiteAuditRepository (Implementation)
**Responsibility:** SQLite implementation of `IAuditRepository`. Persists security audit events to `security_audit_log` table.

**Thread:** Database I/O Thread

**Location:** `z-monitor/src/infrastructure/persistence/SQLiteAuditRepository.cpp/h`

**Key Properties:**
- `m_dbManager`: DatabaseManager instance (for database access)
- `m_pendingEntries`: Queue of pending entries for batch writes
- `m_flushTimer`: Timer for periodic batch writes (every 10 seconds)
- `m_batchSize`: Maximum batch size before flush (default: 100)

**Key Methods:**
- `logAuditEvent(const AuditEntry& entry)`: Queues event for batch write
- `logAuditEvents(const QList<AuditEntry>& entries)`: Queues multiple events
- `queryAuditEvents(const AuditFilter& filter)`: Queries database for audit log entries
- `flushPendingEntries()`: Flushes pending entries to database (batch write)

**Implementation Details:**
- Uses prepared statements for performance
- Implements hash chain for tamper detection (`previous_hash` column)
- Batch writes for performance (reduces database I/O)
- Asynchronous (non-blocking) operations

**Dependencies:**
- `DatabaseManager` - For database access
- `Schema::Columns::SecurityAuditLog` - For column name constants (from schema generation)

### 2.10b. AuditEntry (DTO for Security Audit Logs)
**Purpose:** Data transfer object for security audit log entries persisted to database.

**Location:** `z-monitor/src/domain/dto/AuditEntry.h`

```cpp
/**
 * @struct AuditEntry
 * @brief Data transfer object for security audit log entries.
 * 
 * Represents a single security event logged to security_audit_log table.
 * Used by IAuditRepository for persisting security events.
 * 
 * @note This is a POD (Plain Old Data) struct, no business logic
 * @note All fields match security_audit_log table schema
 */
struct AuditEntry {
    QString eventType;           ///< Event type (LOGIN_FAILED, CERTIFICATE_INSTALLED, etc.)
    QString eventCategory;       ///< Event category (AUTHENTICATION, CERTIFICATE, NETWORK, etc.)
    QString severity;            ///< Severity (INFO, WARNING, ERROR, CRITICAL)
    QString deviceId;            ///< Device identifier
    QString userId;              ///< User identifier (if applicable)
    QString sourceIp;            ///< Source IP address (if available)
    bool success;                ///< Whether operation succeeded
    QJsonObject details;         ///< Additional context (JSON object)
    QString errorCode;           ///< Error code (if applicable)
    QString errorMessage;        ///< Error message (if applicable)
    
    /**
     * @brief Convert details to JSON string for database storage.
     */
    QString detailsToJsonString() const {
        QJsonDocument doc(details);
        return doc.toJson(QJsonDocument::Compact);
    }
    
    /**
     * @brief Parse details from JSON string (from database).
     */
    void detailsFromJsonString(const QString& jsonString) {
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
        if (doc.isObject()) {
            details = doc.object();
        }
    }
};
```

**Event Types:**
- `LOGIN_FAILED`, `LOGIN_SUCCESS`, `SESSION_EXPIRED`, `SESSION_REVOKED`
- `CERTIFICATE_INSTALLED`, `CERTIFICATE_REVOKED`, `CERTIFICATE_VALIDATION_FAILED`
- `CONNECTION_REJECTED`, `CERTIFICATE_PINNING_FAILED`
- `UNAUTHORIZED_ACCESS`, `PERMISSION_DENIED`

**Event Categories:**
- `AUTHENTICATION`, `CERTIFICATE`, `NETWORK`, `SECURITY_VIOLATION`

**Severity Levels:**
- `INFO`, `WARNING`, `ERROR`, `CRITICAL`

### 2.10c. AuditFilter (Query Filter for Security Audit Logs)
**Purpose:** Filter criteria for querying security audit log entries.

**Location:** `z-monitor/src/domain/dto/AuditFilter.h`

```cpp
/**
 * @struct AuditFilter
 * @brief Filter criteria for querying security audit log entries.
 */
struct AuditFilter {
    QString eventType;                   ///< Filter by event type (empty = all events)
    QString eventCategory;               ///< Filter by event category (empty = all categories)
    QString severity;                    ///< Filter by severity (empty = all severities)
    QString userId;                      ///< Filter by user ID (empty = all users)
    QString deviceId;                    ///< Filter by device ID (empty = all devices)
    QDateTime startTime;                 ///< Start time (inclusive)
    QDateTime endTime;                   ///< End time (inclusive)
    bool successOnly;                    ///< Filter by success status (false = all)
    int limit;                           ///< Maximum number of results (0 = no limit)
    int offset;                          ///< Offset for pagination
    
    AuditFilter() : successOnly(false), limit(0), offset(0) {}
};
```

### 2.11. DataArchiver
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

### 3.3. WaveformController
**Responsibility:** Bridges waveform data from C++ to QML for real-time waveform visualization. Exposes waveform samples to QML `WaveformChart` components.

**Thread:** Main/UI Thread (required for QML property bindings)

**Properties (Q_PROPERTY):**
- `samples`: `QVariantList` - Array of waveform sample values exposed to QML (read-only)
- `sampleRate`: `int` - Samples per second (250 Hz for ECG, 125 Hz for Pleth) (read-only)
- `channel`: `QString` - Waveform channel identifier (e.g., "ECG_LEAD_II", "PLETH") (read-only)
- `isActive`: `bool` - Whether waveform data is actively being received (read-only)

**Q_INVOKABLE Methods:**
- `updateSamples(const QVariantList& newSamples)`: Updates waveform samples from `WaveformCache` (called by `MonitoringService`)
- `clearSamples()`: Clears waveform buffer (useful when switching patients or resetting display)
- `setChannel(const QString& channel)`: Sets waveform channel identifier

**Signals:**
- `samplesChanged()`: Emitted when samples property changes (triggers QML binding updates)
- `channelChanged(const QString& channel)`: Emitted when channel changes

**Dependencies:**
- `WaveformCache`: Source of waveform data (accessed via `MonitoringService`)
- `MonitoringService`: Provides waveform samples via signals

**Data Flow:**
1. `MonitoringService` receives waveform samples from `WebSocketSensorDataSource`
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
