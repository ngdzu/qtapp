# Configuration Management Strategy

**Document ID:** DESIGN-024  
**Version:** 2.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document defines the configuration management approach, settings hierarchy, configuration validation, and runtime configuration updates for the Z Monitor application.

> **📋 Related Documents:**
> - [Functional Requirements (03_FUNCTIONAL_REQUIREMENTS.md)](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) - Configuration-related requirements ⭐
> - [Database Design (10_DATABASE_DESIGN.md)](./10_DATABASE_DESIGN.md) - Settings table schema
> - [Class Designs (09_CLASS_DESIGNS.md)](./09_CLASS_DESIGNS.md) - SettingsManager class design

## 1. Guiding Principles

- **Single Source of Truth:** All configuration stored in database (`settings` table)
- **Type Safety:** Strongly-typed configuration values
- **Validation:** All configuration values validated before storage
- **Defaults:** Sensible defaults for all settings
- **Persistence:** Configuration persists across restarts
- **Auditability:** All configuration changes logged
- **Dependency Injection:** `SettingsManager` is injected via constructor, not accessed via singleton (see [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md))

## 2. Configuration Architecture

### 2.1. SettingsManager Design

**Important:** `SettingsManager` follows dependency injection principles (see [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md)). It is **NOT** a singleton and should be injected via constructor.

```cpp
class SettingsManager : public QObject {
    Q_OBJECT
    
public:
    // Constructor: Takes DatabaseManager as dependency (dependency injection)
    explicit SettingsManager(DatabaseManager* databaseManager, QObject* parent = nullptr);
    
    // Generic get/set
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    bool setValue(const QString& key, const QVariant& value, const QString& updatedBy = QString());
    
    // Type-safe getters
    QString getString(const QString& key, const QString& defaultValue = QString()) const;
    int getInt(const QString& key, int defaultValue = 0) const;
    bool getBool(const QString& key, bool defaultValue = false) const;
    double getDouble(const QString& key, double defaultValue = 0.0) const;
    
    // Type-safe setters with validation
    bool setString(const QString& key, const QString& value, const QString& updatedBy = QString());
    bool setInt(const QString& key, int value, const QString& updatedBy = QString());
    bool setBool(const QString& key, bool value, const QString& updatedBy = QString());
    bool setDouble(const QString& key, double value, const QString& updatedBy = QString());
    
    // Device configuration (convenience methods)
    QString deviceId() const;
    bool setDeviceId(const QString& deviceId, const QString& updatedBy = QString());
    
    QString deviceLabel() const;
    bool setDeviceLabel(const QString& label, const QString& updatedBy = QString());
    
    QString measurementUnit() const;  // "metric" or "imperial"
    bool setMeasurementUnit(const QString& unit, const QString& updatedBy = QString());
    
    QString language() const;  // ISO 639-1 code (e.g., "en", "es")
    bool setLanguage(const QString& lang, const QString& updatedBy = QString());
    
    int alarmVolume() const;  // 0-100
    bool setAlarmVolume(int volume, const QString& updatedBy = QString());
    
    // Load defaults
    void loadDefaults();
    void resetToDefaults();
    
signals:
    void settingChanged(const QString& key, const QVariant& oldValue, const QVariant& newValue);
    void configurationReloaded();
    
private:
    DatabaseManager* m_databaseManager;  // Injected dependency
    bool validateValue(const QString& key, const QVariant& value) const;
    void logSettingChange(const QString& key, const QVariant& oldValue, 
                         const QVariant& newValue, const QString& updatedBy);
};
```

**Dependency Injection Pattern:**
- `SettingsManager` receives `DatabaseManager` via constructor injection
- Services that need `SettingsManager` receive it via constructor injection
- No global `instance()` method - use dependency injection instead
- See [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md) for DI guidelines

### 2.2. Configuration Storage

All configuration stored in `settings` table:

```sql
CREATE TABLE IF NOT EXISTS settings (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at INTEGER NOT NULL,
    updated_by TEXT NULL
);
```

