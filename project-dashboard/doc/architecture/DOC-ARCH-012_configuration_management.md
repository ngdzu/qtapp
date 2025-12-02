---
doc_id: DOC-ARCH-012
title: Configuration Management Architecture
version: 1.0
category: Architecture
subcategory: System Design
status: Approved
owner: Architecture Team
reviewers:
  - Architecture Team
  - Infrastructure Team
last_reviewed: 2025-12-01
next_review: 2026-03-01
related_docs:
  - DOC-ARCH-001  # System Architecture
  - DOC-ARCH-017  # Database Design
  - DOC-COMP-022  # SettingsManager
  - DOC-REQ-003   # Functional Requirements
tags:
  - configuration
  - settings
  - validation
  - dependency-injection
source: 24_CONFIGURATION_MANAGEMENT.md (z-monitor/architecture_and_design)
---

# Configuration Management Architecture

## Document Purpose

This document defines the configuration management approach for Z Monitor, covering settings hierarchy, storage, validation, runtime updates, and dependency injection patterns. All configuration is stored in the encrypted SQLite database and accessed through `SettingsManager` with strict dependency injection.

---

## 1. Guiding Principles

- **Single Source of Truth:** All configuration stored in database (`settings` table)
- **Type Safety:** Strongly-typed configuration values with validation
- **Persistence:** Configuration persists across restarts
- **Auditability:** All configuration changes logged with user ID and timestamp
- **Dependency Injection:** `SettingsManager` injected via constructor, never accessed as singleton
- **Validation:** All values validated before storage
- **Defaults:** Sensible defaults for all settings

---

## 2. Configuration Architecture

### 2.1 SettingsManager Design

`SettingsManager` follows dependency injection principles - it receives `DatabaseManager` via constructor injection and is itself injected into services that need configuration.

**Key Interface:**
```cpp
class SettingsManager : public QObject {
    Q_OBJECT
public:
    // Constructor injection
    explicit SettingsManager(DatabaseManager* databaseManager, 
                            QObject* parent = nullptr);
    
    // Generic access
    QVariant getValue(const QString& key, 
                     const QVariant& defaultValue = QVariant()) const;
    bool setValue(const QString& key, const QVariant& value, 
                 const QString& updatedBy = QString());
    
    // Type-safe getters
    QString getString(const QString& key, const QString& defaultValue = QString()) const;
    int getInt(const QString& key, int defaultValue = 0) const;
    bool getBool(const QString& key, bool defaultValue = false) const;
    double getDouble(const QString& key, double defaultValue = 0.0) const;
    
    // Device configuration convenience methods
    QString deviceId() const;
    QString deviceLabel() const;
    QString measurementUnit() const;  // "metric" or "imperial"
    int alarmVolume() const;           // 0-100
    
signals:
    void settingChanged(const QString& key, const QVariant& oldValue, 
                       const QVariant& newValue);
    void configurationReloaded();
    
private:
    DatabaseManager* m_databaseManager;  // Injected dependency
    bool validateValue(const QString& key, const QVariant& value) const;
    void logSettingChange(const QString& key, const QVariant& oldValue,
                         const QVariant& newValue, const QString& updatedBy);
};
```

---

## 3. Configuration Categories

### 3.1 Device Configuration

| Key                | Type   | Default    | Validation                 | Description              |
| ------------------ | ------ | ---------- | -------------------------- | ------------------------ |
| `deviceId`         | String | `"ZM-001"` | Alphanumeric, max 64 chars | Unique device identifier |
| `deviceLabel`      | String | `""`       | Max 128 chars              | Human-readable asset tag |
| `measurementUnit`  | String | `"metric"` | "metric" or "imperial"     | Measurement system       |
| `display.language` | String | `"en"`     | ISO 639-1 code             | UI language              |
| `alarm.volume`     | Int    | `75`       | 0-100                      | Alarm audio volume       |

### 3.2 Network Configuration

| Key                                | Type   | Default            | Validation       |
| ---------------------------------- | ------ | ------------------ | ---------------- |
| `network.provisioning.state`       | String | `"NotProvisioned"` | Valid state enum |
| `network.connection.retryAttempts` | Int    | `3`                | 1-10             |
| `network.connection.retryDelay`    | Int    | `1000`             | 100-10000 ms     |

