/**
 * @file IAuditRepository.h
 * @brief Repository interface for audit log persistence.
 * 
 * This file contains the IAuditRepository interface which defines the contract
 * for persisting and retrieving audit log entries. Repository interfaces are
 * defined in the domain layer to ensure business logic is independent of
 * infrastructure implementations.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace zmon {
/**
 * @class IAuditRepository
 * @brief Repository interface for audit log persistence.
 * 
 * This interface defines the contract for persisting and retrieving audit
 * log entries. Implementations (e.g., SQLiteAuditRepository) live in the
 * infrastructure layer and provide database persistence.
 * 
 * @note Repository interfaces are defined in domain layer (no infrastructure dependencies).
 * @note Implementations are in infrastructure layer (SQLite, in-memory, etc.).
 */
class IAuditRepository {
public:
    /**
     * @brief Audit log entry structure.
     */
    struct AuditEntry {
        int64_t timestampMs;           ///< Timestamp in milliseconds (epoch milliseconds)
        std::string userId;            ///< User ID who performed action (empty if no user)
        std::string userRole;          ///< User role (NURSE, PHYSICIAN, etc.)
        std::string actionType;        ///< Action type (LOGIN, LOGOUT, ADMIT_PATIENT, etc.)
        std::string targetType;        ///< Target type (PATIENT, SETTING, etc.)
        std::string targetId;          ///< Target identifier (MRN, setting name, etc.)
        std::string details;           ///< Additional details (JSON string)
        std::string previousHash;      ///< Hash of previous entry (for hash chain)
        std::string entryHash;         ///< Hash of this entry
    };
    
    /**
     * @brief Virtual destructor.
     */
    virtual ~IAuditRepository() = default;
    
    /**
     * @brief Save audit log entry.
     * 
     * Persists audit log entry to storage. Entries are immutable once written.
     * 
     * @param entry Audit log entry to save
     * @return true if save succeeded, false otherwise
     */
    virtual bool save(const AuditEntry& entry) = 0;
    
    /**
     * @brief Get audit log entries within a time range.
     * 
     * Retrieves audit log entries within the specified time range.
     * 
     * @param startTimeMs Start time in milliseconds (epoch milliseconds)
     * @param endTimeMs End time in milliseconds (epoch milliseconds)
     * @return Vector of audit log entries (most recent first)
     */
    virtual std::vector<AuditEntry> getRange(int64_t startTimeMs, int64_t endTimeMs) = 0;
    
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
    virtual std::vector<AuditEntry> getByUser(const std::string& userId,
                                               int64_t startTimeMs, int64_t endTimeMs) = 0;
    
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
    virtual std::vector<AuditEntry> getByTarget(const std::string& targetType,
                                                 const std::string& targetId,
                                                 int64_t startTimeMs, int64_t endTimeMs) = 0;
    
    /**
     * @brief Get the last audit log entry.
     * 
     * Retrieves the most recent audit log entry (for hash chain verification).
     * 
     * @return Last audit log entry, or empty entry if no entries exist
     */
    virtual AuditEntry getLastEntry() = 0;
    
    /**
     * @brief Verify audit log integrity (hash chain).
     * 
     * Verifies that the audit log hash chain is intact (no tampering).
     * 
     * @return true if hash chain is valid, false if tampering detected
     */
    virtual bool verifyIntegrity() = 0;
    
    /**
     * @brief Archive old audit log entries.
     * 
     * Moves audit log entries older than cutoff to archive storage.
     * 
     * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
     * @return Number of entries archived
     */
    virtual size_t archive(int64_t cutoffTimeMs) = 0;
};

} // namespace zmon
} // namespace zmon