## 3. Configuration Categories

### 3.1. Device Configuration

| Key | Type | Default | Description | Validation | Requirement |
|-----|------|---------|-------------|------------|-------------|
| `deviceId` | String | `"ZM-001"` | Unique device identifier | Non-empty, alphanumeric, max 64 chars | REQ-FUN-DEV-001 |
| `deviceLabel` | String | `""` | Human-readable device label/asset tag | Max 128 chars | REQ-FUN-SYS-001 |
| `measurementUnit` | String | `"metric"` | Measurement unit system | `"metric"` or `"imperial"` | REQ-FUN-SYS-001 |
| `display.language` | String | `"en"` | UI language code | ISO 639-1 code (`"en"`, `"es"`, etc.) | REQ-FUN-SYS-001 |
| `alarm.volume` | Int | `75` | Alarm audio volume (0-100) | 0-100 | REQ-FUN-SYS-001 |

**Note:** Screen brightness is a hardware/OS-level setting and is not managed by the application. Users should adjust brightness through the device's OS settings or hardware controls.

### 3.2. Network Configuration

| Key | Type | Default | Description | Validation |
|-----|------|---------|-------------|------------|
| `network.provisioning.state` | String | `"NotProvisioned"` | Provisioning state | Valid state enum |
| `network.provisioning.pairingCode` | String | `""` | Current pairing code | 6-digit code format |
| `network.connection.retryAttempts` | Int | `3` | Max retry attempts | 1-10 |
| `network.connection.retryDelay` | Int | `1000` | Retry delay in ms | 100-10000 |

### 3.3. Alarm Configuration

**Global Alarm Thresholds (Device Defaults):**

| Key | Type | Default | Description | Validation | Requirement |
|-----|------|---------|-------------|------------|-------------|
| `alarm.heartRate.low` | Int | `60` | Low heart rate threshold (global default) | 30-200 | REQ-FUN-ALARM-001 |
| `alarm.heartRate.high` | Int | `100` | High heart rate threshold (global default) | 30-200 | REQ-FUN-ALARM-001 |
| `alarm.spo2.low` | Int | `90` | Low SpO2 threshold (global default) | 70-100 | REQ-FUN-ALARM-001 |
| `alarm.respirationRate.low` | Int | `12` | Low respiration rate threshold (global default) | 8-30 | REQ-FUN-ALARM-001 |
| `alarm.respirationRate.high` | Int | `20` | High respiration rate threshold (global default) | 8-30 | REQ-FUN-ALARM-001 |
| `alarm.silenceDuration` | Int | `120` | Alarm silence duration (seconds) | 30-600 | REQ-FUN-ALARM-021 |

**Per-Patient Alarm Thresholds:**

Per-patient alarm thresholds are stored in the `patients` table (not in `settings` table) to support patient-specific configuration as required by **REQ-FUN-ALARM-010**.

| Column | Type | Default | Description | Validation |
|--------|------|---------|-------------|------------|
| `alarm_heart_rate_low` | Int | Global default | Patient-specific low HR threshold | 30-200 |
| `alarm_heart_rate_high` | Int | Global default | Patient-specific high HR threshold | 30-200 |
| `alarm_spo2_low` | Int | Global default | Patient-specific low SpO2 threshold | 70-100 |
| `alarm_respiration_rate_low` | Int | Global default | Patient-specific low RR threshold | 8-30 |
| `alarm_respiration_rate_high` | Int | Global default | Patient-specific high RR threshold | 8-30 |

**Per-Patient Threshold Resolution:**
1. Check patient-specific threshold in `patients` table (if set)
2. Fall back to global default from `settings` table
3. Use hard-coded system default if neither exists

**Access Control:**
- **Global thresholds:** Administrators can modify
- **Per-patient thresholds:** Physicians only (requires authorization per REQ-FUN-ALARM-010)
- **Threshold changes:** Logged with user ID and timestamp
- **Immediate effect:** New thresholds take effect immediately upon change

