---
doc_id: DOC-ARCH-012
title: Configuration Management Architecture
version: 2.0
category: Architecture
subcategory: System Design
status: Approved
owner: Architecture Team
reviewers:
  - Architecture Team
  - Infrastructure Team
last_reviewed: 2025-12-08
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
  - environment-variables
  - bootstrap
source: 24_CONFIGURATION_MANAGEMENT.md (z-monitor/architecture_and_design)
---

# Configuration Management Architecture

## Document Purpose

This document defines the configuration management approach for Z Monitor, covering both **bootstrap configuration** (application startup parameters) and **runtime settings** (persistent user preferences and device configuration). It includes settings hierarchy, storage, validation, runtime updates, and dependency injection patterns.

---

## 1. Configuration Types

Z Monitor uses two distinct configuration systems:

### 1.1 Bootstrap Configuration (AppConfig)
- **Purpose:** Application initialization parameters (database path, logging, sensor source, cache sizes)
- **Loaded by:** `ConfigLoader` (application layer)
- **When:** Application startup, before database connection
- **Storage:** Environment variables > config.ini > defaults
- **Mutable:** No (requires restart to change)
- **Use case:** Deployment environment configuration (Docker, CI/CD, development vs production)

### 1.2 Runtime Settings (SettingsManager)
- **Purpose:** Device configuration, user preferences, alarm thresholds, network settings
- **Loaded by:** `SettingsManager` (infrastructure layer)
- **When:** After database initialization
- **Storage:** Encrypted SQLite database (`settings` table)
- **Mutable:** Yes (can be changed at runtime)
- **Use case:** User-configurable preferences and device settings

---

## 2. Bootstrap Configuration (ConfigLoader)

### 2.1 Overview

`ConfigLoader` provides **deployment flexibility** by supporting configuration from multiple sources with strict precedence:

**Priority Order:**
1. **Environment Variables** (highest priority) - for containerized deployments
2. **Configuration File** (config.ini) - for persistent local settings
3. **Default Values** (lowest priority) - hardcoded fallbacks

This enables the same binary to work in:
- **Docker containers** (using environment variables)
- **CI/CD pipelines** (using environment variables)
- **Development environments** (using config.ini)
- **Production deployments** (using config.ini + environment overrides)

### 2.2 Supported Configuration Keys

| Category     | Config Key               | Env Variable                  | Default                      | Description                                         |
| ------------ | ------------------------ | ----------------------------- | ---------------------------- | --------------------------------------------------- |
| **Database** | `database.path`          | `ZMON_DB_PATH`                | `$AppData/zmonitor.db`       | SQLite database file path                           |
| **Sensor**   | `sensor.mode`            | `ZMON_SENSOR_MODE`            | `in_memory`                  | Sensor data source (`in_memory` or `shared_memory`) |
| **Sensor**   | `sensor.shared_socket`   | `ZMON_SENSOR_SHARED_SOCKET`   | `/tmp/z-monitor-sensor.sock` | Socket path for shared memory sensor                |
| **Cache**    | `cache.vitals_seconds`   | `ZMON_CACHE_VITALS_SECONDS`   | `259200`                     | Vitals cache window (3 days @ 60Hz)                 |
| **Cache**    | `cache.waveform_samples` | `ZMON_CACHE_WAVEFORM_SAMPLES` | `22500`                      | Waveform cache capacity (30s @ 250Hz × 3 channels)  |
| **Logging**  | `logging.level`          | `ZMON_LOG_LEVEL`              | `info`                       | Log verbosity (`debug`, `info`, `warning`, `error`) |

### 2.3 ConfigLoader API

```cpp
namespace zmon {

/**
 * @brief Application bootstrap configuration.
 */
struct AppConfig {
    QString databasePath;          ///< Absolute path to SQLite DB file
    SensorSourceMode sensorSource; ///< Sensor source mode
    QString sharedMemorySocket;    ///< Socket path for shared memory sensor source
    int vitalsCacheSeconds;        ///< Vitals cache window in seconds
    int waveformCacheSamples;      ///< Waveform cache sample capacity
    LogLevel logLevel;             ///< Application logging level
};

/**
 * @brief Loads configuration from env vars > file > defaults.
 */
class ConfigLoader {
public:
    /**
     * @brief Load configuration from all available sources.
     * 
     * Priority: Environment Variables > config.ini > Defaults
     * 
     * @return AppConfig Populated configuration for dependency injection
     */
    static AppConfig load();
};

} // namespace zmon
```

### 2.4 Usage Example

```cpp
// In main.cpp (application bootstrap)
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Load bootstrap configuration
    const AppConfig cfg = ConfigLoader::load();
    
    // Use configuration to initialize services
    auto databaseManager = std::make_unique<DatabaseManager>(cfg.databasePath);
    auto vitalsCache = std::make_unique<VitalsCache>(cfg.vitalsCacheSeconds);
    auto waveformCache = std::make_unique<WaveformCache>(cfg.waveformCacheSamples);
    
    // ...
}
```

### 2.5 Docker Deployment Example

```dockerfile
# Dockerfile
ENV ZMON_DB_PATH=/data/monitor.db
ENV ZMON_SENSOR_MODE=shared_memory
ENV ZMON_SENSOR_SHARED_SOCKET=/run/sensor.sock
ENV ZMON_LOG_LEVEL=info
```

```bash
# docker-compose.yml
services:
  z-monitor:
    environment:
      - ZMON_DB_PATH=/data/zmonitor.db
      - ZMON_SENSOR_MODE=shared_memory
      - ZMON_LOG_LEVEL=debug
    volumes:
      - ./data:/data
```

### 2.6 Configuration File Format (config.ini)

