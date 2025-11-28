/**
 * @file AdmissionService.cpp
 * @brief Implementation of AdmissionService for ADT workflow.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "AdmissionService.h"

#include "infrastructure/adapters/SettingsManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

namespace zmon {
AdmissionService::AdmissionService(QObject* parent)
    : QObject(parent)
    , m_admissionState(AdmissionState::NotAdmitted)
{
}

bool AdmissionService::admitPatient(const PatientIdentity& patientIdentity,
                                    const BedLocation& bedLocation,
                                    AdmissionSource admissionSource)
{
    if (!patientIdentity.isValid()) {
        qWarning() << "AdmissionService::admitPatient: Invalid patient identity";
        return false;
    }

    // Check if patient is already admitted
    if (isPatientAdmitted()) {
        qWarning() << "AdmissionService::admitPatient: Patient already admitted";
        return false;
    }

    QString mrn = QString::fromStdString(patientIdentity.mrn);
    QString name = QString::fromStdString(patientIdentity.name);
    QString bedLocationStr = QString::fromStdString(bedLocation.toString());
    QString deviceLabel = getDeviceLabel();

    // TODO: Save patient to database (patients table)
    // TODO: Update patient record with admission information

    // Update current admission
    m_currentAdmission.mrn = mrn;
    m_currentAdmission.name = name;
    m_currentAdmission.bedLocation = bedLocationStr;
    m_currentAdmission.admittedAt = QDateTime::currentDateTimeUtc();
    m_currentAdmission.dischargedAt = QDateTime();  // NULL
    m_currentAdmission.admissionSource = admissionSource;
    m_currentAdmission.deviceLabel = deviceLabel;

    m_admissionState = AdmissionState::Admitted;

    // Log admission event
    QString admissionSourceStr;
    switch (admissionSource) {
        case AdmissionSource::Manual:
            admissionSourceStr = "manual";
            break;
        case AdmissionSource::Barcode:
            admissionSourceStr = "barcode";
            break;
        case AdmissionSource::CentralStation:
            admissionSourceStr = "central_station";
            break;
    }

    logAdmissionEvent("admission", mrn, name, bedLocationStr, admissionSource);

    // Emit signals
    emit patientAdmitted(mrn, name, bedLocationStr);
    emit admissionStateChanged(m_admissionState);

    return true;
}

bool AdmissionService::dischargePatient(const QString& mrn)
{
    if (!isPatientAdmitted()) {
        qWarning() << "AdmissionService::dischargePatient: No patient admitted";
        return false;
    }

    if (m_currentAdmission.mrn != mrn) {
        qWarning() << "AdmissionService::dischargePatient: MRN mismatch";
        return false;
    }

    // TODO: Update patient record in database (set discharged_at)

    // Update current admission
    m_currentAdmission.dischargedAt = QDateTime::currentDateTimeUtc();
    m_admissionState = AdmissionState::Discharged;

    // Log discharge event
    logAdmissionEvent("discharge", mrn, m_currentAdmission.name, m_currentAdmission.bedLocation,
                      m_currentAdmission.admissionSource);

    // Emit signals
    emit patientDischarged(mrn);
    emit admissionStateChanged(m_admissionState);

    // Clear current admission
    m_currentAdmission = AdmissionInfo();
    m_admissionState = AdmissionState::NotAdmitted;
    emit admissionStateChanged(m_admissionState);

    return true;
}

bool AdmissionService::transferPatient(const QString& mrn, const QString& targetDeviceLabel)
{
    if (!isPatientAdmitted()) {
        qWarning() << "AdmissionService::transferPatient: No patient admitted";
        return false;
    }

    if (m_currentAdmission.mrn != mrn) {
        qWarning() << "AdmissionService::transferPatient: MRN mismatch";
        return false;
    }

    // Log transfer event
    QJsonObject details;
    details["targetDevice"] = targetDeviceLabel;
    QJsonDocument doc(details);
    QString detailsJson = doc.toJson(QJsonDocument::Compact);

    logAdmissionEvent("transfer", mrn, m_currentAdmission.name, m_currentAdmission.bedLocation,
                      m_currentAdmission.admissionSource, detailsJson);

    // Discharge from this device
    dischargePatient(mrn);

    // Emit transfer signal
    emit patientTransferred(mrn, targetDeviceLabel);

    return true;
}

AdmissionService::AdmissionInfo AdmissionService::getCurrentAdmission() const
{
    return m_currentAdmission;
}

bool AdmissionService::isPatientAdmitted() const
{
    return m_admissionState == AdmissionState::Admitted;
}

AdmissionService::AdmissionState AdmissionService::admissionState() const
{
    return m_admissionState;
}

void AdmissionService::logAdmissionEvent(const QString& eventType,
                                          const QString& mrn,
                                          const QString& name,
                                          const QString& bedLocation,
                                          AdmissionSource admissionSource,
                                          const QString& details)
{
    QString connectionName = QSqlDatabase::defaultConnection;
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isValid()) {
        qWarning() << "AdmissionService::logAdmissionEvent: No database connection";
        return;
    }

    QSqlQuery query(db);

    QString admissionSourceStr;
    switch (admissionSource) {
        case AdmissionSource::Manual:
            admissionSourceStr = "manual";
            break;
        case AdmissionSource::Barcode:
            admissionSourceStr = "barcode";
            break;
        case AdmissionSource::CentralStation:
            admissionSourceStr = "central_station";
            break;
    }

    QString deviceLabel = getDeviceLabel();

    query.prepare(
        "INSERT INTO admission_events (timestamp, event_type, patient_mrn, patient_name, "
        "device_label, bed_location, admission_source, details) "
        "VALUES (:timestamp, :event_type, :patient_mrn, :patient_name, :device_label, "
        ":bed_location, :admission_source, :details)"
    );
    query.bindValue(":timestamp", QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
    query.bindValue(":event_type", eventType);
    query.bindValue(":patient_mrn", mrn);
    query.bindValue(":patient_name", name);
    query.bindValue(":device_label", deviceLabel);
    query.bindValue(":bed_location", bedLocation);
    query.bindValue(":admission_source", admissionSourceStr);
    query.bindValue(":details", details);

    if (!query.exec()) {
        qWarning() << "AdmissionService::logAdmissionEvent: Failed to log event:"
                   << query.lastError().text();
    }
}

QString AdmissionService::getDeviceLabel() const
{
    return SettingsManager::instance().deviceLabel();
}

} // namespace zmon