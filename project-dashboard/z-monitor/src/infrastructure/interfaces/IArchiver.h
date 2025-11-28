/**
 * @file IArchiver.h
 * @brief Interface for data archival and retention management.
 *
 * This file defines the IArchiver interface which provides a standardized
 * way to archive old data to archive files or remote storage, following
 * data retention policies.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QString>
#include <QDateTime>
#include <QList>
#include <QVariantMap>
#include <cstdint>

namespace ZMonitor {
namespace Infrastructure {
namespace Interfaces {

/**
 * @struct ArchiveResult
 * @brief Result of archival operation.
 */
struct ArchiveResult {
    bool success;              ///< true if archival succeeded
    QString archiveLocation;    ///< Path to archive file or remote location
    int64_t recordsArchived;   ///< Number of records archived
    QString errorMessage;       ///< Error message if failed
};

/**
 * @class IArchiver
 * @brief Interface for data archival and retention management.
 *
 * Provides a standardized interface for archiving old data to archive files
 * or remote storage, following data retention policies. Archival operations
 * are transactional and create archival job records for tracking.
 *
 * @note Archival operations should run on Database I/O Thread
 * @note All operations are synchronous (blocking) to ensure transactional integrity
 *
 * @ingroup Infrastructure
 */
class IArchiver {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IArchiver() = default;

    /**
     * @brief Archive vitals data older than cutoff timestamp.
     *
     * Exports vitals data older than cutoff into an archive file or remote store,
     * creates an archival_jobs entry, and on success, deletes original rows
     * within a transaction.
     *
     * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
     * @return ArchiveResult Result of archival operation
     */
    virtual ArchiveResult archiveVitals(int64_t cutoffTimeMs) = 0;

    /**
     * @brief Archive alarm history older than cutoff timestamp.
     *
     * Exports alarm history older than cutoff into an archive file or remote store,
     * creates an archival_jobs entry, and on success, deletes original rows
     * within a transaction.
     *
     * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
     * @return ArchiveResult Result of archival operation
     */
    virtual ArchiveResult archiveAlarms(int64_t cutoffTimeMs) = 0;

    /**
     * @brief Archive telemetry batches older than cutoff timestamp.
     *
     * Exports telemetry batches older than cutoff into an archive file or remote store,
     * creates an archival_jobs entry, and on success, deletes original rows
     * within a transaction.
     *
     * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
     * @return ArchiveResult Result of archival operation
     */
    virtual ArchiveResult archiveTelemetry(int64_t cutoffTimeMs) = 0;

    /**
     * @brief Archive audit log entries older than cutoff timestamp.
     *
     * Exports audit log entries older than cutoff into an archive file or remote store,
     * creates an archival_jobs entry, and on success, deletes original rows
     * within a transaction.
     *
     * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
     * @return ArchiveResult Result of archival operation
     */
    virtual ArchiveResult archiveAuditLog(int64_t cutoffTimeMs) = 0;

    /**
     * @brief Archive all data types older than cutoff timestamp.
     *
     * Archives vitals, alarms, telemetry, and audit log data in a single
     * operation. Each data type is archived in its own transaction.
     *
     * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
     * @return QList<ArchiveResult> Results for each data type
     */
    virtual QList<ArchiveResult> archiveAll(int64_t cutoffTimeMs) = 0;

    /**
     * @brief Get archival job history.
     *
     * Retrieves list of archival jobs that have been performed.
     *
     * @param startTimeMs Start time in milliseconds (epoch milliseconds)
     * @param endTimeMs End time in milliseconds (epoch milliseconds)
     * @return List of archival job records
     */
    virtual QList<QVariantMap> getArchivalHistory(int64_t startTimeMs, int64_t endTimeMs) = 0;

    /**
     * @brief Restore archived data from archive file.
     *
     * Restores archived data from an archive file back into the database.
     * Used for data recovery or analysis.
     *
     * @param archiveLocation Path to archive file
     * @return ArchiveResult Result of restore operation (recordsArchived = records restored)
     */
    virtual ArchiveResult restoreFromArchive(const QString& archiveLocation) = 0;
};

} // namespace Interfaces
} // namespace Infrastructure
} // namespace ZMonitor