### 3.4. Data Retention Configuration

| Key | Type | Default | Description | Validation |
|-----|------|---------|-------------|------------|
| `data.retention.vitals.days` | Int | `7` | Vitals retention (days) | 1-365 |
| `data.retention.alarms.days` | Int | `90` | Alarms retention (days) | 1-365 |
| `data.retention.audit.days` | Int | `90` | Audit log retention (days) | 30-365 |

### 3.5. Security Configuration

| Key | Type | Default | Description | Validation |
|-----|------|---------|-------------|------------|
| `security.session.timeout` | Int | `1800` | Session timeout (seconds) | 300-7200 |
| `security.login.maxAttempts` | Int | `5` | Max login attempts | 3-10 |
| `security.login.lockoutDuration` | Int | `900` | Account lockout duration (seconds) | 60-3600 |
| `security.certificate.checkInterval` | Int | `3600` | Certificate check interval (seconds) | 300-86400 |

### 3.6. Logging Configuration

| Key | Type | Default | Description | Validation |
|-----|------|---------|-------------|------------|
| `log.level` | String | `"INFO"` | Minimum log level | Valid log level |
| `log.file.enabled` | Bool | `true` | Enable file logging | - |
| `log.file.maxSize` | Int | `10` | Max log file size (MB) | 1-100 |
| `log.file.maxFiles` | Int | `7` | Max rotated log files | 1-30 |
| `log.format` | String | `"json"` | Log format | `"human"` or `"json"` |

## 4. Configuration Validation

### 4.1. Validation Rules

Each configuration key has validation rules:

```cpp
bool SettingsManager::validateValue(const QString& key, const QVariant& value) const {
    if (key == "deviceId") {
        QString str = value.toString();
        if (str.isEmpty() || str.length() > 64) {
            return false;
        }
        // Alphanumeric only
        QRegExp regex("^[A-Za-z0-9-_]+$");
        return regex.exactMatch(str);
    }
    
    if (key == "measurementUnit") {
        QString unit = value.toString();
        return unit == "metric" || unit == "imperial";
    }
    
    if (key == "display.language") {
        QString lang = value.toString();
        // ISO 639-1 language codes
        QStringList validLanguages = {"en", "es", "fr", "de", "it", "pt", "zh", "ja", "ko"};
        return validLanguages.contains(lang);
    }
    
    if (key == "alarm.volume") {
        int volume = value.toInt();
        return volume >= 0 && volume <= 100;
    }
    
    if (key.startsWith("alarm.") && key.contains(".") && !key.contains("silence")) {
        // Global alarm thresholds
        int intValue = value.toInt();
        // Get min/max from validation rules
        auto [min, max] = getAlarmThresholdRange(key);
        return intValue >= min && intValue <= max;
    }
    
    // Default validation
    return true;
}
```

### 4.2. Validation Error Handling

```cpp
bool SettingsManager::setValue(const QString& key, const QVariant& value, 
                               const QString& updatedBy) {
    if (!validateValue(key, value)) {
        LogService::warning("Invalid configuration value", {
            {"key", key},
            {"value", value.toString()},
            {"updatedBy", updatedBy}
        });
        emit settingChangeFailed(key, "Validation failed");
        return false;
    }
    
    QVariant oldValue = getValue(key);
    // Store in database
    // ...
    
    emit settingChanged(key, oldValue, value);
    return true;
}
```

## 5. Configuration Hierarchy

### 5.1. Hierarchy Levels

1. **Database (Persistent):** Stored in `settings` table, persists across restarts
2. **Runtime (In-Memory):** Temporary overrides (not persisted)
3. **Defaults (Code):** Hard-coded defaults used when not in database

### 5.2. Resolution Order

```cpp
QVariant SettingsManager::getValue(const QString& key, const QVariant& defaultValue) const {
    // 1. Check runtime overrides (highest priority)
    if (m_runtimeOverrides.contains(key)) {
        return m_runtimeOverrides[key];
    }
    
    // 2. Check database (persistent)
    QVariant dbValue = getValueFromDatabase(key);
    if (dbValue.isValid()) {
        return dbValue;
    }
    
    // 3. Return default (lowest priority)
    return defaultValue;
}
```

