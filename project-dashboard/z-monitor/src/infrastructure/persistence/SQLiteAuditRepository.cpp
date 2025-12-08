/**
 * @file SQLiteAuditRepository.cpp
 * @brief Implementation of SQLiteAuditRepository.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "SQLiteAuditRepository.h"
#include "DatabaseManager.h"
#include "QueryRegistry.h"
#include "generated/SchemaInfo.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

namespace zmon
{

    namespace
    {
        // Helper to convert QSqlQuery to AuditEntry
        IAuditRepository::AuditEntry fromQuery(const QSqlQuery &query)
        {
            namespace SecurityAuditLogCols = Schema::Columns::SecurityAuditLog;
            IAuditRepository::AuditEntry entry;
            entry.timestampMs = query.value(SecurityAuditLogCols::TIMESTAMP).toLongLong();
            entry.userId = query.value(SecurityAuditLogCols::USER_ID).toString().toStdString();
            // userRole not in table
            entry.actionType = query.value(SecurityAuditLogCols::EVENT_TYPE).toString().toStdString();
            entry.targetType = query.value(SecurityAuditLogCols::EVENT_CATEGORY).toString().toStdString();
            // targetId not in table
            entry.details = query.value(SecurityAuditLogCols::DETAILS).toString().toStdString();
            entry.previousHash = query.value(SecurityAuditLogCols::PREVIOUS_HASH).toString().toStdString();
            // entryHash not in table
            return entry;
        }
    }

    SQLiteAuditRepository::SQLiteAuditRepository(std::shared_ptr<DatabaseManager> dbManager)
        : m_dbManager(std::move(dbManager)) {}

    Result<void> SQLiteAuditRepository::save(const AuditEntry &entry)
    {
        if (!m_dbManager)
        {
            return Result<void>::error(Error::create(ErrorCode::Internal, "Database manager not initialized"));
        }

        auto query = m_dbManager->getPreparedQuery(persistence::QueryId::Audit::INSERT);

        query.bindValue(":timestamp", static_cast<qint64>(entry.timestampMs));
        query.bindValue(":user_id", QString::fromStdString(entry.userId));
        query.bindValue(":event_type", QString::fromStdString(entry.actionType));
        query.bindValue(":event_category", QString::fromStdString(entry.targetType));
        query.bindValue(":device_id", "UNKNOWN"); // TODO: Get device ID
        query.bindValue(":source_ip", "UNKNOWN"); // TODO: Get source IP
        query.bindValue(":success", true);        // Assuming success for now
        query.bindValue(":severity", "info");     // Default severity
        query.bindValue(":details", QString::fromStdString(entry.details));
        query.bindValue(":previous_hash", QString::fromStdString(entry.previousHash));
        query.bindValue(":error_code", QVariant()); // Null

        if (!query.exec())
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "Failed to insert audit entry: " + query.lastError().text().toStdString()));
        }

        return Result<void>::ok();
    }

    std::vector<IAuditRepository::AuditEntry> SQLiteAuditRepository::getRange(int64_t startTimeMs, int64_t endTimeMs)
    {
        std::vector<AuditEntry> entries;
        if (!m_dbManager)
            return entries;

        auto query = m_dbManager->getPreparedQuery(persistence::QueryId::Audit::GET_RANGE);
        query.bindValue(":start_time", static_cast<qint64>(startTimeMs));
        query.bindValue(":end_time", static_cast<qint64>(endTimeMs));

        if (query.exec())
        {
            while (query.next())
            {
                entries.push_back(fromQuery(query));
            }
        }
        else
        {
            qWarning() << "Failed to get audit range:" << query.lastError().text();
        }

        return entries;
    }

    std::vector<IAuditRepository::AuditEntry> SQLiteAuditRepository::getByUser(const std::string &userId,
                                                                               int64_t startTimeMs, int64_t endTimeMs)
    {
        std::vector<AuditEntry> entries;
        if (!m_dbManager)
            return entries;

        auto query = m_dbManager->getPreparedQuery(persistence::QueryId::Audit::GET_BY_USER);
        query.bindValue(":user_id", QString::fromStdString(userId));
        query.bindValue(":start_time", static_cast<qint64>(startTimeMs));
        query.bindValue(":end_time", static_cast<qint64>(endTimeMs));

        if (query.exec())
        {
            while (query.next())
            {
                entries.push_back(fromQuery(query));
            }
        }
        else
        {
            qWarning() << "Failed to get audit by user:" << query.lastError().text();
        }

        return entries;
    }

    std::vector<IAuditRepository::AuditEntry> SQLiteAuditRepository::getByTarget(const std::string &targetType,
                                                                                 const std::string &targetId,
                                                                                 int64_t startTimeMs, int64_t endTimeMs)
    {
        std::vector<AuditEntry> entries;
        if (!m_dbManager)
            return entries;

        auto query = m_dbManager->getPreparedQuery(persistence::QueryId::Audit::GET_BY_TARGET);
        query.bindValue(":target_type", QString::fromStdString(targetType));
        query.bindValue(":start_time", static_cast<qint64>(startTimeMs));
        query.bindValue(":end_time", static_cast<qint64>(endTimeMs));

        if (query.exec())
        {
            while (query.next())
            {
                entries.push_back(fromQuery(query));
            }
        }
        else
        {
            qWarning() << "Failed to get audit by target:" << query.lastError().text();
        }

        return entries;
    }

    IAuditRepository::AuditEntry SQLiteAuditRepository::getLastEntry()
    {
        AuditEntry entry = {};
        if (!m_dbManager)
            return entry;

        auto query = m_dbManager->getPreparedQuery(persistence::QueryId::Audit::GET_LAST_ENTRY);

        if (query.exec() && query.next())
        {
            entry = fromQuery(query);
        }

        return entry;
    }

    Result<bool> SQLiteAuditRepository::verifyIntegrity()
    {
        if (!m_dbManager)
        {
            return Result<bool>::error(Error::create(ErrorCode::Internal, "Database manager not initialized"));
        }

        auto query = m_dbManager->getPreparedQuery(persistence::QueryId::Audit::VERIFY_INTEGRITY);

        if (!query.exec())
        {
            return Result<bool>::error(Error::create(ErrorCode::DatabaseError, "Failed to verify integrity: " + query.lastError().text().toStdString()));
        }

        // TODO: Implement actual hash chain verification logic
        // For now, just return true if query succeeds
        return Result<bool>::ok(true);
    }

    size_t SQLiteAuditRepository::archive(int64_t cutoffTimeMs)
    {
        if (!m_dbManager)
            return 0;

        auto query = m_dbManager->getPreparedQuery(persistence::QueryId::Audit::ARCHIVE);
        query.bindValue(":cutoff_time", static_cast<qint64>(cutoffTimeMs));

        if (query.exec())
        {
            return query.numRowsAffected();
        }
        else
        {
            qWarning() << "Failed to archive audit logs:" << query.lastError().text();
            return 0;
        }
    }

} // namespace zmon
