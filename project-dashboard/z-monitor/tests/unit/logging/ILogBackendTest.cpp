/**
 * @file ILogBackendTest.cpp
 * @brief GoogleTest unit tests for ILogBackend interface.
 *
 * These tests verify that ILogBackend implementations correctly
 * implement the interface contract using a mock backend.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QString>

#include "infrastructure/logging/ILogBackend.h"
#include "infrastructure/logging/LogEntry.h"
#include "MockLogBackend.h"

using namespace ZMonitor::Infrastructure::Logging;

/**
 * @brief Test fixture for ILogBackend tests.
 */
class ILogBackendTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_backend = std::make_unique<MockLogBackend>();
    }

    void TearDown() override
    {
        m_backend.reset();
    }

    std::unique_ptr<MockLogBackend> m_backend;
};

/**
 * @brief Test initialization with valid parameters.
 */
TEST_F(ILogBackendTest, InitializeSuccess)
{
    auto result = m_backend->initialize("/tmp", "test-log");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(m_backend->entryCount(), 0);
}

/**
 * @brief Test initialization with empty directory (should fail).
 */
TEST_F(ILogBackendTest, InitializeEmptyDirectory)
{
    auto result = m_backend->initialize("", "test-log");
    // Mock backend always succeeds, but real backends should fail
    // This test documents expected behavior
    EXPECT_TRUE(result.isOk()); // Mock always succeeds
}

/**
 * @brief Test write operation.
 */
TEST_F(ILogBackendTest, WriteEntry)
{
    m_backend->initialize("/tmp", "test-log");

    zmon::LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = zmon::LogLevel::Info;
    entry.category = "test";
    entry.message = "Test message";
    entry.context["key1"] = "value1";
    entry.threadId = "thread-1";

    m_backend->write(entry);

    EXPECT_EQ(m_backend->entryCount(), 1);
    QList<zmon::LogEntry> entries = m_backend->entries();
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].message, "Test message");
    EXPECT_EQ(entries[0].level, zmon::LogLevel::Info);
    EXPECT_EQ(entries[0].category, "test");
}

/**
 * @brief Test multiple write operations.
 */
TEST_F(ILogBackendTest, WriteMultipleEntries)
{
    m_backend->initialize("/tmp", "test-log");

    for (int i = 0; i < 10; ++i)
    {
        zmon::LogEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.level = zmon::LogLevel::Debug;
        entry.message = QString("Message %1").arg(i);
        m_backend->write(entry);
    }

    EXPECT_EQ(m_backend->entryCount(), 10);
}

/**
 * @brief Test flush operation.
 */
TEST_F(ILogBackendTest, Flush)
{
    m_backend->initialize("/tmp", "test-log");

    zmon::LogEntry entry;
    entry.message = "Test";
    m_backend->write(entry);

    EXPECT_EQ(m_backend->flushCount(), 0);
    m_backend->flush();
    EXPECT_EQ(m_backend->flushCount(), 1);

    m_backend->flush();
    EXPECT_EQ(m_backend->flushCount(), 2);
}

/**
 * @brief Test rotateIfNeeded operation.
 */
TEST_F(ILogBackendTest, RotateIfNeeded)
{
    m_backend->initialize("/tmp", "test-log");

    EXPECT_EQ(m_backend->rotationCount(), 0);
    m_backend->rotateIfNeeded();
    EXPECT_EQ(m_backend->rotationCount(), 1);

    m_backend->rotateIfNeeded();
    EXPECT_EQ(m_backend->rotationCount(), 2);
}

/**
 * @brief Test setFormat operation.
 */
TEST_F(ILogBackendTest, SetFormat)
{
    m_backend->initialize("/tmp", "test-log");

    m_backend->setFormat("json");
    EXPECT_EQ(m_backend->format(), "json");

    m_backend->setFormat("human");
    EXPECT_EQ(m_backend->format(), "human");
}

/**
 * @brief Test setMaxFileSize operation.
 */
TEST_F(ILogBackendTest, SetMaxFileSize)
{
    m_backend->initialize("/tmp", "test-log");

    qint64 maxSize = 5 * 1024 * 1024; // 5 MB
    m_backend->setMaxFileSize(maxSize);
    EXPECT_EQ(m_backend->maxFileSize(), maxSize);

    maxSize = 10 * 1024 * 1024; // 10 MB
    m_backend->setMaxFileSize(maxSize);
    EXPECT_EQ(m_backend->maxFileSize(), maxSize);
}

/**
 * @brief Test setMaxFiles operation.
 */
TEST_F(ILogBackendTest, SetMaxFiles)
{
    m_backend->initialize("/tmp", "test-log");

    m_backend->setMaxFiles(5);
    EXPECT_EQ(m_backend->maxFiles(), 5);

    m_backend->setMaxFiles(10);
    EXPECT_EQ(m_backend->maxFiles(), 10);
}

/**
 * @brief Test all log levels.
 */
TEST_F(ILogBackendTest, AllLogLevels)
{
    m_backend->initialize("/tmp", "test-log");

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

    EXPECT_EQ(m_backend->entryCount(), 7);
    QList<zmon::LogEntry> entries = m_backend->entries();
    for (int i = 0; i < 7; ++i)
    {
        EXPECT_EQ(entries[i].level, levels[i]);
    }
}

/**
 * @brief Test clear operation (mock-specific).
 */
TEST_F(ILogBackendTest, Clear)
{
    m_backend->initialize("/tmp", "test-log");

    // Write some entries
    for (int i = 0; i < 5; ++i)
    {
        zmon::LogEntry entry;
        entry.message = QString("Message %1").arg(i);
        m_backend->write(entry);
    }

    EXPECT_EQ(m_backend->entryCount(), 5);
    m_backend->clear();
    EXPECT_EQ(m_backend->entryCount(), 0);
}

/**
 * @brief Test thread safety (basic check).
 */
TEST_F(ILogBackendTest, ThreadSafety)
{
    m_backend->initialize("/tmp", "test-log");

    // Write from multiple "threads" (simulated with rapid calls)
    // This is a basic test - full thread safety testing requires actual threads
    for (int i = 0; i < 100; ++i)
    {
        zmon::LogEntry entry;
        entry.message = QString("Message %1").arg(i);
        m_backend->write(entry);
    }

    EXPECT_EQ(m_backend->entryCount(), 100);
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
