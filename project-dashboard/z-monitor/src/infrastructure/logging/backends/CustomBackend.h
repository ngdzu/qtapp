/**
 * @file CustomBackend.h
 * @brief Custom Qt-based logging backend (no external dependencies).
 * 
 * This file contains the CustomBackend class which implements ILogBackend
 * using pure Qt (QFile, QTextStream). It supports human-readable and JSON
 * formats, log rotation (size-based and time-based), and file size limits.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/common/Result.h"
#include "infrastructure/logging/ILogBackend.h"
#include "infrastructure/logging/LogEntry.h"
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDir>
#include <QDateTime>
#include <QDate>

namespace zmon {
/**
 * @class CustomBackend
 * @brief Custom Qt-based logging backend (no external dependencies).
 * 
 * Pure Qt implementation using QFile and QTextStream. Suitable for
 * environments where external logging libraries are not available.
 * 
 * Features:
 * - Human-readable and JSON output formats
 * - Size-based log rotation (when file exceeds maxFileSize)
 * - Time-based log rotation (daily at midnight)
 * - Configurable file size limits and retention
 * - Graceful error handling for file I/O operations
 * 
 * @note Thread-safe: All methods are expected to be called from the Database I/O Thread.
 * @note Non-blocking: File I/O is synchronous but fast enough for background thread.
 * 
 * @ingroup Infrastructure
 */
class CustomBackend : public ILogBackend {
public:
    /**
     * @brief Constructor.
     * 
     * Creates a CustomBackend with default settings:
     * - Format: "human"
     * - Max file size: 10 MB
     * - Max files: 5
     */
    CustomBackend();
    
    /**
     * @brief Destructor.
     * 
     * Flushes any pending log entries and closes the log file.
     */
    ~CustomBackend() override;
    
    /**
     * @brief Initializes the logging backend.
     * 
     * Opens the log file in the specified directory. Creates the directory
     * if it doesn't exist. Sets up the log file path and opens it for writing.
     * 
     * @param logDir Directory where log files should be written.
     * @param logFileName Base name for log files (e.g., "z-monitor").
     * @return true if initialization succeeded, false otherwise.
     */
    bool initialize(const QString& logDir, const QString& logFileName) override;
    
    /**
     * @brief Writes a log entry to the backend.
     * 
     * Formats the log entry according to the configured format (human or JSON)
     * and writes it to the log file. Checks for rotation before writing.
     * 
     * @param entry Log entry to write.
     */
    void write(const LogEntry& entry) override;
    
    /**
     * @brief Flushes any buffered log entries.
     * 
     * Ensures all pending log data is written to disk.
     */
    void flush() override;
    
    /**
     * @brief Rotates log files if needed.
     * 
     * Checks if rotation is needed based on file size or time (daily rotation).
     * If rotation is needed, renames the current log file with a timestamp
     * and opens a new log file.
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
     * Older files beyond this limit will be deleted during rotation.
     * 
     * @param maxFiles Maximum number of log files to keep.
     */
    void setMaxFiles(int maxFiles) override;

private:
    QFile* m_logFile;
    QTextStream* m_stream;
    QString m_logDir;
    QString m_logFileName;
    QString m_logFilePath;
    qint64 m_maxFileSize;
    int m_maxFiles;
    QString m_format;
    QDateTime m_lastRotationDate;  // For time-based rotation (daily)
    
    /**
     * @brief Rotates the log file.
     * 
     * Renames the current log file with a timestamp suffix and opens a new
     * log file. Deletes old log files if they exceed maxFiles limit.
     */
    void rotateLogFile();
    
    /**
     * @brief Formats a log entry according to the configured format.
     * 
     * @param entry Log entry to format.
     * @return Formatted string.
     */
    QString formatEntry(const LogEntry& entry) const;
    
    /**
     * @brief Checks if rotation is needed based on file size.
     * 
     * @return true if file size exceeds maxFileSize, false otherwise.
     */
    bool needsSizeRotation() const;
    
    /**
     * @brief Checks if rotation is needed based on time (daily).
     * 
     * @return true if a new day has started since last rotation, false otherwise.
     */
    bool needsTimeRotation() const;
    
    /**
     * @brief Deletes old log files beyond the maxFiles limit.
     * 
     * Scans the log directory for rotated log files and deletes the oldest
     * ones if they exceed maxFiles.
     */
    void cleanupOldFiles();
    
    /**
     * @brief Opens the log file for writing.
     * 
     * @return Result<void> - Success if file opened successfully, Error with details if failed
     */
    Result<void> openLogFile();
    
    /**
     * @brief Closes the log file.
     */
    void closeLogFile();
};

} // namespace zmon
} // namespace zmon