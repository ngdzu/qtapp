/**
 * @file LogEntry.h
 * @brief Defines the LogEntry value type used by the logging subsystem.
 *
 * This file contains the LogEntry struct and LogLevel enumeration used by
 * the asynchronous logging infrastructure. Log entries are created by
 * LogService on any thread, enqueued to a lock-free queue, and consumed
 * by ILogBackend implementations on the Database I/O Thread.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QDateTime>
#include <QVariantMap>
#include <QString>

namespace zmon {
/**
 * @enum LogLevel
 * @brief Log severity levels.
 *
 * These levels align with the LogService API and can be mapped directly
 * to backend-specific levels (e.g., spdlog, glog).
 */
enum class LogLevel {
    Trace = 0,    ///< Detailed trace information
    Debug = 1,    ///< Debug information for developers
    Info = 2,     ///< Informational messages
    Warning = 3,  ///< Warnings that do not stop the system
    Error = 4,    ///< Recoverable errors
    Critical = 5, ///< Critical errors requiring attention
    Fatal = 6     ///< Fatal errors after which the process may terminate
};

/**
 * @struct LogEntry
 * @brief Immutable value type representing a single log record.
 *
 * A LogEntry captures all relevant information for a single logging event,
 * including timestamp, severity, category, message text, structured context,
 * and source location metadata. LogEntry instances are created on the calling
 * thread and passed to ILogBackend implementations on the Database I/O Thread.
 *
 * @note This type is intentionally simple and copyable to support passing
 *       between threads via lock-free queues.
 */
struct LogEntry {
    /**
     * @brief Timestamp when the log entry was created.
     */
    QDateTime timestamp;

    /**
     * @brief Log severity level.
     */
    LogLevel level{LogLevel::Info};

    /**
     * @brief Logical category or subsystem name.
     *
     * Examples: "network", "database", "auth", "ui".
     */
    QString category;

    /**
     * @brief Human-readable log message text.
     */
    QString message;

    /**
     * @brief Structured context for the log entry.
     *
     * Key/value pairs that provide additional details, suitable for
     * JSON serialization by backends.
     */
    QVariantMap context;

    /**
     * @brief Identifier of the thread that produced the log entry.
     */
    QString threadId;

    /**
     * @brief Source file name where the log call originated.
     */
    QString file;

    /**
     * @brief Source line number in the file.
     */
    int line{0};

    /**
     * @brief Function name where the log call originated.
     */
    QString function;
};

} // namespace zmon