---
doc_id: DOC-COMP-022
title: SettingsManager
version: v1.0
category: Component
subcategory: Infrastructure Layer / Adapters
status: Draft
owner: Infrastructure Team
reviewers: [Architecture Team]
last_reviewed: 2025-12-01
next_review: 2026-12-01
related_docs: [DOC-COMP-018, DOC-COMP-012, DOC-COMP-013]
related_tasks: [TASK-3C-001]
tags: [infrastructure, settings, configuration, sqlite, singleton]
diagram_files: []
---

# DOC-COMP-022: SettingsManager

## 1. Overview

**Purpose:** Singleton manager for device configuration settings and user preferences with persistent SQLite storage. Provides key-value storage for application settings.

**Responsibilities:**
- Persist settings to `settings` table (key-value pairs)
- Retrieve setting values with default fallback
- Remove settings
- Check setting existence
- Convenience methods for common settings (deviceId, deviceLabel, measurementUnit, serverUrl)

**Layer:** Infrastructure Layer (Adapter)

**Module:** `z-monitor/src/infrastructure/adapters/SettingsManager.h` (207 lines)

**Thread Affinity:** Any thread (thread-safe)

**Key Settings:**
- `deviceId`: Unique device identifier for telemetry
- `deviceLabel`: Static device identifier/asset tag (e.g., "ICU-MON-04")
- `measurementUnit`: "metric" or "imperial"
- `serverUrl`: Central server URL for telemetry transmission
- `useMockServer`: Boolean flag for testing

## 2. Public API

```cpp
class SettingsManager : public QObject {
    Q_OBJECT

public:
    static SettingsManager& instance();

    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    bool setValue(const QString& key, const QVariant& value, const QString& userId = QString());
    bool removeValue(const QString& key);
    bool contains(const QString& key) const;

    // Convenience methods
    QString deviceId() const;
    bool setDeviceId(const QString& deviceId);
    QString deviceLabel() const;
    bool setDeviceLabel(const QString& deviceLabel);
    QString measurementUnit() const;
    bool setMeasurementUnit(const QString& unit);
    QString serverUrl() const;
    bool setServerUrl(const QString& url);
};
```

## 3. Usage Example

```cpp
// Get settings
QString deviceId = SettingsManager::instance().deviceId();
QString serverUrl = SettingsManager::instance().serverUrl();

// Set settings
SettingsManager::instance().setDeviceId("ZM-ICU-MON-04");
SettingsManager::instance().setServerUrl("https://central.example.com");

// Custom settings
SettingsManager::instance().setValue("alarmVolume", 75);
int volume = SettingsManager::instance().getValue("alarmVolume", 50).toInt();
```

## 4. Related Documentation

- DOC-COMP-012: ProvisioningService (uses SettingsManager for device config)
- DOC-COMP-013: SecurityService (uses SettingsManager for timeout config)
- DOC-COMP-018: DatabaseManager

## 5. Changelog

| Version | Date       | Author      | Changes                                      |
| ------- | ---------- | ----------- | -------------------------------------------- |
| v1.0    | 2025-12-01 | Dustin Wind | Initial documentation from SettingsManager.h |
