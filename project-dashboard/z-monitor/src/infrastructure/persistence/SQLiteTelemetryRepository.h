/**
 * @file SQLiteTelemetryRepository.h
 * @brief SQLite implementation of ITelemetryRepository for telemetry batch persistence.
 *
 * This file contains the SQLiteTelemetryRepository class which implements ITelemetryRepository
 * using SQLite for persistent storage of telemetry batches for central server transmission.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include "domain/repositories/ITelemetryRepository.h"
#include "domain/monitoring/TelemetryBatch.h"
#include "domain/common/Result.h"
#include "infrastructure/persistence/IDatabaseManager.h"
#include <QObject>
#include <QString>
#include <memory>
#include <vector>

namespace zmon
{

    /**
     * @class SQLiteTelemetryRepository
     * @brief SQLite implementation of ITelemetryRepository for telemetry batch persistence.
     *
     * This repository persists telemetry batches to the `telemetry_metrics` table for
     * transmission tracking and performance metrics. It uses:
     * - **Query Registry** for all SQL queries (no magic strings)
     * - **Schema constants** for all table/column names
     * - **Transactions** for batch operations
     * - **Prepared statements** for single operations
     *
     * Performance targets:
     * - Batch save: < 20ms
     * - Pending batch retrieval: < 50ms
     * - Mark as sent: < 5ms
     *
     * @note Runs on Database I/O Thread for non-blocking operations.
     * @note Uses IDatabaseManager for connection management.
     *
     * @thread Database I/O Thread
     * @ingroup Infrastructure
     */
    class SQLiteTelemetryRepository : public QObject, public ITelemetryRepository
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructor.
         *
         * @param dbManager Database manager for connection management (shared_ptr ownership)
         * @param parent Parent QObject (for Qt parent-child ownership)
         */
        explicit SQLiteTelemetryRepository(
            std::shared_ptr<IDatabaseManager> dbManager,
            QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~SQLiteTelemetryRepository() override;

        /**
         * @brief Save telemetry batch.
         *
         * Persists telemetry batch to `telemetry_metrics` table with metadata and status.
         * All vitals/alarms in the batch must already be persisted to their respective tables.
         *
         * @param batch Telemetry batch to save
         * @return Result<void> - Success if save succeeded, Error with details if failed
         *
         * @note Business rule: batch_id must be unique
         * @note Status is set to 'retrying' initially (pending transmission)
         * @note Uses prepared statement for performance
         */
        Result<void> save(const TelemetryBatch &batch) override;

        /**
         * @brief Get historical telemetry batches.
         *
         * Retrieves telemetry batches within a time range from `telemetry_metrics` table.
         * Useful for reporting, performance analysis, and debugging.
         *
         * @param startTimeMs Start time in milliseconds (epoch milliseconds)
         * @param endTimeMs End time in milliseconds (epoch milliseconds)
         * @return Vector of telemetry batches (may be empty if no batches in range)
         *
         * @note Returns batches ordered by batch_created_at ASC
         * @note Does not filter by status (returns all batches in range)
         */
        std::vector<std::shared_ptr<TelemetryBatch>> getHistorical(
            int64_t startTimeMs, int64_t endTimeMs) override;

        /**
         * @brief Archive old telemetry batches.
         *
         * Deletes telemetry batches older than cutoff from `telemetry_metrics` table.
         * This is for data retention compliance (keep metrics for analysis period only).
         *
         * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
         * @return Number of batches archived (deleted)
         *
         * @note Uses transaction for batch delete
         * @note Business rule: Only delete batches with status 'success' (don't delete pending/failed)
         */
        size_t archive(int64_t cutoffTimeMs) override;

        /**
         * @brief Get unsent telemetry batches.
         *
         * Retrieves telemetry batches that have not been successfully transmitted.
         * Used by telemetry transmission service to retry failed batches.
         *
         * @return Vector of unsent telemetry batches (status != 'success')
         *
         * @note Returns batches with status 'retrying', 'failed', or 'timeout'
         * @note Ordered by batch_created_at ASC (oldest first for fairness)
         */
        std::vector<std::shared_ptr<TelemetryBatch>> getUnsent() override;

        /**
         * @brief Mark telemetry batch as sent.
         *
         * Updates batch status to 'success' and records transmission timestamps.
         * Called after successful server acknowledgment.
         *
         * @param batchId Batch identifier (UUID)
         * @return Result<void> - Success if update succeeded, Error with details if failed
         *
         * @note Updates: status='success', transmitted_at, server_received_at, server_ack_at
         * @note Uses current timestamp for all transmission fields
         */
        Result<void> markAsSent(const std::string &batchId) override;

    private:
        /**
         * @brief Database manager for connection access.
         */
        std::shared_ptr<IDatabaseManager> m_dbManager;

        /**
         * @brief Convert database row to TelemetryBatch object.
         *
         * @param query QSqlQuery positioned at a result row
         * @return Shared pointer to TelemetryBatch constructed from row data
         *
         * @note Helper method for result set processing
         * @note Reconstructs TelemetryBatch from telemetry_metrics table
         */
        std::shared_ptr<TelemetryBatch> rowToTelemetryBatch(const QSqlQuery &query) const;
    };

} // namespace zmon
