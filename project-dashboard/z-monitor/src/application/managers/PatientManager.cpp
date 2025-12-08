/**
 * @file PatientManager.cpp
 * @brief Implementation of the Patient Manager.
 *
 * @author Z Monitor Team
 * @date 2025-03-06
 */

#include "application/managers/PatientManager.h"

namespace zmon
{

    PatientManager::PatientManager(std::shared_ptr<AdmissionService> admissionService, QObject *parent)
        : QObject(parent), m_admissionService(std::move(admissionService))
    {

        // Sync with AdmissionService state
        if (m_admissionService->isPatientAdmitted())
        {
            auto info = m_admissionService->getCurrentAdmission();

            // Reconstruct Patient object from AdmissionInfo
            // Note: AdmissionInfo might not have all details (DOB, Sex, etc.)
            // For now, we populate what we have.
            PatientIdentity identity{
                info.mrn.toStdString(),
                info.name.toStdString(),
                0,  // DOB unknown
                "", // Sex unknown
                {}  // Allergies unknown
            };

            BedLocation location{
                info.bedLocation.toStdString(),
                "" // Facility unknown
            };

            m_currentPatient = std::make_shared<Patient>(identity, location);
        }
    }

    Result<void> PatientManager::admitPatient(const Patient &patient)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Call AdmissionService
        auto result = m_admissionService->admitPatient(
            patient.getIdentity(),
            patient.getLocation(),
            AdmissionService::AdmissionSource::Manual);

        if (result.isOk())
        {
            m_currentPatient = std::make_shared<Patient>(patient);
            emit patientAdmitted(patient);
        }

        return result;
    }

    Result<void> PatientManager::dischargePatient()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_currentPatient)
        {
            return Result<void>::error(Error::create(ErrorCode::NotFound, "No patient admitted"));
        }

        // Use the MRN from the current patient
        QString mrn = QString::fromStdString(m_currentPatient->getIdentity().mrn);
        auto result = m_admissionService->dischargePatient(mrn);

        if (result.isOk())
        {
            m_currentPatient = nullptr;
            emit patientDischarged();
        }

        return result;
    }

    std::optional<Patient> PatientManager::getCurrentPatient() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_currentPatient)
        {
            return *m_currentPatient;
        }
        return std::nullopt;
    }

    bool PatientManager::isPatientAdmitted() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_currentPatient != nullptr;
    }

} // namespace zmon
