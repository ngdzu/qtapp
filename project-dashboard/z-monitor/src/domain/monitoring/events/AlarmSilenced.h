/**
 * @file AlarmSilenced.h
 * @brief Domain event representing an alarm being temporarily silenced.
 *
 * Raised when an alarm is silenced for a specified duration. Consumers may
 * adjust UI and audio alerts accordingly.
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
         * @struct AlarmSilenced
         * @brief Domain event raised when an alarm is silenced.
         */
        struct AlarmSilenced
        {
            /**
             * @brief Alarm snapshot at silence time.
             */
            AlarmSnapshot alarm;

            /**
             * @brief Silence duration in milliseconds.
             */
            int64_t durationMs;

            /**
             * @brief Default constructor.
             */
            AlarmSilenced()
                : alarm(), durationMs(0)
            {
            }

            /**
             * @brief Constructor with all parameters.
             *
             * @param alarmSnapshot Alarm snapshot
             * @param duration Silence duration (ms)
             */
            AlarmSilenced(const AlarmSnapshot &alarmSnapshot, int64_t duration)
                : alarm(alarmSnapshot), durationMs(duration)
            {
            }
        };

    } // namespace Events
} // namespace zmon
