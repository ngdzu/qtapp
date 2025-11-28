/**
 * @file AdmissionService.h
 * @brief Service for patient admission, discharge, and transfer (ADT) workflow.
 *
 * This file contains the AdmissionService class which orchestrates the ADT
 * workflow including patient admission, discharge, and transfer operations.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <memory>

#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"

namespace zmon {

// Forward declarations

/**
 * @class AdmissionService
 * @brief Service for patient admission, discharge, and transfer (ADT) workflow.
 *
 * Orchestrates the ADT workflow including:
 * - Patient admission (manual, barcode, central station)
 * - Patient discharge
 * - Patient transfer
 * - Admission history tracking
 *
 * @note Thread-safe: Can be called from any thread.
 * @note Emits signals for UI updates and audit logging.
 *
 * @ingroup Application
 */
class AdmissionService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Admission source types.
     */
    enum class AdmissionSource {
        Manual,          ///< Manual entry by clinician
        Barcode,         ///< Barcode scan
        CentralStation   ///< Central station push
    };
    Q_ENUM(AdmissionSource)

    /**
     * @brief Admission state.
     */
    enum class AdmissionState {
        NotAdmitted,     ///< No patient admitted
        Admitted,        ///< Patient currently admitted
        Discharged       ///< Patient discharged
    };
    Q_ENUM(AdmissionState)

    /**
     * @brief Patient admission information.
     */
    struct AdmissionInfo {
        QString mrn;                    ///< Medical Record Number
        QString name;                   ///< Patient name
        QString bedLocation;            ///< Bed/room location
        QDateTime admittedAt;           ///< Admission timestamp
        QDateTime dischargedAt;         ///< Discharge timestamp (NULL if admitted)
        AdmissionSource admissionSource; ///< Source of admission
        QString deviceLabel;             ///< Device that admitted patient
    };

    /**
     * @brief Constructor.
     *
     * @param parent Parent QObject
     */
    explicit AdmissionService(QObject* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~AdmissionService() override = default;

    /**
     * @brief Admits a patient to the device.
     *
     * @param patientIdentity Patient identity information
     * @param bedLocation Bed/room location
     * @param admissionSource Source of admission
     * @return true if admission succeeded, false otherwise
     */
    bool admitPatient(const PatientIdentity& patientIdentity,
                      const BedLocation& bedLocation,
                      AdmissionSource admissionSource);

    /**
     * @brief Discharges a patient from the device.
     *
     * @param mrn Medical Record Number of patient to discharge
     * @return true if discharge succeeded, false otherwise
     */
    bool dischargePatient(const QString& mrn);

    /**
     * @brief Transfers a patient to another device.
     *
     * @param mrn Medical Record Number of patient to transfer
     * @param targetDeviceLabel Target device label
     * @return true if transfer succeeded, false otherwise
     */
    bool transferPatient(const QString& mrn, const QString& targetDeviceLabel);

    /**
     * @brief Gets the current admission information.
     *
     * @return Current admission info or empty if no patient admitted
     */
    AdmissionInfo getCurrentAdmission() const;

    /**
     * @brief Checks if a patient is currently admitted.
     *
     * @return true if patient is admitted, false otherwise
     */
    bool isPatientAdmitted() const;

    /**
     * @brief Gets the admission state.
     *
     * @return Current admission state
     */
    AdmissionState admissionState() const;

signals:
    /**
     * @brief Emitted when a patient is admitted.
     *
     * @param mrn Medical Record Number
     * @param name Patient name
     * @param bedLocation Bed location
     */
    void patientAdmitted(const QString& mrn, const QString& name, const QString& bedLocation);

    /**
     * @brief Emitted when a patient is discharged.
     *
     * @param mrn Medical Record Number
     */
    void patientDischarged(const QString& mrn);

    /**
     * @brief Emitted when a patient is transferred.
     *
     * @param mrn Medical Record Number
     * @param targetDeviceLabel Target device label
     */
    void patientTransferred(const QString& mrn, const QString& targetDeviceLabel);

    /**
     * @brief Emitted when admission state changes.
     *
     * @param state New admission state
     */
    void admissionStateChanged(AdmissionState state);

private:
    /**
     * @brief Logs an admission event to the database.
     *
     * @param eventType Event type ("admission", "discharge", "transfer")
     * @param mrn Medical Record Number
     * @param name Patient name
     * @param bedLocation Bed location
     * @param admissionSource Admission source
     * @param details Additional event details (JSON)
     */
    void logAdmissionEvent(const QString& eventType,
                           const QString& mrn,
                           const QString& name,
                           const QString& bedLocation,
                           AdmissionSource admissionSource,
                           const QString& details = QString());

    /**
     * @brief Gets the device label from settings.
     *
     * @return Device label string
     */
    QString getDeviceLabel() const;

    AdmissionState m_admissionState;
    AdmissionInfo m_currentAdmission;
};

} // namespace zmon