/**
 * @file ConfigLoaderTest.cpp
 * @brief Unit tests for ConfigLoader with environment variable precedence.
 */

#include <gtest/gtest.h>
#include "application/config/ConfigLoader.h"
#include "application/config/ConfigConstants.h"
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <cstdlib>

using namespace zmon;

/**
 * @brief Test fixture for ConfigLoader tests.
 *
 * Manages test environment setup and cleanup, including:
 * - Clearing environment variables before each test
 * - Cleaning up test configuration files
 */
class ConfigLoaderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Clear all environment variables before each test
        clearAllEnvVars();

        // Remove any existing test config file
        const QString configPath = getConfigFilePath();
        if (QFile::exists(configPath))
        {
            QFile::remove(configPath);
        }
    }

    void TearDown() override
    {
        // Clean up environment variables
        clearAllEnvVars();

        // Clean up test config file
        const QString configPath = getConfigFilePath();
        if (QFile::exists(configPath))
        {
            QFile::remove(configPath);
        }
    }

    QString getConfigFilePath()
    {
        const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        return configDir + "/config.ini";
    }

    void clearAllEnvVars()
    {
        qunsetenv(config::env::DB_PATH);
        qunsetenv(config::env::SENSOR_MODE);
        qunsetenv(config::env::SENSOR_SHARED_SOCKET);
        qunsetenv(config::env::CACHE_VITALS_SECONDS);
        qunsetenv(config::env::CACHE_WAVEFORM_SAMPLES);
        qunsetenv(config::env::LOG_LEVEL);
    }

    void createTestConfigFile(const QString &dbPath,
                              const QString &sensorMode,
                              const QString &socket,
                              int vitalsSeconds,
                              int waveformSamples,
                              const QString &logLevel)
    {
        const QString configPath = getConfigFilePath();
        const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

        QDir dir(configDir);
        if (!dir.exists())
        {
            dir.mkpath(".");
        }

        QSettings settings(configPath, QSettings::IniFormat);

        settings.beginGroup(config::sections::DATABASE);
        settings.setValue(config::keys::DB_PATH, dbPath);
        settings.endGroup();

        settings.beginGroup(config::sections::SENSOR);
        settings.setValue(config::keys::SENSOR_MODE, sensorMode);
        settings.setValue(config::keys::SENSOR_SHARED_SOCKET, socket);
        settings.endGroup();

        settings.beginGroup(config::sections::CACHE);
        settings.setValue(config::keys::CACHE_VITALS_SECONDS, vitalsSeconds);
        settings.setValue(config::keys::CACHE_WAVEFORM_SAMPLES, waveformSamples);
        settings.endGroup();

        settings.beginGroup(config::sections::LOGGING);
        settings.setValue(config::keys::LOG_LEVEL, logLevel);
        settings.endGroup();

        settings.sync();
    }
};

/**
 * @brief Test loading configuration with default values only.
 *
 * Verifies that when no config file or environment variables exist,
 * ConfigLoader creates a default config file and returns sensible defaults.
 */
TEST_F(ConfigLoaderTest, LoadWithDefaults)
{
    AppConfig cfg = ConfigLoader::load();

    // Verify default values are populated
    EXPECT_FALSE(cfg.databasePath.isEmpty());
    EXPECT_TRUE(cfg.databasePath.contains("zmonitor.db"));
    EXPECT_EQ(cfg.sensorSource, SensorSourceMode::InMemory);
    EXPECT_EQ(cfg.vitalsCacheSeconds, config::defaults::CACHE_VITALS_SECONDS_DEFAULT);
    EXPECT_EQ(cfg.waveformCacheSamples, config::defaults::CACHE_WAVEFORM_SAMPLES_DEFAULT);
    EXPECT_EQ(cfg.logLevel, LogLevel::Info);

    // Verify config file was created
    EXPECT_TRUE(QFile::exists(getConfigFilePath()));
}

/**
 * @brief Test loading configuration from file (no environment variables).
 *
 * Verifies that ConfigLoader reads values from the config.ini file
 * when environment variables are not set.
 */
