/**
 * @file AlarmAggregate.h
 * @brief Domain aggregate representing alarm lifecycle and state transitions.
 * 
 * This file contains the AlarmAggregate class which manages alarm lifecycle,
 * state transitions (raise, acknowledge, silence, escalate), and alarm history.
 * It enforces business invariants and raises domain events (AlarmRaised, AlarmAcknowledged).
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "AlarmSnapshot.h"
#include "AlarmThreshold.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

namespace ZMonitor {
namespace Domain {
namespace Monitoring {

// Forward declarations for domain events
class AlarmRaised;
class AlarmAcknowledged;

/**
 * @class AlarmAggregate
 * @brief Domain aggregate managing alarm lifecycle and state transitions.
 * 
 * This aggregate encapsulates alarm state, history, and threshold evaluation.
 * It enforces business rules such as:
 * - Alarms must be raised when vital signs violate thresholds
 * - Alarms can only be acknowledged by authorized users
 * - Alarm history is preserved for audit/compliance
 * - Alarm escalation based on duration and priority
 * 
 * The aggregate raises domain events (AlarmRaised, AlarmAcknowledged) which
 * are consumed by application services and UI controllers.
 * 
 * @note This is a domain aggregate - it has identity (alarm ID) and enforces invariants.
 * @note Domain layer has no Qt dependencies - uses standard C++ only.
 */
class AlarmAggregate {
public:
    /**
     * @brief Constructor.
     * 
     * Creates a new alarm aggregate with no active alarms.
     */
    AlarmAggregate();
    
    /**
     * @brief Destructor.
     */
    ~AlarmAggregate();
    
    /**
     * @brief Raise a new alarm.
     * 
     * Creates a new alarm when a vital sign violates a threshold. Raises
     * AlarmRaised domain event.
     * 
     * @param alarmType Alarm type (e.g., "HR_HIGH", "SPO2_LOW")
     * @param priority Alarm priority level
     * @param value Vital sign value that triggered the alarm
     * @param threshold Threshold value that was exceeded
     * @param patientMrn Patient MRN (empty if no patient admitted)
     * @param deviceId Device identifier
     * @return Alarm snapshot, or empty snapshot if raise failed
     * 
     * @note Business rule: Duplicate alarms (same type, same patient) may be suppressed.
     */
    AlarmSnapshot raise(const std::string& alarmType, AlarmPriority priority,
                        double value, double threshold, const std::string& patientMrn,
                        const std::string& deviceId);
    
    /**
     * @brief Acknowledge an alarm.
     * 
     * Marks an alarm as acknowledged by a user. Raises AlarmAcknowledged
     * domain event.
     * 
     * @param alarmId Alarm identifier
     * @param userId User ID who acknowledged the alarm
     * @return true if acknowledgment succeeded, false if alarm not found or already acknowledged
     */
    bool acknowledge(const std::string& alarmId, const std::string& userId);
    
    /**
     * @brief Silence an alarm temporarily.
     * 
     * Temporarily silences an alarm (e.g., for 5 minutes). Alarm will
     * re-activate if condition persists.
     * 
     * @param alarmId Alarm identifier
     * @param durationMs Silence duration in milliseconds
     * @return true if silence succeeded, false if alarm not found
     */
    bool silence(const std::string& alarmId, int64_t durationMs);
    
    /**
     * @brief Escalate an alarm.
     * 
     * Escalates an alarm to higher priority (e.g., after timeout).
     * 
     * @param alarmId Alarm identifier
     * @return true if escalation succeeded, false if alarm not found
     */
    bool escalate(const std::string& alarmId);
    
    /**
     * @brief Resolve an alarm.
     * 
     * Marks an alarm as resolved (condition no longer present).
     * 
     * @param alarmId Alarm identifier
     * @return true if resolution succeeded, false if alarm not found
     */
    bool resolve(const std::string& alarmId);
    
    /**
     * @brief Get active alarms.
     * 
     * Retrieves all currently active alarms.
     * 
     * @return Vector of active alarm snapshots
     */
    std::vector<AlarmSnapshot> getActiveAlarms() const;
    
    /**
     * @brief Get alarm history.
     * 
     * Retrieves alarm history within a time range.
     * 
     * @param startTimeMs Start time in milliseconds (epoch milliseconds)
     * @param endTimeMs End time in milliseconds (epoch milliseconds)
     * @return Vector of alarm snapshots (most recent first)
     */
    std::vector<AlarmSnapshot> getHistory(int64_t startTimeMs, int64_t endTimeMs) const;
    
    /**
     * @brief Find alarm by ID.
     * 
     * @param alarmId Alarm identifier
     * @return Alarm snapshot, or empty snapshot if not found
     */
    AlarmSnapshot findById(const std::string& alarmId) const;
    
    /**
     * @brief Check if an alarm is active.
     * 
     * @param alarmId Alarm identifier
     * @return true if alarm is active, false otherwise
     */
    bool isActive(const std::string& alarmId) const;

private:
    // Active alarms (keyed by alarm ID)
    std::map<std::string, AlarmSnapshot> m_activeAlarms;
    
    // Alarm history (all alarms, including resolved)
    std::vector<AlarmSnapshot> m_alarmHistory;
    
    static constexpr size_t MAX_HISTORY_SIZE = 10000;
    
    /**
     * @brief Generate a unique alarm ID (UUID).
     * 
     * @return UUID string
     */
    std::string generateAlarmId() const;
    
    /**
     * @brief Get current timestamp in milliseconds.
     * 
     * @return Current Unix timestamp in milliseconds
     */
    int64_t getCurrentTimestampMs() const;
    
    /**
     * @brief Check if duplicate alarm should be suppressed.
     * 
     * @param alarmType Alarm type
     * @param patientMrn Patient MRN
     * @return true if duplicate should be suppressed, false otherwise
     */
    bool shouldSuppressDuplicate(const std::string& alarmType, const std::string& patientMrn) const;
};

} // namespace Monitoring
} // namespace Domain
} // namespace ZMonitor

