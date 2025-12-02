/**
 * @file PatientDischarged.h
 * @brief Domain event representing patient discharge.
 *
 * This file contains the PatientDischarged domain event which is raised when
 * a patient is discharged from the device. Domain events are plain structs that
 * represent something that happened in the domain.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
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
         * @struct PatientDischarged
         * @brief Domain event raised when a patient is discharged from the device.
         *
         * This event is raised by PatientAggregate when a patient is successfully
         * discharged. It is consumed by application services (e.g., AdmissionService)
         * for logging, UI updates, and telemetry service coordination.
         *
         * @note Domain events are plain structs (POD) with no business logic.
         */
        struct PatientDischarged : public zmon::DomainEvents::IDomainEvent
        {
            /**
             * @brief Patient MRN.
             */
            std::string patientMrn;

            /**
             * @brief Timestamp when discharge occurred.
             *
             * Unix timestamp in milliseconds (epoch milliseconds).
             */
            int64_t timestampMs;

            /**
             * @brief Device identifier.
             */
            std::string deviceId;

            /**
             * @brief Default constructor.
             */
            PatientDischarged()
                : patientMrn(""), timestampMs(0), deviceId("")
            {
            }

            /**
             * @brief Constructor with all parameters.
             *
             * @param mrn Patient MRN
             * @param ts Timestamp in milliseconds
             * @param devId Device identifier
             */
            PatientDischarged(const std::string &mrn, int64_t ts, const std::string &devId)
                : patientMrn(mrn), timestampMs(ts), deviceId(devId)
            {
            }

            // IDomainEvent interface
            const std::string &aggregateId() const override { return patientMrn; }
            int64_t occurredAtMs() const override { return timestampMs; }
            const char *eventType() const override { return "PatientDischarged"; }
            std::unique_ptr<zmon::DomainEvents::IDomainEvent> clone() const override { return std::make_unique<PatientDischarged>(*this); }
        };

    } // namespace zmon
} // namespace zmon