TEST_F(ConfigLoaderTest, LoadFromFile)
{
    createTestConfigFile(
        "/custom/path/db.sqlite",
        "shared_memory",
        "/custom/socket.sock",
        100000,
        50000,
        "debug");

    AppConfig cfg = ConfigLoader::load();

    EXPECT_EQ(cfg.databasePath, "/custom/path/db.sqlite");
    EXPECT_EQ(cfg.sensorSource, SensorSourceMode::SharedMemory);
    EXPECT_EQ(cfg.sharedMemorySocket, "/custom/socket.sock");
    EXPECT_EQ(cfg.vitalsCacheSeconds, 100000);
    EXPECT_EQ(cfg.waveformCacheSamples, 50000);
    EXPECT_EQ(cfg.logLevel, LogLevel::Debug);
}

/**
 * @brief Test environment variable override of file configuration.
 *
 * Verifies that environment variables have highest priority and
 * override values from the config file.
 */
TEST_F(ConfigLoaderTest, EnvironmentVariableOverride)
{
    // Create config file with one set of values
    createTestConfigFile(
        "/file/path/db.sqlite",
        "in_memory",
        "/file/socket.sock",
        999,
        888,
        "info");

    // Set environment variables with different values
    qputenv(config::env::DB_PATH, "/env/path/db.sqlite");
    qputenv(config::env::SENSOR_MODE, "shared_memory");
    qputenv(config::env::SENSOR_SHARED_SOCKET, "/env/socket.sock");
    qputenv(config::env::CACHE_VITALS_SECONDS, "123456");
    qputenv(config::env::CACHE_WAVEFORM_SAMPLES, "78900");
    qputenv(config::env::LOG_LEVEL, "error");

    AppConfig cfg = ConfigLoader::load();

    // Verify environment variables take precedence
    EXPECT_EQ(cfg.databasePath, "/env/path/db.sqlite");
    EXPECT_EQ(cfg.sensorSource, SensorSourceMode::SharedMemory);
    EXPECT_EQ(cfg.sharedMemorySocket, "/env/socket.sock");
    EXPECT_EQ(cfg.vitalsCacheSeconds, 123456);
    EXPECT_EQ(cfg.waveformCacheSamples, 78900);
    EXPECT_EQ(cfg.logLevel, LogLevel::Error);
}

/**
 * @brief Test partial environment variable override.
 *
 * Verifies that when only some environment variables are set,
 * they override file values while other values come from file.
 */
TEST_F(ConfigLoaderTest, PartialEnvironmentOverride)
{
    createTestConfigFile(
        "/file/path/db.sqlite",
        "in_memory",
        "/file/socket.sock",
        999,
        888,
        "warning");

    // Only override database path via environment
    qputenv(config::env::DB_PATH, "/env/override.db");
    qputenv(config::env::LOG_LEVEL, "debug");

    AppConfig cfg = ConfigLoader::load();

    // Env vars take precedence
    EXPECT_EQ(cfg.databasePath, "/env/override.db");
    EXPECT_EQ(cfg.logLevel, LogLevel::Debug);

    // File values used for non-overridden settings
    EXPECT_EQ(cfg.sensorSource, SensorSourceMode::InMemory);
    EXPECT_EQ(cfg.sharedMemorySocket, "/file/socket.sock");
    EXPECT_EQ(cfg.vitalsCacheSeconds, 999);
    EXPECT_EQ(cfg.waveformCacheSamples, 888);
}

/**
 * @brief Test log level parsing for all valid values.
 */
