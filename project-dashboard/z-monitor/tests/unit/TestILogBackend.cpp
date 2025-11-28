/**
 * @file TestILogBackend.cpp
 * @brief Unit tests for ILogBackend interface using a simple mock backend.
 *
 * These tests verify that a concrete implementation of ILogBackend can be
 * constructed and invoked with a LogEntry. The tests use basic assertions
 * instead of a full testing framework to keep the dependency surface small.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <cassert>
#include <iostream>

#include <QString>

#include "../../src/infrastructure/logging/ILogBackend.h"

using ZMonitor::Infrastructure::Logging::ILogBackend;
using ZMonitor::Infrastructure::Logging::LogEntry;
using ZMonitor::Infrastructure::Logging::LogLevel;

/**
 * @class MockLogBackend
 * @brief Simple in-memory ILogBackend implementation for testing.
 *
 * This mock implementation records the last log entry it received and
 * tracks how many times each method was called. It does not perform any
 * file I/O and is suitable for unit tests.
 */
class MockLogBackend : public ILogBackend {
public:
    bool initialized{false};
    int writeCount{0};
    int flushCount{0};
    int rotateCount{0};
    QString lastFormat;
    qint64 lastMaxSize{0};
    int lastMaxFiles{0};
    LogEntry lastEntry;

    bool initialize(const QString& logDir, const QString& logFileName) override
    {
        initialized = !logDir.isEmpty() && !logFileName.isEmpty();
        return initialized;
    }

    void write(const LogEntry& entry) override
    {
        ++writeCount;
        lastEntry = entry;
    }

    void flush() override
    {
        ++flushCount;
    }

    void rotateIfNeeded() override
    {
        ++rotateCount;
    }

    void setFormat(const QString& format) override
    {
        lastFormat = format;
    }

    void setMaxFileSize(qint64 maxSizeBytes) override
    {
        lastMaxSize = maxSizeBytes;
    }

    void setMaxFiles(int maxFiles) override
    {
        lastMaxFiles = maxFiles;
    }
};

/**
 * @brief Exercise initialization and configuration methods.
 */
void testInitializationAndConfiguration()
{
    MockLogBackend backend;

    bool ok = backend.initialize(QStringLiteral("/tmp"), QStringLiteral("z-monitor"));
    assert(ok);
    assert(backend.initialized);

    backend.setFormat(QStringLiteral("json"));
    backend.setMaxFileSize(1024 * 1024);
    backend.setMaxFiles(5);

    assert(backend.lastFormat == QStringLiteral("json"));
    assert(backend.lastMaxSize == 1024 * 1024);
    assert(backend.lastMaxFiles == 5);

    std::cout << "✓ ILogBackend initialization and configuration test passed\n";
}

/**
 * @brief Exercise write, flush, and rotate methods.
 */
void testWriteFlushRotate()
{
    MockLogBackend backend;

    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTimeUtc();
    entry.level = LogLevel::Info;
    entry.category = QStringLiteral("test");
    entry.message = QStringLiteral("Test message");
    entry.threadId = QStringLiteral("thread-1");
    entry.file = QStringLiteral("TestILogBackend.cpp");
    entry.line = 123;
    entry.function = QStringLiteral("testWriteFlushRotate");

    backend.write(entry);
    backend.flush();
    backend.rotateIfNeeded();

    assert(backend.writeCount == 1);
    assert(backend.flushCount == 1);
    assert(backend.rotateCount == 1);
    assert(backend.lastEntry.message == QStringLiteral("Test message"));

    std::cout << "✓ ILogBackend write/flush/rotate test passed\n";
}

/**
 * @brief Main entry point for the ILogBackend unit tests.
 */
int main()
{
    std::cout << "Running ILogBackend unit tests...\n";

    testInitializationAndConfiguration();
    testWriteFlushRotate();

    std::cout << "All ILogBackend tests passed.\n";
    return 0;
}


