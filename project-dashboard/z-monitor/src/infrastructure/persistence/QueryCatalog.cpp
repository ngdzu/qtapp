/**
 * @file QueryCatalog.cpp
 * @brief Implementation of QueryCatalog - registry of all SQL queries.
 *
 * This file contains all SQL query definitions using Schema:: constants
 * for table and column names. All queries must be registered here.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "QueryRegistry.h"
#include "DatabaseManager.h"
#include "generated/SchemaInfo.h"
#include <QDateTime>
#include <QDebug>

namespace zmon
{
    namespace persistence
    {

        QMap<QString, QueryCatalog::QueryDefinition> QueryCatalog::getAllQueries()
        {
            static QMap<QString, QueryDefinition> queries;

            if (queries.isEmpty())
            {
                using namespace Schema::Tables;
                namespace Patients = Schema::Columns::Patients;
                namespace AdmissionEvents = Schema::Columns::AdmissionEvents;
                namespace ActionLogCols = Schema::Columns::ActionLog;
                using namespace QueryId;

                // ═══════════════════════════════════════════════════════════
                // PATIENT QUERIES
                // ═══════════════════════════════════════════════════════════

                queries[Patient::FIND_BY_MRN] = {
                    .id = Patient::FIND_BY_MRN,
                    .sql = QString("SELECT * FROM %1 WHERE %2 = :mrn")
                               .arg(PATIENTS)
                               .arg(Patients::MRN),
                    .description = "Find patient by Medical Record Number",
                    .parameters = {":mrn"},
                    .exampleUsage = R"(
                QSqlQuery query = dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
                query.bindValue(":mrn", "MRN-12345");
                query.exec();
            )",
                    .isReadOnly = true};

                queries[Patient::FIND_ALL] = {
                    .id = Patient::FIND_ALL,
                    .sql = QString("SELECT * FROM %1 ORDER BY %2 DESC")
                               .arg(PATIENTS)
                               .arg(Patients::CREATED_AT),
                    .description = "Find all patients ordered by creation date (newest first)",
                    .parameters = {},
                    .isReadOnly = true};

                queries[Patient::CHECK_EXISTS] = {
                    .id = Patient::CHECK_EXISTS,
                    .sql = QString("SELECT COUNT(*) FROM %1 WHERE %2 = :mrn")
                               .arg(PATIENTS)
                               .arg(Patients::MRN),
                    .description = "Check if patient exists by MRN",
                    .parameters = {":mrn"},
                    .isReadOnly = true};

                queries[Patient::INSERT] = {
                    .id = Patient::INSERT,
                    .sql = QString(
                               "INSERT INTO %1 (%2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12) "
                               "VALUES (:mrn, :name, :dob, :sex, :allergies, :bedLocation, "
                               ":admissionStatus, :admittedAt, :dischargedAt, :admissionSource, :createdAt)")
                               .arg(PATIENTS)
                               .arg(Patients::MRN)
                               .arg(Patients::NAME)
                               .arg(Patients::DOB)
                               .arg(Patients::SEX)
                               .arg(Patients::ALLERGIES)
                               .arg(Patients::BED_LOCATION)
                               .arg(Patients::ADMISSION_STATUS)
                               .arg(Patients::ADMITTED_AT)
                               .arg(Patients::DISCHARGED_AT)
                               .arg(Patients::ADMISSION_SOURCE)
                               .arg(Patients::CREATED_AT),
                    .description = "Insert new patient record",
                    .parameters = {":mrn", ":name", ":dob", ":sex", ":allergies", ":bedLocation",
                                   ":admissionStatus", ":admittedAt", ":dischargedAt", ":admissionSource", ":createdAt"},
                    .isReadOnly = false};

                queries[Patient::UPDATE] = {
                    .id = Patient::UPDATE,
                    .sql = QString(
                               "UPDATE %1 SET "
                               "%2 = :name, %3 = :dob, %4 = :sex, %5 = :allergies, "
                               "%6 = :bedLocation, %7 = :admissionStatus, %8 = :admittedAt, "
                               "%9 = :dischargedAt, %10 = :admissionSource "
                               "WHERE %11 = :mrn")
                               .arg(PATIENTS)
                               .arg(Patients::NAME)
                               .arg(Patients::DOB)
                               .arg(Patients::SEX)
                               .arg(Patients::ALLERGIES)
                               .arg(Patients::BED_LOCATION)
                               .arg(Patients::ADMISSION_STATUS)
                               .arg(Patients::ADMITTED_AT)
                               .arg(Patients::DISCHARGED_AT)
                               .arg(Patients::ADMISSION_SOURCE)
                               .arg(Patients::MRN),
                    .description = "Update existing patient record",
                    .parameters = {":mrn", ":name", ":dob", ":sex", ":allergies", ":bedLocation",
                                   ":admissionStatus", ":admittedAt", ":dischargedAt", ":admissionSource"},
                    .isReadOnly = false};

                queries[Patient::DELETE] = {
                    .id = Patient::DELETE,
                    .sql = QString("DELETE FROM %1 WHERE %2 = :mrn")
                               .arg(PATIENTS)
                               .arg(Patients::MRN),
                    .description = "Delete patient by MRN",
                    .parameters = {":mrn"},
                    .isReadOnly = false};

                queries[Patient::GET_ADMISSION_HISTORY] = {
                    .id = Patient::GET_ADMISSION_HISTORY,
                    .sql = QString(
                               "SELECT %1, %2, %3 FROM %4 WHERE %5 = :mrn ORDER BY %3 DESC")
                               .arg(AdmissionEvents::EVENT_TYPE)
                               .arg(AdmissionEvents::DETAILS)
                               .arg(AdmissionEvents::TIMESTAMP)
                               .arg(ADMISSION_EVENTS)
                               .arg(AdmissionEvents::PATIENT_MRN),
                    .description = "Get admission history for patient (admission, discharge, transfer events)",
                    .parameters = {":mrn"},
                    .isReadOnly = true};

                // ═══════════════════════════════════════════════════════════
                // ACTION LOG QUERIES
                // ═══════════════════════════════════════════════════════════

                queries[ActionLog::INSERT] = {
                    .id = ActionLog::INSERT,
                    .sql = QString(R"(
                INSERT INTO %1 (
                    %2, %3, %4, %5, %6,
                    %7, %8, %9, %10, %11, %12,
                    %13, %14, %15, %16
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            )")
                               .arg(ACTION_LOG)
                               .arg(ActionLogCols::TIMESTAMP_MS)
                               .arg(ActionLogCols::TIMESTAMP_ISO)
                               .arg(ActionLogCols::USER_ID)
                               .arg(ActionLogCols::USER_ROLE)
                               .arg(ActionLogCols::ACTION_TYPE)
                               .arg(ActionLogCols::TARGET_TYPE)
                               .arg(ActionLogCols::TARGET_ID)
                               .arg(ActionLogCols::DETAILS)
                               .arg(ActionLogCols::RESULT)
                               .arg(ActionLogCols::ERROR_CODE)
                               .arg(ActionLogCols::ERROR_MESSAGE)
                               .arg(ActionLogCols::DEVICE_ID)
                               .arg(ActionLogCols::SESSION_TOKEN_HASH)
                               .arg(ActionLogCols::IP_ADDRESS)
                               .arg(ActionLogCols::PREVIOUS_HASH),
                    .description = "Insert action log entry with hash chain support",
                    .parameters = {"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?"},
                    .isReadOnly = false};

                queries[ActionLog::GET_LAST_ID] = {
                    .id = ActionLog::GET_LAST_ID,
                    .sql = QString("SELECT MAX(%1) as max_id FROM %2")
                               .arg(ActionLogCols::ID)
                               .arg(ACTION_LOG),
                    .description = "Get last action log entry ID for hash chain computation",
                    .parameters = {},
                    .isReadOnly = true};

                queries[ActionLog::GET_PREVIOUS_ENTRY] = {
                    .id = ActionLog::GET_PREVIOUS_ENTRY,
                    .sql = QString("SELECT %1, %2, %3, %4, %5, %6, %7 FROM %8 WHERE %1 = ?")
                               .arg(ActionLogCols::ID)
                               .arg(ActionLogCols::TIMESTAMP_MS)
                               .arg(ActionLogCols::ACTION_TYPE)
                               .arg(ActionLogCols::USER_ID)
                               .arg(ActionLogCols::TARGET_ID)
                               .arg(ActionLogCols::DETAILS)
                               .arg(ActionLogCols::RESULT)
                               .arg(ACTION_LOG),
                    .description = "Get previous action log entry for hash chain computation",
                    .parameters = {"?"},
                    .isReadOnly = true};

                queries[ActionLog::CREATE_TABLE] = {
                    .id = ActionLog::CREATE_TABLE,
                    .sql = QString(R"(
                CREATE TABLE IF NOT EXISTS %1 (
                    %2 INTEGER PRIMARY KEY AUTOINCREMENT,
                    %3 INTEGER NOT NULL,
                    %4 TEXT NOT NULL,
                    %5 TEXT NULL,
                    %6 TEXT NULL,
                    %7 TEXT NOT NULL,
                    %8 TEXT NULL,
                    %9 TEXT NULL,
                    %10 TEXT NULL,
                    %11 TEXT NOT NULL,
                    %12 TEXT NULL,
                    %13 TEXT NULL,
                    %14 TEXT NOT NULL,
                    %15 TEXT NULL,
                    %16 TEXT NULL,
                    %17 TEXT NULL
                )
            )")
                               .arg(ACTION_LOG)
                               .arg(ActionLogCols::ID)
                               .arg(ActionLogCols::TIMESTAMP_MS)
                               .arg(ActionLogCols::TIMESTAMP_ISO)
                               .arg(ActionLogCols::USER_ID)
                               .arg(ActionLogCols::USER_ROLE)
                               .arg(ActionLogCols::ACTION_TYPE)
                               .arg(ActionLogCols::TARGET_TYPE)
                               .arg(ActionLogCols::TARGET_ID)
                               .arg(ActionLogCols::DETAILS)
                               .arg(ActionLogCols::RESULT)
                               .arg(ActionLogCols::ERROR_CODE)
                               .arg(ActionLogCols::ERROR_MESSAGE)
                               .arg(ActionLogCols::DEVICE_ID)
                               .arg(ActionLogCols::SESSION_TOKEN_HASH)
                               .arg(ActionLogCols::IP_ADDRESS)
                               .arg(ActionLogCols::PREVIOUS_HASH),
                    .description = "Create action_log table if not exists",
                    .parameters = {},
                    .isReadOnly = false};

                queries[ActionLog::CREATE_INDEX_TIMESTAMP] = {
                    .id = ActionLog::CREATE_INDEX_TIMESTAMP,
                    .sql = QString("CREATE INDEX IF NOT EXISTS idx_action_log_timestamp ON %1(%2 DESC)")
                               .arg(ACTION_LOG)
                               .arg(ActionLogCols::TIMESTAMP_MS),
                    .description = "Create timestamp index on action_log table",
                    .parameters = {},
                    .isReadOnly = false};

                queries[ActionLog::CREATE_INDEX_USER] = {
                    .id = ActionLog::CREATE_INDEX_USER,
                    .sql = QString("CREATE INDEX IF NOT EXISTS idx_action_log_user ON %1(%2, %3 DESC)")
                               .arg(ACTION_LOG)
                               .arg(ActionLogCols::USER_ID)
                               .arg(ActionLogCols::TIMESTAMP_MS),
                    .description = "Create user index on action_log table",
                    .parameters = {},
                    .isReadOnly = false};

                queries[ActionLog::CREATE_INDEX_ACTION_TYPE] = {
                    .id = ActionLog::CREATE_INDEX_ACTION_TYPE,
                    .sql = QString("CREATE INDEX IF NOT EXISTS idx_action_log_action_type ON %1(%2, %3 DESC)")
                               .arg(ACTION_LOG)
                               .arg(ActionLogCols::ACTION_TYPE)
                               .arg(ActionLogCols::TIMESTAMP_MS),
                    .description = "Create action_type index on action_log table",
                    .parameters = {},
                    .isReadOnly = false};

                queries[ActionLog::CREATE_INDEX_TARGET] = {
                    .id = ActionLog::CREATE_INDEX_TARGET,
                    .sql = QString("CREATE INDEX IF NOT EXISTS idx_action_log_target ON %1(%2, %3, %4 DESC)")
                               .arg(ACTION_LOG)
                               .arg(ActionLogCols::TARGET_TYPE)
                               .arg(ActionLogCols::TARGET_ID)
                               .arg(ActionLogCols::TIMESTAMP_MS),
                    .description = "Create target index on action_log table",
                    .parameters = {},
                    .isReadOnly = false};

                queries[ActionLog::CREATE_INDEX_DEVICE] = {
                    .id = ActionLog::CREATE_INDEX_DEVICE,
                    .sql = QString("CREATE INDEX IF NOT EXISTS idx_action_log_device ON %1(%2, %3 DESC)")
                               .arg(ACTION_LOG)
                               .arg(ActionLogCols::DEVICE_ID)
                               .arg(ActionLogCols::TIMESTAMP_MS),
                    .description = "Create device index on action_log table",
                    .parameters = {},
                    .isReadOnly = false};
            }

            return queries;
        }

        QueryCatalog::QueryDefinition QueryCatalog::getQuery(const QString &queryId)
        {
            auto queries = getAllQueries();
            if (queries.contains(queryId))
            {
                return queries[queryId];
            }
            return QueryDefinition{}; // Empty if not found
        }

        void QueryCatalog::initializeQueries(zmon::DatabaseManager *dbManager)
        {
            if (!dbManager)
            {
                return;
            }

            auto queries = getAllQueries();

            for (const auto &def : queries)
            {
                dbManager->registerPreparedQuery(def.id, def.sql);
            }
        }

        QString QueryCatalog::generateDocumentation()
        {
            QString doc;
            doc += "# Database Query Reference\n\n";
            doc += "This document is auto-generated from QueryRegistry.h and QueryCatalog.cpp.\n\n";
            doc += "**Generated:** " + QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + "\n\n";
            doc += "---\n\n";

            auto queries = getAllQueries();

            // Group by namespace
            QMap<QString, QList<QueryDefinition>> grouped;
            for (const auto &def : queries)
            {
                QString ns = def.id.split('.').first();
                grouped[ns].append(def);
            }

            for (auto it = grouped.begin(); it != grouped.end(); ++it)
            {
                QString namespaceName = it.key();
                namespaceName[0] = namespaceName[0].toUpper(); // Capitalize first letter

                doc += QString("## %1 Queries\n\n").arg(namespaceName);

                for (const auto &def : it.value())
                {
                    doc += QString("### %1\n\n").arg(def.id);
                    doc += QString("**Description:** %1\n\n").arg(def.description);

                    if (!def.parameters.isEmpty() && def.parameters[0] != "?")
                    {
                        doc += QString("**Parameters:** %1\n\n").arg(def.parameters.join(", "));
                    }
                    else
                    {
                        doc += "**Parameters:** Positional parameters (use `addBindValue()`)\n\n";
                    }

                    doc += QString("**Read-Only:** %1\n\n").arg(def.isReadOnly ? "Yes" : "No");

                    doc += QString("**SQL:**\n```sql\n%1\n```\n\n").arg(def.sql.trimmed());

                    if (!def.exampleUsage.isEmpty())
                    {
                        doc += QString("**Example:**\n```cpp\n%1\n```\n\n").arg(def.exampleUsage.trimmed());
                    }

                    doc += "---\n\n";
                }
            }

            return doc;
        }

    } // namespace persistence
} // namespace zmon
