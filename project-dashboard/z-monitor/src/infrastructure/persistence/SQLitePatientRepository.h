/**
 * @file SQLitePatientRepository.h
 * @brief SQLite implementation of IPatientRepository with hybrid ORM + manual SQL.
 * 
 * This file contains the SQLitePatientRepository class which implements IPatientRepository
 * using a hybrid approach: QxOrm for simple CRUD operations and manual SQL for complex queries.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/repositories/IPatientRepository.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/common/Result.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include <QObject>
#include <QString>
#include <memory>

#ifdef USE_QXORM
#include "infrastructure/persistence/orm/PatientEntity.h"
#include <QxDao.h>
#endif

namespace zmon {

/**
 * @class SQLitePatientRepository
 * @brief SQLite implementation of IPatientRepository with hybrid ORM + manual SQL.
 * 
 * This repository uses a hybrid approach:
 * - **ORM (QxOrm)** for simple CRUD operations: findByMrn(), save(), remove()
 * - **Manual SQL** for complex queries: findAll(), getAdmissionHistory()
 * 
 * The repository converts between PatientEntity (ORM/persistence) and PatientAggregate
 * (domain) to maintain DDD separation.
 * 
 * @note Runs on Database I/O Thread for non-blocking operations.
 * @note Uses DatabaseManager for connection management.
 * 
 * @thread Database I/O Thread
 * @ingroup Infrastructure
 */
class SQLitePatientRepository : public IPatientRepository, public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * 
     * @param dbManager Database manager for connection management
     * @param parent Parent QObject (for Qt parent-child ownership)
     */
    explicit SQLitePatientRepository(DatabaseManager* dbManager, QObject* parent = nullptr);
    
    /**
     * @brief Destructor.
     */
    ~SQLitePatientRepository() override;

    /**
     * @brief Find patient by MRN.
     * 
     * Uses ORM (QxOrm) for simple lookup if USE_QXORM is enabled,
     * otherwise falls back to manual SQL.
     * 
     * @param mrn Medical Record Number
     * @return Result containing shared pointer to patient aggregate if found, Error if not found or database error
     */
    Result<std::shared_ptr<PatientAggregate>> findByMrn(const std::string& mrn) override;
    
    /**
     * @brief Save patient aggregate.
     * 
     * Uses ORM (QxOrm) for save operation if USE_QXORM is enabled,
     * otherwise falls back to manual SQL.
     * 
     * @param patient Patient aggregate to save
     * @return Result<void> - Success if save succeeded, Error with details if failed
     */
    Result<void> save(const PatientAggregate& patient) override;
    
    /**
     * @brief Get admission history for a patient.
     * 
     * Uses manual SQL for complex query with date ranges and joins.
     * 
     * @param mrn Medical Record Number
     * @return Result containing vector of admission event records (most recent first), Error if database error
     */
    Result<std::vector<std::string>> getAdmissionHistory(const std::string& mrn) override;
    
    /**
     * @brief Find all patients.
     * 
     * Uses manual SQL for querying all records. Patients that fail to convert
     * (e.g., invalid data, non-admitted patients) are silently skipped. If all
     * patients fail to convert, an error is returned.
     * 
     * @return Result containing vector of patient aggregates, Error if database error or all patients failed to convert
     */
    Result<std::vector<std::shared_ptr<PatientAggregate>>> findAll() override;
    
    /**
     * @brief Delete patient by MRN.
     * 
     * Uses ORM (QxOrm) for delete operation if USE_QXORM is enabled,
     * otherwise falls back to manual SQL.
     * 
     * @param mrn Medical Record Number
     * @return Result<void> - Success if deletion succeeded, Error with details if failed
     */
    Result<void> remove(const std::string& mrn) override;

private:
    DatabaseManager* m_dbManager;  ///< Database manager for connection management
    
    /**
     * @brief Convert PatientEntity (ORM) to PatientAggregate (domain).
     * 
     * Converts persistence DTO to domain aggregate.
     * 
     * @param entity Patient entity from database
     * @return Result containing patient aggregate, Error if conversion fails
     */
