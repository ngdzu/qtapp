/**
 * @file IPatientManager.h
 * @brief Interface for managing patient context and admission workflows.
 *
 * This interface defines the contract for the Patient Manager, which is responsible
 * for handling patient admission, discharge, and maintaining the current patient context.
 *
 * @author Z Monitor Team
 * @date 2025-03-06
 */

#pragma once

#include <QString>
#include <memory>
#include <optional>

#include "domain/common/Result.h"
#include "domain/admission/Patient.h"

namespace zmon
{

    /**
     * @class IPatientManager
     * @brief Interface for patient management operations.
     */
    class IPatientManager
    {
    public:
        virtual ~IPatientManager() = default;

        /**
         * @brief Admits a new patient.
         * @param patient The patient to admit.
         * @return Result<void> Success or error.
         */
        virtual Result<void> admitPatient(const Patient &patient) = 0;

        /**
         * @brief Discharges the current patient.
         * @return Result<void> Success or error.
         */
        virtual Result<void> dischargePatient() = 0;

        /**
         * @brief Gets the currently admitted patient.
         * @return std::optional<Patient> The current patient if any, or std::nullopt.
         */
        virtual std::optional<Patient> getCurrentPatient() const = 0;

        /**
         * @brief Checks if a patient is currently admitted.
         * @return true if a patient is admitted, false otherwise.
         */
        virtual bool isPatientAdmitted() const = 0;
    };

} // namespace zmon
