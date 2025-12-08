/**
 * @file PatientTransferred.h
 * @brief Domain event representing patient transfer to another device.
 *
 * This file contains the PatientTransferred domain event which is raised when
 * a patient is transferred from this device to another device. Domain events
 * are plain structs that represent something that happened in the domain.
 *
 * @author Z Monitor Team
 * @date 2025-12-01
 */

#pragma once

#include <string>
#include <cstdint>
#include "domain/events/IDomainEvent.h"

namespace zmon
{
    namespace Events
    {

        /**
         * @struct PatientTransferred
         * @brief Domain event raised when a patient is transferred to another device.
         *
         * This event is raised by PatientAggregate when a patient is successfully
         * transferred to another monitoring device. It is consumed by application
         * services (e.g., AdmissionService) for logging, UI updates, and telemetry
         * service coordination.
         *
         * @note Domain events are plain structs (POD) with no business logic.
         */
        struct PatientTransferred : public zmon::IDomainEvent
        {
            /**
             * @brief Patient MRN.
             */
            std::string patientMrn;

            /**
             * @brief Target device identifier to transfer to.
             */
            std::string targetDevice;

            /**
             * @brief Source device identifier (this device).
             */
            std::string sourceDevice;

            /**
             * @brief Timestamp when transfer occurred.
             *
             * Unix timestamp in milliseconds (epoch milliseconds).
             */
            int64_t timestampMs;

            /**
             * @brief Default constructor.
             */
            PatientTransferred()
                : patientMrn(""), targetDevice(""), sourceDevice(""), timestampMs(0)
            {
            }

            /**
             * @brief Constructor with all parameters.
             *
             * @param mrn Patient MRN
             * @param target Target device identifier
             * @param source Source device identifier
             * @param ts Timestamp in milliseconds
             */
            PatientTransferred(const std::string &mrn, const std::string &target,
                               const std::string &source, int64_t ts)
                : patientMrn(mrn), targetDevice(target), sourceDevice(source), timestampMs(ts)
            {
            }

            // IDomainEvent interface
            const std::string &aggregateId() const override { return patientMrn; }
            int64_t occurredAtMs() const override { return timestampMs; }
            const char *eventType() const override { return "PatientTransferred"; }
            std::unique_ptr<zmon::IDomainEvent> clone() const override { return std::make_unique<PatientTransferred>(*this); }
        };

    } // namespace Events
} // namespace zmon
