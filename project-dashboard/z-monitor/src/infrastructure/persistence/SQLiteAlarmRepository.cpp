/**
 * @file SQLiteAlarmRepository.cpp
 * @brief Implementation of SQLite alarm repository.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "infrastructure/persistence/SQLiteAlarmRepository.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "infrastructure/persistence/generated/SchemaInfo.h"
#include "domain/common/Result.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>

namespace zmon
{

    using namespace persistence;

    // Namespace alias for convenience
    namespace AlarmsCols = Schema::Columns::Alarms;

    SQLiteAlarmRepository::SQLiteAlarmRepository(std::shared_ptr<IDatabaseManager> dbManager,
                                                 QObject *parent)
        : QObject(parent), m_dbManager(dbManager)
    {
    }

    Result<void> SQLiteAlarmRepository::save(const AlarmSnapshot &alarm)
    {
        if (!m_dbManager || !m_dbManager->isOpen())
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Alarms::INSERT);
        if (query.lastQuery().isEmpty())
        {
            qCritical() << "Failed to get prepared query for Alarms::INSERT";
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "Failed to prepare INSERT query"));
        }

        // Bind all parameters
        query.bindValue(":alarm_id", QString::fromStdString(alarm.alarmId));
        query.bindValue(":alarm_type", QString::fromStdString(alarm.alarmType));
        query.bindValue(":priority", priorityToString(alarm.priority));
        query.bindValue(":status", statusToString(alarm.status));
        query.bindValue(":raw_value", alarm.value);
        query.bindValue(":threshold_value", alarm.thresholdValue);
        query.bindValue(":start_time", static_cast<qint64>(alarm.timestampMs));
        query.bindValue(":patient_mrn", QString::fromStdString(alarm.patientMrn));

        // Optional fields (acknowledged_by, acknowledged_time)
        if (!alarm.acknowledgedBy.empty())
        {
            query.bindValue(":acknowledged_by", QString::fromStdString(alarm.acknowledgedBy));
            query.bindValue(":acknowledged_time", static_cast<qint64>(alarm.acknowledgedAtMs));
        }
        else
        {
            query.bindValue(":acknowledged_by", QVariant());
            query.bindValue(":acknowledged_time", QVariant());
        }

        if (!query.exec())
        {
            QString errorMsg = QString("Failed to save alarm: %1").arg(query.lastError().text());
            qCritical() << errorMsg;
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
        }

        return Result<void>::ok();
    }

    std::vector<AlarmSnapshot> SQLiteAlarmRepository::getActive()
    {
        std::vector<AlarmSnapshot> result;

        if (!m_dbManager || !m_dbManager->isOpen())
        {
            qWarning() << "Database not open, returning empty active alarms list";
            return result;
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Alarms::GET_ACTIVE);
        if (query.lastQuery().isEmpty())
        {
            qCritical() << "Failed to get prepared query for Alarms::GET_ACTIVE";
            return result;
        }

        if (!query.exec())
        {
            qCritical() << "Failed to execute GET_ACTIVE query:" << query.lastError().text();
            return result;
        }

        while (query.next())
        {
            result.push_back(rowToAlarmSnapshot(query));
        }

        return result;
    }

    std::vector<AlarmSnapshot> SQLiteAlarmRepository::getHistory(
        const std::string &patientMrn, int64_t startTimeMs, int64_t endTimeMs)
    {
        std::vector<AlarmSnapshot> result;

        if (!m_dbManager || !m_dbManager->isOpen())
        {
            qWarning() << "Database not open, returning empty history list";
            return result;
        }

        QSqlQuery query;

        // Use different query depending on whether patientMrn is specified
        if (patientMrn.empty())
        {
            query = m_dbManager->getPreparedQuery(QueryId::Alarms::GET_HISTORY_ALL);
            if (!query.isValid())
            {
                qCritical() << "Failed to get prepared query for Alarms::GET_HISTORY_ALL";
                return result;
            }
            query.bindValue(":start_time", static_cast<qint64>(startTimeMs));
            query.bindValue(":end_time", static_cast<qint64>(endTimeMs));
        }
        else
        {
            query = m_dbManager->getPreparedQuery(QueryId::Alarms::GET_HISTORY_BY_PATIENT);
            if (query.lastQuery().isEmpty())
            {
                qCritical() << "Failed to get prepared query for Alarms::GET_HISTORY_BY_PATIENT";
                return result;
            }
            query.bindValue(":patient_mrn", QString::fromStdString(patientMrn));
            query.bindValue(":start_time", static_cast<qint64>(startTimeMs));
            query.bindValue(":end_time", static_cast<qint64>(endTimeMs));
        }

        if (!query.exec())
        {
            qCritical() << "Failed to execute GET_HISTORY query:" << query.lastError().text();
            return result;
        }

        while (query.next())
        {
            result.push_back(rowToAlarmSnapshot(query));
        }

        return result;
    }

    AlarmSnapshot SQLiteAlarmRepository::findById(const std::string &alarmId)
    {
        if (!m_dbManager || !m_dbManager->isOpen())
        {
            qWarning() << "Database not open, returning empty alarm snapshot";
            return AlarmSnapshot();
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Alarms::FIND_BY_ID);
        if (query.lastQuery().isEmpty())
        {
            qCritical() << "Failed to get prepared query for Alarms::FIND_BY_ID";
            return AlarmSnapshot();
        }

        query.bindValue(":alarm_id", QString::fromStdString(alarmId));

        if (!query.exec())
        {
            qCritical() << "Failed to execute FIND_BY_ID query:" << query.lastError().text();
            return AlarmSnapshot();
        }

        if (query.next())
        {
            return rowToAlarmSnapshot(query);
        }

        return AlarmSnapshot(); // Not found
    }

    Result<void> SQLiteAlarmRepository::updateStatus(const std::string &alarmId,
                                                     AlarmStatus status,
                                                     const std::string &userId)
    {
        if (!m_dbManager || !m_dbManager->isOpen())
        {
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "Database is not open"));
        }

        QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Alarms::UPDATE_STATUS);
        if (query.lastQuery().isEmpty())
        {
            qCritical() << "Failed to get prepared query for Alarms::UPDATE_STATUS";
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, "Failed to prepare UPDATE_STATUS query"));
        }

        query.bindValue(":status", statusToString(status));
        query.bindValue(":alarm_id", QString::fromStdString(alarmId));

        // If status is ACKNOWLEDGED, also update acknowledged_by and acknowledged_time
        if (status == AlarmStatus::Acknowledged)
        {
            query.bindValue(":acknowledged_by", QString::fromStdString(userId));
            query.bindValue(":acknowledged_time", static_cast<qint64>(QDateTime::currentMSecsSinceEpoch()));
        }

        if (!query.exec())
        {
            QString errorMsg = QString("Failed to update alarm status: %1").arg(query.lastError().text());
            qCritical() << errorMsg;
            return Result<void>::error(Error::create(ErrorCode::DatabaseError, errorMsg.toStdString()));
        }

        return Result<void>::ok();
    }

    // Private helper methods

    AlarmSnapshot SQLiteAlarmRepository::rowToAlarmSnapshot(const QSqlQuery &query) const
    {
        QString alarmId = query.value(AlarmsCols::ALARM_ID).toString();
        QString alarmType = query.value(AlarmsCols::ALARM_TYPE).toString();
        QString priorityStr = query.value(AlarmsCols::PRIORITY).toString();
        QString statusStr = query.value(AlarmsCols::STATUS).toString();
        double rawValue = query.value(AlarmsCols::RAW_VALUE).toDouble();
        double thresholdValue = query.value(AlarmsCols::THRESHOLD_VALUE).toDouble();
        qint64 startTime = query.value(AlarmsCols::START_TIME).toLongLong();
        QString patientMrn = query.value(AlarmsCols::PATIENT_MRN).toString();
        QString acknowledgedBy = query.value(AlarmsCols::ACKNOWLEDGED_BY).toString();
        qint64 acknowledgedTime = query.value(AlarmsCols::ACKNOWLEDGED_TIME).toLongLong();

        return AlarmSnapshot(
            alarmId.toStdString(),
            alarmType.toStdString(),
            stringToPriority(priorityStr),
            stringToStatus(statusStr),
            rawValue,
            thresholdValue,
            startTime,
            patientMrn.toStdString(),
            "", // deviceId not stored in schema, use empty string
            acknowledgedBy.toStdString(),
            acknowledgedTime);
    }

    QString SQLiteAlarmRepository::statusToString(AlarmStatus status) const
    {
        switch (status)
        {
        case AlarmStatus::Active:
            return "ACTIVE";
        case AlarmStatus::Acknowledged:
            return "ACKNOWLEDGED";
        case AlarmStatus::Silenced:
            return "SILENCED";
        case AlarmStatus::Resolved:
            return "RESOLVED";
        default:
            return "ACTIVE";
        }
    }

    AlarmStatus SQLiteAlarmRepository::stringToStatus(const QString &statusStr) const
    {
        if (statusStr == "ACTIVE")
            return AlarmStatus::Active;
        if (statusStr == "ACKNOWLEDGED")
            return AlarmStatus::Acknowledged;
        if (statusStr == "SILENCED")
            return AlarmStatus::Silenced;
        if (statusStr == "RESOLVED")
            return AlarmStatus::Resolved;
        return AlarmStatus::Active; // Default
    }

    QString SQLiteAlarmRepository::priorityToString(AlarmPriority priority) const
    {
        switch (priority)
        {
        case AlarmPriority::HIGH:
            return "HIGH";
        case AlarmPriority::MEDIUM:
            return "MEDIUM";
        case AlarmPriority::LOW:
            return "LOW";
        default:
            return "LOW";
        }
    }

    AlarmPriority SQLiteAlarmRepository::stringToPriority(const QString &priorityStr) const
    {
        if (priorityStr == "CRITICAL" || priorityStr == "HIGH")
            return AlarmPriority::HIGH;
        if (priorityStr == "MEDIUM")
            return AlarmPriority::MEDIUM;
        if (priorityStr == "LOW")
            return AlarmPriority::LOW;
        return AlarmPriority::LOW; // Default
    }

} // namespace zmon
