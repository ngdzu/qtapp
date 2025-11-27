# Configuration Management Strategy

This document defines the configuration management approach, settings hierarchy, configuration validation, and runtime configuration updates for the Z Monitor application.

## 1. Guiding Principles

- **Single Source of Truth:** All configuration stored in database (`settings` table)
- **Type Safety:** Strongly-typed configuration values
- **Validation:** All configuration values validated before storage
- **Defaults:** Sensible defaults for all settings
- **Persistence:** Configuration persists across restarts
- **Auditability:** All configuration changes logged

## 2. Configuration Architecture

### 2.1. SettingsManager Design

```cpp
class SettingsManager : public QObject {
    Q_OBJECT
    
public:
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
    
    // Load defaults
    void loadDefaults();
    void resetToDefaults();
    
signals:
    void settingChanged(const QString& key, const QVariant& oldValue, const QVariant& newValue);
    void configurationReloaded();
    
private:
    bool validateValue(const QString& key, const QVariant& value) const;
    void logSettingChange(const QString& key, const QVariant& oldValue, 
                         const QVariant& newValue, const QString& updatedBy);
};
```

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

| Key | Type | Default | Description | Validation |
|-----|------|---------|-------------|------------|
| `deviceId` | String | `"ZM-001"` | Unique device identifier | Non-empty, alphanumeric, max 64 chars |
| `deviceLabel` | String | `""` | Human-readable device label/asset tag | Max 128 chars |
| `measurementUnit` | String | `"metric"` | Measurement unit system | `"metric"` or `"imperial"` |

### 3.2. Network Configuration

| Key | Type | Default | Description | Validation |
|-----|------|---------|-------------|------------|
| `network.provisioning.state` | String | `"NotProvisioned"` | Provisioning state | Valid state enum |
| `network.provisioning.pairingCode` | String | `""` | Current pairing code | 6-digit code format |
| `network.connection.retryAttempts` | Int | `3` | Max retry attempts | 1-10 |
| `network.connection.retryDelay` | Int | `1000` | Retry delay in ms | 100-10000 |

### 3.3. Alarm Configuration

| Key | Type | Default | Description | Validation |
|-----|------|---------|-------------|------------|
| `alarm.heartRate.low` | Int | `60` | Low heart rate threshold | 30-200 |
| `alarm.heartRate.high` | Int | `100` | High heart rate threshold | 30-200 |
| `alarm.spo2.low` | Int | `90` | Low SpO2 threshold | 70-100 |
| `alarm.respirationRate.low` | Int | `12` | Low respiration rate threshold | 8-30 |
| `alarm.respirationRate.high` | Int | `20` | High respiration rate threshold | 8-30 |
| `alarm.silenceDuration` | Int | `120` | Alarm silence duration (seconds) | 30-600 |

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
    
    if (key.startsWith("alarm.")) {
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
    // Log to audit_log table
    DatabaseManager::instance()->logAuditEvent({
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
    setValue("alarm.heartRate.low", 60, "system");
    setValue("alarm.heartRate.high", 100, "system");
    // ... more defaults
    
    // Mark as loaded
    setValue("_defaults_loaded", true, "system");
}
```

### 7.2. Reset to Defaults

```cpp
void SettingsManager::resetToDefaults() {
    // Clear all settings except system settings
    DatabaseManager::instance()->clearSettings();
    
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

### 9.1. Direct Access (Core Services)

Core services access configuration directly:

```cpp
class NetworkManager {
private:
    SettingsManager* m_settings;
    
public:
    void connectToServer() {
        // Access configuration directly
        int retryAttempts = m_settings->getInt("network.connection.retryAttempts", 3);
        int retryDelay = m_settings->getInt("network.connection.retryDelay", 1000);
        // Use configuration...
    }
};
```

### 9.2. Property Binding (QML)

QML accesses configuration via controller properties:

```cpp
class SettingsController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString deviceId READ deviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString measurementUnit READ measurementUnit NOTIFY measurementUnitChanged)
    
public:
    QString deviceId() const {
        return SettingsManager::instance()->deviceId();
    }
    
    QString measurementUnit() const {
        return SettingsManager::instance()->measurementUnit();
    }
    
    Q_INVOKABLE bool setMeasurementUnit(const QString& unit) {
        return SettingsManager::instance()->setMeasurementUnit(unit, currentUserId());
    }
};
```

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
    if (requiresAuthentication(key) && !AuthenticationService::instance()->isAuthenticated()) {
        LogService::warning("Unauthorized configuration change attempt", {
            {"key", key},
            {"updatedBy", updatedBy}
        });
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

## 13. Related Documents

- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - SettingsManager class design
- [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) - Settings table schema
- [06_SECURITY.md](./06_SECURITY.md) - Security configuration
- [21_LOGGING_STRATEGY.md](./21_LOGGING_STRATEGY.md) - Logging configuration

