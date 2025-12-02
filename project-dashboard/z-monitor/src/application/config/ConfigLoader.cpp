/**
 * @file ConfigLoader.cpp
 */

#include "application/config/ConfigLoader.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

namespace zmon
{

    static QString defaultDbPath()
    {
        const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        return base + "/zmonitor.db";
    }

    static QString defaultSocketPath()
    {
        return "/tmp/z-monitor-sensor.sock";
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
            settings.beginGroup("database");
            settings.setValue("path", defaultDbPath());
            settings.endGroup();

            settings.beginGroup("sensor");
            settings.setValue("mode", "in_memory"); // in_memory | shared_memory
            settings.setValue("shared_socket", defaultSocketPath());
            settings.endGroup();

            settings.beginGroup("cache");
            settings.setValue("vitals_seconds", 259200);  // 3 days @ 60 Hz
            settings.setValue("waveform_samples", 22500); // 30s @ 250Hz × 3 ch
            settings.endGroup();
            settings.sync();
        }

        // Read values
        AppConfig cfg;
        settings.beginGroup("database");
        cfg.databasePath = settings.value("path", defaultDbPath()).toString();
        settings.endGroup();

        settings.beginGroup("sensor");
        const QString mode = settings.value("mode", "in_memory").toString();
        cfg.sensorSource = (mode == "shared_memory") ? SensorSourceMode::SharedMemory : SensorSourceMode::InMemory;
        cfg.sharedMemorySocket = settings.value("shared_socket", defaultSocketPath()).toString();
        settings.endGroup();

        settings.beginGroup("cache");
        cfg.vitalsCacheSeconds = settings.value("vitals_seconds", 259200).toInt();
        cfg.waveformCacheSamples = settings.value("waveform_samples", 22500).toInt();
        settings.endGroup();

        return cfg;
    }

} // namespace zmon
