/**
 * @file ITelemetryRepository.h
 * @brief Repository interface for telemetry batch persistence.
 * 
 * This file contains the ITelemetryRepository interface which defines the contract
 * for persisting and retrieving telemetry batch data. Repository interfaces are
 * defined in the domain layer to ensure business logic is independent of
 * infrastructure implementations.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace ZMonitor {
namespace Domain {
namespace Monitoring {
    class TelemetryBatch;
}

namespace Repositories {

/**
 * @class ITelemetryRepository
 * @brief Repository interface for telemetry batch persistence.
 * 
 * This interface defines the contract for persisting and retrieving telemetry
 * batch data. Implementations (e.g., SQLiteTelemetryRepository) live in the
 * infrastructure layer and provide database persistence.
 * 
 * @note Repository interfaces are defined in domain layer (no infrastructure dependencies).
 * @note Implementations are in infrastructure layer (SQLite, in-memory, etc.).
 */
class ITelemetryRepository {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ITelemetryRepository() = default;
    
    /**
     * @brief Save telemetry batch.
     * 
     * Persists telemetry batch to storage.
     * 
     * @param batch Telemetry batch to save
     * @return true if save succeeded, false otherwise
     */
    virtual bool save(const Monitoring::TelemetryBatch& batch) = 0;
    
    /**
     * @brief Get historical telemetry batches.
     * 
     * Retrieves telemetry batches within a time range.
     * 
     * @param startTimeMs Start time in milliseconds (epoch milliseconds)
     * @param endTimeMs End time in milliseconds (epoch milliseconds)
     * @return Vector of telemetry batches
     */
    virtual std::vector<std::shared_ptr<Monitoring::TelemetryBatch>> getHistorical(
        int64_t startTimeMs, int64_t endTimeMs) = 0;
    
    /**
     * @brief Archive old telemetry batches.
     * 
     * Moves telemetry batches older than cutoff to archive storage.
     * 
     * @param cutoffTimeMs Cutoff time in milliseconds (epoch milliseconds)
     * @return Number of batches archived
     */
    virtual size_t archive(int64_t cutoffTimeMs) = 0;
    
    /**
     * @brief Get unsent telemetry batches.
     * 
     * Retrieves telemetry batches that have not been successfully transmitted.
     * 
     * @return Vector of unsent telemetry batches
     */
    virtual std::vector<std::shared_ptr<Monitoring::TelemetryBatch>> getUnsent() = 0;
    
    /**
     * @brief Mark telemetry batch as sent.
     * 
     * Updates batch status to indicate successful transmission.
     * 
     * @param batchId Batch identifier
     * @return true if update succeeded, false otherwise
     */
    virtual bool markAsSent(const std::string& batchId) = 0;
};

} // namespace Repositories
} // namespace Domain
} // namespace ZMonitor

