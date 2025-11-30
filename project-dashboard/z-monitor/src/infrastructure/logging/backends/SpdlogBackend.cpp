/**
 * @file SpdlogBackend.cpp
 * @brief Implementation of SpdlogBackend logging backend.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/logging/backends/SpdlogBackend.h"

#include "domain/common/Result.h"
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

// Include spdlog headers
// Note: spdlog is a header-only library, so we include it directly
// TODO: Add spdlog as a dependency via CMake FetchContent or find_package
//
// For now, we'll use conditional compilation
// In production, spdlog should be added as a dependency
#ifdef Z_MONITOR_USE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/logger.h>
#include <spdlog/common.h>
#else
// Stub implementation when spdlog is not available
// This allows the code to compile without spdlog
#include <string>
namespace spdlog
{
    namespace sinks
    {
        template <typename Mutex>
        class rotating_file_sink_mt
        {
        };
    }
    enum level_enum
    {
        trace = 0,
        debug = 1,
        info = 2,
        warn = 3,
        err = 4,
        critical = 5,
        off = 6
    };
    class logger
    {
    public:
        void log(level_enum, const std::string &) {}
        void flush() {}
        void set_level(level_enum) {}
        void set_pattern(const std::string &) {}
    };
    template <typename T>
    std::shared_ptr<logger> rotating_logger_mt(const std::string &, const std::string &, size_t, size_t)
    {
        return nullptr;
    }
}
#endif

namespace zmon
{
    SpdlogBackend::SpdlogBackend()
        : m_logger(nullptr), m_logDir(""), m_logFileName(""), m_format("human"), m_maxFileSize(10 * 1024 * 1024) // Default: 10 MB
          ,
          m_maxFiles(5) // Default: keep 5 files
          ,
          m_initialized(false)
    {
    }

    SpdlogBackend::~SpdlogBackend()
    {
        flush();
        m_logger.reset();
    }

    Result<void> SpdlogBackend::initialize(const QString &logDir, const QString &logFileName)
    {
        if (logDir.isEmpty() || logFileName.isEmpty())
        {
            return Result<void>::error(Error::create(
                ErrorCode::InvalidArgument,
                "Invalid logDir or logFileName",
                {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}));
        }

#ifdef Z_MONITOR_USE_SPDLOG
        m_logDir = logDir;
        m_logFileName = logFileName;

        // Create log directory if it doesn't exist
        QDir dir(logDir);
        if (!dir.exists())
        {
            if (!dir.mkpath("."))
            {
                return Result<void>::error(Error::create(
                    ErrorCode::Internal,
                    "Failed to create log directory",
                    {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}));
            }
        }

        // Build log file path
        QString logFilePath = QDir(logDir).filePath(logFileName + ".log");
        std::string logPath = logFilePath.toStdString();

        try
        {
            // Create rotating file logger (thread-safe, multi-threaded)
            // Parameters: logger name, file path, max file size, max number of files
            m_logger = spdlog::rotating_logger_mt(
                "z-monitor",
                logPath,
                static_cast<size_t>(m_maxFileSize),
                static_cast<size_t>(m_maxFiles));

            // Set log level (spdlog uses its own level enum)
            m_logger->set_level(spdlog::level::trace); // Log all levels

            // Set format pattern based on configured format
            if (m_format == "json")
            {
                // JSON format: timestamp, level, message
                m_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");
            }
            else
            {
                // Human-readable format: timestamp, level, category, message
                m_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] [%n] %v");
            }

            m_initialized = true;
            return Result<void>::ok();
        }
        catch (const std::exception &e)
        {
            return Result<void>::error(Error::create(
                ErrorCode::Internal,
                "spdlog initialization failed: " + std::string(e.what()),
                {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}));
        }
#else
        // spdlog not available - return error
        return Result<void>::error(Error::create(
            ErrorCode::Unavailable,
            "spdlog library not available. Define Z_MONITOR_USE_SPDLOG and link spdlog to use SpdlogBackend.",
            {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}));
#endif
    }

    void SpdlogBackend::write(const LogEntry &entry)
    {
#ifdef Z_MONITOR_USE_SPDLOG
        if (!m_logger || !m_initialized)
        {
            qWarning() << "SpdlogBackend::write: Logger not initialized";
            return;
        }

        try
        {
            // Convert LogLevel to spdlog level
            spdlog::level::level_enum spdlogLevel = static_cast<spdlog::level::level_enum>(logLevelToSpdlog(entry.level));

            // Format log entry according to configured format
            QString formattedMessage = formatLogEntry(entry);
            std::string message = formattedMessage.toStdString();

            // Write to spdlog
            // Note: spdlog's rotating_file_sink_mt is thread-safe and performs async I/O internally
            m_logger->log(spdlogLevel, message);
        }
        catch (const std::exception &e)
        {
            qWarning() << "SpdlogBackend::write: Exception:" << e.what();
        }
#else
        Q_UNUSED(entry);
        // spdlog not available - do nothing
#endif
    }

    void SpdlogBackend::flush()
    {
#ifdef Z_MONITOR_USE_SPDLOG
        if (m_logger)
        {
            try
            {
                m_logger->flush();
            }
            catch (const std::exception &e)
            {
                qWarning() << "SpdlogBackend::flush: Exception:" << e.what();
            }
        }
#endif
    }

    void SpdlogBackend::rotateIfNeeded()
    {
#ifdef Z_MONITOR_USE_SPDLOG
        // spdlog handles rotation automatically based on file size
        // This method is called periodically to ensure rotation happens
        // No explicit action needed - spdlog's rotating_file_sink handles it
        if (m_logger)
        {
            flush(); // Ensure current data is written before rotation
        }
#endif
    }

    void SpdlogBackend::setFormat(const QString &format)
    {
        m_format = format;

#ifdef Z_MONITOR_USE_SPDLOG
        if (m_logger)
        {
            try
            {
                if (format == "json")
                {
                    m_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");
                }
                else
                {
                    m_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] [%n] %v");
                }
            }
            catch (const std::exception &e)
            {
                qWarning() << "SpdlogBackend::setFormat: Exception:" << e.what();
            }
        }
#endif
    }

    void SpdlogBackend::setMaxFileSize(qint64 maxSizeBytes)
    {
        m_maxFileSize = maxSizeBytes;
        // Note: spdlog rotating logger is created with max file size,
        // so changing it requires recreating the logger
        // For simplicity, we store the value but don't recreate the logger
        // A full implementation would recreate the logger if already initialized
    }

    void SpdlogBackend::setMaxFiles(int maxFiles)
    {
        m_maxFiles = maxFiles;
        // Note: spdlog rotating logger is created with max files,
        // so changing it requires recreating the logger
        // For simplicity, we store the value but don't recreate the logger
        // A full implementation would recreate the logger if already initialized
    }

    int SpdlogBackend::logLevelToSpdlog(LogLevel level) const
    {
#ifdef Z_MONITOR_USE_SPDLOG
        // Convert LogLevel enum to spdlog level_enum
        switch (level)
        {
        case LogLevel::Trace:
            return static_cast<int>(spdlog::level::trace);
        case LogLevel::Debug:
            return static_cast<int>(spdlog::level::debug);
        case LogLevel::Info:
            return static_cast<int>(spdlog::level::info);
        case LogLevel::Warning:
            return static_cast<int>(spdlog::level::warn);
        case LogLevel::Error:
            return static_cast<int>(spdlog::level::err);
        case LogLevel::Critical:
            return static_cast<int>(spdlog::level::critical);
        case LogLevel::Fatal:
            return static_cast<int>(spdlog::level::critical); // Fatal maps to critical
        default:
            return static_cast<int>(spdlog::level::info);
        }
#else
        // Stub implementation
        Q_UNUSED(level);
        return 2; // info level
#endif
    }

    QString SpdlogBackend::formatLogEntry(const LogEntry &entry) const
    {
        if (m_format == "json")
        {
            return formatJson(entry);
        }
        else
        {
            return formatHuman(entry);
        }
    }

    QString SpdlogBackend::formatJson(const LogEntry &entry) const
    {
        QJsonObject json;
        json["timestamp"] = entry.timestamp.toString(Qt::ISODate);
        json["level"] = static_cast<int>(entry.level);
        json["category"] = entry.category;
        json["message"] = entry.message;
        json["threadId"] = entry.threadId;

        // Add context as nested object
        if (!entry.context.isEmpty())
        {
            QJsonObject contextObj;
            for (auto it = entry.context.begin(); it != entry.context.end(); ++it)
            {
                contextObj[it.key()] = QJsonValue::fromVariant(it.value());
            }
            json["context"] = contextObj;
        }

        // Add source location if available
        if (!entry.file.isEmpty())
        {
            json["file"] = entry.file;
            json["line"] = entry.line;
            json["function"] = entry.function;
        }

        QJsonDocument doc(json);
        return doc.toJson(QJsonDocument::Compact);
    }

    QString SpdlogBackend::formatHuman(const LogEntry &entry) const
    {
        QString formatted;
        formatted += entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
        formatted += " [";

        // Add level string
        switch (entry.level)
        {
        case LogLevel::Trace:
            formatted += "TRACE";
            break;
        case LogLevel::Debug:
            formatted += "DEBUG";
            break;
        case LogLevel::Info:
            formatted += "INFO";
            break;
        case LogLevel::Warning:
            formatted += "WARN";
            break;
        case LogLevel::Error:
            formatted += "ERROR";
            break;
        case LogLevel::Critical:
            formatted += "CRITICAL";
            break;
        case LogLevel::Fatal:
            formatted += "FATAL";
            break;
        default:
            formatted += "UNKNOWN";
            break;
        }

        formatted += "]";

        // Add category if present
        if (!entry.category.isEmpty())
        {
            formatted += " [" + entry.category + "]";
        }

        formatted += " " + entry.message;

        // Add context if present
        if (!entry.context.isEmpty())
        {
            formatted += " {";
            bool first = true;
            for (auto it = entry.context.begin(); it != entry.context.end(); ++it)
            {
                if (!first)
                {
                    formatted += ", ";
                }
                formatted += it.key() + "=" + it.value().toString();
                first = false;
            }
            formatted += "}";
        }

        // Add source location if available
        if (!entry.file.isEmpty())
        {
            formatted += " (" + entry.file + ":" + QString::number(entry.line) + ")";
        }

        return formatted;
    }

} // namespace zmon