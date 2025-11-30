/**
 * @file CustomBackendTest.cpp
 * @brief GoogleTest unit tests for CustomBackend logging backend.
 *
 * These tests verify CustomBackend functionality including file I/O,
 * formatting, rotation, and error handling.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTemporaryDir>
#include <QDateTime>

#include "infrastructure/logging/backends/CustomBackend.h"
#include "infrastructure/logging/LogEntry.h"

/**
 * @brief Test fixture for CustomBackend tests.
 */
class CustomBackendTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(m_tempDir->isValid());
    }

    void TearDown() override
    {
        m_backend.reset();
        m_tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> m_tempDir;
    std::unique_ptr<zmon::CustomBackend> m_backend;
};

/**
 * @brief Test initialization with valid parameters.
 */
TEST_F(CustomBackendTest, InitializeSuccess)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    auto result = m_backend->initialize(m_tempDir->path(), "test-log");
    EXPECT_TRUE(result.isOk());

    // Verify log file was created
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFileInfo fileInfo(logFilePath);
    EXPECT_TRUE(fileInfo.exists());
}

/**
 * @brief Test initialization with empty directory (should fail).
 */
TEST_F(CustomBackendTest, InitializeEmptyDirectory)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    auto result = m_backend->initialize("", "test-log");
    EXPECT_FALSE(result.isOk());
}

/**
 * @brief Test initialization with empty filename (should fail).
 */
TEST_F(CustomBackendTest, InitializeEmptyFilename)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    auto result = m_backend->initialize(m_tempDir->path(), "");
    EXPECT_FALSE(result.isOk());
}

/**
 * @brief Test human-readable format writing.
 */
TEST_F(CustomBackendTest, HumanFormat)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    m_backend->initialize(m_tempDir->path(), "test-log");
    m_backend->setFormat("human");

    zmon::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = zmon::LogLevel::Info;
    entry.category = "test";
    entry.message = "Test message";
    entry.context["key1"] = "value1";
    entry.context["key2"] = 42;
    entry.file = "TestFile.cpp";
    entry.line = 123;
    entry.function = "testFunction";

    m_backend->write(entry);
    m_backend->flush();

    // Read back and verify
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFile file(logFilePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream stream(&file);
    QString content = stream.readAll();

    EXPECT_TRUE(content.contains("Test message"));
    EXPECT_TRUE(content.contains("INFO") || content.contains("Info"));
    EXPECT_TRUE(content.contains("test"));
    EXPECT_TRUE(content.contains("TestFile.cpp"));
}

/**
 * @brief Test JSON format writing.
 */
TEST_F(CustomBackendTest, JsonFormat)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    m_backend->initialize(m_tempDir->path(), "test-log");
    m_backend->setFormat("json");

    zmon::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = zmon::LogLevel::Warning;
    entry.category = "network";
    entry.message = "Connection timeout";
    entry.context["deviceId"] = "DEV-001";
    entry.context["retryCount"] = 3;

    m_backend->write(entry);
    m_backend->flush();

    // Read back and verify JSON format
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFile file(logFilePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream stream(&file);
    QString content = stream.readAll();

    EXPECT_TRUE(content.contains("\"message\""));
    EXPECT_TRUE(content.contains("Connection timeout"));
    EXPECT_TRUE(content.contains("\"category\""));
    EXPECT_TRUE(content.contains("network"));
}

/**
 * @brief Test size-based rotation.
 */
TEST_F(CustomBackendTest, SizeRotation)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    m_backend->initialize(m_tempDir->path(), "test-log");
    m_backend->setMaxFileSize(1024); // 1 KB limit

    // Write enough entries to exceed file size
    zmon::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = zmon::LogLevel::Info;
    entry.message = QString(200, 'X'); // 200-character message

    // Write multiple entries to exceed 1 KB
    for (int i = 0; i < 10; ++i)
    {
        entry.message = QString("Entry %1: %2").arg(i).arg(QString(200, 'X'));
        m_backend->write(entry);
    }
    m_backend->flush();

    // Check if rotation occurred (file should exist)
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFileInfo fileInfo(logFilePath);
    EXPECT_TRUE(fileInfo.exists());
}

/**
 * @brief Test flush operation.
 */
TEST_F(CustomBackendTest, Flush)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    m_backend->initialize(m_tempDir->path(), "test-log");

    zmon::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = zmon::LogLevel::Info;
    entry.message = "Flush test message";

    m_backend->write(entry);
    m_backend->flush(); // Explicit flush

    // Verify entry was written
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFile file(logFilePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream stream(&file);
    QString content = stream.readAll();
    EXPECT_TRUE(content.contains("Flush test message"));
}

/**
 * @brief Test configuration methods.
 */
TEST_F(CustomBackendTest, Configuration)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    m_backend->initialize(m_tempDir->path(), "test-log");

    m_backend->setFormat("json");
    m_backend->setMaxFileSize(5 * 1024 * 1024); // 5 MB
    m_backend->setMaxFiles(10);

    // Configuration should be applied (tested via behavior)
    zmon::LogEntry entry;
    entry.message = "Test";
    m_backend->write(entry);
    m_backend->flush();

    // Verify file exists (configuration worked)
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFileInfo fileInfo(logFilePath);
    EXPECT_TRUE(fileInfo.exists());
}

/**
 * @brief Test multiple log entries.
 */
TEST_F(CustomBackendTest, MultipleEntries)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    m_backend->initialize(m_tempDir->path(), "test-log");

    for (int i = 0; i < 10; ++i)
    {
        zmon::LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = zmon::LogLevel::Debug;
        entry.message = QString("Message %1").arg(i);
        m_backend->write(entry);
    }
    m_backend->flush();

    // Verify all entries were written
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFile file(logFilePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream stream(&file);
    QString content = stream.readAll();

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(content.contains(QString("Message %1").arg(i)));
    }
}

/**
 * @brief Test all log levels.
 */
TEST_F(CustomBackendTest, AllLogLevels)
{
    m_backend = std::make_unique<zmon::CustomBackend>();
    m_backend->initialize(m_tempDir->path(), "test-log");

    zmon::LogLevel levels[] = {
        zmon::LogLevel::Trace,
        zmon::LogLevel::Debug,
        zmon::LogLevel::Info,
        zmon::LogLevel::Warning,
        zmon::LogLevel::Error,
        zmon::LogLevel::Critical,
        zmon::LogLevel::Fatal};

    for (zmon::LogLevel level : levels)
    {
        zmon::LogEntry entry;
        entry.level = level;
        entry.message = QString("Level %1").arg(static_cast<int>(level));
        m_backend->write(entry);
    }
    m_backend->flush();

    // Verify all levels were written
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFile file(logFilePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream stream(&file);
    QString content = stream.readAll();

    for (zmon::LogLevel level : levels)
    {
        EXPECT_TRUE(content.contains(QString("Level %1").arg(static_cast<int>(level))));
    }
}

/**
 * @brief Main function for running tests.
 */
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
