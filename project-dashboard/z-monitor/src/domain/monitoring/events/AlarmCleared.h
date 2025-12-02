/**
 * @file AlarmCleared.h
 * @brief Domain event representing an alarm being resolved/cleared.
 *
 * Raised when an alarm condition is no longer present and the alarm is
 * resolved. Consumers may update history and UI state.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/monitoring/AlarmSnapshot.h"
#include <cstdint>

namespace zmon
{
    namespace Events
    {

        /**
         * @struct AlarmCleared
         * @brief Domain event raised when an alarm is resolved.
         */
        struct AlarmCleared
        {
            /**
             * @brief Alarm snapshot at resolution time.
             */
            AlarmSnapshot alarm;

            /**
             * @brief Timestamp when alarm was resolved (epoch ms).
             */
            int64_t resolvedAtMs;

            /**
             * @brief Default constructor.
             */
            AlarmCleared()
                : alarm(), resolvedAtMs(0)
            {
            }

            /**
             * @brief Constructor with all parameters.
             *
             * @param alarmSnapshot Alarm snapshot
             * @param ts Resolution timestamp
             */
            AlarmCleared(const AlarmSnapshot &alarmSnapshot, int64_t ts)
                : alarm(alarmSnapshot), resolvedAtMs(ts)
            {
            }
        };

    } // namespace Events
} // namespace zmon
