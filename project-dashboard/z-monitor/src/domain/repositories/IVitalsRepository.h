/**
 * @file IVitalsRepository.h
 * @brief Repository interface for vital record persistence.
 * 
 * This file contains the IVitalsRepository interface which defines the contract
 * for persisting and retrieving vital record data. Repository interfaces are
 * defined in the domain layer to ensure business logic is independent of
 * infrastructure implementations.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/monitoring/VitalRecord.h"
#include "domain/common/Result.h"
#include <string>
#include <vector>
#include <cstdint>

namespace zmon {
/**
 * @class IVitalsRepository
 * @brief Repository interface for vital record persistence.
 * 
 * This interface defines the contract for persisting and retrieving vital
 * record data. Implementations (e.g., SQLiteVitalsRepository) live in the
 * infrastructure layer and provide database persistence.
 * 
 * @note Repository interfaces are defined in domain layer (no infrastructure dependencies).
 * @note Implementations are in infrastructure layer (SQLite, in-memory, etc.).
 */
class IVitalsRepository {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IVitalsRepository() = default;
    
    /**
     * @brief Save a single vital record.
     * 
     * Persists a vital record to storage.
     * 
     * @param vital Vital record to save
     * @return Result<void> - Success if save succeeded, Error with details if failed
     */
    virtual Result<void> save(const VitalRecord& vital) = 0;
    
    /**
     * @brief Save multiple vital records in batch.
     * 
     * Persists multiple vital records in a single transaction for efficiency.
     * 
     * @param vitals Vector of vital records to save
     * @return Result<size_t> - Success with number of records saved, Error with details if failed
     */
    virtual Result<size_t> saveBatch(const std::vector<VitalRecord>& vitals) = 0;
    
    /**
     * @brief Get vital records within a time range.
     * 
     * Retrieves vital records for a patient within the specified time range.
     * 
     * @param patientMrn Patient MRN (empty string for all patients)
     * @param startTimeMs Start time in milliseconds (epoch milliseconds)
     * @param endTimeMs End time in milliseconds (epoch milliseconds)
     * @return Vector of vital records
     */
    virtual std::vector<VitalRecord> getRange(
        const std::string& patientMrn, int64_t startTimeMs, int64_t endTimeMs) = 0;
    
    /**
     * @brief Get unsent vital records.
     * 
     * Retrieves vital records that have not been included in a transmitted batch.
     * 
     * @return Vector of unsent vital records
     */
    virtual std::vector<VitalRecord> getUnsent() = 0;
    
    /**
     * @brief Mark vital records as sent.
     * 
     * Updates vital records to indicate they have been included in a transmitted batch.
     * 
     * @param vitalIds Vector of vital record identifiers
     * @return Number of records marked as sent
     */
    virtual size_t markAsSent(const std::vector<std::string>& vitalIds) = 0;
};

} // namespace zmon