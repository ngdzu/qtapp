/**
 * @file SettingsManager.h
 * @brief Manages device configuration settings and user preferences.
 *
 * This file contains the SettingsManager class which provides persistent
 * storage for device settings using SQLite. Settings are stored as key-value
 * pairs in the `settings` table.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QDateTime>

namespace zmon {
/**
 * @class SettingsManager
 * @brief Manages device configuration settings and user preferences.
 *
 * Provides persistent storage for device settings using SQLite database.
 * Settings are stored as key-value pairs in the `settings` table.
 *
 * Key settings:
 * - `deviceId`: Unique device identifier for telemetry transmission
 * - `deviceLabel`: Static device identifier/asset tag (e.g., "ICU-MON-04")
 * - `measurementUnit`: Measurement system preference ("metric" or "imperial")
 * - `serverUrl`: Central server URL for telemetry transmission
 * - `useMockServer`: Boolean flag to use mock server for testing
 *
 * @note Thread-safe: Can be called from any thread (uses database connection).
 * @note Settings are persisted to SQLite database.
 *
 * @ingroup Infrastructure
 */
class SettingsManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Gets the singleton instance of SettingsManager.
     *
     * @return Reference to SettingsManager instance
     */
    static SettingsManager& instance();

    /**
     * @brief Gets a setting value by key.
     *
     * @param key Setting key
     * @param defaultValue Default value if setting not found
     * @return Setting value or defaultValue if not found
     */
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief Sets a setting value.
     *
     * @param key Setting key
     * @param value Setting value
     * @param userId Optional user ID who made the change (for audit)
     * @return true if setting was saved successfully, false otherwise
     */
    bool setValue(const QString& key, const QVariant& value, const QString& userId = QString());

    /**
     * @brief Removes a setting.
     *
     * @param key Setting key to remove
     * @return true if setting was removed successfully, false otherwise
     */
    bool removeValue(const QString& key);

    /**
     * @brief Checks if a setting exists.
     *
     * @param key Setting key
     * @return true if setting exists, false otherwise
     */
    bool contains(const QString& key) const;

    // Convenience methods for common settings

    /**
     * @brief Gets the device ID.
     *
     * @return Device ID string
     */
    QString deviceId() const;

    /**
     * @brief Sets the device ID.
     *
     * @param deviceId Device ID string
     * @return true if saved successfully
     */
    bool setDeviceId(const QString& deviceId);

    /**
     * @brief Gets the device label (asset tag).
     *
     * @return Device label string (e.g., "ICU-MON-04")
     */
    QString deviceLabel() const;

    /**
     * @brief Sets the device label (asset tag).
     *
     * @param deviceLabel Device label string
     * @return true if saved successfully
     */
    bool setDeviceLabel(const QString& deviceLabel);

    /**
     * @brief Gets the measurement unit preference.
     *
     * @return "metric" or "imperial"
     */
    QString measurementUnit() const;

    /**
     * @brief Sets the measurement unit preference.
     *
     * @param unit "metric" or "imperial"
     * @return true if saved successfully
     */
    bool setMeasurementUnit(const QString& unit);

    /**
     * @brief Gets the central server URL.
     *
     * @return Server URL string
     */
    QString serverUrl() const;

    /**
     * @brief Sets the central server URL.
     *
     * @param url Server URL string
     * @return true if saved successfully
     */
    bool setServerUrl(const QString& url);

    /**
     * @brief Gets whether to use mock server.
     *
     * @return true if mock server enabled, false otherwise
     */
    bool useMockServer() const;

    /**
     * @brief Sets whether to use mock server.
     *
     * @param useMock true to enable mock server, false to disable
     * @return true if saved successfully
     */
    bool setUseMockServer(bool useMock);

signals:
    /**
     * @brief Emitted when a setting value changes.
     *
     * @param key Setting key that changed
     * @param value New setting value
     */
    void settingChanged(const QString& key, const QVariant& value);

private:
    /**
     * @brief Private constructor (singleton pattern).
     *
     * @param parent Parent QObject
     */
    explicit SettingsManager(QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~SettingsManager() override = default;

    // Disable copy and assignment
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    /**
     * @brief Initializes the settings table if it doesn't exist.
     *
     * @return true if initialization succeeded, false otherwise
     */
    bool initializeSettingsTable();

    /**
     * @brief Gets database connection (for settings table access).
     *
     * @return Database connection name or empty if not available
     */
    QString getDatabaseConnection() const;

    // Singleton instance
    static SettingsManager* s_instance;
};

} // namespace zmon
} // namespace zmon