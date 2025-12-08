/**
 * @file AlarmRaised.h
 * @brief Domain event representing an alarm being raised.
 *
 * This file contains the AlarmRaised domain event which is raised when
 * a new alarm is triggered. Domain events are plain structs that represent
 * something that happened in the domain.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/monitoring/AlarmSnapshot.h"
#include "domain/events/IDomainEvent.h"
#include <string>
#include <cstdint>
#include <memory>

namespace zmon
{
    namespace Events
    {

        /**
         * @struct AlarmRaised
         * @brief Domain event raised when a new alarm is triggered.
         *
         * This event is raised by AlarmAggregate when a new alarm condition is
         * detected. It is consumed by UI controllers (e.g., AlarmController) for
         * visual/audio alerts and by telemetry service for transmission.
         *
         * @note Domain events are plain structs (POD) with no business logic.
         */
        struct AlarmRaised : public zmon::IDomainEvent
        {
            /**
             * @brief Alarm snapshot.
             */
            AlarmSnapshot alarm;

            /**
             * @brief Timestamp when alarm was raised.
             *
             * Unix timestamp in milliseconds (epoch milliseconds).
             */
            int64_t timestampMs;

            /**
             * @brief Default constructor.
             */
            AlarmRaised()
                : alarm(), timestampMs(0)
            {
            }

            /**
             * @brief Constructor with all parameters.
             *
             * @param alarmSnapshot Alarm snapshot
             * @param ts Timestamp in milliseconds
             */
            AlarmRaised(const AlarmSnapshot &alarmSnapshot, int64_t ts)
                : alarm(alarmSnapshot), timestampMs(ts)
            {
            }

            // IDomainEvent interface implementation
            const std::string &aggregateId() const override { return alarm.alarmId; }
            int64_t occurredAtMs() const override { return timestampMs; }
            const char *eventType() const override { return "AlarmRaised"; }
            std::unique_ptr<zmon::IDomainEvent> clone() const override
            {
                return std::make_unique<AlarmRaised>(*this);
            }
        };

    } // namespace Events
} // namespace zmon