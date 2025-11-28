/**
 * @file MockLogBackend.h
 * @brief Mock implementation of ILogBackend for testing.
 *
 * This file contains a mock backend implementation used for testing
 * LogService and other components that depend on ILogBackend.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "infrastructure/logging/ILogBackend.h"
#include "infrastructure/logging/LogEntry.h"
#include <QList>
#include <QMutex>
#include <QString>

namespace ZMonitor {
namespace Infrastructure {
namespace Logging {

/**
 * @class MockLogBackend
 * @brief Mock implementation of ILogBackend for testing.
 *
 * This mock backend captures all log entries written to it, allowing
 * tests to verify that LogService correctly calls the backend and
 * that log entries are formatted correctly.
 *
 * @note Thread-safe: All methods are protected by mutex.
 */
class MockLogBackend : public ILogBackend {
public:
    /**
     * @brief Constructor.
     */
    MockLogBackend();

    /**
     * @brief Destructor.
     */
    ~MockLogBackend() override = default;

    /**
     * @brief Initializes the mock backend.
     *
     * Always succeeds for testing purposes.
     *
     * @param logDir Ignored (for interface compatibility)
     * @param logFileName Ignored (for interface compatibility)
     * @return Always returns true
     */
    bool initialize(const QString& logDir, const QString& logFileName) override;

    /**
     * @brief Writes a log entry to the mock backend.
     *
     * Stores the log entry in an internal list for later inspection.
     *
     * @param entry Log entry to write
     */
    void write(const LogEntry& entry) override;

    /**
     * @brief Flushes any buffered log entries.
     *
     * No-op for mock backend.
     */
    void flush() override;

    /**
     * @brief Rotates log files if needed.
     *
     * No-op for mock backend.
     */
    void rotateIfNeeded() override;

    /**
     * @brief Sets the log format.
     *
     * @param format Format identifier (stored for inspection)
     */
    void setFormat(const QString& format) override;

    /**
     * @brief Sets the maximum log file size.
     *
     * @param maxSizeBytes Maximum size (stored for inspection)
     */
    void setMaxFileSize(qint64 maxSizeBytes) override;

    /**
     * @brief Sets the maximum number of log files.
     *
     * @param maxFiles Maximum files (stored for inspection)
     */
    void setMaxFiles(int maxFiles) override;

    // Test helper methods

    /**
     * @brief Gets all log entries written to this backend.
     *
     * @return List of all log entries
     */
    QList<LogEntry> entries() const;

    /**
     * @brief Gets the number of log entries written.
     *
     * @return Number of entries
     */
    int entryCount() const;

    /**
     * @brief Clears all stored log entries.
     */
    void clear();

    /**
     * @brief Gets the configured format.
     *
     * @return Format string
     */
    QString format() const;

    /**
     * @brief Gets the configured max file size.
     *
     * @return Max file size in bytes
     */
    qint64 maxFileSize() const;

    /**
     * @brief Gets the configured max files.
     *
     * @return Max number of files
     */
    int maxFiles() const;

    /**
     * @brief Gets the flush count.
     *
     * @return Number of times flush() was called
     */
    int flushCount() const;

    /**
     * @brief Gets the rotation count.
     *
     * @return Number of times rotateIfNeeded() was called
     */
    int rotationCount() const;

private:
    mutable QMutex m_mutex;
    QList<LogEntry> m_entries;
    QString m_format;
    qint64 m_maxFileSize{0};
    int m_maxFiles{0};
    int m_flushCount{0};
    int m_rotationCount{0};
    bool m_initialized{false};
};

} // namespace Logging
} // namespace Infrastructure
} // namespace ZMonitor