#ifdef USE_QXORM
    Result<std::shared_ptr<PatientAggregate>> entityToAggregate(const persistence::PatientEntity& entity);
#endif
    
    /**
     * @brief Convert QSqlQuery result to PatientAggregate (domain).
     * 
     * Converts SQL query result to domain aggregate.
     * 
     * @param query SQL query result
     * @return Result containing patient aggregate, Error if conversion fails
     */
    Result<std::shared_ptr<PatientAggregate>> queryToAggregate(const QSqlQuery& query);
    
    /**
     * @brief Convert PatientAggregate (domain) to PatientEntity (ORM).
     * 
     * Converts domain aggregate to persistence DTO.
     * 
     * @param aggregate Patient aggregate from domain layer
     * @return Patient entity for database
     */
#ifdef USE_QXORM
    persistence::PatientEntity aggregateToEntity(const PatientAggregate& aggregate);
#endif
    
    /**
     * @brief Find patient by MRN using ORM.
     * 
     * Uses QxOrm to fetch patient by MRN.
     * 
     * @param mrn Medical Record Number
     * @return Result containing shared pointer to patient aggregate if found, Error if not found or database error
     */
#ifdef USE_QXORM
    Result<std::shared_ptr<PatientAggregate>> findByMrnOrm(const std::string& mrn);
#endif
    
    /**
     * @brief Find patient by MRN using manual SQL.
     * 
     * Uses manual SQL query to fetch patient by MRN.
     * 
     * @param mrn Medical Record Number
     * @return Result containing shared pointer to patient aggregate if found, Error if not found or database error
     */
    Result<std::shared_ptr<PatientAggregate>> findByMrnSql(const std::string& mrn);
    
    /**
     * @brief Save patient using ORM.
     * 
     * Uses QxOrm to save patient entity.
     * 
     * @param patient Patient aggregate to save
     * @return Result<void> - Success or error details
     */
#ifdef USE_QXORM
    Result<void> saveOrm(const PatientAggregate& patient);
#endif
    
    /**
     * @brief Save patient using manual SQL.
     * 
     * Uses manual SQL INSERT/UPDATE to save patient.
     * 
     * @param patient Patient aggregate to save
     * @return Result<void> - Success or error details
     */
    Result<void> saveSql(const PatientAggregate& patient);
    
    /**
     * @brief Remove patient using ORM.
     * 
     * Uses QxOrm to delete patient entity.
     * 
     * @param mrn Medical Record Number
     * @return Result<void> - Success or error details
     */
#ifdef USE_QXORM
    Result<void> removeOrm(const std::string& mrn);
#endif
    
    /**
     * @brief Remove patient using manual SQL.
     * 
     * Uses manual SQL DELETE to remove patient.
     * 
     * @param mrn Medical Record Number
     * @return Result<void> - Success or error details
     */
    Result<void> removeSql(const std::string& mrn);
    
    /**
     * @brief Convert admission status string to enum.
     * 
     * @param status Admission status string (ADMITTED/DISCHARGED/TRANSFERRED)
     * @return AdmissionState enum value
     */
    static AdmissionState stringToAdmissionState(const QString& status);
    
    /**
     * @brief Convert admission state enum to string.
     * 
     * @param state Admission state enum
     * @return Admission status string
     */
    static QString admissionStateToString(AdmissionState state);
    
    /**
     * @brief Convert date string (ISO 8601) to Unix milliseconds.
     * 
     * @param dateStr Date string in ISO 8601 format (YYYY-MM-DD)
     * @return Unix timestamp in milliseconds, or 0 if invalid
     */
    static int64_t dateStringToMs(const QString& dateStr);
    
    /**
     * @brief Convert Unix milliseconds to date string (ISO 8601).
     * 
     * @param timestampMs Unix timestamp in milliseconds
     * @return Date string in ISO 8601 format (YYYY-MM-DD)
     */
    static QString msToDateString(int64_t timestampMs);
};

} // namespace zmon

