/**
 * @file PatientController.cpp
 * @brief Implementation of PatientController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "PatientController.h"
#include "application/services/AdmissionService.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"

namespace zmon
{

    PatientController::PatientController(AdmissionService *admissionService, QObject *parent)
        : QObject(parent), m_admissionService(admissionService), m_isAdmitted(false), m_patientName(""), m_patientMrn(""), m_bedLocation(""), m_admissionState("NOT_ADMITTED"), m_isAdmissionModalOpen(false)
    {
        // Connect to AdmissionService signals if provided
        if (m_admissionService)
        {
            connect(m_admissionService, &AdmissionService::patientAdmitted, this, &PatientController::onPatientAdmitted);
            connect(m_admissionService, &AdmissionService::patientDischarged, this, &PatientController::onPatientDischarged);

            // Initialize from current admission state
            const auto info = m_admissionService->getCurrentAdmission();
            const bool admitted = m_admissionService->isPatientAdmitted();
            m_isAdmitted = admitted;
            m_patientMrn = info.mrn;
            m_patientName = info.name;
            m_bedLocation = info.bedLocation;
            m_admittedAt = info.admittedAt;
            m_admissionState = admitted ? QStringLiteral("ADMITTED") : QStringLiteral("NOT_ADMITTED");
        }
    }

    void PatientController::admitPatient(const QString &mrn, const QString &bedLocation)
    {
        if (!m_admissionService)
            return;

        // TODO: Permission check via SecurityService

        PatientIdentity identity{
            mrn.toStdString(),
            m_patientName.toStdString(),
            0 /* dobMs unknown */,
            std::string("U") /* sex unknown */,
            std::vector<std::string>{}};
        BedLocation bed{bedLocation.toStdString()};
        auto result = m_admissionService->admitPatient(identity, bed, AdmissionService::AdmissionSource::Manual);
        if (result.isError())
        {
            // TODO: Emit error signal or log
            return;
        }
        // onPatientAdmitted will update properties via signal
    }

    void PatientController::admitFromUi(const QString &mrn, const QString &unit, const QString &loc)
    {
        if (!m_admissionService)
            return;
        const std::string smrn = mrn.toStdString();
        PatientIdentity identity(smrn, m_patientName.toStdString(), 0, std::string("Unknown"));
        BedLocation bed(loc.toStdString(), unit.toStdString());
        auto result = m_admissionService->admitPatient(identity, bed, AdmissionService::AdmissionSource::Manual);
        if (result.isError())
        {
            // TODO: emit error signal for UI
            return;
        }
        // Properties updated via onPatientAdmitted()
    }

    void PatientController::dischargePatient()
    {
        if (!m_admissionService)
            return;
        // TODO: Permission check via SecurityService
        auto result = m_admissionService->dischargePatient(m_patientMrn);
        if (result.isError())
        {
            // TODO: Emit error signal or log
            return;
        }
        // onPatientDischarged will update properties via signal
    }

    void PatientController::transferPatient(const QString &newBedLocation)
    {
        if (!m_admissionService)
            return;
        // TODO: Permission check via SecurityService
        // For local device transfer, update bed location via admit/discharge semantics depending on architecture.
        // Here, call transfer on AdmissionService using device label semantics.
        auto result = m_admissionService->transferPatient(m_patientMrn, newBedLocation);
        if (result.isError())
        {
            // TODO: Emit error signal or log
            return;
        }
        // Update local bed location
        m_bedLocation = newBedLocation;
        emit bedLocationChanged();
    }

    void PatientController::openAdmissionModal()
    {
        m_isAdmissionModalOpen = true;
        emit isAdmissionModalOpenChanged();
    }

    void PatientController::closeAdmissionModal()
    {
        m_isAdmissionModalOpen = false;
        emit isAdmissionModalOpenChanged();
    }

    void PatientController::scanBarcode(const QString &barcodeData)
    {
        if (!m_admissionService)
            return;
        // Simple barcode = MRN
        const QString mrn = barcodeData.trimmed();
        if (mrn.isEmpty())
            return;
        // Use existing bedLocation property if set
        PatientIdentity identity{
            mrn.toStdString(),
            m_patientName.toStdString(),
            0 /* dobMs unknown */,
            std::string("U") /* sex unknown */,
            std::vector<std::string>{}};
        BedLocation bed{m_bedLocation.toStdString()};
        auto result = m_admissionService->admitPatient(identity, bed, AdmissionService::AdmissionSource::Barcode);
        if (result.isError())
        {
            // TODO: Emit error signal or log
        }
    }

    void PatientController::onPatientAdmitted()
    {
        if (!m_admissionService)
            return;
        const auto info = m_admissionService->getCurrentAdmission();

        // Only emit signals if values actually changed
        bool changed = false;

        if (!m_isAdmitted)
        {
            m_isAdmitted = true;
            emit isAdmittedChanged();
            changed = true;
        }

        if (m_patientMrn != info.mrn)
        {
            m_patientMrn = info.mrn;
            emit patientMrnChanged();
            changed = true;
        }

        if (m_patientName != info.name)
        {
            m_patientName = info.name;
            emit patientNameChanged();
            changed = true;
        }

        if (m_bedLocation != info.bedLocation)
        {
            m_bedLocation = info.bedLocation;
            emit bedLocationChanged();
            changed = true;
        }

        if (m_admittedAt != info.admittedAt)
        {
            m_admittedAt = info.admittedAt;
            emit admittedAtChanged();
            changed = true;
        }

        QString newState = QStringLiteral("ADMITTED");
        if (m_admissionState != newState)
        {
            m_admissionState = newState;
            emit admissionStateChanged();
            changed = true;
        }
    }

    void PatientController::onPatientDischarged()
    {
        // Only emit signals if values actually changed
        if (m_isAdmitted)
        {
            m_isAdmitted = false;
            emit isAdmittedChanged();
        }

        QString newState = QStringLiteral("NOT_ADMITTED");
        if (m_admissionState != newState)
        {
            m_admissionState = newState;
            emit admissionStateChanged();
        }

        if (!m_patientName.isEmpty())
        {
            m_patientName.clear();
            emit patientNameChanged();
        }

        if (!m_patientMrn.isEmpty())
        {
            m_patientMrn.clear();
            emit patientMrnChanged();
        }

        if (!m_bedLocation.isEmpty())
        {
            m_bedLocation.clear();
            emit bedLocationChanged();
        }

        if (m_admittedAt.isValid())
        {
            m_admittedAt = QDateTime();
            emit admittedAtChanged();
        }
    }

} // namespace zmon
