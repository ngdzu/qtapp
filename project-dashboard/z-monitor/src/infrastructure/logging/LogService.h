/**
 * @file LogService.h
 * @brief Provides asynchronous, non-blocking logging for application events.
 *
 * This file defines the LogService class which implements the asynchronous
 * logging architecture. All logging methods return immediately (< 1μs) by
 * enqueueing log entries to a lock-free queue. The Database I/O Thread
 * processes the queue and writes to the configured logging backend.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QVariantMap>
#include <QList>
#include <QMap>
#include <QMutex>
#include <memory>

#include "domain/common/Result.h"
#include "LogEntry.h"
#include "ILogBackend.h"

// Forward declaration for lock-free queue
// Using moodycamel::ConcurrentQueue (header-only, MIT license)
// Will be included in implementation file
namespace moodycamel {
    template<typename T>
    class ConcurrentQueue;
}

namespace zmon {

/**
 * @class LogService
 * @brief Provides asynchronous, non-blocking logging for application events.
 *
 * All logging methods return immediately (< 1μs) by enqueueing log entries
 * to a lock-free queue. The Database I/O Thread processes the queue
 * and writes to the configured logging backend (shared with database operations).
 *
 * @note Thread-safe: Can be called from any thread without synchronization.
 * @note Non-blocking: All methods return immediately.
 * @note Thread: Runs on Database I/O Thread (shared with database operations).
 *
 * @ingroup Infrastructure
 */
class LogService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a LogService with the specified backend.
     *
     * @param backend Logging backend implementation (ownership transferred to LogService)
     * @param parent Parent QObject
     */
    explicit LogService(ILogBackend* backend, QObject* parent = nullptr);

    /**
     * @brief Destructor.
     *
     * Flushes all pending log entries before destruction.
     */
    ~LogService() override;

    /**
     * @brief Initializes the LogService.
     *
     * Must be called after moving LogService to the Database I/O Thread.
     * Initializes the backend and starts queue processing.
     *
     * @param logDir Directory for log files
     * @param logFileName Base name for log files (e.g., "z-monitor")
     * @return Result<void> - Success if initialization succeeded, Error with details if failed
     */
    Result<void> initialize(const QString& logDir, const QString& logFileName);

    // Public logging methods - all return immediately (< 1μs)

    /**
     * @brief Logs a trace-level message.
     *
     * @param message Log message text
     * @param context Structured context data (key-value pairs)
     */
    void trace(const QString& message, const QVariantMap& context = {});

    /**
     * @brief Logs a debug-level message.
     *
     * @param message Log message text
     * @param context Structured context data (key-value pairs)
     */
    void debug(const QString& message, const QVariantMap& context = {});

    /**
     * @brief Logs an info-level message.
     *
     * @param message Log message text
     * @param context Structured context data (key-value pairs)
     */
    void info(const QString& message, const QVariantMap& context = {});

    /**
     * @brief Logs a warning-level message.
     *
     * @param message Log message text
     * @param context Structured context data (key-value pairs)
     */
    void warning(const QString& message, const QVariantMap& context = {});

    /**
     * @brief Logs an error-level message.
     *
     * @param message Log message text
     * @param context Structured context data (key-value pairs)
     */
    void error(const QString& message, const QVariantMap& context = {});

    /**
     * @brief Logs a critical-level message.
     *
     * @param message Log message text
     * @param context Structured context data (key-value pairs)
     */
    void critical(const QString& message, const QVariantMap& context = {});

    /**
     * @brief Logs a fatal-level message.
     *
     * @param message Log message text
     * @param context Structured context data (key-value pairs)
     */
    void fatal(const QString& message, const QVariantMap& context = {});

    // Configuration

    /**
     * @brief Sets the minimum log level.
     *
     * Log entries below this level are discarded.
     *
     * @param level Minimum log level (from LogEntry.h)
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Gets the current minimum log level.
     *
     * @return Current minimum log level (from LogEntry.h)
     */
    LogLevel logLevel() const;

    /**
     * @brief Enables or disables a log category.
     *
     * @param category Category name (e.g., "network", "database")
     * @param enabled true to enable, false to disable
     */
    void setCategoryEnabled(const QString& category, bool enabled);

    /**
     * @brief Checks if a category is enabled.
     *
     * @param category Category name
     * @return true if enabled, false otherwise
     */
    bool isCategoryEnabled(const QString& category) const;

    /**
     * @brief Gets recent log entries for Diagnostics View.
     *
     * Returns the last MAX_RECENT_LOGS entries (up to 1000).
     * Thread-safe: Can be called from any thread.
     *
     * @return List of recent log entries
     */
    QList<LogEntry> recentLogs() const;

    /**
     * @brief Flushes all pending log entries.
     *
     * Blocks until all queued entries are written. Should be called
     * during shutdown or when explicitly requested.
     */
    void flush();

signals:
    /**
     * @brief Emitted when a log entry is added to the in-memory buffer.
     *
     * This signal is emitted from the Database I/O Thread when a log entry
     * is processed and added to the recent logs buffer. Can be used by
     * Diagnostics View to update in real-time.
     *
     * @param entry The log entry that was added
     */
    void logEntryAdded(const LogEntry& entry);

private slots:
    /**
     * @brief Processes log entries from the queue.
     *
     * Called periodically by the Database I/O Thread event loop.
     * Processes up to MAX_BATCH entries per call to avoid blocking
     * database operations.
     */
    void processLogQueue();

private:
    /**
     * @brief Enqueues a log entry to the lock-free queue.
     *
     * Creates a LogEntry with current timestamp, thread ID, and source
     * location, then enqueues it to the lock-free queue. Returns immediately.
     *
     * @param level Log severity level (from LogEntry.h)
     * @param message Log message text
     * @param context Structured context data
     * @param category Optional category name (defaults to empty)
     */
    void enqueueLog(LogLevel level, const QString& message,
                    const QVariantMap& context = {},
                    const QString& category = QString());

    /**
     * @brief Gets the current thread ID as a string.
     *
     * @return Thread ID string
     */
    static QString getCurrentThreadId();

    // Configuration
    LogLevel m_minLevel{LogLevel::Info};
    QMap<QString, bool> m_categoryEnabled;

    // Lock-free queue for async logging (MPSC - Multiple Producer Single Consumer)
    std::unique_ptr<moodycamel::ConcurrentQueue<LogEntry>> m_logQueue;

    // Backend abstraction
    std::unique_ptr<ILogBackend> m_backend;

    // In-memory buffer for Diagnostics View (last 1000 entries)
    mutable QMutex m_recentLogsMutex;  // Protects m_recentLogs
    QList<LogEntry> m_recentLogs;
    static constexpr int MAX_RECENT_LOGS = 1000;

    // Queue processing configuration
    static constexpr int MAX_BATCH = 100;  // Process up to 100 entries per batch
    QTimer* m_processTimer;  // Timer for periodic queue processing

    // Initialization state
    bool m_initialized{false};
};

} // namespace zmon