# Background Tasks Module: Class Designs

**Document ID:** DESIGN-009f  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides detailed class designs for the **Background Tasks Module**, which handles low-priority system maintenance tasks on the Background Thread.

> **📋 Related Documents:**
> - [Class Designs Overview (09_CLASS_DESIGNS_OVERVIEW.md)](./09_CLASS_DESIGNS_OVERVIEW.md) - High-level module architecture
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Thread architecture (Section 4.6: Background Tasks Thread)

---

## 1. Module Overview

**Thread:** Background Thread  
**Priority:** Low (background, yield to RT/UI)  
**Component Count:** 9 components

**Purpose:**
- System health monitoring (CPU, memory, disk)
- Scheduled backups
- Firmware updates
- Time synchronization
- Watchdog monitoring

**Low Priority:** Yields to RT/UI threads

---

## 2. Module Diagram

[View Background Module Diagram (Mermaid)](./09f_BACKGROUND_MODULE.mmd)  
[View Background Module Diagram (SVG)](./09f_BACKGROUND_MODULE.svg)

---

## 3. Application Services

### 3.1. FirmwareUpdateService

**Responsibility:** Firmware update management.

**Thread:** Background Thread

**Key Methods:**
- `checkForUpdates()`: Checks for firmware updates from central server
- `downloadUpdate(const QString& version)`: Downloads firmware update
- `installUpdate(const QString& updatePath)`: Installs firmware update (requires restart)
- `validateUpdateSignature(const QString& updatePath)`: Validates firmware signature

**Dependencies:**
- `FirmwareManager` - Firmware file management
- `SecurityService` - Signature validation (Application Services Module)

---

### 3.2. BackupService

**Responsibility:** Database backup and restore.

**Thread:** Background Thread

**Key Methods:**
- `scheduleBackup()`: Schedules periodic backups (daily at 2 AM)
- `createBackup(const QString& backupPath)`: Creates encrypted backup of database
- `restoreBackup(const QString& backupPath, const QString& password)`: Restores database from backup
- `getBackupList()`: Returns list of available backups

**Dependencies:**
- `DatabaseManager` - Database access (Database Module, via queued call)
- `KeyManager` - Encryption key (for backup encryption)

---

## 4. Infrastructure Adapters

### 4.1. SettingsManager

**Responsibility:** Persistent configuration storage.

**Thread:** Background Thread

**Note:** `KeyManager` (if it exists as a separate service) would manage encryption keys for database, network, and signing. It would run on the Background Thread and coordinate with `SecureStorage`. For detailed `KeyManager` class design, see the legacy content in [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) section 2.4b.

**Key Properties:**
- `settings`: `QVariantMap` - Map of all configuration settings

**Key Methods:**
- `loadSettings()`: Loads settings from persistent storage
- `saveSettings()`: Persists settings to storage
- `getValue(const QString& key)`: Retrieves a setting value
- `setValue(const QString& key, const QVariant& value)`: Updates a setting

**Configuration Settings:**
- `deviceId`: Device identifier for telemetry transmission
- `deviceLabel`: Static device identifier/asset tag
- `measurementUnit`: "metric" or "imperial"
- `serverUrl`: Central server URL
- `useMockServer`: Boolean flag for testing/development

**Storage:**
- SQLite `settings` table (via `DatabaseManager`)

**Signals:**
- `settingsChanged()`: Emitted when settings are modified

---

### 4.2. QRCodeGenerator

**Responsibility:** QR code generation for device provisioning.

**Thread:** Background Thread

**Key Methods:**
- `generateQRCode(const QString& data)`: Generates QR code image
- `generateQRCodeWithPairingCode(const QString& deviceId, const QString& pairingCode)`: Generates QR code with device info and pairing code

**Dependencies:**
- QR code library (e.g., qrcodegen)

---

### 4.3. SecureStorage

**Responsibility:** Secure key storage (platform keychain).

**Thread:** Background Thread

**Key Methods:**
- `storeKey(const QString& keyId, const QByteArray& key)`: Stores key securely
- `retrieveKey(const QString& keyId)`: Retrieves key from secure storage
- `deleteKey(const QString& keyId)`: Securely deletes key

**Platform Support:**
- macOS: Keychain Access (Security framework)
- Windows: DPAPI (Data Protection API)
- Linux: Secret Service API (libsecret) or encrypted file with TPM