## 6. Configuration Updates

### 6.1. Runtime Updates

Configuration can be updated at runtime:

```cpp
// Update device ID
if (settingsManager->setDeviceId("ZM-002", currentUserId)) {
    LogService::info("Device ID updated", {
        {"oldValue", oldDeviceId},
        {"newValue", "ZM-002"},
        {"updatedBy", currentUserId}
    });
} else {
    // Validation failed
}
```

### 6.2. Change Notification

All configuration changes emit signals:

```cpp
// In SettingsController (QML bridge)
SettingsController {
    Connections {
        target: settingsManager
        function onSettingChanged(key, oldValue, newValue) {
            // Update UI
            if (key === "measurementUnit") {
                updateMeasurementUnit(newValue);
            }
        }
    }
}
```

### 6.3. Audit Logging

All configuration changes are logged:

```cpp
void SettingsManager::logSettingChange(const QString& key, const QVariant& oldValue,
                                      const QVariant& newValue, const QString& updatedBy) {
    // Log to audit_log table via injected DatabaseManager
    m_databaseManager->logAuditEvent({
        {"event_type", "setting_changed"},
        {"key", key},
        {"old_value", oldValue.toString()},
        {"new_value", newValue.toString()},
        {"updated_by", updatedBy},
        {"timestamp", QDateTime::currentDateTimeUtc()}
    });
    
    // Log to security_audit_log for security-related settings
    if (key.startsWith("security.")) {
        SecurityAuditLogger::log({
            {"event_type", "configuration_change"},
            {"key", key},
            {"updated_by", updatedBy}
        });
    }
}
```

**Note:** `SettingsManager` uses injected `DatabaseManager` (via `m_databaseManager`), not `DatabaseManager::instance()`. All dependencies are injected via constructor.

## 7. Configuration Defaults

### 7.1. Default Loading

Defaults are loaded on first startup:

```cpp
void SettingsManager::loadDefaults() {
    // Check if defaults already loaded
    if (getValue("_defaults_loaded").toBool()) {
        return;  // Already loaded
    }
    
    // Load all defaults
    setValue("deviceId", "ZM-001", "system");
    setValue("deviceLabel", "", "system");
    setValue("measurementUnit", "metric", "system");
    setValue("display.language", "en", "system");
    setValue("alarm.volume", 75, "system");
    setValue("alarm.heartRate.low", 60, "system");
    setValue("alarm.heartRate.high", 100, "system");
    setValue("alarm.spo2.low", 90, "system");
    setValue("alarm.respirationRate.low", 12, "system");
    setValue("alarm.respirationRate.high", 20, "system");
    setValue("alarm.silenceDuration", 120, "system");
    // ... more defaults
    
    // Mark as loaded
    setValue("_defaults_loaded", true, "system");
}
```

### 7.2. Reset to Defaults

```cpp
void SettingsManager::resetToDefaults() {
    // Clear all settings except system settings via injected DatabaseManager
    m_databaseManager->clearSettings();
    
    // Reload defaults
    loadDefaults();
    
    emit configurationReloaded();
}
```

## 8. Configuration Migration

### 8.1. Version Management

Track configuration schema version:

```cpp
const int CONFIG_VERSION = 2;  // Increment on schema changes

void SettingsManager::migrateConfiguration() {
    int currentVersion = getValue("_config_version", 0).toInt();
    
    if (currentVersion < 1) {
        // Migration from version 0 to 1
        migrateV0ToV1();
    }
    
    if (currentVersion < 2) {
        // Migration from version 1 to 2
        migrateV1ToV2();
    }
    
    setValue("_config_version", CONFIG_VERSION, "system");
}
```

### 8.2. Migration Examples

