/**
 * @file IAlarmRepository.h
 * @brief Repository interface for alarm aggregate persistence.
 * 
 * This file contains the IAlarmRepository interface which defines the contract
 * for persisting and retrieving alarm aggregate data. Repository interfaces are
 * defined in the domain layer to ensure business logic is independent of
 * infrastructure implementations.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/monitoring/AlarmSnapshot.h"
#include "domain/common/Result.h"
#include <string>
#include <vector>
#include <cstdint>

namespace zmon {
/**
 * @class IAlarmRepository
 * @brief Repository interface for alarm aggregate persistence.
 * 
 * This interface defines the contract for persisting and retrieving alarm
 * aggregate data. Implementations (e.g., SQLiteAlarmRepository) live in the
 * infrastructure layer and provide database persistence.
 * 
 * @note Repository interfaces are defined in domain layer (no infrastructure dependencies).
 * @note Implementations are in infrastructure layer (SQLite, in-memory, etc.).
 */
class IAlarmRepository {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IAlarmRepository() = default;
    
    /**
     * @brief Save alarm snapshot.
     * 
     * Persists alarm snapshot to storage.
     * 
     * @param alarm Alarm snapshot to save
     * @return Result<void> - Success if save succeeded, Error with details if failed
     */
    virtual Result<void> save(const Monitoring::AlarmSnapshot& alarm) = 0;
    
    /**
     * @brief Get active alarms.
     * 
     * Retrieves all currently active alarms.
     * 
     * @return Vector of active alarm snapshots
     */
    virtual std::vector<Monitoring::AlarmSnapshot> getActive() = 0;
    
    /**
     * @brief Get alarm history.
     * 
     * Retrieves alarm history for a patient within a time range.
     * 
     * @param patientMrn Patient MRN (empty string for all patients)
     * @param startTimeMs Start time in milliseconds (epoch milliseconds)
     * @param endTimeMs End time in milliseconds (epoch milliseconds)
     * @return Vector of alarm snapshots (most recent first)
     */
    virtual std::vector<Monitoring::AlarmSnapshot> getHistory(
        const std::string& patientMrn, int64_t startTimeMs, int64_t endTimeMs) = 0;
    
    /**
     * @brief Find alarm by ID.
     * 
     * Retrieves alarm snapshot by alarm identifier.
     * 
     * @param alarmId Alarm identifier (UUID)
     * @return Alarm snapshot, or empty snapshot if not found
     */
    virtual Monitoring::AlarmSnapshot findById(const std::string& alarmId) = 0;
    
    /**
     * @brief Update alarm status.
     * 
     * Updates alarm status (e.g., acknowledge, silence, resolve).
     * 
     * @param alarmId Alarm identifier
     * @param status New alarm status
     * @param userId User ID who performed the action
     * @return Result<void> - Success if update succeeded, Error with details if failed
     */
    virtual Result<void> updateStatus(const std::string& alarmId, 
                              Monitoring::AlarmStatus status,
                              const std::string& userId) = 0;
};

} // namespace zmon
} // namespace zmon