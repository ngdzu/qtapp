/**
 * @file SQLiteVitalsRepository.cpp
 * @brief Implementation of SQLiteVitalsRepository for vital signs persistence.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "SQLiteVitalsRepository.h"
#include "QueryRegistry.h"
#include "generated/SchemaInfo.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

namespace zmon
{

    SQLiteVitalsRepository::SQLiteVitalsRepository(
        std::shared_ptr<IDatabaseManager> dbManager,
        QObject *parent)
        : QObject(parent), m_dbManager(dbManager)
    {
        if (!m_dbManager)
        {
            qCritical() << "SQLiteVitalsRepository: DatabaseManager is null!";
        }
    }

    SQLiteVitalsRepository::~SQLiteVitalsRepository() = default;

    Result<void> SQLiteVitalsRepository::save(const VitalRecord &vital)
    {
        if (!m_dbManager)
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "DatabaseManager is null"));
        }

        using namespace Schema::Tables;
        using namespace Schema::Columns::Vitals;
        using namespace persistence::QueryId;

        // Get prepared query for single insert
        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Vitals::INSERT);

        // Bind values from VitalRecord
        query.bindValue(":patient_mrn", QString::fromStdString(vital.patientMrn));
        query.bindValue(":timestamp", static_cast<qint64>(vital.timestampMs));
        // Bind vital value to appropriate column based on type
        query.bindValue(":heart_rate", vital.vitalType == "HR" ? vital.value : QVariant());
        query.bindValue(":spo2", vital.vitalType == "SPO2" ? vital.value : QVariant());
        query.bindValue(":respiration_rate", vital.vitalType == "RR" ? vital.value : QVariant());
        query.bindValue(":signal_quality", vital.signalQuality);
        query.bindValue(":source", QString::fromStdString(vital.deviceId));
        query.bindValue(":is_synced", 0); // New records are unsynced

        // Execute insert
        if (!query.exec())
        {
            QString errorMsg = QString("Failed to save vital: %1").arg(query.lastError().text());
            qWarning() << "SQLiteVitalsRepository::save -" << errorMsg;
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
        }

        return Result<void>::ok();
    }

    Result<size_t> SQLiteVitalsRepository::saveBatch(const std::vector<VitalRecord> &vitals)
    {
        if (!m_dbManager)
        {
            return Result<size_t>::error(Error::create(ErrorCode::DatabaseError, "DatabaseManager is null"));
        }

        if (vitals.empty())
        {
            return Result<size_t>::ok(0); // Success with 0 records saved
        }

        using namespace Schema::Tables;
        using namespace Schema::Columns::Vitals;
        using namespace persistence::QueryId;

        // Begin transaction for batch insert
        auto txResult = m_dbManager->beginTransaction();
        if (txResult.isError())
        {
            return Result<size_t>::error(txResult.error());
        }

        // Get prepared query for batch insert
        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Vitals::INSERT);

        size_t successCount = 0;

        // Insert each vital in the transaction
        for (const auto &vital : vitals)
        {
            query.bindValue(":patient_mrn", QString::fromStdString(vital.patientMrn));
            query.bindValue(":timestamp", static_cast<qint64>(vital.timestampMs));
            // Bind vital value to appropriate column based on type
            query.bindValue(":heart_rate", vital.vitalType == "HR" ? vital.value : QVariant());
            query.bindValue(":spo2", vital.vitalType == "SPO2" ? vital.value : QVariant());
            query.bindValue(":respiration_rate", vital.vitalType == "RR" ? vital.value : QVariant());
            query.bindValue(":signal_quality", vital.signalQuality);
            query.bindValue(":source", QString::fromStdString(vital.deviceId));
            query.bindValue(":is_synced", 0);

            if (!query.exec())
            {
                QString errorMsg = QString("Failed to insert vital in batch: %1").arg(query.lastError().text());
                qWarning() << "SQLiteVitalsRepository::saveBatch -" << errorMsg;
                (void)m_dbManager->rollback(); // Ignore rollback errors
                return Result<size_t>::error(Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
            }

            successCount++;
        }

        // Commit transaction
        auto commitResult = m_dbManager->commit();
        if (commitResult.isError())
        {
            return Result<size_t>::error(commitResult.error());
        }

        return Result<size_t>::ok(successCount);
    }

    std::vector<VitalRecord> SQLiteVitalsRepository::getRange(
        const std::string &patientMrn, int64_t startTimeMs, int64_t endTimeMs)
    {
        std::vector<VitalRecord> results;

        if (!m_dbManager)
        {
            qWarning() << "SQLiteVitalsRepository::getRange - DatabaseManager is null";
            return results;
        }

        using namespace persistence::QueryId;

        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Vitals::FIND_BY_PATIENT_RANGE);

        // Bind parameters
        if (patientMrn.empty())
        {
            // Query all patients if MRN is empty
            query.bindValue(":patient_mrn", "%"); // SQL LIKE wildcard
        }
        else
        {
            query.bindValue(":patient_mrn", QString::fromStdString(patientMrn));
        }
        query.bindValue(":start_time", static_cast<qint64>(startTimeMs));
        query.bindValue(":end_time", static_cast<qint64>(endTimeMs));

        // Execute query
        if (!query.exec())
        {
            qWarning() << "SQLiteVitalsRepository::getRange - Query failed:" << query.lastError().text();
            return results;
        }

        // Process results
        while (query.next())
        {
            results.push_back(rowToVitalRecord(query));
        }

        return results;
    }

    std::vector<VitalRecord> SQLiteVitalsRepository::getUnsent()
    {
        std::vector<VitalRecord> results;

        if (!m_dbManager)
        {
            qWarning() << "SQLiteVitalsRepository::getUnsent - DatabaseManager is null";
            return results;
        }

        using namespace persistence::QueryId;

        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Vitals::FIND_UNSENT);

        // Execute query (no parameters needed - finds all unsynced records)
        if (!query.exec())
        {
            qWarning() << "SQLiteVitalsRepository::getUnsent - Query failed:" << query.lastError().text();
            return results;
        }

        // Process results
        while (query.next())
        {
            results.push_back(rowToVitalRecord(query));
        }

        return results;
    }

    size_t SQLiteVitalsRepository::markAsSent(const std::vector<std::string> &vitalIds)
    {
        if (!m_dbManager)
        {
            qWarning() << "SQLiteVitalsRepository::markAsSent - DatabaseManager is null";
            return 0;
        }

        if (vitalIds.empty())
        {
            return 0;
        }

        using namespace persistence::QueryId;

        // Begin transaction
        auto txResult = m_dbManager->beginTransaction();
        if (txResult.isError())
        {
            qWarning() << "SQLiteVitalsRepository::markAsSent - Failed to begin transaction";
            return 0;
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Vitals::MARK_SENT);

        size_t markedCount = 0;

        // Mark each vital as sent
        for (const auto &vitalId : vitalIds)
        {
            query.bindValue(":vital_id", QString::fromStdString(vitalId));

            if (query.exec())
            {
                markedCount += query.numRowsAffected();
            }
            else
            {
                qWarning() << "SQLiteVitalsRepository::markAsSent - Failed to mark vital:"
                           << vitalId.c_str() << "-" << query.lastError().text();
            }
        }

        // Commit transaction
        auto commitResult = m_dbManager->commit();
        if (commitResult.isError())
            (void)m_dbManager->rollback(); // Ignore rollback errors
        {
            qWarning() << "SQLiteVitalsRepository::markAsSent - Failed to commit transaction";
            return 0;
        }

        return markedCount;
    }

    VitalRecord SQLiteVitalsRepository::rowToVitalRecord(const QSqlQuery &query) const
    {
        using namespace Schema::Columns::Vitals;

        // Extract values from current row
        std::string vitalType = "HR"; // Default, will be overwritten
        std::string patientMrn = query.value(PATIENT_MRN).toString().toStdString();
        double value = 0.0;
        int64_t timestampMs = query.value(TIMESTAMP).toLongLong();
        int signalQuality = query.value(SIGNAL_QUALITY).toInt();
        std::string deviceId = query.value(SOURCE).toString().toStdString();

        // Determine vital type from column values (check which column has a value)
        if (!query.value(HEART_RATE).isNull())
        {
            vitalType = "HR";
            value = query.value(HEART_RATE).toDouble();
        }
        else if (!query.value(SPO2).isNull())
        {
            vitalType = "SPO2";
            value = query.value(SPO2).toDouble();
        }
        else if (!query.value(RESPIRATION_RATE).isNull())
        {
            vitalType = "RR";
            value = query.value(RESPIRATION_RATE).toDouble();
        }

        // Construct VitalRecord (immutable value object)
        return VitalRecord(vitalType, value, timestampMs, signalQuality, patientMrn, deviceId);
    }

} // namespace zmon
