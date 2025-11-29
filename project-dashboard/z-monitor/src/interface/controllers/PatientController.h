/**
 * @file PatientController.h
 * @brief QML controller for patient management UI (ADT workflow).
 *
 * This file contains the PatientController class which provides QML bindings
 * for patient admission, discharge, transfer, and patient information display.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>

namespace zmon
{

    // Forward declarations
    class AdmissionService;

    /**
     * @class PatientController
     * @brief QML controller for patient management UI (ADT workflow).
     *
     * Provides QML bindings for patient management following ADT (Admission, Discharge, Transfer)
     * workflow. Exposes patient information, admission state, and ADT methods for the Patient View.
     *
     * @note Business logic delegated to AdmissionService
     * @note Implements ADT workflow per doc/19_ADT_WORKFLOW.md
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     */
    class PatientController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool isAdmitted READ isAdmitted NOTIFY isAdmittedChanged)
        Q_PROPERTY(QString patientName READ patientName NOTIFY patientNameChanged)
        Q_PROPERTY(QString patientMrn READ patientMrn NOTIFY patientMrnChanged)
        Q_PROPERTY(QString bedLocation READ bedLocation NOTIFY bedLocationChanged)
        Q_PROPERTY(QDateTime admittedAt READ admittedAt NOTIFY admittedAtChanged)
        Q_PROPERTY(QString admissionState READ admissionState NOTIFY admissionStateChanged)
        Q_PROPERTY(bool isAdmissionModalOpen READ isAdmissionModalOpen NOTIFY isAdmissionModalOpenChanged)

    public:
        /**
         * @brief Constructor.
         *
         * @param admissionService Admission service for ADT operations
         * @param parent Parent QObject
         */
        explicit PatientController(AdmissionService *admissionService = nullptr, QObject *parent = nullptr);

        /**
         * @brief Destructor.
         */
        ~PatientController() override = default;

        /**
         * @brief Gets whether patient is currently admitted.
         *
         * @return true if patient is admitted, false otherwise
         */
        bool isAdmitted() const { return m_isAdmitted; }

        /**
         * @brief Gets current patient name.
         *
         * @return Patient name, or empty string if no patient admitted
         */
        QString patientName() const { return m_patientName; }

        /**
         * @brief Gets current patient MRN.
         *
         * @return Patient MRN, or empty string if no patient admitted
         */
        QString patientMrn() const { return m_patientMrn; }

        /**
         * @brief Gets current bed location.
         *
         * @return Bed location (e.g., "ICU-4B"), or empty string if no patient admitted
         */
        QString bedLocation() const { return m_bedLocation; }

        /**
         * @brief Gets admission timestamp.
         *
         * @return Admission timestamp, or invalid QDateTime if no patient admitted
         */
        QDateTime admittedAt() const { return m_admittedAt; }

        /**
         * @brief Gets admission state.
         *
         * @return Admission state ("NOT_ADMITTED", "ADMITTED", "DISCHARGED")
         */
        QString admissionState() const { return m_admissionState; }

        /**
         * @brief Gets whether admission modal is open.
         *
         * @return true if admission modal is open, false otherwise
         */
        bool isAdmissionModalOpen() const { return m_isAdmissionModalOpen; }

    public slots:
        /**
         * @brief Handle patient admitted from AdmissionService.
         *
         * @note Called when AdmissionService emits patientAdmitted signal
         */
        void onPatientAdmitted();

        /**
         * @brief Handle patient discharged from AdmissionService.
         *
         * @note Called when AdmissionService emits patientDischarged signal
         */
        void onPatientDischarged();

    public:
        /**
         * @brief Admit patient with MRN.
         *
         * @param mrn Patient Medical Record Number
         * @param bedLocation Bed location (e.g., "ICU-4B")
         *
         * @note This method is callable from QML
         * @note Requires user login (permission check)
         * @note Implements manual admission from ADT workflow
         */
        Q_INVOKABLE void admitPatient(const QString &mrn, const QString &bedLocation);

        /**
         * @brief Discharge current patient.
         *
         * @note This method is callable from QML
         * @note Requires user login (permission check)
         * @note Implements manual discharge from ADT workflow
         */
        Q_INVOKABLE void dischargePatient();

        /**
         * @brief Transfer patient to different bed.
         *
         * @param newBedLocation New bed location
         *
         * @note This method is callable from QML
         * @note Requires user login (permission check)
         */
        Q_INVOKABLE void transferPatient(const QString &newBedLocation);

        /**
         * @brief Open admission modal.
         *
         * @note This method is callable from QML
         */
        Q_INVOKABLE void openAdmissionModal();

        /**
         * @brief Close admission modal.
         *
         * @note This method is callable from QML
         */
        Q_INVOKABLE void closeAdmissionModal();

        /**
         * @brief Scan barcode for patient admission.
         *
         * @param barcodeData Barcode data (MRN encoded)
         *
         * @note This method is callable from QML
         * @note Implements barcode scan admission from ADT workflow
         */
        Q_INVOKABLE void scanBarcode(const QString &barcodeData);

    signals:
        /**
         * @brief Emitted when admission state changes.
         */
        void isAdmittedChanged();

        /**
         * @brief Emitted when patient name changes.
         */
        void patientNameChanged();

        /**
         * @brief Emitted when patient MRN changes.
         */
        void patientMrnChanged();

        /**
         * @brief Emitted when bed location changes.
         */
        void bedLocationChanged();

        /**
         * @brief Emitted when admission timestamp changes.
         */
        void admittedAtChanged();

        /**
         * @brief Emitted when admission state changes.
         */
        void admissionStateChanged();

        /**
         * @brief Emitted when admission modal state changes.
         */
        void isAdmissionModalOpenChanged();

    private:
        AdmissionService *m_admissionService; ///< Admission service (not owned)

        bool m_isAdmitted;           ///< Whether patient is admitted
        QString m_patientName;       ///< Current patient name
        QString m_patientMrn;        ///< Current patient MRN
        QString m_bedLocation;       ///< Current bed location
        QDateTime m_admittedAt;      ///< Admission timestamp
        QString m_admissionState;    ///< Admission state
        bool m_isAdmissionModalOpen; ///< Whether admission modal is open
    };

} // namespace zmon
