/**
 * @file PatientManager.h
 * @brief Implementation of the Patient Manager.
 *
 * Manages the current patient context and delegates admission/discharge operations
 * to the AdmissionService.
 *
 * @author Z Monitor Team
 * @date 2025-03-06
 */

#pragma once

#include <QObject>
#include <memory>
#include <optional>
#include <mutex>

#include "application/interfaces/IPatientManager.h"
#include "application/services/AdmissionService.h"
#include "domain/admission/Patient.h"

namespace zmon
{

    /**
     * @class PatientManager
     * @brief Manages patient state and workflow.
     */
    class PatientManager : public QObject, public IPatientManager
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructor.
         * @param admissionService Pointer to the AdmissionService.
         * @param parent Parent QObject.
         */
        explicit PatientManager(std::shared_ptr<AdmissionService> admissionService, QObject *parent = nullptr);

        ~PatientManager() override = default;

        /**
         * @brief Admits a new patient.
         * @param patient The patient to admit.
         * @return Result<void> Success or error.
         */
        Result<void> admitPatient(const Patient &patient) override;

        /**
         * @brief Discharges the current patient.
         * @return Result<void> Success or error.
         */
        Result<void> dischargePatient() override;

        /**
         * @brief Gets the currently admitted patient.
         * @return std::optional<Patient> The current patient if any, or std::nullopt.
         */
        std::optional<Patient> getCurrentPatient() const override;

        /**
         * @brief Checks if a patient is currently admitted.
         * @return true if a patient is admitted, false otherwise.
         */
        bool isPatientAdmitted() const override;

    signals:
        /**
         * @brief Signal emitted when a patient is admitted.
         * @param patient The admitted patient.
         */
        void patientAdmitted(const Patient &patient);

        /**
         * @brief Signal emitted when a patient is discharged.
         */
        void patientDischarged();

    private:
        std::shared_ptr<AdmissionService> m_admissionService;
        std::shared_ptr<Patient> m_currentPatient;
        mutable std::mutex m_mutex;
    };

} // namespace zmon
