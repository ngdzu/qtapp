/**
 * @file AlarmThreshold.h
 * @brief Value object representing alarm threshold configuration.
 * 
 * This file contains the AlarmThreshold value object which represents alarm
 * threshold configuration (min/max values, priority, hysteresis). Value objects
 * are immutable and defined by their attributes.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>

namespace zmon {
/**
 * @enum AlarmPriority
 * @brief Alarm priority levels.
 */
enum class AlarmPriority {
    LOW,      ///< Low priority (advisory)
    MEDIUM,   ///< Medium priority (warning)
    HIGH      ///< High priority (critical)
};

/**
 * @class AlarmThreshold
 * @brief Immutable value object representing alarm threshold configuration.
 * 
 * This value object encapsulates alarm threshold configuration including
 * low/high limits, hysteresis, priority, and enabled state. It is immutable
 * and can be safely shared across threads.
 * 
 * @note This is a value object - it has no identity and is defined by its attributes.
 * @note All members are const to enforce immutability.
 */
struct AlarmThreshold {
    /**
     * @brief Vital sign type identifier.
     * 
     * Examples: "HR" (heart rate), "SPO2" (oxygen saturation), "RR" (respiration rate).
     */
    const std::string vitalType;
    
    /**
     * @brief Low threshold value.
     * 
     * Alarm triggers if vital sign value falls below this threshold.
     */
    const double lowLimit;
    
    /**
     * @brief High threshold value.
     * 
     * Alarm triggers if vital sign value exceeds this threshold.
     */
    const double highLimit;
    
    /**
     * @brief Hysteresis range.
     * 
     * Prevents alarm flutter. Typical value: ±5% of threshold.
     * Example: If threshold is 60, hysteresis of 5% means alarm clears at 63.
     */
    const double hysteresis;
    
    /**
     * @brief Alarm priority level.
     */
    const AlarmPriority priority;
    
    /**
     * @brief Whether this alarm is enabled.
     * 
     * If false, this threshold is ignored during alarm evaluation.
     */
    const bool enabled;
    
    /**
     * @brief Default constructor.
     * 
     * Creates an empty alarm threshold with default values.
     */
    AlarmThreshold()
        : vitalType("")
        , lowLimit(0.0)
        , highLimit(0.0)
        , hysteresis(0.0)
        , priority(AlarmPriority::LOW)
        , enabled(false)
    {}
    
    /**
     * @brief Constructor with all parameters.
     * 
     * @param type Vital sign type (e.g., "HR", "SPO2", "RR")
     * @param low Low threshold value
     * @param high High threshold value
     * @param hyst Hysteresis range (default: 0.0)
     * @param prio Alarm priority (default: LOW)
     * @param en Whether alarm is enabled (default: true)
     */
    AlarmThreshold(const std::string& type, double low, double high,
                   double hyst = 0.0, AlarmPriority prio = AlarmPriority::LOW,
                   bool en = true)
        : vitalType(type)
        , lowLimit(low)
        , highLimit(high)
        , hysteresis(hyst)
        , priority(prio)
        , enabled(en)
    {}
    
    /**
     * @brief Copy constructor.
     * 
     * @param other Source alarm threshold
     */
    AlarmThreshold(const AlarmThreshold& other) = default;
    
    /**
     * @brief Assignment operator (deleted - value objects are immutable).
     */
    AlarmThreshold& operator=(const AlarmThreshold&) = delete;
    
    /**
     * @brief Equality comparison.
     * 
     * Two alarm thresholds are equal if all their attributes match.
     * 
     * @param other Other alarm threshold to compare
     * @return true if all attributes are equal, false otherwise
     */
    bool operator==(const AlarmThreshold& other) const {
        return vitalType == other.vitalType &&
               lowLimit == other.lowLimit &&
               highLimit == other.highLimit &&
               hysteresis == other.hysteresis &&
               priority == other.priority &&
               enabled == other.enabled;
    }
    
    /**
     * @brief Inequality comparison.
     * 
     * @param other Other alarm threshold to compare
     * @return true if any attribute differs, false otherwise
     */
    bool operator!=(const AlarmThreshold& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief Check if a vital sign value violates this threshold.
     * 
     * @param value Vital sign value to check
     * @return true if value is below lowLimit or above highLimit, false otherwise
     */
    bool isViolated(double value) const {
        if (!enabled) {
            return false;
        }
        return value < lowLimit || value > highLimit;
    }
};

} // namespace zmon
} // namespace zmon