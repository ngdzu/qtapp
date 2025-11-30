/**
 * @file MockLogBackend.cpp
 * @brief Implementation of MockLogBackend for testing.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "MockLogBackend.h"
#include <QMutexLocker>

namespace ZMonitor
{
    namespace Infrastructure
    {
        namespace Logging
        {

            MockLogBackend::MockLogBackend()
                : m_format("human"), m_maxFileSize(10 * 1024 * 1024) // 10 MB
                  ,
                  m_maxFiles(5), m_flushCount(0), m_rotationCount(0), m_initialized(false)
            {
            }

            zmon::Result<void> MockLogBackend::initialize(const QString &logDir, const QString &logFileName)
            {
                Q_UNUSED(logDir);
                Q_UNUSED(logFileName);

                QMutexLocker locker(&m_mutex);
                m_initialized = true;
                return zmon::Result<void>::ok();
            }

            void MockLogBackend::write(const zmon::LogEntry &entry)
            {
                QMutexLocker locker(&m_mutex);
                m_entries.append(entry);
            }

            void MockLogBackend::flush()
            {
                QMutexLocker locker(&m_mutex);
                m_flushCount++;
            }

            void MockLogBackend::rotateIfNeeded()
            {
                QMutexLocker locker(&m_mutex);
                m_rotationCount++;
            }

            void MockLogBackend::setFormat(const QString &format)
            {
                QMutexLocker locker(&m_mutex);
                m_format = format;
            }

            void MockLogBackend::setMaxFileSize(qint64 maxSizeBytes)
            {
                QMutexLocker locker(&m_mutex);
                m_maxFileSize = maxSizeBytes;
            }

            void MockLogBackend::setMaxFiles(int maxFiles)
            {
                QMutexLocker locker(&m_mutex);
                m_maxFiles = maxFiles;
            }

            QList<zmon::LogEntry> MockLogBackend::entries() const
            {
                QMutexLocker locker(&m_mutex);
                return m_entries;
            }

            int MockLogBackend::entryCount() const
            {
                QMutexLocker locker(&m_mutex);
                return m_entries.size();
            }

            void MockLogBackend::clear()
            {
                QMutexLocker locker(&m_mutex);
                m_entries.clear();
            }

            QString MockLogBackend::format() const
            {
                QMutexLocker locker(&m_mutex);
                return m_format;
            }

            qint64 MockLogBackend::maxFileSize() const
            {
                QMutexLocker locker(&m_mutex);
                return m_maxFileSize;
            }

            int MockLogBackend::maxFiles() const
            {
                QMutexLocker locker(&m_mutex);
                return m_maxFiles;
            }

            int MockLogBackend::flushCount() const
            {
                QMutexLocker locker(&m_mutex);
                return m_flushCount;
            }

            int MockLogBackend::rotationCount() const
            {
                QMutexLocker locker(&m_mutex);
                return m_rotationCount;
            }

        } // namespace Logging
    } // namespace Infrastructure
} // namespace ZMonitor
