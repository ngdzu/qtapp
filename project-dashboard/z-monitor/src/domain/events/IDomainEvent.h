/**
 * @file IDomainEvent.h
 * @brief Interface for all domain events in the system.
 *
 * Domain events represent something that has happened in the domain that
 * the system cares about. They are immutable, timestamped and carry the
 * identifier of the aggregate that produced them. Events are dispatched
 * via the DomainEventDispatcher to decouple producers (aggregates) from
 * consumers (application services, logging, telemetry, etc.).
 */

#pragma once

#include <string>
#include <cstdint>

namespace zmon
{
    namespace DomainEvents
    {

        /**
         * @class IDomainEvent
         * @brief Base polymorphic interface for domain events.
         */
        class IDomainEvent
        {
        public:
            virtual ~IDomainEvent() = default;

            /**
             * @brief Returns the aggregate identifier associated with this event.
             */
            virtual const std::string &aggregateId() const = 0;

            /**
             * @brief Returns the epoch milliseconds timestamp when the event occurred.
             */
            virtual int64_t occurredAtMs() const = 0;

            /**
             * @brief Returns the event type name (stable identifier for handlers / logging).
             */
            virtual const char *eventType() const = 0;

            /**
             * @brief Polymorphic clone for safe asynchronous dispatch.
             */
            virtual std::unique_ptr<IDomainEvent> clone() const = 0;
        };

    }
} // namespace zmon::DomainEvents
