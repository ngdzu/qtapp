/**
 * @file LogService.cpp
 * @brief Implementation of asynchronous, non-blocking logging service.
 *
 * This file implements the LogService class which provides asynchronous
 * logging using a lock-free queue and the Database I/O Thread for processing.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "LogService.h"

#include <QThread>
#include <QCoreApplication>
#include <QMutexLocker>
#include <QDebug>
#include "domain/common/Result.h"

// Include moodycamel::ConcurrentQueue
// This is a header-only library, so we include it directly
// For now, we'll use a placeholder - in production, add the library as a dependency
// Option 1: Add as git submodule: externals/concurrentqueue
// Option 2: Use FetchContent in CMake
// Option 3: Download header file during build
//
// For now, we'll create a simple wrapper that can be replaced with the real library
// TODO: Add moodycamel::ConcurrentQueue as a dependency
//
// Temporary implementation using QQueue with mutex (not lock-free, but functional)
// This will be replaced with moodycamel::ConcurrentQueue
#include <QQueue>
#include <QMutex>

namespace zmon {
// Temporary lock-free queue wrapper using QQueue + mutex
// TODO: Replace with moodycamel::ConcurrentQueue
template<typename T>
class TemporaryQueue {
public:
    TemporaryQueue(size_t capacity) : m_capacity(capacity) {}
    
    bool enqueue(const T& item) {
        QMutexLocker locker(&m_mutex);
        if (m_queue.size() >= static_cast<int>(m_capacity)) {
            // Queue full - drop oldest entry
            m_queue.dequeue();
        }
        m_queue.enqueue(item);
        return true;
    }
    
    bool try_dequeue(T& item) {
        QMutexLocker locker(&m_mutex);
        if (m_queue.isEmpty()) {
            return false;
        }
        item = m_queue.dequeue();
        return true;
    }
    
    bool empty_approx() const {
        QMutexLocker locker(&m_mutex);
        return m_queue.isEmpty();
    }
    
    size_t size_approx() const {
        QMutexLocker locker(&m_mutex);
        return static_cast<size_t>(m_queue.size());
    }

private:
    mutable QMutex m_mutex;
    QQueue<T> m_queue;
    size_t m_capacity;
};

// Alias for the queue type - replace with moodycamel::ConcurrentQueue when available
using LogQueue = TemporaryQueue<LogEntry>;

LogService::LogService(ILogBackend* backend, QObject* parent)
    : QObject(parent)
    , m_backend(backend ? std::unique_ptr<ILogBackend>(backend) : nullptr)
    , m_logQueue(std::make_unique<LogQueue>(10000))  // 10,000 entry capacity
    , m_processTimer(new QTimer(this))
{
    // Connect timer to queue processing slot
    connect(m_processTimer, &QTimer::timeout, this, &LogService::processLogQueue);
    
    // Start periodic processing (every 10ms)
    // This ensures queue is processed regularly without blocking
    m_processTimer->setInterval(10);
    m_processTimer->setSingleShot(false);
}

LogService::~LogService()
{
    // Flush all pending entries before destruction
    flush();
    
    // Stop timer
    if (m_processTimer) {
        m_processTimer->stop();
    }
}

Result<void> LogService::initialize(const QString& logDir, const QString& logFileName)
{
    if (m_initialized) {
        return Result<void>::error(Error::create(
            ErrorCode::AlreadyExists,
            "LogService already initialized",
            {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}
        ));
    }
    
    if (!m_backend) {
        return Result<void>::error(Error::create(
            ErrorCode::InvalidArgument,
            "LogService backend is null",
            {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}
        ));
    }
    
    // Initialize backend
    auto backendResult = m_backend->initialize(logDir, logFileName);
    if (backendResult.isError()) {
        return Result<void>::error(Error::create(
            ErrorCode::Internal,
            "Backend initialization failed: " + backendResult.error().message,
            {{"logDir", logDir.toStdString()}, {"logFileName", logFileName.toStdString()}}
        ));
    }
    
    // Start queue processing timer
    // Note: This must be called after LogService is moved to Database I/O Thread
    m_processTimer->start();
    
    m_initialized = true;
    return Result<void>::ok();
}

void LogService::trace(const QString& message, const QVariantMap& context)
{
    enqueueLog(::ZMonitor::LogLevel::Trace, message, context);
}

void LogService::debug(const QString& message, const QVariantMap& context)
{
    enqueueLog(::ZMonitor::LogLevel::Debug, message, context);
}

void LogService::info(const QString& message, const QVariantMap& context)
{
    enqueueLog(::ZMonitor::LogLevel::Info, message, context);
}

void LogService::warning(const QString& message, const QVariantMap& context)
{
    enqueueLog(::ZMonitor::LogLevel::Warning, message, context);
}

void LogService::error(const QString& message, const QVariantMap& context)
{
    enqueueLog(::ZMonitor::LogLevel::Error, message, context);
}

void LogService::critical(const QString& message, const QVariantMap& context)
{
    enqueueLog(::ZMonitor::LogLevel::Critical, message, context);
}

void LogService::fatal(const QString& message, const QVariantMap& context)
{
    enqueueLog(::ZMonitor::LogLevel::Fatal, message, context);
}

void LogService::setLogLevel(::ZMonitor::LogLevel level)
{
    m_minLevel = level;
}

::ZMonitor::LogLevel LogService::logLevel() const
{
    return m_minLevel;
}

void LogService::setCategoryEnabled(const QString& category, bool enabled)
{
    m_categoryEnabled[category] = enabled;
}

bool LogService::isCategoryEnabled(const QString& category) const
{
    // If category not explicitly set, default to enabled
    return m_categoryEnabled.value(category, true);
}

QList<LogEntry> LogService::recentLogs() const
{
    QMutexLocker locker(&m_recentLogsMutex);
    return m_recentLogs;
}

void LogService::flush()
{
    if (!m_initialized || !m_backend) {
        return;
    }
    
    // Process all remaining entries in the queue
    int processed = 0;
    const int MAX_FLUSH_ITERATIONS = 10000;  // Prevent infinite loop
    
    while (processed < MAX_FLUSH_ITERATIONS) {
        LogEntry entry;
        if (!m_logQueue->try_dequeue(entry)) {
            break;  // Queue is empty
        }
        
        // Write to backend
        m_backend->write(entry);
        processed++;
    }
    
    // Flush backend
    m_backend->flush();
}

void LogService::processLogQueue()
{
    if (!m_initialized || !m_backend) {
        return;
    }
    
    // Process queue in batches to avoid blocking database operations
    int processed = 0;
    LogEntry entry;
    
    while (processed < MAX_BATCH && m_logQueue->try_dequeue(entry)) {
        // Write to backend
        m_backend->write(entry);
        
        // Update in-memory buffer for Diagnostics View
        {
            QMutexLocker locker(&m_recentLogsMutex);
            m_recentLogs.append(entry);
            
            // Keep only last MAX_RECENT_LOGS entries
            if (m_recentLogs.size() > MAX_RECENT_LOGS) {
                m_recentLogs.removeFirst();
            }
        }
        
        // Emit signal for Diagnostics View
        emit logEntryAdded(entry);
        
        processed++;
    }
    
    // Rotate logs if needed (periodic check)
    static int rotationCounter = 0;
    rotationCounter++;
    if (rotationCounter >= 100) {  // Check every 100 batches (~1 second)
        m_backend->rotateIfNeeded();
        rotationCounter = 0;
    }
}

void LogService::enqueueLog(::ZMonitor::LogLevel level, const QString& message,
                            const QVariantMap& context, const QString& category)
{
    // Check log level filter
    if (static_cast<int>(level) < static_cast<int>(m_minLevel)) {
        return;  // Discard entry
    }
    
    // Check category filter
    if (!category.isEmpty() && !isCategoryEnabled(category)) {
        return;  // Category disabled
    }
    
    // Create log entry
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;  // LogLevel is now the same type from LogEntry
    entry.category = category;
    entry.message = message;
    entry.context = context;
    entry.threadId = getCurrentThreadId();
    // Note: file, line, function would be populated by macros (e.g., LOG_INFO, LOG_WARNING)
    // For now, leave them empty - can be enhanced with __FILE__, __LINE__, __FUNCTION__ macros
    
    // Enqueue to lock-free queue (returns immediately)
    m_logQueue->enqueue(entry);
}

QString LogService::getCurrentThreadId()
{
    // Get current thread ID as string
    return QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()), 16);
}

} // namespace zmon
} // namespace zmon