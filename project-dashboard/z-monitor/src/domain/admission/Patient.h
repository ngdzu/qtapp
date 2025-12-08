/**
 * @file Patient.h
 * @brief Domain entity representing a patient.
 *
 * This class aggregates patient identity, location, and other demographic information.
 *
 * @author Z Monitor Team
 * @date 2025-03-06
 */

#pragma once

#include "PatientIdentity.h"
#include "BedLocation.h"

namespace zmon
{

    /**
     * @class Patient
     * @brief Represents a patient in the system.
     */
    class Patient
    {
    public:
        Patient() = default;

        /**
         * @brief Constructor.
         * @param id Patient identity.
         * @param loc Bed location.
         */
        Patient(const PatientIdentity &id, const BedLocation &loc)
            : m_identity(id), m_location(loc) {}

        /**
         * @brief Gets the patient identity.
         * @return The patient identity.
         */
        const PatientIdentity &getIdentity() const { return m_identity; }

        /**
         * @brief Gets the bed location.
         * @return The bed location.
         */
        const BedLocation &getLocation() const { return m_location; }

    private:
        PatientIdentity m_identity;
        BedLocation m_location;
    };

} // namespace zmon