```cpp
void SettingsManager::migrateV1ToV2() {
    // Rename "bedId" to "deviceLabel"
    QString bedId = getValue("bedId").toString();
    if (!bedId.isEmpty()) {
        setValue("deviceLabel", bedId, "system");
        // Remove old key
        removeValue("bedId");
    }
    
    // Add new settings with defaults
    if (!hasValue("network.provisioning.state")) {
        setValue("network.provisioning.state", "NotProvisioned", "system");
    }
}
```

## 9. Configuration Access Patterns

### 9.1. Direct Access (Core Services) - Dependency Injection

Core services receive `SettingsManager` via constructor injection:

```cpp
class NetworkManager {
private:
    SettingsManager* m_settings;  // Injected via constructor
    
public:
    // Constructor injection
    explicit NetworkManager(SettingsManager* settings, QObject* parent = nullptr)
        : QObject(parent), m_settings(settings) {
        // SettingsManager injected, not accessed via singleton
    }
    
    void connectToServer() {
        // Access configuration via injected dependency
        int retryAttempts = m_settings->getInt("network.connection.retryAttempts", 3);
        int retryDelay = m_settings->getInt("network.connection.retryDelay", 1000);
        // Use configuration...
    }
};
```

**Bootstrap/Wiring Example:**
```cpp
// In main() or AppContainer
auto databaseManager = std::make_unique<DatabaseManager>(...);
auto settingsManager = std::make_unique<SettingsManager>(databaseManager.get());
auto networkManager = std::make_unique<NetworkManager>(settingsManager.get());
// Services wired via constructor injection
```

**Key Principle:** No `SettingsManager::instance()` - always inject via constructor. See [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md) for DI guidelines.

### 9.2. Property Binding (QML) - Dependency Injection

QML controllers receive `SettingsManager` via constructor injection:

```cpp
class SettingsController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString deviceId READ deviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString deviceLabel READ deviceLabel NOTIFY deviceLabelChanged)
    Q_PROPERTY(QString measurementUnit READ measurementUnit NOTIFY measurementUnitChanged)
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(int alarmVolume READ alarmVolume NOTIFY alarmVolumeChanged)
    
public:
    // Constructor injection
    explicit SettingsController(SettingsManager* settings, QObject* parent = nullptr)
        : QObject(parent), m_settings(settings) {
        // Connect to settings change signals
        connect(m_settings, &SettingsManager::settingChanged,
                this, &SettingsController::onSettingChanged);
    }
    
    QString deviceId() const {
        return m_settings->deviceId();
    }
    
    QString deviceLabel() const {
        return m_settings->deviceLabel();
    }
    
    QString measurementUnit() const {
        return m_settings->measurementUnit();
    }
    
    QString language() const {
        return m_settings->language();
    }
    
    int alarmVolume() const {
        return m_settings->alarmVolume();
    }
    
    Q_INVOKABLE bool setMeasurementUnit(const QString& unit) {
        return m_settings->setMeasurementUnit(unit, currentUserId());
    }
    
    Q_INVOKABLE bool setLanguage(const QString& lang) {
        return m_settings->setLanguage(lang, currentUserId());
    }
    
    Q_INVOKABLE bool setAlarmVolume(int volume) {
        return m_settings->setAlarmVolume(volume, currentUserId());
    }
    
private slots:
    void onSettingChanged(const QString& key, const QVariant& oldValue, const QVariant& newValue) {
        // Emit property change signals when settings change
        if (key == "deviceId") emit deviceIdChanged();
        else if (key == "deviceLabel") emit deviceLabelChanged();
        else if (key == "measurementUnit") emit measurementUnitChanged();
        else if (key == "display.language") emit languageChanged();
        else if (key == "alarm.volume") emit alarmVolumeChanged();
    }
    
signals:
    void deviceIdChanged();
    void deviceLabelChanged();
    void measurementUnitChanged();
    void languageChanged();
    void alarmVolumeChanged();
    
private:
    SettingsManager* m_settings;  // Injected dependency
};
```

