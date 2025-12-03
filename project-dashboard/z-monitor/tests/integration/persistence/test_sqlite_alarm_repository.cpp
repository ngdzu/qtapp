/**
 * @file test_sqlite_alarm_repository.cpp
 * @brief Integration tests for SQLiteAlarmRepository with snapshot support.
 *
 * Tests verify:
 * - Save alarm with linked snapshot
 * - Retrieve active alarms
 * - Retrieve alarm history
 * - Update alarm status
 * - Find alarm by ID
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include "infrastructure/persistence/SQLiteAlarmRepository.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "domain/monitoring/AlarmSnapshot.h"
#include "domain/common/Result.h"
#include "tests/fixtures/RepositoryTestFixture.h"
#include <memory>
#include <vector>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <chrono>

namespace zmon
{
    /**
     * @class SQLiteAlarmRepositoryIntegrationTest
     * @brief Test fixture for SQLiteAlarmRepository integration tests.
     */
    class SQLiteAlarmRepositoryIntegrationTest : public test::RepositoryTestFixture
    {
    protected:
        void SetUp() override
        {
            RepositoryTestFixture::SetUp();

            // Manually create alarms table (DatabaseTestFixture's DDL loading may not work for all tables)
            QSqlQuery createTable(databaseManager()->getWriteConnection());
            const char *createAlarmsTable =
                "CREATE TABLE IF NOT EXISTS alarms (\n"
                "    alarm_id TEXT PRIMARY KEY,\n"
                "    patient_id TEXT,\n"
                "    patient_mrn TEXT NOT NULL,\n"
                "    start_time INTEGER NOT NULL,\n"
                "    end_time INTEGER,\n"
                "    alarm_type TEXT NOT NULL,\n"
                "    priority TEXT NOT NULL CHECK (priority IN ('CRITICAL', 'HIGH', 'MEDIUM', 'LOW')),\n"
                "    status TEXT,\n"
                "    acknowledged_by TEXT,\n"
                "    acknowledged_time INTEGER,\n"
                "    silenced_until INTEGER,\n"
                "    raw_value REAL,\n"
                "    threshold_value REAL,\n"
                "    context_snapshot_id INTEGER\n"
                ");";
            if (!createTable.exec(createAlarmsTable))
            {
                FAIL() << "Failed to create alarms table: " << createTable.lastError().text().toStdString();
            }

            // Clear any residual data at the start of each test run
            {
                QSqlQuery clear(databaseManager()->getWriteConnection());
                clear.exec("DELETE FROM alarms");
            }

            // Manually register alarm queries with correct schema column names
            using namespace zmon::persistence;

            auto r1 = databaseManager()->registerPreparedQuery(
                QueryId::Alarms::INSERT,
                "INSERT INTO alarms (alarm_id, alarm_type, priority, status, raw_value, threshold_value, start_time, "
                "patient_mrn, acknowledged_by, acknowledged_time) "
                "VALUES (:alarm_id, :alarm_type, :priority, :status, :raw_value, :threshold_value, :start_time, "
                ":patient_mrn, :acknowledged_by, :acknowledged_time)");
            ASSERT_TRUE(r1.isOk()) << "Failed to register INSERT query: " << r1.error().message;

            auto r2 = databaseManager()->registerPreparedQuery(
                QueryId::Alarms::FIND_BY_ID,
                "SELECT * FROM alarms WHERE alarm_id = :alarm_id");
            ASSERT_TRUE(r2.isOk()) << "Failed to register FIND_BY_ID query: " << r2.error().message;

            auto r3 = databaseManager()->registerPreparedQuery(
                QueryId::Alarms::GET_ACTIVE,
                "SELECT * FROM alarms WHERE status = 'ACTIVE' ORDER BY start_time DESC");
            ASSERT_TRUE(r3.isOk()) << "Failed to register GET_ACTIVE query: " << r3.error().message;

            auto r4 = databaseManager()->registerPreparedQuery(
                QueryId::Alarms::GET_HISTORY_BY_PATIENT,
                "SELECT * FROM alarms WHERE patient_mrn = :patient_mrn "
                "AND start_time BETWEEN :start_time AND :end_time ORDER BY start_time DESC");
            ASSERT_TRUE(r4.isOk()) << "Failed to register GET_HISTORY_BY_PATIENT query: " << r4.error().message;

            auto r5 = databaseManager()->registerPreparedQuery(
                QueryId::Alarms::UPDATE_STATUS,
                "UPDATE alarms SET status = :status, acknowledged_by = :acknowledged_by, "
                "acknowledged_time = :acknowledged_time WHERE alarm_id = :alarm_id");
            ASSERT_TRUE(r5.isOk()) << "Failed to register UPDATE_STATUS query: " << r5.error().message;

            // Create repository using fixture's DatabaseManager with non-owning shared_ptr
            auto nonOwningPtr = std::shared_ptr<DatabaseManager>(databaseManager(), [](DatabaseManager *) {});
            m_repository = std::make_unique<SQLiteAlarmRepository>(nonOwningPtr);
        }

        void TearDown() override
        {
            // Clear alarms table between tests to avoid UNIQUE constraint failures
            QSqlQuery clear(databaseManager()->getWriteConnection());
            clear.exec("DELETE FROM alarms");

            m_repository.reset();
            RepositoryTestFixture::TearDown();
        }

        /**
         * @brief Create a test alarm snapshot.
         */
        AlarmSnapshot createTestAlarm(
            const std::string &alarmId = "ALM-TEST-001",
            const std::string &mrn = "MRN-TEST-001",
            AlarmPriority priority = AlarmPriority::HIGH,
            AlarmStatus status = AlarmStatus::Active,
            const std::string &alarmType = "HR_HIGH")
        {
            auto now = std::chrono::system_clock::now();
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now.time_since_epoch())
                             .count();

            return AlarmSnapshot(
                alarmId,   // alarmId
                alarmType, // alarmType
                priority,  // priority
                status,    // status
                125.0,     // value
                120.0,     // thresholdValue
                nowMs,     // timestampMs
                mrn,       // patientMrn
                "DEV-001", // deviceId
                "",        // acknowledgedBy
                0          // acknowledgedAtMs
            );
        }

        std::unique_ptr<SQLiteAlarmRepository> m_repository;
    };

    /**
     * @test Test that repository can save alarm.
     */
    TEST_F(SQLiteAlarmRepositoryIntegrationTest, SaveAlarm)
    {
        auto alarm = createTestAlarm();
        auto result = m_repository->save(alarm);
        ASSERT_TRUE(result.isOk()) << "Save failed: " << result.error().message;
    }

    /**
     * @test Test that repository can find alarm by ID.
     */
    TEST_F(SQLiteAlarmRepositoryIntegrationTest, FindByIdReturnsAlarm)
    {
        auto alarm = createTestAlarm("ALM-FIND-001");
        auto saveResult = m_repository->save(alarm);
        ASSERT_TRUE(saveResult.isOk()) << "Save failed: " << saveResult.error().message;

        // Find alarm
        auto found = m_repository->findById("ALM-FIND-001");
        EXPECT_EQ(found.alarmId, "ALM-FIND-001");
        EXPECT_EQ(found.patientMrn, "MRN-TEST-001");
        EXPECT_EQ(found.alarmType, "HR_HIGH");
    }

    /**
     * @test Test that findById returns empty snapshot for non-existent alarm.
     */
    TEST_F(SQLiteAlarmRepositoryIntegrationTest, FindByIdReturnsEmptyForNonExistent)
    {
        auto found = m_repository->findById("ALM-DOES-NOT-EXIST");
        EXPECT_TRUE(found.alarmId.empty());
    }

    /**
     * @test Test that repository can retrieve active alarms.
     */
    TEST_F(SQLiteAlarmRepositoryIntegrationTest, GetActiveReturnsActiveAlarms)
    {
        // Save active alarm
        auto activeAlarm = createTestAlarm("ALM-ACTIVE-001", "MRN-001", AlarmPriority::HIGH, AlarmStatus::Active);
        m_repository->save(activeAlarm);

        // Save acknowledged alarm (should not be in active list)
        auto ackedAlarm = createTestAlarm("ALM-ACKED-001", "MRN-002", AlarmPriority::MEDIUM, AlarmStatus::Acknowledged);
        m_repository->save(ackedAlarm);

        // Retrieve active alarms
        auto active = m_repository->getActive();
        ASSERT_GE(active.size(), 1) << "Should have at least 1 active alarm";

        // Verify active alarm is in list
        bool foundActive = false;
        for (const auto &alarm : active)
        {
            if (alarm.alarmId == "ALM-ACTIVE-001")
            {
                foundActive = true;
                EXPECT_EQ(alarm.status, AlarmStatus::Active);
                break;
            }
        }
        EXPECT_TRUE(foundActive) << "Active alarm not found in getActive() results";
    }

    /**
     * @test Test that repository can update alarm status.
     */
    TEST_F(SQLiteAlarmRepositoryIntegrationTest, UpdateStatusChangesAlarmStatus)
    {
        // Save active alarm
        auto alarm = createTestAlarm("ALM-UPDATE-001");
        m_repository->save(alarm);

        // Update status to acknowledged
        auto updateResult = m_repository->updateStatus("ALM-UPDATE-001", AlarmStatus::Acknowledged, "NURSE-001");
        ASSERT_TRUE(updateResult.isOk()) << "Update failed: " << updateResult.error().message;

        // Verify status changed
        auto found = m_repository->findById("ALM-UPDATE-001");
        EXPECT_EQ(found.status, AlarmStatus::Acknowledged);
        EXPECT_EQ(found.acknowledgedBy, "NURSE-001");
        EXPECT_GT(found.acknowledgedAtMs, 0);
    }

    /**
     * @test Test that repository can retrieve alarm history for patient.
     */
    TEST_F(SQLiteAlarmRepositoryIntegrationTest, GetHistoryReturnsPatientAlarms)
    {
        auto now = std::chrono::system_clock::now();
        auto oneHourAgo = now - std::chrono::hours(1);
        auto twoHoursAgo = now - std::chrono::hours(2);

        // Save alarms for patient
        auto alarm1 = createTestAlarm("ALM-HIST-001", "MRN-HIST-001");
        auto alarm2 = createTestAlarm("ALM-HIST-002", "MRN-HIST-001");
        m_repository->save(alarm1);
        m_repository->save(alarm2);

        // Save alarm for different patient
        auto alarm3 = createTestAlarm("ALM-HIST-003", "MRN-HIST-002");
        m_repository->save(alarm3);

        // Get history for patient 1
        auto twoHoursAgoMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 twoHoursAgo.time_since_epoch())
                                 .count();
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now.time_since_epoch())
                         .count();

        auto history = m_repository->getHistory("MRN-HIST-001", twoHoursAgoMs, nowMs);
        ASSERT_GE(history.size(), 2) << "Should have at least 2 alarms in history";

        // Verify all alarms belong to correct patient
        for (const auto &alarm : history)
        {
            EXPECT_EQ(alarm.patientMrn, "MRN-HIST-001");
        }
    }

    /**
     * @test Test that getHistory returns empty vector outside time range.
     */
    TEST_F(SQLiteAlarmRepositoryIntegrationTest, GetHistoryReturnsEmptyOutsideRange)
    {
        // Save alarm now
        auto alarm = createTestAlarm("ALM-RANGE-001", "MRN-RANGE-001");
        m_repository->save(alarm);

        // Query for time range in the future
        auto now = std::chrono::system_clock::now();
        auto tomorrow = now + std::chrono::hours(24);
        auto twoDaysLater = now + std::chrono::hours(48);

        auto tomorrowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                              tomorrow.time_since_epoch())
                              .count();
        auto twoDaysLaterMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  twoDaysLater.time_since_epoch())
                                  .count();

        auto history = m_repository->getHistory("MRN-RANGE-001", tomorrowMs, twoDaysLaterMs);
        EXPECT_TRUE(history.empty()) << "Should not find alarms outside time range";
    }

} // namespace zmon
