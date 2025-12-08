/**
 * @file ConfigConstants.h
 * @brief Configuration key and environment variable name constants.
 *
 * Centralizes all configuration-related string constants to eliminate
 * hardcoded values and provide a single source of truth for configuration
 * naming conventions.
 */
#pragma once

namespace zmon
{
    namespace config
    {

        /**
         * @brief INI file section names.
         */
        namespace sections
        {
            constexpr const char *DATABASE = "database";
            constexpr const char *SENSOR = "sensor";
            constexpr const char *CACHE = "cache";
            constexpr const char *LOGGING = "logging";
        } // namespace sections

        /**
         * @brief INI file key names within sections.
         */
        namespace keys
        {
            // Database section keys
            constexpr const char *DB_PATH = "path";

            // Sensor section keys
            constexpr const char *SENSOR_MODE = "mode";
            constexpr const char *SENSOR_SHARED_SOCKET = "shared_socket";

            // Cache section keys
            constexpr const char *CACHE_VITALS_SECONDS = "vitals_seconds";
            constexpr const char *CACHE_WAVEFORM_SAMPLES = "waveform_samples";

            // Logging section keys
            constexpr const char *LOG_LEVEL = "level";
        } // namespace keys

        /**
         * @brief Environment variable names (highest priority).
         *
         * Environment variables follow the convention: ZMON_<SECTION>_<KEY>
         * and override both file-based and default configuration values.
         */
        namespace env
        {
            constexpr const char *DB_PATH = "ZMON_DB_PATH";
            constexpr const char *SENSOR_MODE = "ZMON_SENSOR_MODE";
            constexpr const char *SENSOR_SHARED_SOCKET = "ZMON_SENSOR_SHARED_SOCKET";
            constexpr const char *CACHE_VITALS_SECONDS = "ZMON_CACHE_VITALS_SECONDS";
            constexpr const char *CACHE_WAVEFORM_SAMPLES = "ZMON_CACHE_WAVEFORM_SAMPLES";
            constexpr const char *LOG_LEVEL = "ZMON_LOG_LEVEL";
        } // namespace env

        /**
         * @brief Default configuration values.
         */
        namespace defaults
        {
            constexpr const char *SENSOR_MODE_IN_MEMORY = "in_memory";
            constexpr const char *SENSOR_MODE_SHARED_MEMORY = "shared_memory";
            constexpr const char *LOG_LEVEL_INFO = "info";
            constexpr const char *LOG_LEVEL_DEBUG = "debug";
            constexpr const char *LOG_LEVEL_WARNING = "warning";
            constexpr const char *LOG_LEVEL_ERROR = "error";

            constexpr int CACHE_VITALS_SECONDS_DEFAULT = 259200;  // 3 days @ 60 Hz
            constexpr int CACHE_WAVEFORM_SAMPLES_DEFAULT = 22500; // 30s @ 250Hz × 3 ch
        } // namespace defaults

    } // namespace config
} // namespace zmon