TEST_F(ConfigLoaderTest, LogLevelParsing)
{
    // Test "debug"
    qputenv(config::env::LOG_LEVEL, "debug");
    EXPECT_EQ(ConfigLoader::load().logLevel, LogLevel::Debug);

    // Test "info"
    qputenv(config::env::LOG_LEVEL, "info");
    EXPECT_EQ(ConfigLoader::load().logLevel, LogLevel::Info);

    // Test "warning"
    qputenv(config::env::LOG_LEVEL, "warning");
    EXPECT_EQ(ConfigLoader::load().logLevel, LogLevel::Warning);

    // Test "error"
    qputenv(config::env::LOG_LEVEL, "error");
    EXPECT_EQ(ConfigLoader::load().logLevel, LogLevel::Error);

    // Test case insensitivity
    qputenv(config::env::LOG_LEVEL, "DEBUG");
    EXPECT_EQ(ConfigLoader::load().logLevel, LogLevel::Debug);

    qputenv(config::env::LOG_LEVEL, "WaRnInG");
    EXPECT_EQ(ConfigLoader::load().logLevel, LogLevel::Warning);

    // Test invalid value defaults to Info
    qputenv(config::env::LOG_LEVEL, "invalid");
    EXPECT_EQ(ConfigLoader::load().logLevel, LogLevel::Info);
}

/**
 * @brief Test sensor mode parsing.
 */
TEST_F(ConfigLoaderTest, SensorModeParsing)
{
    // Test "in_memory"
    qputenv(config::env::SENSOR_MODE, "in_memory");
    EXPECT_EQ(ConfigLoader::load().sensorSource, SensorSourceMode::InMemory);

    // Test "shared_memory"
    qputenv(config::env::SENSOR_MODE, "shared_memory");
    EXPECT_EQ(ConfigLoader::load().sensorSource, SensorSourceMode::SharedMemory);

    // Test invalid value defaults to InMemory
    qputenv(config::env::SENSOR_MODE, "invalid_mode");
    EXPECT_EQ(ConfigLoader::load().sensorSource, SensorSourceMode::InMemory);
}

/**
 * @brief Test invalid integer environment variable handling.
 *
 * Verifies that invalid integer env vars fall back to file or default values.
 */
TEST_F(ConfigLoaderTest, InvalidIntegerEnvironmentVariable)
{
    createTestConfigFile(
        "/test.db",
        "in_memory",
        "/test.sock",
        5000,
        10000,
        "info");

    // Set invalid integer environment variables
    qputenv(config::env::CACHE_VITALS_SECONDS, "not_a_number");
    qputenv(config::env::CACHE_WAVEFORM_SAMPLES, "also_invalid");

    AppConfig cfg = ConfigLoader::load();

    // Should fall back to file values (not defaults)
    EXPECT_EQ(cfg.vitalsCacheSeconds, 5000);
    EXPECT_EQ(cfg.waveformCacheSamples, 10000);
}

/**
 * @brief Test configuration precedence order: Env > File > Default.
 *
 * Comprehensive test verifying the full precedence chain.
 */
TEST_F(ConfigLoaderTest, PrecedenceOrder)
{
    // Start with defaults (no file, no env vars)
    AppConfig cfg1 = ConfigLoader::load();
    const QString defaultDbPath = cfg1.databasePath;
    EXPECT_FALSE(defaultDbPath.isEmpty());

    // Clear for next test
    TearDown();
    SetUp();

    // Create file with custom values
    createTestConfigFile(
        "/file/custom.db",
        "shared_memory",
        "/file/socket",
        7777,
        8888,
        "warning");

    AppConfig cfg2 = ConfigLoader::load();
    EXPECT_EQ(cfg2.databasePath, "/file/custom.db");
    EXPECT_EQ(cfg2.logLevel, LogLevel::Warning);

    // Now override with environment variable
    qputenv(config::env::DB_PATH, "/env/override.db");
    qputenv(config::env::LOG_LEVEL, "error");

    AppConfig cfg3 = ConfigLoader::load();
    EXPECT_EQ(cfg3.databasePath, "/env/override.db");
    EXPECT_EQ(cfg3.logLevel, LogLevel::Error);

    // File values still used for non-overridden items
    EXPECT_EQ(cfg3.vitalsCacheSeconds, 7777);
    EXPECT_EQ(cfg3.waveformCacheSamples, 8888);
}
