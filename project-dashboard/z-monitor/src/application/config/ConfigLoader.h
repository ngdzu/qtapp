/**
 * @file ConfigLoader.h
 * @brief Loads application configuration from multiple sources with priority.
 *
 * Configuration loading follows a strict precedence order:
 * 1. **Environment Variables** (highest priority)
 * 2. **Configuration File** (config.ini in platform-specific config directory)
 * 3. **Default Values** (lowest priority)
 *
 * Supported environment variables:
 * - `ZMON_DB_PATH`: Database file path
 * - `ZMON_SENSOR_MODE`: Sensor source mode ("in_memory" or "shared_memory")
 * - `ZMON_SENSOR_SHARED_SOCKET`: Shared memory socket path
 * - `ZMON_CACHE_VITALS_SECONDS`: Vitals cache window in seconds
 * - `ZMON_CACHE_WAVEFORM_SAMPLES`: Waveform cache sample capacity
 * - `ZMON_LOG_LEVEL`: Logging level ("debug", "info", "warning", "error")
 *
 * @see ConfigConstants.h for all configuration key names
 * @see AppConfig for configuration data model
 */
#pragma once

#include "application/config/AppConfig.h"
#include <QString>

namespace zmon
{

    /**
     * @class ConfigLoader
     * @brief Utility to load and initialize configuration values from multiple sources.
     *
     * Implements a three-tier configuration system with environment variable support
     * for deployment flexibility (Docker, CI/CD, production environments).
     */
    class ConfigLoader
    {
    public:
        /**
         * @brief Load configuration from all available sources.
         *
         * Searches for configuration in the following order:
         * 1. Environment variables (e.g., ZMON_DB_PATH)
         * 2. Configuration file (config.ini in standard app config directory)
         * 3. Hardcoded default values
         *
         * If no configuration file exists, creates one with default values.
         * Environment variables always override file and default settings.
         *
         * @return AppConfig Populated configuration struct ready for dependency injection
         */
        static AppConfig load();
    };

} // namespace zmon