**Key Principle:** Controllers receive `SettingsManager` via constructor injection, not via singleton. See [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md) for DI guidelines.

### 9.3. Per-Patient Alarm Threshold Access - Dependency Injection

Per-patient alarm thresholds are accessed through `PatientManager` or `AdmissionService`, which receive `SettingsManager` via constructor injection:

```cpp
class PatientManager {
public:
    // Constructor injection
    explicit PatientManager(SettingsManager* settings, 
                           DatabaseManager* database,
                           SecurityService* security,
                           QObject* parent = nullptr)
        : QObject(parent)
        , m_settings(settings)      // Injected dependency
        , m_database(database)       // Injected dependency
        , m_security(security)       // Injected dependency
    {
    }
    
    // Get patient-specific threshold (falls back to global default)
    int getAlarmHeartRateLow(const QString& patientMrn) const {
        // 1. Check patient-specific threshold
        int patientThreshold = getPatientThreshold(patientMrn, "alarm_heart_rate_low");
        if (patientThreshold > 0) {
            return patientThreshold;
        }
        
        // 2. Fall back to global default via injected SettingsManager
        return m_settings->getInt("alarm.heartRate.low", 60);
    }
    
    // Set patient-specific threshold (requires physician authorization)
    bool setAlarmHeartRateLow(const QString& patientMrn, int threshold, 
                              const QString& updatedBy) {
        // Check authorization (physician only per REQ-FUN-ALARM-010)
        if (!m_security->hasPermission(updatedBy, "configure_alarm_thresholds")) {
            return false;
        }
        
        // Validate threshold
        if (threshold < 30 || threshold > 200) {
            return false;
        }
        
        // Store in patients table
        updatePatientThreshold(patientMrn, "alarm_heart_rate_low", threshold);
        
        // Log change
        logThresholdChange(patientMrn, "alarm_heart_rate_low", threshold, updatedBy);
        
        // Emit signal for immediate effect
        emit patientThresholdChanged(patientMrn, "alarm_heart_rate_low", threshold);
        
        return true;
    }
    
private:
    SettingsManager* m_settings;     // Injected dependency
    DatabaseManager* m_database;     // Injected dependency
    SecurityService* m_security;     // Injected dependency
};
```

**Key Principle:** All services receive dependencies via constructor injection. No global singletons. See [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md) for DI guidelines.

## 10. Configuration Security

### 10.1. Sensitive Settings

Some settings are sensitive and require special handling:

- **Security settings:** Require authentication to change
- **Device ID:** Should not be changed after provisioning
- **Certificate settings:** Managed by provisioning service

### 10.2. Access Control

```cpp
bool SettingsManager::setValue(const QString& key, const QVariant& value,
                               const QString& updatedBy) {
    // Check if setting requires authentication
    // Note: AuthenticationService should be injected if needed, or passed as parameter
    // For now, assume authentication check is done by caller (controller/service layer)
    if (requiresAuthentication(key)) {
        // Authentication check should be done by SecurityService in application layer
        // SettingsManager focuses on storage/retrieval, not authorization
        LogService::warning("Setting requires authentication", {
            {"key", key},
            {"updatedBy", updatedBy}
        });
        // Return false - caller should check authentication before calling
        return false;
    }
    
    // Check if setting is locked (e.g., device ID after provisioning)
    if (isLocked(key)) {
        LogService::warning("Attempted to change locked setting", {
            {"key", key},
            {"updatedBy", updatedBy}
        });
        return false;
    }
    
    // Proceed with normal validation and storage
    // ...
}
```

**Note:** Authentication checks should be performed by `SecurityService` in the application layer before calling `SettingsManager::setValue()`. `SettingsManager` focuses on storage/retrieval, not authorization.

## 11. Configuration Backup and Restore

### 11.1. Export Configuration

```cpp
QJsonObject SettingsManager::exportConfiguration() const {
    QJsonObject config;
    QList<Setting> allSettings = getAllSettings();
    
    for (const Setting& setting : allSettings) {
        // Exclude system/internal settings
        if (!setting.key.startsWith("_")) {
            config[setting.key] = QJsonValue::fromVariant(setting.value);
        }
    }
    
    return config;
}
```

