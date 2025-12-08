/**
 * @file AppConfig.h
 * @brief Application configuration model (DI inputs).
 */
#pragma once

#include <QString>

namespace zmon
{

    /**
     * @brief Sensor source selection.
     */
    enum class SensorSourceMode
    {
        InMemory,
        SharedMemory
    };

    /**
     * @brief Logging level enumeration.
     */
    enum class LogLevel
    {
        Debug,   ///< Verbose debug information
        Info,    ///< Informational messages (default)
        Warning, ///< Warning messages
        Error    ///< Error messages only
    };

    /**
     * @brief Application configuration values.
     */
    struct AppConfig
    {
        QString databasePath;          ///< Absolute path to SQLite DB file
        SensorSourceMode sensorSource; ///< Sensor source mode
        QString sharedMemorySocket;    ///< Socket path for shared memory sensor source
        int vitalsCacheSeconds;        ///< Vitals cache window in seconds
        int waveformCacheSamples;      ///< Waveform cache sample capacity
        LogLevel logLevel;             ///< Application logging level
    };

} // namespace zmon
