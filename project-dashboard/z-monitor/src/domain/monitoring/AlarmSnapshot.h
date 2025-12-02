/**
 * @file AlarmSnapshot.h
 * @brief Value object representing alarm state at a point in time.
 *
 * This file contains the AlarmSnapshot value object which represents alarm
 * state at a specific point in time. Value objects are immutable and defined
 * by their attributes.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/monitoring/AlarmThreshold.h"
#include <chrono>
#include <string>
#include <cstdint>

namespace zmon
{
    /**
     * @enum AlarmStatus
     * @brief Alarm status/state.
     */
    enum class AlarmStatus
    {
        Active,       ///< Alarm is currently active
        Acknowledged, ///< Alarm has been acknowledged
        Silenced,     ///< Alarm is temporarily silenced
        Resolved      ///< Alarm condition has resolved
    };

    /**
     * @class AlarmSnapshot
     * @brief Immutable value object representing alarm state at a point in time.
     *
     * This value object encapsulates alarm state including type, priority, status,
     * value that triggered it, and threshold that was exceeded. It is immutable
     * and can be safely shared across threads.
     *
     * @note This is a value object - it has no identity and is defined by its attributes.
     * @note All members are const to enforce immutability.
     */
    struct AlarmSnapshot
    {
        /**
         * @brief Alarm identifier (UUID).
         */
        const std::string alarmId;

        /**
         * @brief Alarm type.
         *
         * Examples: "HR_HIGH", "SPO2_LOW", "RR_HIGH".
         */
        const std::string alarmType;

        /**
         * @brief Alarm priority level.
         */
        const AlarmPriority priority;

        /**
         * @brief Alarm status/state.
         */
        const AlarmStatus status;

        /**
         * @brief Vital sign value that triggered the alarm.
         */
        const double value;

        /**
         * @brief Threshold value that was exceeded.
         *
         * Historical snapshot of threshold at time of alarm (for audit/compliance).
         */
        const double thresholdValue;

        /**
         * @brief Timestamp when alarm was triggered.
         *
         * Unix timestamp in milliseconds (epoch milliseconds).
         */
        const int64_t timestampMs;

        /**
         * @brief Patient MRN associated with this alarm.
         */
        const std::string patientMrn;

        /**
         * @brief Device identifier.
         */
        const std::string deviceId;

        /**
         * @brief User ID who acknowledged the alarm (if acknowledged).
         */
        const std::string acknowledgedBy;

        /**
         * @brief Timestamp when alarm was acknowledged (0 if not acknowledged).
         */
        const int64_t acknowledgedAtMs;

        /**
         * @brief Default constructor.
         *
         * Creates an empty alarm snapshot with default values.
         */
        AlarmSnapshot()
            : alarmId(""), alarmType(""), priority(AlarmPriority::LOW), status(AlarmStatus::Active), value(0.0), thresholdValue(0.0), timestampMs(0), patientMrn(""), deviceId(""), acknowledgedBy(""), acknowledgedAtMs(0)
        {
        }

        /**
         * @brief Constructor with all parameters.
         *
         * @param id Alarm identifier (UUID)
         * @param type Alarm type (e.g., "HR_HIGH", "SPO2_LOW")
         * @param prio Alarm priority
         * @param stat Alarm status
         * @param val Vital sign value that triggered alarm
         * @param threshold Threshold value that was exceeded
         * @param ts Timestamp in milliseconds (epoch milliseconds)
         * @param mrn Patient MRN
         * @param devId Device identifier
         * @param ackBy User ID who acknowledged (default: empty)
         * @param ackAt Timestamp when acknowledged (default: 0)
         */
        AlarmSnapshot(const std::string &id, const std::string &type,
                      AlarmPriority prio, AlarmStatus stat, double val,
                      double threshold, int64_t ts, const std::string &mrn,
                      const std::string &devId, const std::string &ackBy = "",
                      int64_t ackAt = 0)
            : alarmId(id), alarmType(type), priority(prio), status(stat), value(val), thresholdValue(threshold), timestampMs(ts), patientMrn(mrn), deviceId(devId), acknowledgedBy(ackBy), acknowledgedAtMs(ackAt)
        {
        }

        /**
         * @brief Copy constructor.
         *
         * @param other Source alarm snapshot
         */
        AlarmSnapshot(const AlarmSnapshot &other) = default;

        /**
         * @brief Assignment operator (deleted - value objects are immutable).
         */
        AlarmSnapshot &operator=(const AlarmSnapshot &) = delete;

        /**
         * @brief Equality comparison.
         *
         * Two alarm snapshots are equal if all their attributes match.
         *
         * @param other Other alarm snapshot to compare
         * @return true if all attributes are equal, false otherwise
         */
        bool operator==(const AlarmSnapshot &other) const
        {
            return alarmId == other.alarmId &&
                   alarmType == other.alarmType &&
                   priority == other.priority &&
                   status == other.status &&
                   value == other.value &&
                   thresholdValue == other.thresholdValue &&
                   timestampMs == other.timestampMs &&
                   patientMrn == other.patientMrn &&
                   deviceId == other.deviceId &&
                   acknowledgedBy == other.acknowledgedBy &&
                   acknowledgedAtMs == other.acknowledgedAtMs;
        }

        /**
         * @brief Inequality comparison.
         *
         * @param other Other alarm snapshot to compare
         * @return true if any attribute differs, false otherwise
         */
        bool operator!=(const AlarmSnapshot &other) const
        {
            return !(*this == other);
        }
    };

} // namespace zmon