**Dependencies:**
- Platform-specific APIs (Keychain, DPAPI, TPM)

---

### 4.4. HealthMonitor

**Responsibility:** System health monitoring (CPU, memory, disk).

**Thread:** Background Thread

**Key Methods:**
- `startMonitoring()`: Starts periodic health monitoring (every 60 seconds)
- `getCpuUsage()`: Returns current CPU usage (%)
- `getMemoryUsage()`: Returns current memory usage (MB)
- `getDiskUsage()`: Returns current disk usage (MB)
- `getSystemHealth()`: Returns complete system health metrics

**Signals:**
- `healthUpdated(const QVariantMap& health)`: Emitted when health metrics updated
- `healthWarning(const QString& metric, double value)`: Emitted when metric exceeds warning threshold
- `healthCritical(const QString& metric, double value)`: Emitted when metric exceeds critical threshold

**Dependencies:**
- Qt System Info APIs

---

### 4.5. ClockSyncService

**Responsibility:** NTP time synchronization.

**Thread:** Background Thread

**Key Methods:**
- `syncTime()`: Synchronizes system time with NTP server
- `getLastSyncTime()`: Returns timestamp of last successful sync
- `isTimeSynced()`: Returns whether time is synchronized

**Frequency:** Syncs every 1 hour

**Fallback:** Uses central server timestamp if NTP unavailable

---

### 4.6. FirmwareManager

**Responsibility:** Firmware update file management.

**Thread:** Background Thread

**Key Methods:**
- `downloadFirmware(const QString& version, const QString& url)`: Downloads firmware file
- `validateFirmwareSignature(const QString& filePath)`: Validates firmware signature
- `installFirmware(const QString& filePath)`: Installs firmware (requires restart)

**Dependencies:**
- `SecurityService` - Signature validation (Application Services Module)

---

### 4.7. WatchdogService

**Responsibility:** Application crash detection and recovery.

**Thread:** Background Thread

**Key Methods:**
- `startMonitoring()`: Starts watchdog monitoring (every 10 seconds)
- `checkAllThreads()`: Checks heartbeat timestamps from all critical threads
- `registerThread(const QString& threadName, std::atomic<qint64>* heartbeat)`: Registers thread for monitoring
- `onThreadHeartbeat(const QString& threadName)`: Called by threads to update heartbeat

**Monitored Threads:**
- Real-Time Processing Thread (critical, 100ms timeout)
- Database Thread (critical, 500ms timeout)
- Network Thread (normal, 1s timeout)
- Application Services (normal, 1s timeout)

**Actions on Missed Heartbeat:**
1. Log event to `security_audit_log`
2. Emit UI warning
3. Attempt thread restart (configurable)
4. Escalate to system-level restart if multiple threads fail

**Signals:**
- `threadHeartbeatMissed(const QString& threadName, int timeoutMs)`: Emitted when heartbeat missed
- `threadRecovered(const QString& threadName)`: Emitted when thread recovers

---

## 5. Module Communication

### 5.1. Inbound (From Other Modules)

**From Interface Module (UI Thread):**
- `Qt::QueuedConnection` calls for settings updates
- `Qt::QueuedConnection` calls for backup/restore operations
- `Qt::QueuedConnection` calls for firmware updates

**From Application Services Module (App Services Thread):**
- `Qt::QueuedConnection` calls for settings access

**From All Modules:**
- Heartbeat updates (via atomic timestamps)

### 5.2. Outbound (To Other Modules)

**To Interface Module (UI Thread):**
- `Qt::QueuedConnection` signals for system health updates
- `Qt::QueuedConnection` signals for settings changes

**To Database Module (Database I/O Thread):**
- `Qt::QueuedConnection` calls for settings persistence

**To Network Module (Network I/O Thread):**
- `Qt::QueuedConnection` calls for certificate/key storage

**To All Modules:**
- Watchdog monitoring (read-only, via atomic timestamps)

---

## 6. Related Documents

- **[09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md)** - High-level module architecture
- **[12_THREAD_MODEL.md](./12_THREAD_MODEL.md)** - Thread architecture (Section 4.6: Background Tasks Thread)
- **[24_CONFIGURATION_MANAGEMENT.md](./24_CONFIGURATION_MANAGEMENT.md)** - Configuration management strategy

---

*This document provides detailed class designs for the Background Tasks Module. For other modules, see the module-specific documents listed in [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md).*

