/**
 * @file SpdlogBackend.h
 * @brief spdlog-based logging backend implementation.
 *
 * This file contains the SpdlogBackend class which implements ILogBackend
 * using the spdlog library. Provides high-performance async logging with
 * automatic file rotation and formatting.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "infrastructure/logging/ILogBackend.h"
#include "infrastructure/logging/LogEntry.h"
#include <QString>
#include <memory>

// Forward declaration for spdlog logger
// Will be included in implementation file
namespace spdlog {
    class logger;
}

namespace zmon {
/**
 * @class SpdlogBackend
 * @brief spdlog-based logging backend implementation.
 *
 * Uses spdlog library for high-performance, async logging with
 * automatic file rotation and formatting. This backend provides
 * better performance than CustomBackend for high-frequency logging.
 *
 * Features:
 * - High-performance async logging
 * - Automatic file rotation (size-based)
 * - Human-readable and JSON formatting support
 * - Configurable file size limits and retention
 * - Thread-safe (spdlog handles thread safety internally)
 *
 * @note Requires spdlog library dependency.
 * @note Thread-safe: All methods are expected to be called from the Database I/O Thread.
 * @note Non-blocking: spdlog async mode ensures non-blocking writes.
 *
 * @ingroup Infrastructure
 */
class SpdlogBackend : public ILogBackend {
public:
    /**
     * @brief Constructor.
     *
     * Creates a SpdlogBackend with default settings:
     * - Format: "human"
     * - Max file size: 10 MB
     * - Max files: 5
     */
    SpdlogBackend();

    /**
     * @brief Destructor.
     *
     * Flushes any pending log entries and closes the logger.
     */
    ~SpdlogBackend() override;

    /**
     * @brief Initializes the logging backend.
     *
     * Creates a rotating file logger using spdlog. Sets up the log file
     * path, rotation parameters, and formatting.
     *
     * @param logDir Directory where log files should be written.
     * @param logFileName Base name for log files (e.g., "z-monitor").
     * @return true if initialization succeeded, false otherwise.
     */
    bool initialize(const QString& logDir, const QString& logFileName) override;

    /**
     * @brief Writes a log entry to the backend.
     *
     * Converts the LogEntry to spdlog format and writes it using the
     * appropriate log level. Uses spdlog's async mode for non-blocking writes.
     *
     * @param entry Log entry to write.
     */
    void write(const LogEntry& entry) override;

    /**
     * @brief Flushes any buffered log entries.
     *
     * Ensures all pending log data is written to disk via spdlog.
     */
    void flush() override;

    /**
     * @brief Rotates log files if needed.
     *
     * spdlog handles rotation automatically based on file size.
     * This method triggers a manual rotation check.
     */
    void rotateIfNeeded() override;

    /**
     * @brief Sets the log format.
     *
     * @param format Format identifier ("human" or "json").
     */
    void setFormat(const QString& format) override;

    /**
     * @brief Sets the maximum log file size before rotation.
     *
     * @param maxSizeBytes Maximum size in bytes for a single log file.
     */
    void setMaxFileSize(qint64 maxSizeBytes) override;

    /**
     * @brief Sets the maximum number of rotated log files to keep.
     *
     * @param maxFiles Maximum number of log files to keep.
     */
    void setMaxFiles(int maxFiles) override;

private:
    /**
     * @brief Converts LogLevel to spdlog level.
     *
     * @param level LogLevel from LogEntry.
     * @return spdlog level enum value.
     */
    int logLevelToSpdlog(LogLevel level) const;

    /**
     * @brief Formats log entry for spdlog.
     *
     * Converts LogEntry to a string format suitable for spdlog,
     * including context data if present.
     *
     * @param entry Log entry to format.
     * @return Formatted string.
     */
    QString formatLogEntry(const LogEntry& entry) const;

    /**
     * @brief Formats log entry as JSON.
     *
     * @param entry Log entry to format.
     * @return JSON-formatted string.
     */
    QString formatJson(const LogEntry& entry) const;

    /**
     * @brief Formats log entry as human-readable text.
     *
     * @param entry Log entry to format.
     * @return Human-readable formatted string.
     */
    QString formatHuman(const LogEntry& entry) const;

    // spdlog logger instance
    std::shared_ptr<spdlog::logger> m_logger;

    // Configuration
    QString m_logDir;
    QString m_logFileName;
    QString m_format;
    qint64 m_maxFileSize;
    int m_maxFiles;

    // Initialization state
    bool m_initialized{false};
};

} // namespace zmon
} // namespace zmon