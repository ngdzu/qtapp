/**
 * @file SQLiteTelemetryRepository.cpp
 * @brief Implementation of SQLiteTelemetryRepository for telemetry batch persistence.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "SQLiteTelemetryRepository.h"
#include "QueryRegistry.h"
#include "generated/SchemaInfo.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDateTime>

namespace zmon
{
    using namespace persistence;
    namespace TelemetryMetricsCols = Schema::Columns::TelemetryMetrics;
    const QString TELEMETRY_METRICS = Schema::Tables::TELEMETRY_METRICS;

    SQLiteTelemetryRepository::SQLiteTelemetryRepository(
        std::shared_ptr<IDatabaseManager> dbManager,
        QObject *parent)
        : QObject(parent), m_dbManager(dbManager)
    {
        if (!m_dbManager)
        {
            qCritical() << "SQLiteTelemetryRepository: DatabaseManager is null!";
        }
    }

    SQLiteTelemetryRepository::~SQLiteTelemetryRepository() = default;

    Result<void> SQLiteTelemetryRepository::save(const TelemetryBatch &batch)
    {
        if (!m_dbManager)
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "DatabaseManager is null"));
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Telemetry::INSERT);
        if (query.lastQuery().isEmpty())
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "Failed to get prepared query for telemetry insert"));
        }

        // Bind parameters for telemetry_metrics INSERT
        query.bindValue(":batch_id", QString::fromStdString(batch.getBatchId()));
        query.bindValue(":device_id", QString::fromStdString(batch.getDeviceId()));
        query.bindValue(":patient_mrn", batch.getPatientMrn().empty() ? QVariant() : QString::fromStdString(batch.getPatientMrn()));
        query.bindValue(":data_created_at", static_cast<qint64>(batch.getCreatedAt()));
        query.bindValue(":batch_created_at", static_cast<qint64>(batch.getCreatedAt()));
        query.bindValue(":signed_at", batch.getSignedAt() > 0 ? static_cast<qint64>(batch.getSignedAt()) : QVariant());
        query.bindValue(":record_count", static_cast<int>(batch.getVitals().size() + batch.getAlarms().size()));
        query.bindValue(":batch_size_bytes", static_cast<qint64>(batch.getEstimatedSizeBytes()));
        query.bindValue(":status", QString("retrying")); // Initial status = retrying (pending transmission)
        query.bindValue(":retry_count", 0);
        query.bindValue(":created_at", QDateTime::currentMSecsSinceEpoch());

        if (!query.exec())
        {
            QString errorMsg = QString("Failed to save telemetry batch: %1").arg(query.lastError().text());
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
        }

        return Result<void>::ok();
    }

    std::vector<std::shared_ptr<TelemetryBatch>> SQLiteTelemetryRepository::getHistorical(
        int64_t startTimeMs, int64_t endTimeMs)
    {
        std::vector<std::shared_ptr<TelemetryBatch>> batches;

        if (!m_dbManager)
        {
            qWarning() << "SQLiteTelemetryRepository: DatabaseManager is null, returning empty vector";
            return batches;
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Telemetry::GET_HISTORICAL);
        if (query.lastQuery().isEmpty())
        {
            qWarning() << "Failed to get prepared query for telemetry historical retrieval";
            return batches;
        }

        query.bindValue(":start_time", static_cast<qint64>(startTimeMs));
        query.bindValue(":end_time", static_cast<qint64>(endTimeMs));

        if (!query.exec())
        {
            qWarning() << "Failed to execute historical telemetry query:" << query.lastError().text();
            return batches;
        }

        while (query.next())
        {
            batches.push_back(rowToTelemetryBatch(query));
        }

        return batches;
    }

    size_t SQLiteTelemetryRepository::archive(int64_t cutoffTimeMs)
    {
        if (!m_dbManager)
        {
            qWarning() << "SQLiteTelemetryRepository: DatabaseManager is null, archive failed";
            return 0;
        }

        // Use transaction for batch delete
        auto txResult = m_dbManager->beginTransaction();
        if (txResult.isError())
        {
            qWarning() << "Failed to begin transaction for archive:" << QString::fromStdString(txResult.error().message);
            return 0;
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Telemetry::ARCHIVE);
        if (query.lastQuery().isEmpty())
        {
            qWarning() << "Failed to get prepared query for telemetry archive";
            (void)m_dbManager->rollback();
            return 0;
        }

        query.bindValue(":cutoff_time", static_cast<qint64>(cutoffTimeMs));

        if (!query.exec())
        {
            qWarning() << "Failed to execute archive query:" << query.lastError().text();
            (void)m_dbManager->rollback();
            return 0;
        }

        size_t archivedCount = static_cast<size_t>(query.numRowsAffected());

        auto commitResult = m_dbManager->commit();
        if (commitResult.isError())
        {
            qWarning() << "Failed to commit archive transaction:" << QString::fromStdString(commitResult.error().message);
            (void)m_dbManager->rollback();
            return 0;
        }

        return archivedCount;
    }

    std::vector<std::shared_ptr<TelemetryBatch>> SQLiteTelemetryRepository::getUnsent()
    {
        std::vector<std::shared_ptr<TelemetryBatch>> batches;

        if (!m_dbManager)
        {
            qWarning() << "SQLiteTelemetryRepository: DatabaseManager is null, returning empty vector";
            return batches;
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Telemetry::GET_UNSENT);
        if (query.lastQuery().isEmpty())
        {
            qWarning() << "Failed to get prepared query for unsent telemetry retrieval";
            return batches;
        }

        if (!query.exec())
        {
            qWarning() << "Failed to execute unsent telemetry query:" << query.lastError().text();
            return batches;
        }

        while (query.next())
        {
            batches.push_back(rowToTelemetryBatch(query));
        }

        return batches;
    }

    Result<void> SQLiteTelemetryRepository::markAsSent(const std::string &batchId)
    {
        if (!m_dbManager)
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "DatabaseManager is null"));
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(persistence::QueryId::Telemetry::MARK_SENT);
        if (query.lastQuery().isEmpty())
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "Failed to get prepared query for mark sent"));
        }

        int64_t currentTime = QDateTime::currentMSecsSinceEpoch();

        query.bindValue(":transmitted_at", currentTime);
        query.bindValue(":server_received_at", currentTime);
        query.bindValue(":server_ack_at", currentTime);
        query.bindValue(":updated_at", currentTime);
        query.bindValue(":batch_id", QString::fromStdString(batchId));

        if (!query.exec())
        {
            QString errorMsg = QString("Failed to mark batch as sent: %1").arg(query.lastError().text());
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
        }

        if (query.numRowsAffected() == 0)
        {
            return Result<void>::error(Error::create(ErrorCode::NotFound, "Batch ID not found: " + batchId));
        }

        return Result<void>::ok();
    }

    std::shared_ptr<TelemetryBatch> SQLiteTelemetryRepository::rowToTelemetryBatch(const QSqlQuery &query) const
    {
        // Create a new TelemetryBatch from database row
        auto batch = std::make_shared<TelemetryBatch>();

        // Note: TelemetryBatch generates its own batch_id in constructor
        // We can't easily reconstruct the exact state from DB, so we create a minimal representation
        // For full reconstruction, TelemetryBatch would need additional constructors or setters

        QString deviceId = query.value(TelemetryMetricsCols::DEVICE_ID).toString();
        QString patientMrn = query.value(TelemetryMetricsCols::PATIENT_MRN).toString();

        batch->setDeviceId(deviceId.toStdString());
        if (!patientMrn.isEmpty())
        {
            batch->setPatientMrn(patientMrn.toStdString());
        }

        // Note: Cannot reconstruct vitals/alarms from telemetry_metrics table alone
        // This method is for retrieving batch metadata only
        // Full batch reconstruction would require joining with vitals/alarms tables

        return batch;
    }

} // namespace zmon
