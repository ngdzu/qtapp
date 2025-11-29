/**
 * @file CustomBackend.cpp
 * @brief Implementation of CustomBackend logging backend.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/logging/backends/CustomBackend.h"
#include "infrastructure/logging/utils/LogFormatter.h"
#include "domain/common/Result.h"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDate>
#include <QDebug>
#include <QIODevice>
#include <algorithm>

namespace zmon {
CustomBackend::CustomBackend()
    : m_logFile(nullptr)
    , m_stream(nullptr)
    , m_logDir("")
    , m_logFileName("")
    , m_logFilePath("")
    , m_maxFileSize(10 * 1024 * 1024)  // Default: 10 MB
    , m_maxFiles(5)  // Default: keep 5 files
    , m_format("human")
    , m_lastRotationDate(QDate::currentDate().startOfDay())
{
}

CustomBackend::~CustomBackend() {
    flush();
    closeLogFile();
}

Result<void> CustomBackend::initialize(const QString& logDir, const QString& logFileName) {
    if (logDir.isEmpty() || logFileName.isEmpty()) {
        return Result<void>::error(Error::create(
            ErrorCode::InvalidArgument,
            "Invalid logDir or logFileName",
            {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}
        ));
    }
    
    m_logDir = logDir;
    m_logFileName = logFileName;
    
    // Create log directory if it doesn't exist
    QDir dir(logDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            return Result<void>::error(Error::create(
                ErrorCode::Internal,
                "Failed to create log directory",
                {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}
            ));
        }
    }
    
    // Set log file path
    m_logFilePath = QDir(logDir).filePath(logFileName + ".log");
    
    // Open log file
    auto openResult = openLogFile();
    if (openResult.isError()) {
        return openResult;
    }
    
    m_lastRotationDate = QDate::currentDate().startOfDay();
    
    return Result<void>::ok();
}

void CustomBackend::write(const LogEntry& entry) {
    if (!m_logFile || !m_stream) {
        qWarning() << "CustomBackend::write: Log file not initialized";
        return;
    }
    
    // Check if rotation is needed before writing
    if (needsSizeRotation() || needsTimeRotation()) {
        rotateLogFile();
    }
    
    // Format and write the entry
    QString formatted = formatEntry(entry);
    *m_stream << formatted << "\n";
    
    // Auto-flush for critical/error/fatal levels
    if (entry.level >= LogLevel::Error) {
        m_stream->flush();
        if (m_logFile) {
            m_logFile->flush();
        }
    }
}

void CustomBackend::flush() {
    if (m_stream) {
        m_stream->flush();
    }
    if (m_logFile) {
        m_logFile->flush();
    }
}

void CustomBackend::rotateIfNeeded() {
    if (needsSizeRotation() || needsTimeRotation()) {
        rotateLogFile();
    }
}

void CustomBackend::setFormat(const QString& format) {
    if (format == "human" || format == "json") {
        m_format = format;
    } else {
        qWarning() << "CustomBackend::setFormat: Unknown format:" << format << "(using 'human')";
        m_format = "human";
    }
}

void CustomBackend::setMaxFileSize(qint64 maxSizeBytes) {
    if (maxSizeBytes > 0) {
        m_maxFileSize = maxSizeBytes;
    }
}

void CustomBackend::setMaxFiles(int maxFiles) {
    if (maxFiles > 0) {
        m_maxFiles = maxFiles;
    }
}

void CustomBackend::rotateLogFile() {
    if (!m_logFile || m_logFilePath.isEmpty()) {
        return;
    }
    
    // Close current file
    closeLogFile();
    
    // Generate rotated file name with timestamp
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMdd_HHmmss");
    QString rotatedPath = QDir(m_logDir).filePath(
        QString("%1_%2.log").arg(m_logFileName, timestamp));
    
    // Rename current file to rotated name
    QFile currentFile(m_logFilePath);
    if (currentFile.exists()) {
        if (!currentFile.rename(rotatedPath)) {
            qWarning() << "CustomBackend::rotateLogFile: Failed to rename log file:" 
                       << m_logFilePath << "to" << rotatedPath;
        }
    }
    
    // Clean up old files
    cleanupOldFiles();
    
    // Open new log file
    auto openResult = openLogFile();
    if (openResult.isError()) {
        qWarning() << "CustomBackend::rotateLogFile: Failed to open new log file:" << openResult.error().message.c_str();
    }
    
    // Update last rotation date
    m_lastRotationDate = QDate::currentDate().startOfDay();
}

QString CustomBackend::formatEntry(const LogEntry& entry) const {
    if (m_format == "json") {
        return Utils::formatJson(entry);
    } else {
        return Utils::formatHuman(entry);
    }
}

bool CustomBackend::needsSizeRotation() const {
    if (!m_logFile || m_logFilePath.isEmpty()) {
        return false;
    }
    
    QFileInfo fileInfo(m_logFilePath);
    if (fileInfo.exists()) {
        return fileInfo.size() >= m_maxFileSize;
    }
    
    return false;
}

bool CustomBackend::needsTimeRotation() const {
    QDate today = QDate::currentDate();
    QDate lastRotationDate = m_lastRotationDate.date();
    
    // Rotate if we've crossed into a new day
    return today > lastRotationDate;
}

void CustomBackend::cleanupOldFiles() {
    if (m_maxFiles <= 0) {
        return;  // Keep all files
    }
    
    QDir dir(m_logDir);
    if (!dir.exists()) {
        return;
    }
    
    // Find all rotated log files matching the pattern: logFileName_YYYYMMDD_HHMMSS.log
    QString pattern = QString("%1_*.log").arg(m_logFileName);
    QFileInfoList files = dir.entryInfoList({pattern}, QDir::Files, QDir::Time | QDir::Reversed);
    
    // Also include the current log file if it exists
    QFileInfo currentFile(m_logFilePath);
    if (currentFile.exists()) {
        files.append(currentFile);
    }
    
    // Sort by modification time (oldest first)
    std::sort(files.begin(), files.end(),
              [](const QFileInfo& a, const QFileInfo& b) {
                  return a.lastModified() < b.lastModified();
              });
    
    // Delete oldest files beyond maxFiles limit
    int filesToDelete = files.size() - m_maxFiles;
    for (int i = 0; i < filesToDelete && i < files.size(); ++i) {
        QFile file(files[i].absoluteFilePath());
        if (!file.remove()) {
            qWarning() << "CustomBackend::cleanupOldFiles: Failed to delete old log file:" 
                       << files[i].absoluteFilePath();
        }
    }
}

Result<void> CustomBackend::openLogFile() {
    closeLogFile();
    
    m_logFile = new QFile(m_logFilePath);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QString errorString = m_logFile->errorString();
        delete m_logFile;
        m_logFile = nullptr;
        return Result<void>::error(Error::create(
            ErrorCode::Internal,
            "Failed to open log file: " + errorString.toStdString(),
            {{"logFilePath", m_logFilePath.toStdString()}}
        ));
    }
    
    m_stream = new QTextStream(m_logFile);
    // QTextStream in Qt6 uses UTF-8 by default, no need to set encoding
    
    return Result<void>::ok();
}

void CustomBackend::closeLogFile() {
    if (m_stream) {
        delete m_stream;
        m_stream = nullptr;
    }
    
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
}

} // namespace zmon