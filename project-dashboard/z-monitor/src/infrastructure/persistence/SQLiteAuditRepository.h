/**
 * @file SQLiteAuditRepository.h
 * @brief SQLite implementation of IAuditRepository.
 *
 * This file contains the SQLiteAuditRepository class which persists audit
 * log entries to the security_audit_log table in SQLite database.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/repositories/IAuditRepository.h"
#include "domain/common/Result.h"
#include <memory>

namespace zmon
{

    class DatabaseManager;

    /**
     * @class SQLiteAuditRepository
     * @brief SQLite implementation of IAuditRepository.
     *
     * Persists audit log entries to security_audit_log table in SQLite database.
     *
     * @ingroup Infrastructure
     */
    class SQLiteAuditRepository : public IAuditRepository
    {
    public:
        /**
         * @brief Constructor.
         *
         * @param dbManager Database manager instance
         */
        explicit SQLiteAuditRepository(std::shared_ptr<DatabaseManager> dbManager);

        /**
         * @brief Destructor.
         */
        ~SQLiteAuditRepository() override = default;

        /**
         * @brief Save audit log entry.
         *
         * Persists audit log entry to storage. Entries are immutable once written.
         *
         * @param entry Audit log entry to save
         * @return Result<void> - Success if save succeeded, Error with details if failed
         */
        Result<void> save(const AuditEntry &entry) override;

        /**
         * @brief Get audit log entries within a time range.
         *
         * Retrieves audit log entries within the specified time range.
         *
         * @param startTimeMs Start time in milliseconds (epoch milliseconds)
         * @param endTimeMs End time in milliseconds (epoch milliseconds)
         * @return Vector of audit log entries (most recent first)
         */
        std::vector<AuditEntry> getRange(int64_t startTimeMs, int64_t endTimeMs) override;

        /**
         * @brief Get audit log entries for a user.
         *
         * Retrieves audit log entries for a specific user.
         *
         * @param userId User identifier
         * @param startTimeMs Start time in milliseconds (epoch milliseconds)
         * @param endTimeMs End time in milliseconds (epoch milliseconds)
         * @return Vector of audit log entries (most recent first)
         */
        std::vector<AuditEntry> getByUser(const std::string &userId,
                                          int64_t startTimeMs, int64_t endTimeMs) override;

        /**
         * @brief Get audit log entries for a target.
         *
         * Retrieves audit log entries for a specific target (e.g., patient MRN).
         *
         * @param targetType Target type (PATIENT, SETTING, etc.)
         * @param targetId Target identifier
         * @param startTimeMs Start time in milliseconds (epoch milliseconds)
         * @param endTimeMs End time in milliseconds (epoch milliseconds)
         * @return Vector of audit log entries (most recent first)
         */
        std::vector<AuditEntry> getByTarget(const std::string &targetType,
                                            const std::string &targetId,
                                            int64_t startTimeMs, int64_t endTimeMs) override;

        /**
         * @brief Get the last audit log entry.
         *
         * Retrieves the most recent audit log entry (for hash chain verification).
         *
         * @return Last audit log entry, or empty entry if no entries exist
         */
        AuditEntry getLastEntry() override;

        /**
         * @brief Verify audit log integrity (hash chain).
         *
         * Verifies that the audit log hash chain is intact (no tampering).
         *
         * @return Result<bool> - Success with verification result (true if valid, false if tampering detected), Error if verification failed (e.g., database error)
         */
        Result<bool> verifyIntegrity() override;

        /**
         * @brief Archive old audit log entries.
         *
         * Moves audit log entries older than cutoff to archive storage.
         *
         * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
         * @return Number of entries archived
         */
        size_t archive(int64_t cutoffTimeMs) override;

    private:
        std::shared_ptr<DatabaseManager> m_dbManager;
    };

} // namespace zmon
