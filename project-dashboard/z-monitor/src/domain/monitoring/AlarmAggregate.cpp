/**
 * @file AlarmAggregate.cpp
 * @brief Implementation of AlarmAggregate domain aggregate.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "AlarmAggregate.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <map>
#include <chrono>

namespace zmon {
AlarmAggregate::AlarmAggregate()
    : m_activeAlarms()
    , m_alarmHistory()
{
    // Note: deque doesn't support reserve(), but it grows efficiently
}

AlarmAggregate::~AlarmAggregate() = default;

AlarmSnapshot AlarmAggregate::raise(const std::string& alarmType, AlarmPriority priority,
                                    double value, double threshold, const std::string& patientMrn,
                                    const std::string& deviceId) {
    // Business rule: Suppress duplicate alarms (same type, same patient)
    if (shouldSuppressDuplicate(alarmType, patientMrn)) {
        return AlarmSnapshot();  // Empty snapshot indicates failure
    }
    
    // Generate alarm ID
    std::string alarmId = generateAlarmId();
    
    // Create alarm snapshot
    AlarmSnapshot alarm(alarmId, alarmType, priority, AlarmStatus::Active,
                       value, threshold, getCurrentTimestampMs(), patientMrn, deviceId);
    
    // Add to active alarms (erase if exists, then insert)
    m_activeAlarms.erase(alarmId);
    m_activeAlarms.insert({alarmId, alarm});
    
    // Add to history
    m_alarmHistory.push_back(alarm);
    
    // Maintain history size limit
    if (m_alarmHistory.size() > MAX_HISTORY_SIZE) {
        m_alarmHistory.pop_front();
    }
    
    // Note: Domain event AlarmRaised would be raised here
    // (event publishing handled by application service)
    
    return alarm;
}

bool AlarmAggregate::acknowledge(const std::string& alarmId, const std::string& userId) {
    auto it = m_activeAlarms.find(alarmId);
    if (it == m_activeAlarms.end()) {
        return false;
    }
    
    // Business rule: Cannot acknowledge already acknowledged alarm
    if (it->second.status == AlarmStatus::Acknowledged) {
        return false;
    }
    
    // Update alarm status
    AlarmSnapshot updated = it->second;
    // Note: In real implementation, we'd need a mutable AlarmSnapshot or use a different approach
    // For now, we'll remove and re-add with updated status
    m_activeAlarms.erase(it);
    
    // Create updated snapshot (in real implementation, use mutable state or builder pattern)
    AlarmSnapshot acknowledged(alarmId, updated.alarmType, updated.priority,
                              AlarmStatus::Acknowledged, updated.value, updated.thresholdValue,
                              updated.timestampMs, updated.patientMrn, updated.deviceId,
                              userId, getCurrentTimestampMs());
    
    m_activeAlarms.erase(alarmId);
    m_activeAlarms.insert({alarmId, acknowledged});
    
    // Update history (rebuild deque without the old entry, then add new one)
    std::deque<AlarmSnapshot> newHistory;
    for (const auto& alarm : m_alarmHistory) {
        if (alarm.alarmId != alarmId) {
            newHistory.push_back(alarm);
        }
    }
    newHistory.push_back(acknowledged);
    m_alarmHistory = std::move(newHistory);
    
    // Note: Domain event AlarmAcknowledged would be raised here
    // (event publishing handled by application service)
    
    return true;
}

bool AlarmAggregate::silence(const std::string& alarmId, int64_t durationMs) {
    auto it = m_activeAlarms.find(alarmId);
    if (it == m_activeAlarms.end()) {
        return false;
    }
    
    // Update alarm status to silenced
    AlarmSnapshot updated = it->second;
    m_activeAlarms.erase(it);
    
    AlarmSnapshot silenced(alarmId, updated.alarmType, updated.priority,
                          AlarmStatus::Silenced, updated.value, updated.thresholdValue,
                          updated.timestampMs, updated.patientMrn, updated.deviceId);
    
    m_activeAlarms.erase(alarmId);
    m_activeAlarms.insert({alarmId, silenced});
    
    return true;
}

bool AlarmAggregate::escalate(const std::string& alarmId) {
    auto it = m_activeAlarms.find(alarmId);
    if (it == m_activeAlarms.end()) {
        return false;
    }
    
    // Escalate priority (LOW -> MEDIUM -> HIGH)
    AlarmPriority newPriority = it->second.priority;
    if (newPriority == AlarmPriority::LOW) {
        newPriority = AlarmPriority::MEDIUM;
    } else if (newPriority == AlarmPriority::MEDIUM) {
        newPriority = AlarmPriority::HIGH;
    }
    // HIGH priority cannot be escalated further
    
    AlarmSnapshot updated = it->second;
    m_activeAlarms.erase(it);
    
    AlarmSnapshot escalated(alarmId, updated.alarmType, newPriority, updated.status,
                           updated.value, updated.thresholdValue, updated.timestampMs,
                           updated.patientMrn, updated.deviceId);
    
    m_activeAlarms.erase(alarmId);
    m_activeAlarms.insert({alarmId, escalated});
    
    return true;
}

bool AlarmAggregate::resolve(const std::string& alarmId) {
    auto it = m_activeAlarms.find(alarmId);
    if (it == m_activeAlarms.end()) {
        return false;
    }
    
    // Remove from active alarms
    AlarmSnapshot resolved = it->second;
    m_activeAlarms.erase(it);
    
    // Update history with resolved status (rebuild deque without the old entry, then add new one)
    AlarmSnapshot resolvedSnapshot(alarmId, resolved.alarmType, resolved.priority,
                                  AlarmStatus::Resolved, resolved.value,
                                  resolved.thresholdValue, resolved.timestampMs,
                                  resolved.patientMrn, resolved.deviceId);
    std::deque<AlarmSnapshot> newHistory;
    for (const auto& alarm : m_alarmHistory) {
        if (alarm.alarmId != alarmId) {
            newHistory.push_back(alarm);
        }
    }
    newHistory.push_back(resolvedSnapshot);
    m_alarmHistory = std::move(newHistory);
    
    return true;
}

std::vector<AlarmSnapshot> AlarmAggregate::getActiveAlarms() const {
    std::vector<AlarmSnapshot> result;
    result.reserve(m_activeAlarms.size());
    
    for (const auto& pair : m_activeAlarms) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::vector<AlarmSnapshot> AlarmAggregate::getHistory(int64_t startTimeMs, int64_t endTimeMs) const {
    std::vector<AlarmSnapshot> result;
    
    for (const auto& alarm : m_alarmHistory) {
        if (alarm.timestampMs >= startTimeMs && alarm.timestampMs <= endTimeMs) {
            result.push_back(alarm);
        }
    }
    
    // Sort by timestamp (most recent first)
    // Since AlarmSnapshot has deleted copy assignment, we can't use std::sort directly.
    // Instead, we'll sort indices and build a new vector in sorted order
    std::vector<size_t> indices(result.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
              [&result](size_t a, size_t b) {
                  return result[a].timestampMs > result[b].timestampMs;
              });
    std::vector<AlarmSnapshot> sortedResult;
    sortedResult.reserve(result.size());
    for (size_t idx : indices) {
        sortedResult.push_back(result[idx]);
    }
    result = std::move(sortedResult);
    
    return result;
}

AlarmSnapshot AlarmAggregate::findById(const std::string& alarmId) const {
    auto it = m_activeAlarms.find(alarmId);
    if (it != m_activeAlarms.end()) {
        return it->second;
    }
    
    // Search history
    for (const auto& alarm : m_alarmHistory) {
        if (alarm.alarmId == alarmId) {
            return alarm;
        }
    }
    
    return AlarmSnapshot();  // Empty snapshot indicates not found
}

bool AlarmAggregate::isActive(const std::string& alarmId) const {
    return m_activeAlarms.find(alarmId) != m_activeAlarms.end();
}

std::string AlarmAggregate::generateAlarmId() const {
    // Simple UUID v4 generation (for domain layer, no external dependencies)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    for (int i = 0; i < 32; i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << "-";
        }
        int n = (i == 12) ? dis2(gen) : dis(gen);
        ss << n;
    }
    
    return ss.str();
}

int64_t AlarmAggregate::getCurrentTimestampMs() const {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return ms.count();
}

bool AlarmAggregate::shouldSuppressDuplicate(const std::string& alarmType,
                                              const std::string& patientMrn) const {
    // Business rule: Suppress duplicate alarms of same type for same patient
    // within a short time window (e.g., 5 seconds)
    int64_t currentTime = getCurrentTimestampMs();
    int64_t windowMs = 5000;  // 5 seconds
    
    for (const auto& alarm : m_activeAlarms) {
        if (alarm.second.alarmType == alarmType &&
            alarm.second.patientMrn == patientMrn &&
            (currentTime - alarm.second.timestampMs) < windowMs) {
            return true;  // Duplicate found within time window
        }
    }
    
    return false;
}

} // namespace zmon