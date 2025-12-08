/**
 * @file ConfigLoader.cpp
 */

#include "application/config/ConfigLoader.h"
#include "application/config/ConfigConstants.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace zmon
{

    /**
     * @brief Get default database path (platform-specific).
     */
    static QString defaultDbPath()
    {
        const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        return base + "/zmonitor.db";
    }

    /**
     * @brief Get default socket path for shared memory sensor source.
     */
    static QString defaultSocketPath()
    {
        return "/tmp/z-monitor-sensor.sock";
    }

    /**
     * @brief Parse log level string to enum.
     * @param levelStr String representation ("debug", "info", "warning", "error")
     * @return LogLevel enum value (defaults to Info if invalid)
     */
    static LogLevel parseLogLevel(const QString &levelStr)
    {
        const QString lower = levelStr.toLower();
        if (lower == config::defaults::LOG_LEVEL_DEBUG)
            return LogLevel::Debug;
        if (lower == config::defaults::LOG_LEVEL_WARNING)
            return LogLevel::Warning;
        if (lower == config::defaults::LOG_LEVEL_ERROR)
            return LogLevel::Error;
        return LogLevel::Info; // Default
    }

    /**
     * @brief Read value from environment variable, fallback to QSettings, then default.
     * @param envVarName Environment variable name
     * @param settings QSettings instance
     * @param settingsKey Key within current group
     * @param defaultValue Default value if neither env nor settings provide a value
     * @return Resolved value (priority: Env > File > Default)
     */
    static QString getConfigValue(const char *envVarName,
                                  QSettings &settings,
                                  const char *settingsKey,
                                  const QString &defaultValue)
    {
        // Priority 1: Environment variable
        const QByteArray envValue = qgetenv(envVarName);
        if (!envValue.isEmpty())
        {
            qInfo() << "Config: Using environment variable" << envVarName << "=" << envValue;
            return QString::fromUtf8(envValue);
        }

        // Priority 2: Settings file
        if (settings.contains(settingsKey))
        {
            return settings.value(settingsKey).toString();
        }

        // Priority 3: Default value
        return defaultValue;
    }

    /**
     * @brief Read integer value from environment variable, fallback to QSettings, then default.
     */
    static int getConfigValueInt(const char *envVarName,
                                 QSettings &settings,
                                 const char *settingsKey,
                                 int defaultValue)
    {
        // Priority 1: Environment variable
        const QByteArray envValue = qgetenv(envVarName);
        if (!envValue.isEmpty())
        {
            bool ok = false;
            const int value = envValue.toInt(&ok);
            if (ok)
            {
                qInfo() << "Config: Using environment variable" << envVarName << "=" << value;
                return value;
            }
            qWarning() << "Config: Invalid integer in environment variable" << envVarName << "=" << envValue;
        }

        // Priority 2: Settings file
        if (settings.contains(settingsKey))
        {
            return settings.value(settingsKey).toInt();
        }

        // Priority 3: Default value
        return defaultValue;
    }

    AppConfig ConfigLoader::load()
    {
        // Resolve config directory and file
        const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        const QString filePath = configDir + "/config.ini";

        // Ensure directory exists
        QDir dir(configDir);
        if (!dir.exists())
        {
            dir.mkpath(".");
        }

        // Initialize QSettings (INI format)
        QSettings settings(filePath, QSettings::IniFormat);

        // Write defaults on first run
        if (!QFileInfo::exists(filePath))
        {
            settings.beginGroup(config::sections::DATABASE);
            settings.setValue(config::keys::DB_PATH, defaultDbPath());
            settings.endGroup();

            settings.beginGroup(config::sections::SENSOR);
            settings.setValue(config::keys::SENSOR_MODE, config::defaults::SENSOR_MODE_IN_MEMORY);
            settings.setValue(config::keys::SENSOR_SHARED_SOCKET, defaultSocketPath());
            settings.endGroup();

            settings.beginGroup(config::sections::CACHE);
            settings.setValue(config::keys::CACHE_VITALS_SECONDS, config::defaults::CACHE_VITALS_SECONDS_DEFAULT);
            settings.setValue(config::keys::CACHE_WAVEFORM_SAMPLES, config::defaults::CACHE_WAVEFORM_SAMPLES_DEFAULT);
            settings.endGroup();

            settings.beginGroup(config::sections::LOGGING);
            settings.setValue(config::keys::LOG_LEVEL, config::defaults::LOG_LEVEL_INFO);
            settings.endGroup();

            settings.sync();
            qInfo() << "Config: Created default configuration file:" << filePath;
        }

        // Read configuration values with priority: Env > File > Default
        AppConfig cfg;

        // Database configuration
        settings.beginGroup(config::sections::DATABASE);
        cfg.databasePath = getConfigValue(
            config::env::DB_PATH,
            settings,
            config::keys::DB_PATH,
            defaultDbPath());
        settings.endGroup();

        // Sensor configuration
        settings.beginGroup(config::sections::SENSOR);
        const QString mode = getConfigValue(
            config::env::SENSOR_MODE,
            settings,
            config::keys::SENSOR_MODE,
            config::defaults::SENSOR_MODE_IN_MEMORY);
        cfg.sensorSource = (mode == config::defaults::SENSOR_MODE_SHARED_MEMORY)
                               ? SensorSourceMode::SharedMemory
                               : SensorSourceMode::InMemory;

        cfg.sharedMemorySocket = getConfigValue(
            config::env::SENSOR_SHARED_SOCKET,
            settings,
            config::keys::SENSOR_SHARED_SOCKET,
            defaultSocketPath());
        settings.endGroup();

        // Cache configuration
        settings.beginGroup(config::sections::CACHE);
        cfg.vitalsCacheSeconds = getConfigValueInt(
            config::env::CACHE_VITALS_SECONDS,
            settings,
            config::keys::CACHE_VITALS_SECONDS,
            config::defaults::CACHE_VITALS_SECONDS_DEFAULT);

        cfg.waveformCacheSamples = getConfigValueInt(
            config::env::CACHE_WAVEFORM_SAMPLES,
            settings,
            config::keys::CACHE_WAVEFORM_SAMPLES,
            config::defaults::CACHE_WAVEFORM_SAMPLES_DEFAULT);
        settings.endGroup();

        // Logging configuration
        settings.beginGroup(config::sections::LOGGING);
        const QString logLevelStr = getConfigValue(
            config::env::LOG_LEVEL,
            settings,
            config::keys::LOG_LEVEL,
            config::defaults::LOG_LEVEL_INFO);
        cfg.logLevel = parseLogLevel(logLevelStr);
        settings.endGroup();

        qInfo() << "Config: Loaded configuration - DB:" << cfg.databasePath
                << "Sensor:" << (cfg.sensorSource == SensorSourceMode::SharedMemory ? "SharedMemory" : "InMemory")
                << "LogLevel:" << logLevelStr;

        return cfg;
    }

} // namespace zmon