```ini
[database]
path=/var/lib/z-monitor/zmonitor.db

[sensor]
mode=in_memory
shared_socket=/tmp/z-monitor-sensor.sock

[cache]
vitals_seconds=259200
waveform_samples=22500

[logging]
level=info
```

**Location:** Platform-specific application config directory
- **Linux:** `~/.config/z-monitor/config.ini`
- **macOS:** `~/Library/Preferences/z-monitor/config.ini`
- **Windows:** `%APPDATA%/z-monitor/config.ini`

### 2.7 Constants (No Hardcoded Strings)

All configuration keys and environment variable names are defined in `ConfigConstants.h`:

```cpp
namespace zmon::config {

namespace sections {
    constexpr const char* DATABASE = "database";
    constexpr const char* SENSOR = "sensor";
    constexpr const char* CACHE = "cache";
    constexpr const char* LOGGING = "logging";
}

namespace keys {
    constexpr const char* DB_PATH = "path";
    constexpr const char* SENSOR_MODE = "mode";
    constexpr const char* LOG_LEVEL = "level";
    // ...
}

namespace env {
    constexpr const char* DB_PATH = "ZMON_DB_PATH";
    constexpr const char* SENSOR_MODE = "ZMON_SENSOR_MODE";
    constexpr const char* LOG_LEVEL = "ZMON_LOG_LEVEL";
    // ...
}

} // namespace zmon::config
```

---

## 3. Runtime Settings (SettingsManager)

### 3.1 Guiding Principles

- **Single Source of Truth:** All configuration stored in database (`settings` table)
- **Type Safety:** Strongly-typed configuration values with validation
- **Persistence:** Configuration persists across restarts
- **Auditability:** All configuration changes logged with user ID and timestamp
- **Dependency Injection:** `SettingsManager` injected via constructor, never accessed as singleton
- **Validation:** All values validated before storage
- **Defaults:** Sensible defaults for all settings

---

## 4. SettingsManager Architecture

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

## 4. SettingsManager Architecture

### 4.1 SettingsManager Design

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

## 5. Configuration Categories (Runtime Settings)

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

### 7.1 Bootstrap Configuration (ConfigLoader)
**Resolution Order:**
1. Environment Variables (highest priority)
2. Configuration File (config.ini)
3. Code defaults (lowest priority)

### 7.2 Runtime Settings (SettingsManager)
**Resolution Order:**
1. Runtime overrides (highest priority, in-memory)
2. Database persistent storage
3. Code defaults (lowest priority)

---

## 8. Complete Bootstrap Example

```cpp
// main.cpp - Full application bootstrap with both config systems

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Step 1: Load bootstrap configuration (env vars > file > defaults)
    const AppConfig cfg = ConfigLoader::load();
    
    // Step 2: Initialize database using bootstrap config
    auto databaseManager = std::make_unique<DatabaseManager>(cfg.databasePath);
    if (!databaseManager->initialize()) {
        qCritical() << "Failed to initialize database";
        return 1;
    }
    
    // Step 3: Create SettingsManager for runtime settings
    auto settingsManager = std::make_unique<SettingsManager>(
        databaseManager.get()
    );
    
    // Step 4: Create services with injected dependencies
    auto vitalsCache = std::make_unique<VitalsCache>(cfg.vitalsCacheSeconds);
    auto waveformCache = std::make_unique<WaveformCache>(cfg.waveformCacheSamples);
    
    auto monitoringService = std::make_unique<MonitoringService>(
        vitalsCache.get(),
        waveformCache.get(),
        settingsManager.get()  // Runtime settings injected
    );
    
    // Step 5: Run application
    return app.exec();
}
```

---

## 9. Audit Logging

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

## 10. Testing Strategy

### 10.1 Bootstrap Configuration Tests (ConfigLoaderTest.cpp)

- ✅ Test default value loading
- ✅ Test config file loading
- ✅ Test environment variable override
- ✅ Test partial environment override
- ✅ Test precedence order (Env > File > Default)
- ✅ Test log level parsing
- ✅ Test sensor mode parsing
- ✅ Test invalid integer handling

### 10.2 Runtime Settings Tests (SettingsManagerTest.cpp)

- Test database persistence
- Test validation rules
- Test audit logging
- Test signal emission on change
- Test type-safe getters

---

## 11. Summary: When to Use What

| Use Case           | System                    | Example                                    |
| ------------------ | ------------------------- | ------------------------------------------ |
| Database file path | Bootstrap (ConfigLoader)  | `ZMON_DB_PATH=/data/monitor.db`            |
| Sensor data source | Bootstrap (ConfigLoader)  | `ZMON_SENSOR_MODE=shared_memory`           |
| Log verbosity      | Bootstrap (ConfigLoader)  | `ZMON_LOG_LEVEL=debug`                     |
| Cache sizes        | Bootstrap (ConfigLoader)  | `ZMON_CACHE_VITALS_SECONDS=259200`         |
| Device label       | Runtime (SettingsManager) | `deviceLabel="ICU-BED-4"`                  |
| Alarm volume       | Runtime (SettingsManager) | `alarm.volume=85`                          |
| Network state      | Runtime (SettingsManager) | `network.provisioning.state="Provisioned"` |
| User preferences   | Runtime (SettingsManager) | `display.language="es"`                    |

**Key Difference:**
- **Bootstrap:** System-level, deployment-specific, requires restart to change
- **Runtime:** User-level, device-specific, can change while running

---

## 12. Related Documents

- DOC-ARCH-017: Database Design (settings table schema)
- DOC-COMP-022: SettingsManager Component Specification
- DOC-ARCH-013: Dependency Injection Architecture
- DOC-REQ-003: Functional Requirements (configuration requirements)
- ConfigLoader.h: Bootstrap configuration API
- ConfigConstants.h: Configuration key and environment variable name constants
