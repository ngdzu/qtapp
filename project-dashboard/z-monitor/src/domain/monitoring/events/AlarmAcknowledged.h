/**
 * @file AlarmAcknowledged.h
 * @brief Domain event representing an alarm being acknowledged by a user.
 *
 * This event is raised when an active alarm is acknowledged. It includes
 * the alarm snapshot and acknowledgment metadata.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/monitoring/AlarmSnapshot.h"
#include <string>
#include <cstdint>

namespace zmon
{
    namespace Events
    {

        /**
         * @struct AlarmAcknowledged
         * @brief Domain event raised when an alarm is acknowledged.
         */
        struct AlarmAcknowledged
        {
            /**
             * @brief Alarm snapshot at acknowledgment time.
             */
            AlarmSnapshot alarm;

            /**
             * @brief User ID who acknowledged the alarm.
             */
            std::string acknowledgedBy;

            /**
             * @brief Timestamp when alarm was acknowledged (epoch ms).
             */
            int64_t acknowledgedAtMs;

            /**
             * @brief Default constructor.
             */
            AlarmAcknowledged()
                : alarm(), acknowledgedBy(""), acknowledgedAtMs(0)
            {
            }

            /**
             * @brief Constructor with all parameters.
             *
             * @param alarmSnapshot Alarm snapshot
             * @param userId User who acknowledged
             * @param ts Acknowledgment timestamp
             */
            AlarmAcknowledged(const AlarmSnapshot &alarmSnapshot,
                              const std::string &userId,
                              int64_t ts)
                : alarm(alarmSnapshot), acknowledgedBy(userId), acknowledgedAtMs(ts)
            {
            }
        };

    } // namespace Events
} // namespace zmon
