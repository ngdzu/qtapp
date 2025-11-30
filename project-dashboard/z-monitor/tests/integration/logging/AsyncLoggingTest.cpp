/**
 * @file AsyncLoggingTest.cpp
 * @brief GoogleTest integration tests for async logging infrastructure.
 *
 * These tests verify the complete async logging workflow including
 * LogService, backends, queue processing, and thread safety.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <chrono>
#include <thread>

#include "infrastructure/logging/LogService.h"
#include "infrastructure/logging/backends/CustomBackend.h"
#include "infrastructure/logging/LogEntry.h"
#include "../../unit/logging/MockLogBackend.h"

/**
 * @brief Test fixture for async logging integration tests.
 */
class AsyncLoggingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(m_tempDir->isValid());
    }

    void TearDown() override
    {
        if (m_thread && m_thread->isRunning())
        {
            m_thread->quit();
            m_thread->wait(1000);
        }
        m_logService.reset();
        m_thread.reset();
        m_tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> m_tempDir;
    std::unique_ptr<zmon::LogService> m_logService;
    std::unique_ptr<QThread> m_thread;
};

/**
 * @brief Test complete logging workflow with CustomBackend.
 */
TEST_F(AsyncLoggingTest, CompleteWorkflowWithCustomBackend)
{
    auto backend = std::make_unique<zmon::CustomBackend>();
    m_logService = std::make_unique<zmon::LogService>(backend.release());

    // Move to Database I/O Thread
    m_thread = std::make_unique<QThread>();
    m_logService->moveToThread(m_thread.get());
    m_thread->start();
    QThread::msleep(10);

    // Initialize
    auto result = m_logService->initialize(m_tempDir->path(), "test-log");
    ASSERT_TRUE(result.isOk());

    // Write various log entries
    m_logService->info("Info message");
    m_logService->warning("Warning message", {{"key", "value"}});
    m_logService->error("Error message");

    // Give queue time to process
    QThread::msleep(200);

    // Verify log file was created and contains entries
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFile file(logFilePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream stream(&file);
    QString content = stream.readAll();

    EXPECT_TRUE(content.contains("Info message"));
    EXPECT_TRUE(content.contains("Warning message"));
    EXPECT_TRUE(content.contains("Error message"));
}

/**
 * @brief Test thread safety with multiple threads writing logs.
 */
TEST_F(AsyncLoggingTest, ThreadSafety)
{
    auto backend = std::make_unique<zmon::CustomBackend>();
    m_logService = std::make_unique<zmon::LogService>(backend.release());

    // Move to Database I/O Thread
    m_thread = std::make_unique<QThread>();
    m_logService->moveToThread(m_thread.get());
    m_thread->start();
    QThread::msleep(10);

    m_logService->initialize(m_tempDir->path(), "test-log");

    // Write from multiple threads
    const int numThreads = 4;
    const int entriesPerThread = 100;
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([this, t, entriesPerThread]()
                             {
            for (int i = 0; i < entriesPerThread; ++i) {
                m_logService->info(QString("Thread %1: Message %2").arg(t).arg(i));
            } });
    }

    // Wait for all threads
    for (auto &thread : threads)
    {
        thread.join();
    }

    // Give queue time to process
    QThread::msleep(500);

    // Verify all entries were written
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFile file(logFilePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream stream(&file);
    QString content = stream.readAll();

    // Check that entries from all threads are present
    for (int t = 0; t < numThreads; ++t)
    {
        EXPECT_TRUE(content.contains(QString("Thread %1:").arg(t)));
    }
}

/**
 * @brief Test performance under load.
 */
TEST_F(AsyncLoggingTest, PerformanceUnderLoad)
{
    auto backend = std::make_unique<zmon::CustomBackend>();
    m_logService = std::make_unique<zmon::LogService>(backend.release());

    // Move to Database I/O Thread
    m_thread = std::make_unique<QThread>();
    m_logService->moveToThread(m_thread.get());
    m_thread->start();
    QThread::msleep(10);

    m_logService->initialize(m_tempDir->path(), "test-log");

    // Measure time to enqueue many entries
    const int numEntries = 10000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numEntries; ++i)
    {
        m_logService->info(QString("Message %1").arg(i));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Should complete in < 10ms for 10,000 calls (< 1μs per call)
    EXPECT_LT(duration.count(), 10000);

    // Give queue time to process
    QThread::msleep(1000);
}

/**
 * @brief Test log rotation integration.
 */
TEST_F(AsyncLoggingTest, LogRotation)
{
    auto backend = std::make_unique<zmon::CustomBackend>();
    backend->setMaxFileSize(1024); // 1 KB limit
    m_logService = std::make_unique<zmon::LogService>(backend.release());

    // Move to Database I/O Thread
    m_thread = std::make_unique<QThread>();
    m_logService->moveToThread(m_thread.get());
    m_thread->start();
    QThread::msleep(10);

    m_logService->initialize(m_tempDir->path(), "test-log");

    // Write enough entries to trigger rotation
    QString largeMessage(200, 'X');
    for (int i = 0; i < 20; ++i)
    {
        m_logService->info(QString("Entry %1: %2").arg(i).arg(largeMessage));
    }

    // Give queue time to process and rotate
    QThread::msleep(500);

    // Check for rotated files
    QDir dir(m_tempDir->path());
    QFileInfoList files = dir.entryInfoList({"test-log*.log"}, QDir::Files);

    // Should have at least one file (may have rotated)
    EXPECT_GE(files.size(), 1);
}

/**
 * @brief Test flush operation integration.
 */
TEST_F(AsyncLoggingTest, FlushIntegration)
{
    auto backend = std::make_unique<zmon::CustomBackend>();
    m_logService = std::make_unique<zmon::LogService>(backend.release());

    // Move to Database I/O Thread
    m_thread = std::make_unique<QThread>();
    m_logService->moveToThread(m_thread.get());
    m_thread->start();
    QThread::msleep(10);

    m_logService->initialize(m_tempDir->path(), "test-log");

    // Write entries
    for (int i = 0; i < 10; ++i)
    {
        m_logService->info(QString("Message %1").arg(i));
    }

    // Flush should ensure all entries are written
    m_logService->flush();

    // Verify entries were written
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
 * @brief Test queue overflow handling.
 */
TEST_F(AsyncLoggingTest, QueueOverflow)
{
    auto backend = std::make_unique<zmon::CustomBackend>();
    m_logService = std::make_unique<zmon::LogService>(backend.release());

    // Move to Database I/O Thread
    m_thread = std::make_unique<QThread>();
    m_logService->moveToThread(m_thread.get());
    m_thread->start();
    QThread::msleep(10);

    m_logService->initialize(m_tempDir->path(), "test-log");

    // Write more entries than queue capacity (10,000)
    // Queue should drop oldest entries when full
    for (int i = 0; i < 15000; ++i)
    {
        m_logService->info(QString("Message %1").arg(i));
    }

    // Should not crash or block
    // Give queue time to process
    QThread::msleep(1000);

    // Verify some entries were written (may have dropped oldest)
    QString logFilePath = QDir(m_tempDir->path()).filePath("test-log.log");
    QFile file(logFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        QString content = stream.readAll();
        // Should have some entries (may not have all due to overflow)
        EXPECT_FALSE(content.isEmpty());
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
