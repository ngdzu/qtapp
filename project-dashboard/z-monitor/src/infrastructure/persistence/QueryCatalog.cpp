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
                // ═══════════════════════════════════════════════════════════
                // VITALS QUERIES
                // ═══════════════════════════════════════════════════════════

                namespace VitalsCols = Schema::Columns::Vitals;

                queries[Vitals::INSERT] = {
                    .id = Vitals::INSERT,
                    .sql = QString(
                               "INSERT INTO %1 (%2, %3, %4, %5, %6, %7, %8, %9) "
                               "VALUES (:patient_mrn, :timestamp, :heart_rate, :spo2, :respiration_rate, :signal_quality, :source, :is_synced)")
                               .arg(VITALS)
                               .arg(VitalsCols::PATIENT_MRN)
                               .arg(VitalsCols::TIMESTAMP)
                               .arg(VitalsCols::HEART_RATE)
                               .arg(VitalsCols::SPO2)
                               .arg(VitalsCols::RESPIRATION_RATE)
                               .arg(VitalsCols::SIGNAL_QUALITY)
                               .arg(VitalsCols::SOURCE)
                               .arg(VitalsCols::IS_SYNCED),
                    .description = "Insert single vital record with NULL for unused vital type columns",
                    .parameters = {":patient_mrn", ":timestamp", ":heart_rate", ":spo2", ":respiration_rate", ":signal_quality", ":source", ":is_synced"},
                    .isReadOnly = false};

                queries[Vitals::FIND_BY_PATIENT_RANGE] = {
                    .id = Vitals::FIND_BY_PATIENT_RANGE,
                    .sql = QString(
                               "SELECT * FROM %1 "
                               "WHERE (%2 = :patient_mrn OR :patient_mrn = '%%') "
                               "AND %3 >= :start_time AND %3 <= :end_time "
                               "ORDER BY %3 ASC")
                               .arg(VITALS)
                               .arg(VitalsCols::PATIENT_MRN)
                               .arg(VitalsCols::TIMESTAMP),
                    .description = "Find vitals by patient MRN and time range (empty MRN = all patients)",
                    .parameters = {":patient_mrn", ":start_time", ":end_time"},
                    .isReadOnly = true};

                queries[Vitals::FIND_UNSENT] = {
                    .id = Vitals::FIND_UNSENT,
                    .sql = QString(
                               "SELECT * FROM %1 "
                               "WHERE %2 = 0 "
                               "ORDER BY %3 ASC")
                               .arg(VITALS)
                               .arg(VitalsCols::IS_SYNCED)
                               .arg(VitalsCols::TIMESTAMP),
                    .description = "Find all unsent vital records (is_synced = 0)",
                    .parameters = {},
                    .isReadOnly = true};

                queries[Vitals::MARK_SENT] = {
                    .id = Vitals::MARK_SENT,
                    .sql = QString(
                               "UPDATE %1 SET %2 = 1 WHERE %3 = :vital_id")
                               .arg(VITALS)
                               .arg(VitalsCols::IS_SYNCED)
                               .arg(VitalsCols::ID),
                    .description = "Mark vital record as sent (is_synced = 1)",
                    .parameters = {":vital_id"},
                    .isReadOnly = false};

                queries[Vitals::DELETE_OLDER_THAN] = {
                    .id = Vitals::DELETE_OLDER_THAN,
                    .sql = QString(
                               "DELETE FROM %1 WHERE %2 < :timestamp")
                               .arg(VITALS)
                               .arg(VitalsCols::TIMESTAMP),
                    .description = "Delete vitals older than specified timestamp (for archival)",
                    .parameters = {":timestamp"},
                    .isReadOnly = false};

                queries[Vitals::COUNT_BY_PATIENT] = {
                    .id = Vitals::COUNT_BY_PATIENT,
                    .sql = QString(
                               "SELECT COUNT(*) FROM %1 WHERE %2 = :patient_mrn")
                               .arg(VITALS)
                               .arg(VitalsCols::PATIENT_MRN),
                    .description = "Count total vitals for patient",
                    .parameters = {":patient_mrn"},
                    .isReadOnly = true};

                // ===========================================================================
                // TELEMETRY BATCH QUERIES
                // ===========================================================================

                namespace TelemetryMetricsCols = Schema::Columns::TelemetryMetrics;
                const QString TELEMETRY_METRICS = Schema::Tables::TELEMETRY_METRICS;

                queries[Telemetry::INSERT] = {
                    .id = Telemetry::INSERT,
                    .sql = QString(
                               "INSERT INTO %1 (%2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12) "
                               "VALUES (:batch_id, :device_id, :patient_mrn, :data_created_at, "
                               ":batch_created_at, :signed_at, :record_count, :batch_size_bytes, "
                               ":status, :retry_count, :created_at)")
                               .arg(TELEMETRY_METRICS)
                               .arg(TelemetryMetricsCols::BATCH_ID)
                               .arg(TelemetryMetricsCols::DEVICE_ID)
                               .arg(TelemetryMetricsCols::PATIENT_MRN)
                               .arg(TelemetryMetricsCols::DATA_CREATED_AT)
                               .arg(TelemetryMetricsCols::BATCH_CREATED_AT)
                               .arg(TelemetryMetricsCols::SIGNED_AT)
                               .arg(TelemetryMetricsCols::RECORD_COUNT)
                               .arg(TelemetryMetricsCols::BATCH_SIZE_BYTES)
                               .arg(TelemetryMetricsCols::STATUS)
                               .arg(TelemetryMetricsCols::RETRY_COUNT)
                               .arg(TelemetryMetricsCols::CREATED_AT),
                    .description = "Insert telemetry batch metadata with initial status 'retrying'",
                    .parameters = {":batch_id", ":device_id", ":patient_mrn", ":data_created_at",
                                   ":batch_created_at", ":signed_at", ":record_count", ":batch_size_bytes",
                                   ":status", ":retry_count", ":created_at"},
                    .isReadOnly = false};

                queries[Telemetry::GET_HISTORICAL] = {
                    .id = Telemetry::GET_HISTORICAL,
                    .sql = QString(
                               "SELECT * FROM %1 WHERE %2 >= :start_time AND %2 <= :end_time "
                               "ORDER BY %2 ASC")
                               .arg(TELEMETRY_METRICS)
                               .arg(TelemetryMetricsCols::BATCH_CREATED_AT),
                    .description = "Get telemetry batches within time range (for reporting/analysis)",
                    .parameters = {":start_time", ":end_time"},
                    .isReadOnly = true};

                queries[Telemetry::ARCHIVE] = {
                    .id = Telemetry::ARCHIVE,
                    .sql = QString(
                               "DELETE FROM %1 WHERE %2 < :cutoff_time AND %3 = 'success'")
                               .arg(TELEMETRY_METRICS)
                               .arg(TelemetryMetricsCols::BATCH_CREATED_AT)
                               .arg(TelemetryMetricsCols::STATUS),
                    .description = "Archive (delete) old telemetry batches with status 'success'",
                    .parameters = {":cutoff_time"},
                    .isReadOnly = false};

                queries[Telemetry::GET_UNSENT] = {
                    .id = Telemetry::GET_UNSENT,
                    .sql = QString(
                               "SELECT * FROM %1 WHERE %2 != 'success' ORDER BY %3 ASC")
                               .arg(TELEMETRY_METRICS)
                               .arg(TelemetryMetricsCols::STATUS)
                               .arg(TelemetryMetricsCols::BATCH_CREATED_AT),
                    .description = "Get unsent telemetry batches (status != 'success'), oldest first",
                    .parameters = {},
                    .isReadOnly = true};

                queries[Telemetry::MARK_SENT] = {
                    .id = Telemetry::MARK_SENT,
                    .sql = QString(
                               "UPDATE %1 SET %2 = 'success', %3 = :transmitted_at, "
                               "%4 = :server_received_at, %5 = :server_ack_at, %6 = :updated_at "
                               "WHERE %7 = :batch_id")
                               .arg(TELEMETRY_METRICS)
                               .arg(TelemetryMetricsCols::STATUS)
                               .arg(TelemetryMetricsCols::TRANSMITTED_AT)
                               .arg(TelemetryMetricsCols::SERVER_RECEIVED_AT)
                               .arg(TelemetryMetricsCols::SERVER_ACK_AT)
                               .arg(TelemetryMetricsCols::UPDATED_AT)
                               .arg(TelemetryMetricsCols::BATCH_ID),
                    .description = "Mark telemetry batch as successfully sent (status = 'success')",
                    .parameters = {":transmitted_at", ":server_received_at", ":server_ack_at",
                                   ":updated_at", ":batch_id"},
                    .isReadOnly = false};
            }

            // =====================================================================
            // Alarms Queries
            // =====================================================================
            {
                namespace AlarmsCols = Schema::Columns::Alarms;
                const QString ALARMS = Schema::Tables::ALARMS;

                queries[QueryId::Alarms::INSERT] = {
                    .id = QueryId::Alarms::INSERT,
                    .sql = QString("INSERT INTO %1 (%2, %3, %4, %5, %6, %7, %8, %9, %10, %11) "
                                   "VALUES (:alarm_id, :alarm_type, :priority, :status, "
                                   ":raw_value, :threshold_value, :start_time, :patient_mrn, "
                                   ":acknowledged_by, :acknowledged_time)")
                               .arg(ALARMS)
                               .arg(AlarmsCols::ALARM_ID)
                               .arg(AlarmsCols::ALARM_TYPE)
                               .arg(AlarmsCols::PRIORITY)
                               .arg(AlarmsCols::STATUS)
                               .arg(AlarmsCols::RAW_VALUE)
                               .arg(AlarmsCols::THRESHOLD_VALUE)
                               .arg(AlarmsCols::START_TIME)
                               .arg(AlarmsCols::PATIENT_MRN)
                               .arg(AlarmsCols::ACKNOWLEDGED_BY)
                               .arg(AlarmsCols::ACKNOWLEDGED_TIME),
                    .description = "Insert alarm event with all metadata",
                    .parameters = {":alarm_id", ":alarm_type", ":priority", ":status",
                                   ":raw_value", ":threshold_value", ":start_time", ":patient_mrn",
                                   ":acknowledged_by", ":acknowledged_time"},
                    .isReadOnly = false};

                queries[QueryId::Alarms::GET_ACTIVE] = {
                    .id = QueryId::Alarms::GET_ACTIVE,
                    .sql = QString("SELECT * FROM %1 WHERE %2 = 'ACTIVE' "
                                   "ORDER BY CASE %3 "
                                   "WHEN 'CRITICAL' THEN 1 "
                                   "WHEN 'HIGH' THEN 2 "
                                   "WHEN 'MEDIUM' THEN 3 "
                                   "WHEN 'LOW' THEN 4 "
                                   "END, %4 DESC")
                               .arg(ALARMS)
                               .arg(AlarmsCols::STATUS)
                               .arg(AlarmsCols::PRIORITY)
                               .arg(AlarmsCols::START_TIME),
                    .description = "Get all active alarms ordered by priority and start time",
                    .parameters = {},
                    .isReadOnly = true};

                queries[QueryId::Alarms::GET_HISTORY_BY_PATIENT] = {
                    .id = QueryId::Alarms::GET_HISTORY_BY_PATIENT,
                    .sql = QString("SELECT * FROM %1 WHERE %2 = :patient_mrn "
                                   "AND %3 BETWEEN :start_time AND :end_time "
                                   "ORDER BY %3 DESC")
                               .arg(ALARMS)
                               .arg(AlarmsCols::PATIENT_MRN)
                               .arg(AlarmsCols::START_TIME),
                    .description = "Get alarm history for specific patient in time range",
                    .parameters = {":patient_mrn", ":start_time", ":end_time"},
                    .isReadOnly = true};

                queries[QueryId::Alarms::GET_HISTORY_ALL] = {
                    .id = QueryId::Alarms::GET_HISTORY_ALL,
                    .sql = QString("SELECT * FROM %1 WHERE %2 BETWEEN :start_time AND :end_time "
                                   "ORDER BY %2 DESC")
                               .arg(ALARMS)
                               .arg(AlarmsCols::START_TIME),
                    .description = "Get alarm history for all patients in time range",
                    .parameters = {":start_time", ":end_time"},
                    .isReadOnly = true};

                queries[QueryId::Alarms::FIND_BY_ID] = {
                    .id = QueryId::Alarms::FIND_BY_ID,
                    .sql = QString("SELECT * FROM %1 WHERE %2 = :alarm_id")
                               .arg(ALARMS)
                               .arg(AlarmsCols::ALARM_ID),
                    .description = "Find alarm by alarm ID (primary key lookup)",
                    .parameters = {":alarm_id"},
                    .isReadOnly = true};

                queries[QueryId::Alarms::UPDATE_STATUS] = {
                    .id = QueryId::Alarms::UPDATE_STATUS,
                    .sql = QString("UPDATE %1 SET %2 = :status, "
                                   "%3 = :acknowledged_by, %4 = :acknowledged_time "
                                   "WHERE %5 = :alarm_id")
                               .arg(ALARMS)
                               .arg(AlarmsCols::STATUS)
                               .arg(AlarmsCols::ACKNOWLEDGED_BY)
                               .arg(AlarmsCols::ACKNOWLEDGED_TIME)
                               .arg(AlarmsCols::ALARM_ID),
                    .description = "Update alarm status and acknowledgment info",
                    .parameters = {":status", ":acknowledged_by", ":acknowledged_time", ":alarm_id"},
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
                qCritical() << "QueryCatalog::initializeQueries - dbManager is null!";
                return;
            }

            auto queries = getAllQueries();
            qInfo() << "QueryCatalog::initializeQueries - Registering" << queries.size() << "queries";

            int successCount = 0;
            for (const auto &def : queries)
            {
                auto result = dbManager->registerPreparedQuery(def.id, def.sql);
                if (result.isOk())
                {
                    successCount++;
                }
                else
                {
                    qWarning() << "Failed to register query:" << def.id << "-" << QString::fromStdString(result.error().message);
                }
            }

            qInfo() << "QueryCatalog::initializeQueries - Successfully registered" << successCount << "of" << queries.size() << "queries";
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
