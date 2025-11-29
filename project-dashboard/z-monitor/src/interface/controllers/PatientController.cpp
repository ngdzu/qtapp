/**
 * @file PatientController.cpp
 * @brief Implementation of PatientController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "PatientController.h"
#include "application/services/AdmissionService.h"

namespace zmon
{

    PatientController::PatientController(AdmissionService *admissionService, QObject *parent)
        : QObject(parent), m_admissionService(admissionService), m_isAdmitted(false), m_patientName(""), m_patientMrn(""), m_bedLocation(""), m_admissionState("NOT_ADMITTED"), m_isAdmissionModalOpen(false)
    {
        // Connect to AdmissionService signals if provided
        if (m_admissionService)
        {
            // TODO: Connect signals once AdmissionService is fully implemented
            // connect(m_admissionService, &AdmissionService::patientAdmitted, this, &PatientController::onPatientAdmitted);
            // connect(m_admissionService, &AdmissionService::patientDischarged, this, &PatientController::onPatientDischarged);
        }
    }

    void PatientController::admitPatient(const QString &mrn, const QString &bedLocation)
    {
        // TODO: Call AdmissionService to admit patient
        // Requires permission check
        Q_UNUSED(mrn)
        Q_UNUSED(bedLocation)
    }

    void PatientController::dischargePatient()
    {
        // TODO: Call AdmissionService to discharge patient
        // Requires permission check
    }

    void PatientController::transferPatient(const QString &newBedLocation)
    {
        // TODO: Call AdmissionService to transfer patient
        // Requires permission check
        Q_UNUSED(newBedLocation)
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
        // TODO: Parse barcode and admit patient
        // Implements barcode scan admission from ADT workflow
        Q_UNUSED(barcodeData)
    }

    void PatientController::onPatientAdmitted()
    {
        // TODO: Update patient information from AdmissionService
        m_isAdmitted = true;
        m_admissionState = "ADMITTED";

        emit isAdmittedChanged();
        emit patientNameChanged();
        emit patientMrnChanged();
        emit bedLocationChanged();
        emit admittedAtChanged();
        emit admissionStateChanged();
    }

    void PatientController::onPatientDischarged()
    {
        // TODO: Clear patient information
        m_isAdmitted = false;
        m_admissionState = "NOT_ADMITTED";
        m_patientName = "";
        m_patientMrn = "";
        m_bedLocation = "";

        emit isAdmittedChanged();
        emit patientNameChanged();
        emit patientMrnChanged();
        emit bedLocationChanged();
        emit admissionStateChanged();
    }

} // namespace zmon