### 3.3 Global Alarm Thresholds

| Key                          | Type | Default | Validation     |
| ---------------------------- | ---- | ------- | -------------- |
| `alarm.heartRate.low`        | Int  | `60`    | 30-200         |
| `alarm.heartRate.high`       | Int  | `100`   | 30-200         |
| `alarm.spo2.low`             | Int  | `90`    | 70-100         |
| `alarm.respirationRate.low`  | Int  | `12`    | 8-30           |
| `alarm.respirationRate.high` | Int  | `20`    | 8-30           |
| `alarm.silenceDuration`      | Int  | `120`   | 30-600 seconds |

**Note:** Per-patient thresholds stored in `patients` table, not `settings` table.

### 3.4 Data Retention

| Key                          | Type | Default | Validation |
| ---------------------------- | ---- | ------- | ---------- |
| `data.retention.vitals.days` | Int  | `7`     | 1-365      |
| `data.retention.alarms.days` | Int  | `90`    | 1-365      |
| `data.retention.audit.days`  | Int  | `90`    | 30-365     |

### 3.5 Security

| Key                              | Type | Default | Validation       |
| -------------------------------- | ---- | ------- | ---------------- |
| `security.session.timeout`       | Int  | `1800`  | 300-7200 seconds |
| `security.login.maxAttempts`     | Int  | `5`     | 3-10             |
| `security.login.lockoutDuration` | Int  | `900`   | 60-3600 seconds  |

---

## 4. Configuration Storage

All configuration stored in encrypted database:

```sql
CREATE TABLE IF NOT EXISTS settings (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at INTEGER NOT NULL,
    updated_by TEXT NULL
);
```

---

## 5. Dependency Injection Pattern

### 5.1 Service Access Pattern

Services receive `SettingsManager` via constructor injection:

```cpp
class NetworkManager {
public:
    explicit NetworkManager(SettingsManager* settings, QObject* parent = nullptr)
        : QObject(parent), m_settings(settings) {
    }
    
    void connectToServer() {
        int retryAttempts = m_settings->getInt("network.connection.retryAttempts", 3);
        // Use configuration...
    }
    
private:
    SettingsManager* m_settings;  // Injected dependency
};
```

### 5.2 Bootstrap Example

```cpp
// In main() or AppContainer
auto databaseManager = std::make_unique<DatabaseManager>(...);
auto settingsManager = std::make_unique<SettingsManager>(databaseManager.get());
auto networkManager = std::make_unique<NetworkManager>(settingsManager.get());
```

**Key Principle:** No `SettingsManager::instance()` - always inject via constructor.

---

## 6. Validation

All configuration values validated before storage:

```cpp
bool SettingsManager::validateValue(const QString& key, 
                                   const QVariant& value) const {
    if (key == "deviceId") {
        QString str = value.toString();
        if (str.isEmpty() || str.length() > 64) return false;
        QRegExp regex("^[A-Za-z0-9-_]+$");
        return regex.exactMatch(str);
    }
    
    if (key == "measurementUnit") {
        QString unit = value.toString();
        return unit == "metric" || unit == "imperial";
    }
    
    if (key == "alarm.volume") {
        int volume = value.toInt();
        return volume >= 0 && volume <= 100;
    }
    
    // Additional validation rules...
    return true;
}
```

---

## 7. Configuration Hierarchy

**Resolution Order:**
1. Runtime overrides (highest priority, in-memory)
2. Database persistent storage
3. Code defaults (lowest priority)

---

## 8. Audit Logging

All configuration changes logged:

```cpp
void SettingsManager::logSettingChange(const QString& key, 
                                       const QVariant& oldValue,
                                       const QVariant& newValue, 
                                       const QString& updatedBy) {
    m_databaseManager->logAuditEvent({
        {"event_type", "setting_changed"},
        {"key", key},
        {"old_value", oldValue.toString()},
        {"new_value", newValue.toString()},
        {"updated_by", updatedBy},
        {"timestamp", QDateTime::currentDateTimeUtc()}
    });
}
```

---

## 9. Related Documents

- DOC-ARCH-017: Database Design (settings table schema)
- DOC-COMP-022: SettingsManager Component Specification
- DOC-ARCH-013: Dependency Injection Architecture
- DOC-REQ-003: Functional Requirements (configuration requirements)