### 11.2. Import Configuration

```cpp
bool SettingsManager::importConfiguration(const QJsonObject& config, 
                                         const QString& importedBy) {
    // Validate all values before importing
    for (auto it = config.begin(); it != config.end(); ++it) {
        if (!validateValue(it.key(), it.value().toVariant())) {
            LogService::error("Invalid configuration in import", {
                {"key", it.key()}
            });
            return false;
        }
    }
    
    // Import all values
    for (auto it = config.begin(); it != config.end(); ++it) {
        setValue(it.key(), it.value().toVariant(), importedBy);
    }
    
    return true;
}
```

## 12. Best Practices

### 12.1. Do's

- ✅ Use SettingsManager for all configuration
- ✅ Validate all configuration values
- ✅ Log all configuration changes
- ✅ Provide sensible defaults
- ✅ Use type-safe getters/setters
- ✅ Emit signals on changes
- ✅ Test configuration validation

### 12.2. Don'ts

- ❌ Don't hard-code configuration values
- ❌ Don't bypass validation
- ❌ Don't store sensitive data in plaintext
- ❌ Don't change configuration without logging
- ❌ Don't access database directly for settings

## 13. Requirements Traceability

### 13.1. Functional Requirements

| Requirement ID | Description | Configuration Keys | Section |
|----------------|-------------|-------------------|---------|
| **REQ-FUN-SYS-001** | Device Settings Management | `deviceLabel`, `measurementUnit`, `display.language`, `alarm.volume` | 3.1 |

**Note:** Screen brightness is not included as it is a hardware/OS-level setting, not an application configuration setting.
| **REQ-FUN-ALARM-010** | Per-Patient Alarm Threshold Configuration | Per-patient thresholds in `patients` table | 3.3 |
| **REQ-FUN-ALARM-001** | Threshold-Based Alarm Triggering | Global defaults: `alarm.heartRate.*`, `alarm.spo2.*`, `alarm.respirationRate.*` | 3.3 |
| **REQ-FUN-ALARM-021** | Alarm Silence | `alarm.silenceDuration` | 3.3 |
| **REQ-FUN-DEV-001** | Device Provisioning | `deviceId`, `network.provisioning.*` | 3.1, 3.2 |

### 13.2. Key Requirements Details

**REQ-FUN-SYS-001 (Device Settings Management):**
- **Requirement:** System shall provide interface for authorized users to configure device settings including device label, measurement units, language, and display preferences.
- **Implementation:** All settings stored in `settings` table, accessible via `SettingsManager`
- **Access Control:** Technicians and administrators can modify (see Section 10.2)
- **See:** [03_FUNCTIONAL_REQUIREMENTS.md](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) Section 10

**REQ-FUN-ALARM-010 (Per-Patient Alarm Threshold Configuration):**
- **Requirement:** System shall allow authorized users (physicians) to configure alarm thresholds per patient based on clinical needs.
- **Implementation:** Per-patient thresholds stored in `patients` table, not `settings` table
- **Access Control:** Physicians only (requires authorization)
- **Default Behavior:** Default thresholds loaded at patient admission from global defaults
- **Immediate Effect:** New thresholds take effect immediately upon change
- **See:** [03_FUNCTIONAL_REQUIREMENTS.md](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) Section 7

## 14. Related Documents

- [Functional Requirements (03_FUNCTIONAL_REQUIREMENTS.md)](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) - Configuration-related requirements ⭐
- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - SettingsManager class design
- [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) - Settings table schema and patients table schema
- [06_SECURITY.md](./06_SECURITY.md) - Security configuration
- [21_LOGGING_STRATEGY.md](./21_LOGGING_STRATEGY.md) - Logging configuration
- [19_ADT_WORKFLOW.md](./19_ADT_WORKFLOW.md) - Patient admission workflow (threshold loading)

