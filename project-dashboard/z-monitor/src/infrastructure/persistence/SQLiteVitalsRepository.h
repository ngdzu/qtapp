/**
 * @file SQLiteVitalsRepository.h
 * @brief SQLite implementation of IVitalsRepository for vital signs persistence.
 *
 * This file contains the SQLiteVitalsRepository class which implements IVitalsRepository
 * using SQLite for persistent storage of vital sign data beyond the 3-day in-memory cache.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include "domain/repositories/IVitalsRepository.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/common/Result.h"
#include "infrastructure/persistence/IDatabaseManager.h"
#include <QObject>
#include <QString>
#include <memory>
#include <vector>

namespace zmon
{

    /**
     * @class SQLiteVitalsRepository
     * @brief SQLite implementation of IVitalsRepository for vital signs persistence.
     *
     * This repository persists vital sign data to the `vitals` table for long-term
     * storage beyond the 3-day in-memory cache. It uses:
     * - **Query Registry** for all SQL queries (no magic strings)
     * - **Schema constants** for all table/column names
     * - **Transactions** for batch inserts (performance optimization)
     * - **Prepared statements** for single inserts
     *
     * Performance targets:
     * - Batch insert: 100+ vitals/second
     * - Single insert: < 10ms
     * - Range queries: < 50ms for 1-hour range
     *
     * @note Runs on Database I/O Thread for non-blocking operations.
     * @note Uses DatabaseManager for connection management.
     *
     * @thread Database I/O Thread
     * @ingroup Infrastructure
     */
    class SQLiteVitalsRepository : public QObject, public IVitalsRepository
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructor.
         *
         * @param dbManager Database manager for connection management (shared_ptr ownership)
         * @param parent Parent QObject (for Qt parent-child ownership)
         */
        explicit SQLiteVitalsRepository(
            std::shared_ptr<IDatabaseManager> dbManager,
            QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~SQLiteVitalsRepository() override;

        /**
         * @brief Save a single vital record.
         *
         * Persists a vital record to the `vitals` table using a prepared statement.
         *
         * @param vital Vital record to save
         * @return Result<void> - Success if save succeeded, Error with details if failed
         *
         * @note Uses QueryId::Vitals::INSERT with prepared statement
         * @note Converts VitalRecord to database row using Schema::Columns::Vitals constants
         */
        Result<void> save(const VitalRecord &vital) override;

        /**
         * @brief Save multiple vital records in batch.
         *
         * Persists multiple vital records in a single transaction for efficiency.
         * Target throughput: 100+ vitals/second.
         *
         * @param vitals Vector of vital records to save
         * @return Result<size_t> - Success with number of records saved, Error with details if failed
         *
         * @note Uses transaction with multiple prepared statement executions
         * @note Rolls back entire transaction if any insert fails
         */
        Result<size_t> saveBatch(const std::vector<VitalRecord> &vitals) override;

        /**
         * @brief Get vital records within a time range.
         *
         * Retrieves vital records for a patient within the specified time range.
         * Uses index on (patient_mrn, timestamp) for efficient range scan.
         *
         * @param patientMrn Patient MRN (empty string for all patients)
         * @param startTimeMs Start time in milliseconds (epoch milliseconds)
         * @param endTimeMs End time in milliseconds (epoch milliseconds)
         * @return Vector of vital records (ordered by timestamp ascending)
         *
         * @note Uses QueryId::Vitals::FIND_BY_PATIENT_RANGE
         * @note Returns empty vector if no records found (not an error)
         */
        std::vector<VitalRecord> getRange(
            const std::string &patientMrn, int64_t startTimeMs, int64_t endTimeMs) override;

        /**
         * @brief Get unsent vital records.
         *
         * Retrieves vital records that have not been included in a transmitted batch.
         * Uses index on is_synced for efficient filtering.
         *
         * @return Vector of unsent vital records (ordered by timestamp ascending)
         *
         * @note Uses QueryId::Vitals::FIND_UNSENT
         * @note Returns empty vector if no unsent records (not an error)
         */
        std::vector<VitalRecord> getUnsent() override;

        /**
         * @brief Mark vital records as sent.
         *
         * Updates vital records to indicate they have been included in a transmitted batch.
         * Sets is_synced = 1 and batch_id for the specified records.
         *
         * @param vitalIds Vector of vital record identifiers (stringified IDs)
         * @return Number of records marked as sent
         *
         * @note Uses QueryId::Vitals::MARK_SENT
         * @note Uses transaction to ensure atomicity
         */
        size_t markAsSent(const std::vector<std::string> &vitalIds) override;

        /**
         * @brief Delete vitals older than specified timestamp (retention policy).
         *
         * Implements 7-day retention policy by deleting vitals older than the
         * specified timestamp. Typically called daily with timestamp = now - 7 days.
         *
         * @param timestampMs Unix milliseconds threshold
         * @return Number of records deleted
         *
         * @note Uses QueryId::Vitals::DELETE_OLDER_THAN
         */
        size_t deleteOlderThan(int64_t timestampMs);

        /**
         * @brief Count vitals for a patient.
         *
         * Returns total count of vital records for the specified patient.
         * Useful for monitoring database growth and cache statistics.
         *
         * @param patientMrn Patient MRN
         * @return Total count of vitals
         *
         * @note Uses QueryId::Vitals::COUNT_BY_PATIENT
         */
        size_t countByPatient(const std::string &patientMrn);

    private:
        /**
         * @brief Database manager for connection access.
         */
        std::shared_ptr<IDatabaseManager> m_dbManager;

        /**
         * @brief Convert database row to VitalRecord value object.
         *
         * @param query QSqlQuery positioned at a result row
         * @return VitalRecord constructed from row data
         *
         * @note Helper method for result set processing
         */
        VitalRecord rowToVitalRecord(const QSqlQuery &query) const;
    };

} // namespace zmon
