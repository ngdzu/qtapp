/**
 * @file AdmissionService.cpp
 * @brief Implementation of AdmissionService for ADT workflow.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "AdmissionService.h"

#include "domain/common/Result.h"
#include "domain/constants/ActionTypes.h"
#include "domain/events/DomainEventDispatcher.h"
#include "domain/monitoring/events/PatientAdmitted.h"
#include "domain/monitoring/events/PatientDischarged.h"
#include "domain/monitoring/events/PatientTransferred.h"
#include "infrastructure/adapters/SettingsManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QJsonDocument>

namespace zmon
{
    AdmissionService::AdmissionService(IActionLogRepository *actionLogRepo, std::shared_ptr<DomainEventDispatcher> eventDispatcher, QObject *parent)
        : QObject(parent), m_actionLogRepo(actionLogRepo), m_eventDispatcher(eventDispatcher), m_admissionState(AdmissionState::NotAdmitted)
    {
    }

    Result<void> AdmissionService::admitPatient(const PatientIdentity &patientIdentity,
                                                const BedLocation &bedLocation,
                                                AdmissionSource admissionSource)
    {
        if (!patientIdentity.isValid())
        {
            return Result<void>::error(
                Error::create(ErrorCode::InvalidArgument,
                              "Invalid patient identity",
                              {{"mrn", patientIdentity.mrn}}));
        }

        // Check if patient is already admitted
        if (isPatientAdmitted())
        {
            return Result<void>::error(
                Error::create(ErrorCode::AlreadyExists,
                              "Patient is already admitted",
                              {{"currentMrn", m_currentAdmission.mrn.toStdString()}}));
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
        m_currentAdmission.dischargedAt = QDateTime(); // NULL
        m_currentAdmission.admissionSource = admissionSource;
        m_currentAdmission.deviceLabel = deviceLabel;

        m_admissionState = AdmissionState::Admitted;

        // Log admission event
        QString admissionSourceStr;
        switch (admissionSource)
        {
        case AdmissionSource::Manual:
            admissionSourceStr = AdmissionSources::MANUAL;
            break;
        case AdmissionSource::Barcode:
            admissionSourceStr = AdmissionSources::BARCODE;
            break;
        case AdmissionSource::CentralStation:
            admissionSourceStr = AdmissionSources::CENTRAL_STATION;
            break;
        }

        logAdmissionEvent(EventTypes::ADMISSION, mrn, name, bedLocationStr, admissionSource);

        // Log to action log repository
        if (m_actionLogRepo)
        {
            QJsonObject details;
            details[JsonKeys::PATIENT_NAME] = name;
            details[JsonKeys::ADMISSION_METHOD] = admissionSourceStr;
            details[JsonKeys::BED_LOCATION] = bedLocationStr;
            details[JsonKeys::DEVICE_LABEL] = deviceLabel;

            ActionLogEntry entry;
            entry.actionType = ActionTypes::ADMIT_PATIENT;
            entry.targetType = TargetTypes::PATIENT;
            entry.targetId = mrn;
            entry.details = details;
            entry.result = ActionResults::SUCCESS;
            // Note: userId, userRole, sessionTokenHash should be set by SecurityService
            // when calling recordConfigurationAction(). For now, leave empty.

            m_actionLogRepo->logAction(entry);
        }
        // Emit signals
        emit patientAdmitted(mrn, name, bedLocationStr);
        emit admissionStateChanged(m_admissionState);

        // Dispatch domain event
        if (m_eventDispatcher)
        {
            Events::PatientAdmitted event(
                patientIdentity,
                bedLocation,
                admissionSourceStr.toStdString(),
                QDateTime::currentMSecsSinceEpoch(),
                "DEVICE_001" // Placeholder for device ID
            );
            m_eventDispatcher->dispatch(event);
        }

        return Result<void>::ok();
    }

    Result<void> AdmissionService::dischargePatient(const QString &mrn)
    {
        if (!isPatientAdmitted())
        {
            return Result<void>::error(
                Error::create(ErrorCode::NotFound,
                              "No patient is currently admitted",
                              {{"mrn", mrn.toStdString()}}));
        }

        if (m_currentAdmission.mrn != mrn)
        {
            return Result<void>::error(
                Error::create(ErrorCode::Conflict,
                              "Discharge MRN does not match current admission",
                              {{"expectedMrn", m_currentAdmission.mrn.toStdString()},
                               {"requestedMrn", mrn.toStdString()}}));
        }

        // TODO: Update patient record in database (set discharged_at)

        // Update current admission
        m_currentAdmission.dischargedAt = QDateTime::currentDateTimeUtc();
        m_admissionState = AdmissionState::Discharged;

        // Log discharge event
        logAdmissionEvent(EventTypes::DISCHARGE, mrn, m_currentAdmission.name, m_currentAdmission.bedLocation,
                          m_currentAdmission.admissionSource);

        // Log to action log repository
        if (m_actionLogRepo)
        {
            QJsonObject details;
            details[JsonKeys::PATIENT_NAME] = m_currentAdmission.name;
            details[JsonKeys::BED_LOCATION] = m_currentAdmission.bedLocation;

            ActionLogEntry entry;
            entry.actionType = ActionTypes::DISCHARGE_PATIENT;
            entry.targetType = TargetTypes::PATIENT;
            entry.targetId = mrn;
            entry.details = details;
            entry.result = ActionResults::SUCCESS;

            m_actionLogRepo->logAction(entry);
        }

        // Emit signals
        emit patientDischarged(mrn);
        emit admissionStateChanged(m_admissionState);

        // Dispatch domain event
        if (m_eventDispatcher)
        {
            Events::PatientDischarged event(mrn.toStdString(), QDateTime::currentMSecsSinceEpoch(), "DEVICE_001");
            m_eventDispatcher->dispatch(event);
        }

        // Clear current admission
        m_currentAdmission = AdmissionInfo();
        m_admissionState = AdmissionState::NotAdmitted;
        emit admissionStateChanged(m_admissionState);

        return Result<void>::ok();
    }

    Result<void> AdmissionService::transferPatient(const QString &mrn, const QString &targetDeviceLabel)
    {
        if (!isPatientAdmitted())
        {
            return Result<void>::error(
                Error::create(ErrorCode::NotFound,
                              "No patient is currently admitted",
                              {{"mrn", mrn.toStdString()}}));
        }

        if (m_currentAdmission.mrn != mrn)
        {
            return Result<void>::error(
                Error::create(ErrorCode::Conflict,
                              "Transfer MRN does not match current admission",
                              {{"expectedMrn", m_currentAdmission.mrn.toStdString()},
                               {"requestedMrn", mrn.toStdString()}}));
        }

        // Log transfer event
        QJsonObject details;
        details[JsonKeys::TARGET_DEVICE] = targetDeviceLabel;
        QJsonDocument doc(details);
        QString detailsJson = doc.toJson(QJsonDocument::Compact);

        logAdmissionEvent(EventTypes::TRANSFER, mrn, m_currentAdmission.name, m_currentAdmission.bedLocation,
                          m_currentAdmission.admissionSource, detailsJson);

        // Log to action log repository
        if (m_actionLogRepo)
        {
            QJsonObject logDetails;
            logDetails[JsonKeys::PATIENT_NAME] = m_currentAdmission.name;
            logDetails[JsonKeys::TARGET_DEVICE] = targetDeviceLabel;
            logDetails[JsonKeys::BED_LOCATION] = m_currentAdmission.bedLocation;

            ActionLogEntry entry;
            entry.actionType = ActionTypes::TRANSFER_PATIENT;
            entry.targetType = TargetTypes::PATIENT;
            entry.targetId = mrn;
            entry.details = logDetails;
            entry.result = ActionResults::SUCCESS;

            m_actionLogRepo->logAction(entry);
        }

        // Discharge from this device
        dischargePatient(mrn);

        // Emit transfer signal
        emit patientTransferred(mrn, targetDeviceLabel);

        // Dispatch domain event
        if (m_eventDispatcher)
        {
            Events::PatientTransferred event(
                mrn.toStdString(),
                targetDeviceLabel.toStdString(),
                "DEVICE_001", // Source device
                QDateTime::currentMSecsSinceEpoch());
            m_eventDispatcher->dispatch(event);
        }

        return Result<void>::ok();
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

    void AdmissionService::logAdmissionEvent(const QString &eventType,
                                             const QString &mrn,
                                             const QString &name,
                                             const QString &bedLocation,
                                             AdmissionSource admissionSource,
                                             const QString &details)
    {
        QString connectionName = QSqlDatabase::defaultConnection;
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (!db.isValid())
        {
            return;
        }

        QSqlQuery query(db);

        QString admissionSourceStr;
        switch (admissionSource)
        {
        case AdmissionSource::Manual:
            admissionSourceStr = AdmissionSources::MANUAL;
            break;
        case AdmissionSource::Barcode:
            admissionSourceStr = AdmissionSources::BARCODE;
            break;
        case AdmissionSource::CentralStation:
            admissionSourceStr = AdmissionSources::CENTRAL_STATION;
            break;
        }

        QString deviceLabel = getDeviceLabel();

        query.prepare(
            "INSERT INTO admission_events (timestamp, event_type, patient_mrn, patient_name, "
            "device_label, bed_location, admission_source, details) "
            "VALUES (:timestamp, :event_type, :patient_mrn, :patient_name, :device_label, "
            ":bed_location, :admission_source, :details)");
        query.bindValue(":timestamp", QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
        query.bindValue(":event_type", eventType);
        query.bindValue(":patient_mrn", mrn);
        query.bindValue(":patient_name", name);
        query.bindValue(":device_label", deviceLabel);
        query.bindValue(":bed_location", bedLocation);
        query.bindValue(":admission_source", admissionSourceStr);
        query.bindValue(":details", details);

        // Note: Error is silently ignored here as this is a logging function.
        // If logging fails, it should not prevent the main operation from completing.
        // Consider emitting a signal or using proper logging infrastructure for critical errors.
        query.exec();
    }

    QString AdmissionService::getDeviceLabel() const
    {
        return SettingsManager::instance().deviceLabel();
    }

} // namespace zmon