/**
 * @file SQLitePatientRepository.cpp
 * @brief Implementation of SQLitePatientRepository with hybrid ORM + manual SQL.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/persistence/SQLitePatientRepository.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include "domain/common/Result.h"
#include "infrastructure/persistence/generated/SchemaInfo.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QStringList>
#include <algorithm>

#ifdef USE_QXORM
#include "infrastructure/persistence/orm/PatientEntity.h"
#include <QxDao/QxDao.h>
#endif

namespace zmon
{

    SQLitePatientRepository::SQLitePatientRepository(DatabaseManager *dbManager, QObject *parent)
        : QObject(parent), m_dbManager(dbManager)
    {
        // DatabaseManager is required - caller should ensure it's not null
        // If null, operations will fail gracefully by returning errors
    }

    SQLitePatientRepository::~SQLitePatientRepository() = default;

    Result<std::shared_ptr<PatientAggregate>> SQLitePatientRepository::findByMrn(const std::string &mrn)
    {
        if (!m_dbManager || !m_dbManager->isOpen())
        {
            return Result<std::shared_ptr<PatientAggregate>>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

#ifdef USE_QXORM
        // Use ORM for simple lookup if QxOrm is enabled
        if (DatabaseManager::isQxOrmEnabled())
        {
            return findByMrnOrm(mrn);
        }
#endif

        // Fallback to manual SQL
        return findByMrnSql(mrn);
    }

    Result<void> SQLitePatientRepository::save(const PatientAggregate &patient)
    {
        if (!m_dbManager || !m_dbManager->isOpen())
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

#ifdef USE_QXORM
        // Use ORM for save if QxOrm is enabled
        if (DatabaseManager::isQxOrmEnabled())
        {
            return saveOrm(patient);
        }
#endif

        // Fallback to manual SQL
        return saveSql(patient);
    }

    Result<void> SQLitePatientRepository::remove(const std::string &mrn)
    {
        if (!m_dbManager || !m_dbManager->isOpen())
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

#ifdef USE_QXORM
        // Use ORM for delete if QxOrm is enabled
        if (DatabaseManager::isQxOrmEnabled())
        {
            return removeOrm(mrn);
        }
#endif

        // Fallback to manual SQL
        return removeSql(mrn);
    }

    Result<std::vector<std::shared_ptr<PatientAggregate>>> SQLitePatientRepository::findAll()
    {
        // Always use manual SQL for complex queries (findAll)
        if (!m_dbManager || !m_dbManager->isOpen())
        {
            return Result<std::vector<std::shared_ptr<PatientAggregate>>>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

        QSqlQuery query = m_dbManager->getPreparedQueryForRead(persistence::QueryId::Patient::FIND_ALL);
        if (!query.isValid())
        {
            return Result<std::vector<std::shared_ptr<PatientAggregate>>>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Query not registered: %1").arg(persistence::QueryId::Patient::FIND_ALL).toStdString()));
        }

        if (!query.exec())
        {
            QSqlError error = query.lastError();
            return Result<std::vector<std::shared_ptr<PatientAggregate>>>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot query all patients: %1").arg(error.text()).toStdString()));
        }

        std::vector<std::shared_ptr<PatientAggregate>> patients;
        int failedConversions = 0;
        while (query.next())
        {
            auto patientResult = queryToAggregate(query);
            if (patientResult.isOk())
            {
                patients.push_back(patientResult.value());
            }
            else
            {
                // Skip patients that fail to convert (e.g., invalid data, non-admitted patients)
                // This allows findAll() to return partial results even if some records are invalid
                // Caller can detect missing expected patients by comparing count
                failedConversions++;
            }
        }

        // If all patients failed to convert, return error
        if (patients.empty() && failedConversions > 0)
        {
            return Result<std::vector<std::shared_ptr<PatientAggregate>>>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("All %1 patient(s) failed to convert from database records").arg(failedConversions).toStdString()));
        }

        // Return partial results (some patients may have been skipped due to conversion failures)
        return Result<std::vector<std::shared_ptr<PatientAggregate>>>::ok(patients);
    }

    Result<std::vector<std::string>> SQLitePatientRepository::getAdmissionHistory(const std::string &mrn)
    {
        // Always use manual SQL for complex queries (admission history with joins)
        if (!m_dbManager || !m_dbManager->isOpen())
        {
            return Result<std::vector<std::string>>::error(
                Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

        using namespace Schema::Columns::AdmissionEvents;

        QSqlQuery query = m_dbManager->getPreparedQueryForRead(persistence::QueryId::Patient::GET_ADMISSION_HISTORY);
        if (!query.isValid())
        {
            return Result<std::vector<std::string>>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Query not registered: %1").arg(persistence::QueryId::Patient::GET_ADMISSION_HISTORY).toStdString()));
        }

        query.bindValue(":mrn", QString::fromStdString(mrn));

        if (!query.exec())
        {
            QSqlError error = query.lastError();
            return Result<std::vector<std::string>>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot query admission history: %1").arg(error.text()).toStdString()));
        }

        std::vector<std::string> history;
        while (query.next())
        {
            QString eventType = query.value(EVENT_TYPE).toString();
            QString details = query.value(DETAILS).toString();
            qint64 timestamp = query.value(TIMESTAMP).toLongLong();

            QString historyEntry = QString("%1|%2|%3")
                                       .arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate))
                                       .arg(eventType)
                                       .arg(details);

            history.push_back(historyEntry.toStdString());
        }

        return Result<std::vector<std::string>>::ok(history);
    }

#ifdef USE_QXORM
    Result<std::shared_ptr<PatientAggregate>> SQLitePatientRepository::findByMrnOrm(const std::string &mrn)
    {
        try
        {
            persistence::PatientEntity entity;
            entity.mrn = QString::fromStdString(mrn);

            qx::QxSqlDatabase *qxDb = m_dbManager->getQxOrmConnection();
            qx::dao::fetch_by_id(entity);

            // Check if patient was found
            if (entity.name.isEmpty() && entity.mrn.isEmpty())
            {
                return Result<std::shared_ptr<PatientAggregate>>::error(
                    Error::create(ErrorCode::NotFound,
                                  QString("Patient not found: %1").arg(QString::fromStdString(mrn)).toStdString()));
            }

            return entityToAggregate(entity);
        }
        catch (const std::exception &e)
        {
            return Result<std::shared_ptr<PatientAggregate>>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("QxOrm fetch failed: %1").arg(e.what()).toStdString()));
        }
    }
#endif // USE_QXORM

#ifdef USE_QXORM
    Result<void> SQLitePatientRepository::saveOrm(const PatientAggregate &patient)
    {
        try
        {
            persistence::PatientEntity entity = aggregateToEntity(patient);

            qx::QxSqlDatabase *qxDb = m_dbManager->getQxOrmConnection();
            qx::dao::save(entity);

            // QxOrm throws exceptions on error, so if we get here, save succeeded
            return Result<void>::ok();
        }
        catch (const std::exception &e)
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("QxOrm save failed: %1").arg(e.what()).toStdString()));
        }
    }

    Result<void> SQLitePatientRepository::removeOrm(const std::string &mrn)
    {
        try
        {
            persistence::PatientEntity entity;
            entity.mrn = QString::fromStdString(mrn);

            qx::QxSqlDatabase *qxDb = m_dbManager->getQxOrmConnection();
            qx::dao::delete_by_id(entity);

            // QxOrm throws exceptions on error, so if we get here, delete succeeded
            return Result<void>::ok();
        }
        catch (const std::exception &e)
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("QxOrm delete failed: %1").arg(e.what()).toStdString()));
        }
    }
#endif // USE_QXORM

#ifdef USE_QXORM
    persistence::PatientEntity SQLitePatientRepository::aggregateToEntity(const PatientAggregate &aggregate)
    {
        persistence::PatientEntity entity;

        const PatientIdentity &identity = aggregate.getPatientIdentity();
        const BedLocation &bedLocation = aggregate.getBedLocation();

        // Basic patient info
        entity.mrn = QString::fromStdString(identity.mrn);
        entity.name = QString::fromStdString(identity.name);
        entity.dob = msToDateString(identity.dateOfBirthMs);
        entity.sex = QString::fromStdString(identity.sex);

        // Allergies (convert vector to comma-separated string or JSON)
        QStringList allergyList;
        for (const auto &allergy : identity.allergies)
        {
            allergyList << QString::fromStdString(allergy);
        }
        entity.allergies = allergyList.join(",");

        // Bed location
        entity.bedLocation = QString::fromStdString(bedLocation.location);

        // Admission info
        entity.admissionStatus = admissionStateToString(aggregate.getAdmissionState());
        entity.admittedAt = aggregate.getAdmittedAt();
        entity.dischargedAt = aggregate.getDischargedAt();

        // Metadata
        entity.createdAt = QDateTime::currentMSecsSinceEpoch();
        entity.lastLookupAt = 0;     // Will be updated when lookup occurs
        entity.lookupSource = "";    // Will be updated when lookup occurs
        entity.deviceLabel = "";     // Will be set from settings
        entity.admissionSource = ""; // Will be set from admission workflow
        entity.room = "";            // Deprecated

        return entity;
    }
#endif // USE_QXORM

#ifdef USE_QXORM
    Result<std::shared_ptr<PatientAggregate>> SQLitePatientRepository::entityToAggregate(const persistence::PatientEntity &entity)
    {
        // Convert from PatientEntity (ORM)
        if (entity.mrn.isEmpty())
        {
            return Result<std::shared_ptr<PatientAggregate>>::error(
                Error::create(ErrorCode::InvalidArgument, "Patient entity has empty MRN"));
        }

        auto aggregate = std::make_shared<PatientAggregate>();

        // Parse patient identity
        std::vector<std::string> allergies;
        if (!entity.allergies.isEmpty())
        {
            QStringList allergyList = entity.allergies.split(",");
            for (const QString &allergy : allergyList)
            {
                allergies.push_back(allergy.trimmed().toStdString());
            }
        }

        PatientIdentity identity(
            entity.mrn.toStdString(),
            entity.name.toStdString(),
            dateStringToMs(entity.dob),
            entity.sex.toStdString(),
            allergies);

        // Parse bed location
        BedLocation bedLocation(entity.bedLocation.toStdString());

        // Set admission state
        AdmissionState state = stringToAdmissionState(entity.admissionStatus);

        // Create aggregate and set state
        // Note: We need to use the aggregate's internal state setters
        // Since PatientAggregate doesn't expose setters, we'll need to reconstruct
        // the aggregate by calling admit() if admitted

        if (state == AdmissionState::Admitted)
        {
            // Reconstruct admission state
            std::string admissionSource = entity.admissionSource.toStdString();
            auto result = aggregate->admit(identity, bedLocation, admissionSource);
            if (result.isError())
            {
                return Result<std::shared_ptr<PatientAggregate>>::error(
                    Error::create(ErrorCode::DatabaseError,
                                  QString("Failed to reconstruct patient admission: %1")
                                      .arg(QString::fromStdString(result.error().message))
                                      .toStdString()));
            }
        }
        else
        {
            // For non-admitted patients, we can't fully reconstruct the aggregate
            // This is a limitation - we'd need to add a factory method to PatientAggregate
            return Result<std::shared_ptr<PatientAggregate>>::error(
                Error::create(ErrorCode::InvalidArgument,
                              "Cannot reconstruct non-admitted patient from entity (PatientAggregate requires admission state)"));
        }

        return Result<std::shared_ptr<PatientAggregate>>::ok(aggregate);
    }
#endif // USE_QXORM

    Result<std::shared_ptr<PatientAggregate>> SQLitePatientRepository::queryToAggregate(const QSqlQuery &query)
    {
        // Convert from QSqlQuery (manual SQL)
        using namespace Schema::Columns::Patients;

        if (query.value(MRN).toString().isEmpty())
        {
            return Result<std::shared_ptr<PatientAggregate>>::error(
                Error::create(ErrorCode::InvalidArgument, "Query result has empty MRN"));
        }

        auto aggregate = std::make_shared<PatientAggregate>();

        // Parse patient identity
        QString mrn = query.value(MRN).toString();
        QString name = query.value(NAME).toString();
        QString dob = query.value(DOB).toString();
        QString sex = query.value(SEX).toString();
        QString allergiesStr = query.value(ALLERGIES).toString();

        std::vector<std::string> allergies;
        if (!allergiesStr.isEmpty())
        {
            QStringList allergyList = allergiesStr.split(",");
            for (const QString &allergy : allergyList)
            {
                allergies.push_back(allergy.trimmed().toStdString());
            }
        }

        PatientIdentity identity(
            mrn.toStdString(),
            name.toStdString(),
            dateStringToMs(dob),
            sex.toStdString(),
            allergies);

        // Parse bed location
        QString bedLocationStr = query.value(BED_LOCATION).toString();
        BedLocation bedLocation(bedLocationStr.toStdString());

        // Parse admission status
        QString admissionStatus = query.value(ADMISSION_STATUS).toString();
        AdmissionState state = stringToAdmissionState(admissionStatus);

        // Create aggregate
        if (state == AdmissionState::Admitted)
        {
            QString admissionSource = query.value(ADMISSION_SOURCE).toString();
            auto result = aggregate->admit(identity, bedLocation, admissionSource.toStdString());
            if (result.isError())
            {
                return Result<std::shared_ptr<PatientAggregate>>::error(
                    Error::create(ErrorCode::DatabaseError,
                                  QString("Failed to reconstruct patient admission: %1")
                                      .arg(QString::fromStdString(result.error().message))
                                      .toStdString()));
            }
        }
        else
        {
            // For non-admitted patients, we can't fully reconstruct the aggregate
            return Result<std::shared_ptr<PatientAggregate>>::error(
                Error::create(ErrorCode::InvalidArgument,
                              "Cannot reconstruct non-admitted patient from query (PatientAggregate requires admission state)"));
        }

        return Result<std::shared_ptr<PatientAggregate>>::ok(aggregate);
    }

    Result<std::shared_ptr<PatientAggregate>> SQLitePatientRepository::findByMrnSql(const std::string &mrn)
    {
        QSqlQuery query = m_dbManager->getPreparedQueryForRead(persistence::QueryId::Patient::FIND_BY_MRN);
        if (!query.isValid())
        {
            // Fallback to direct SQL for tests when registry not initialized
            query = QSqlQuery(m_dbManager->getReadConnection());
            query.prepare("SELECT * FROM patients WHERE mrn = :mrn");
        }

        query.bindValue(":mrn", QString::fromStdString(mrn));

        if (!query.exec())
        {
            QSqlError error = query.lastError();
            return Result<std::shared_ptr<PatientAggregate>>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot query patient by MRN: %1").arg(error.text()).toStdString()));
        }

        if (!query.next())
        {
            return Result<std::shared_ptr<PatientAggregate>>::error(
                Error::create(ErrorCode::NotFound,
                              QString("Patient not found: %1").arg(QString::fromStdString(mrn)).toStdString()));
        }

        return queryToAggregate(query);
    }

    Result<void> SQLitePatientRepository::saveSql(const PatientAggregate &patient)
    {
        const PatientIdentity &identity = patient.getPatientIdentity();
        const BedLocation &bedLocation = patient.getBedLocation();

        // Check if patient exists
        QSqlQuery checkQuery = m_dbManager->getPreparedQueryForRead(persistence::QueryId::Patient::CHECK_EXISTS);
        if (!checkQuery.isValid())
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Query not registered: %1").arg(persistence::QueryId::Patient::CHECK_EXISTS).toStdString()));
        }

        checkQuery.bindValue(":mrn", QString::fromStdString(identity.mrn));

        if (!checkQuery.exec() || !checkQuery.next())
        {
            QSqlError error = checkQuery.lastError();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot check patient existence: %1").arg(error.text()).toStdString()));
        }

        bool exists = checkQuery.value(0).toInt() > 0;

        // Prepare allergies string
        QStringList allergyList;
        for (const auto &allergy : identity.allergies)
        {
            allergyList << QString::fromStdString(allergy);
        }
        QString allergiesStr = allergyList.join(",");

        QSqlQuery query = exists
                              ? m_dbManager->getPreparedQuery(persistence::QueryId::Patient::UPDATE)
                              : m_dbManager->getPreparedQuery(persistence::QueryId::Patient::INSERT);

        if (!query.isValid())
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Query not registered: %1").arg(exists ? persistence::QueryId::Patient::UPDATE : persistence::QueryId::Patient::INSERT).toStdString()));
        }

        if (exists)
        {
            // UPDATE
            query.bindValue(":mrn", QString::fromStdString(identity.mrn));
            query.bindValue(":name", QString::fromStdString(identity.name));
            query.bindValue(":dob", msToDateString(identity.dateOfBirthMs));
            query.bindValue(":sex", QString::fromStdString(identity.sex));
            query.bindValue(":allergies", allergiesStr);
            query.bindValue(":bedLocation", QString::fromStdString(bedLocation.location));
            query.bindValue(":admissionStatus", admissionStateToString(patient.getAdmissionState()));
            query.bindValue(":admittedAt", patient.getAdmittedAt());
            query.bindValue(":dischargedAt", patient.getDischargedAt());
            query.bindValue(":admissionSource", "");
        }
        else
        {
            // INSERT
            query.bindValue(":mrn", QString::fromStdString(identity.mrn));
            query.bindValue(":name", QString::fromStdString(identity.name));
            query.bindValue(":dob", msToDateString(identity.dateOfBirthMs));
            query.bindValue(":sex", QString::fromStdString(identity.sex));
            query.bindValue(":allergies", allergiesStr);
            query.bindValue(":bedLocation", QString::fromStdString(bedLocation.location));
            query.bindValue(":admissionStatus", admissionStateToString(patient.getAdmissionState()));
            query.bindValue(":admittedAt", patient.getAdmittedAt());
            query.bindValue(":dischargedAt", patient.getDischargedAt());
            query.bindValue(":admissionSource", "");
            query.bindValue(":createdAt", QDateTime::currentMSecsSinceEpoch());
        }

        if (!query.exec())
        {
            QSqlError error = query.lastError();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot save patient: %1").arg(error.text()).toStdString()));
        }

        return Result<void>::ok();
    }

    Result<void> SQLitePatientRepository::removeSql(const std::string &mrn)
    {
        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Patient::DELETE);
        if (!query.isValid())
        {
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Query not registered: %1").arg(persistence::QueryId::Patient::DELETE).toStdString()));
        }

        query.bindValue(":mrn", QString::fromStdString(mrn));

        if (!query.exec())
        {
            QSqlError error = query.lastError();
            return Result<void>::error(
                Error::create(ErrorCode::DatabaseError,
                              QString("Cannot delete patient: %1").arg(error.text()).toStdString()));
        }

        if (query.numRowsAffected() == 0)
        {
            return Result<void>::error(
                Error::create(ErrorCode::NotFound,
                              QString("Patient not found: %1").arg(QString::fromStdString(mrn)).toStdString()));
        }

        return Result<void>::ok();
    }

    AdmissionState SQLitePatientRepository::stringToAdmissionState(const QString &status)
    {
        if (status == "ADMITTED")
        {
            return AdmissionState::Admitted;
        }
        else if (status == "DISCHARGED")
        {
            return AdmissionState::Discharged;
        }
        else if (status == "TRANSFERRED")
        {
            return AdmissionState::Discharged; // Transferred is treated as discharged
        }
        return AdmissionState::NotAdmitted;
    }

    QString SQLitePatientRepository::admissionStateToString(AdmissionState state)
    {
        switch (state)
        {
        case AdmissionState::Admitted:
            return "ADMITTED";
        case AdmissionState::Discharged:
            return "DISCHARGED";
        case AdmissionState::NotAdmitted:
        default:
            return "NOT_ADMITTED";
        }
    }

    int64_t SQLitePatientRepository::dateStringToMs(const QString &dateStr)
    {
        if (dateStr.isEmpty())
        {
            return 0;
        }

        // Parse ISO 8601 date (YYYY-MM-DD)
        QDate date = QDate::fromString(dateStr, Qt::ISODate);
        if (!date.isValid())
        {
            return 0;
        }

        // Convert to Unix milliseconds
        QDateTime dateTime(date, QTime(0, 0, 0), Qt::UTC);
        return dateTime.toMSecsSinceEpoch();
    }

    QString SQLitePatientRepository::msToDateString(int64_t timestampMs)
    {
        if (timestampMs == 0)
        {
            return QString();
        }

        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timestampMs, Qt::UTC);
        return dateTime.date().toString(Qt::ISODate);
    }

} // namespace zmon
