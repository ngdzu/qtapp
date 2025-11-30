/**
 * @file SQLiteAlarmRepository.h
 * @brief SQLite implementation of IAlarmRepository interface.
 *
 * This file contains the SQLiteAlarmRepository class which implements the
 * IAlarmRepository interface using Qt SQL framework and SQLite database.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include "domain/repositories/IAlarmRepository.h"
#include "infrastructure/persistence/IDatabaseManager.h"
#include <QObject>
#include <memory>

namespace zmon
{

    /**
     * @class SQLiteAlarmRepository
     * @brief SQLite implementation of alarm repository.
     *
     * This class provides SQLite-based persistence for alarm aggregates. It uses
     * the Query Registry pattern for all SQL queries and Schema constants for
     * column names to avoid magic strings.
     *
     * Key responsibilities:
     * - Persist alarm snapshots to database
     * - Retrieve active alarms
     * - Query alarm history by patient and time range
     * - Update alarm status (acknowledge, silence, resolve)
     * - Find alarms by ID
     *
     * Performance characteristics:
     * - Save operation: < 10ms (single insert with prepared statement)
     * - Active alarms query: < 20ms (indexed by status)
     * - History query: < 50ms (indexed by patient_mrn and start_time)
     * - Status update: < 5ms (UPDATE by primary key)
     *
     * Thread safety: All database operations are executed via DatabaseManager
     * which ensures thread-safe access through the Database I/O Thread.
     *
     * @note Uses Query Registry pattern - all SQL queries are defined in QueryCatalog
     * @note Uses Schema constants - all column names from SchemaInfo.h
     * @note Returns Result<void> for write operations (save, update)
     * @note Returns vectors for read operations (empty vector on error)
     */
    class SQLiteAlarmRepository : public QObject, public IAlarmRepository
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructor.
         *
         * @param dbManager Shared pointer to database manager
         * @param parent Optional QObject parent for lifecycle management
         */
        explicit SQLiteAlarmRepository(std::shared_ptr<IDatabaseManager> dbManager,
                                       QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~SQLiteAlarmRepository() override = default;

        /**
         * @brief Save alarm snapshot to database.
         *
         * Inserts alarm snapshot into alarms table with all metadata. Uses prepared
         * statement for performance.
         *
         * @param alarm Alarm snapshot to save
         * @return Result<void> - Success if insert succeeded, Error with details if failed
         *
         * @note Performance: < 10ms per save (prepared statement)
         * @note Thread-safe: Uses DatabaseManager (Database I/O Thread)
         */
        Result<void> save(const AlarmSnapshot &alarm) override;

        /**
         * @brief Get all active alarms.
         *
         * Retrieves alarms with status = 'ACTIVE', ordered by priority (CRITICAL first)
         * and start time (most recent first).
         *
         * @return Vector of active alarm snapshots (empty if none or on error)
         *
         * @note Performance: < 20ms (indexed by status)
         * @note Returns empty vector on error (check logs for details)
         */
        std::vector<AlarmSnapshot> getActive() override;

        /**
         * @brief Get alarm history for patient in time range.
         *
         * Retrieves alarms for specified patient within time range, ordered by
         * start time descending (most recent first).
         *
         * @param patientMrn Patient MRN (empty string for all patients)
         * @param startTimeMs Start time in milliseconds (epoch milliseconds)
         * @param endTimeMs End time in milliseconds (epoch milliseconds)
         * @return Vector of alarm snapshots (empty if none or on error)
         *
         * @note Performance: < 50ms (indexed by patient_mrn and start_time)
         * @note Returns empty vector on error (check logs for details)
         */
        std::vector<AlarmSnapshot> getHistory(
            const std::string &patientMrn, int64_t startTimeMs, int64_t endTimeMs) override;

        /**
         * @brief Find alarm by ID.
         *
         * Retrieves alarm snapshot by alarm identifier (primary key lookup).
         *
         * @param alarmId Alarm identifier (UUID)
         * @return Alarm snapshot, or empty snapshot if not found
         *
         * @note Performance: < 5ms (primary key lookup)
         * @note Returns empty AlarmSnapshot if not found (check alarmId field)
         */
        AlarmSnapshot findById(const std::string &alarmId) override;

        /**
         * @brief Update alarm status.
         *
         * Updates alarm status (e.g., ACKNOWLEDGED, SILENCED, RESOLVED) and records
         * user who performed the action and timestamp.
         *
         * @param alarmId Alarm identifier
         * @param status New alarm status
         * @param userId User ID who performed the action
         * @return Result<void> - Success if update succeeded, Error with details if failed
         *
         * @note Performance: < 5ms (UPDATE by primary key)
         * @note Thread-safe: Uses DatabaseManager (Database I/O Thread)
         * @note Sets acknowledged_time and acknowledged_by if status is ACKNOWLEDGED
         */
        Result<void> updateStatus(const std::string &alarmId,
                                  AlarmStatus status,
                                  const std::string &userId) override;

    private:
        std::shared_ptr<IDatabaseManager> m_dbManager;

        /**
         * @brief Convert database row to AlarmSnapshot.
         *
         * Helper method to construct AlarmSnapshot from QSqlQuery result set.
         *
         * @param query Query positioned at valid row
         * @return AlarmSnapshot object
         */
        AlarmSnapshot rowToAlarmSnapshot(const QSqlQuery &query) const;

        /**
         * @brief Convert AlarmStatus enum to database string.
         *
         * @param status Alarm status enum value
         * @return Status string ("ACTIVE", "ACKNOWLEDGED", "SILENCED", "RESOLVED")
         */
        QString statusToString(AlarmStatus status) const;

        /**
         * @brief Convert database string to AlarmStatus enum.
         *
         * @param statusStr Status string from database
         * @return AlarmStatus enum value (defaults to Active if unknown)
         */
        AlarmStatus stringToStatus(const QString &statusStr) const;

        /**
         * @brief Convert AlarmPriority enum to database string.
         *
         * @param priority Alarm priority enum value
         * @return Priority string ("CRITICAL", "HIGH", "MEDIUM", "LOW")
         */
        QString priorityToString(AlarmPriority priority) const;

        /**
         * @brief Convert database string to AlarmPriority enum.
         *
         * @param priorityStr Priority string from database
         * @return AlarmPriority enum value (defaults to LOW if unknown)
         */
        AlarmPriority stringToPriority(const QString &priorityStr) const;
    };

} // namespace zmon
