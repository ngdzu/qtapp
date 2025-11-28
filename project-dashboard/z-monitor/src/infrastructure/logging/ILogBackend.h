/**
 * @file ILogBackend.h
 * @brief Abstract interface for logging backends.
 *
 * This file defines the ILogBackend interface used by LogService to write
 * log entries to concrete logging backends (CustomBackend, SpdlogBackend, etc.).
 * Implementations must be thread-safe and suitable for use on the Database
 * I/O Thread as part of the asynchronous logging architecture.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QString>

#include "LogEntry.h"

namespace ZMonitor {
namespace Infrastructure {
namespace Logging {

/**
 * @class ILogBackend
 * @brief Abstract interface for logging backends.
 *
 * This interface allows LogService to work with any logging library
 * (spdlog, glog, custom, etc.) by abstracting the write operations.
 *
 * Implementations are responsible for:
 * - Opening and managing log files or sinks.
 * - Formatting log entries (human-readable or JSON).
 * - Handling log rotation (size-based or time-based).
 * - Flushing buffered entries on demand or at shutdown.
 *
 * @note Implementations must be thread-safe and handle log rotation.
 * @note All methods are expected to be called from the Database I/O Thread.
 *
 * @ingroup Infrastructure
 */
class ILogBackend {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ILogBackend() = default;

    /**
     * @brief Initializes the logging backend.
     *
     * Called once during LogService construction. Implementations should
     * open log files, configure formatters, and prepare any resources
     * required for logging.
     *
     * @param logDir Directory where log files should be written.
     * @param logFileName Base name for log files (for example, "z-monitor").
     * @return true if initialization succeeded, false otherwise.
     */
    virtual bool initialize(const QString& logDir, const QString& logFileName) = 0;

    /**
     * @brief Writes a log entry to the backend.
     *
     * Called from the Database I/O Thread for each log entry dequeued
     * by LogService. Implementations should be as fast as reasonably
     * possible to avoid blocking the Database I/O Thread.
     *
     * @param entry Log entry to write.
     */
    virtual void write(const LogEntry& entry) = 0;

    /**
     * @brief Flushes any buffered log entries.
     *
     * Called during shutdown or when explicitly requested (for example,
     * from diagnostics tools). Implementations should ensure all pending
     * log data is durably written.
     */
    virtual void flush() = 0;

    /**
     * @brief Rotates log files if needed.
     *
     * Called periodically or when file size or time thresholds are reached.
     * Implementations should perform rotation in a way that minimizes
     * impact on the calling thread.
     */
    virtual void rotateIfNeeded() = 0;

    /**
     * @brief Sets the log format.
     *
     * Implementations should support at least the formats described in
     * the async logging architecture (for example, "human" or "json").
     *
     * @param format Format identifier string.
     */
    virtual void setFormat(const QString& format) = 0;

    /**
     * @brief Sets the maximum log file size before rotation.
     *
     * @param maxSizeBytes Maximum size in bytes for a single log file.
     */
    virtual void setMaxFileSize(qint64 maxSizeBytes) = 0;

    /**
     * @brief Sets the maximum number of rotated log files to keep.
     *
     * Older files beyond this limit may be deleted or archived,
     * depending on implementation.
     *
     * @param maxFiles Maximum number of log files to keep.
     */
    virtual void setMaxFiles(int maxFiles) = 0;
};

} // namespace Logging
} // namespace Infrastructure
} // namespace ZMonitor


