/**
 * @file ConfigLoader.h
 * @brief Loads application configuration from disk (QSettings INI).
 */
#pragma once

#include "application/config/AppConfig.h"
#include <QString>

namespace zmon
{

    /**
     * @class ConfigLoader
     * @brief Utility to load and initialize configuration values.
     */
    class ConfigLoader
    {
    public:
        /**
         * @brief Load configuration from default location.
         *
         * Searches standard app config paths for `config.ini`. If not found,
         * creates a default configuration file. Returns populated AppConfig.
         */
        static AppConfig load();
    };

} // namespace zmon
