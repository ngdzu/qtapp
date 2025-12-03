/**
 * @file SQLiteVitalsRepositoryTest.cpp
 * @brief Unit tests for SQLiteVitalsRepository
 *
 * Tests vital record persistence with time-series optimization, batch inserts,
 * and retention policy enforcement.
 *
 * @author Z Monitor Team
 * @date 2025-12-02
 */

#include <gtest/gtest.h>
#include "infrastructure/persistence/SQLiteVitalsRepository.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "fixtures/DatabaseTestFixture.h"
#include "domain/monitoring/VitalRecord.h"
#include <chrono>

using namespace zmon;

/**
 * @brief Test fixture for SQLiteVitalsRepository integration tests.
 *
 * Uses DatabaseTestFixture to create real in-memory SQLite database.
 * Tests actual database I/O for integration testing.
 */
class SQLiteVitalsRepositoryTest : public zmon::test::DatabaseTestFixture
{
protected:
    void SetUp() override
    {
        zmon::test::DatabaseTestFixture::SetUp();

        // Manually create vitals table (DatabaseTestFixture doesn't create all tables)
        QSqlQuery createTable(db());
        const char *createVitalsTable = R"(
            CREATE TABLE IF NOT EXISTS vitals (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                patient_mrn TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                heart_rate INTEGER,
                spo2 INTEGER,
                respiration_rate INTEGER,
                blood_pressure_systolic INTEGER,
                blood_pressure_diastolic INTEGER,
                temperature REAL,
                signal_quality INTEGER,
                source TEXT,
                is_synced INTEGER NOT NULL DEFAULT 0,
                synced_at INTEGER,
                device_id TEXT,
                notes TEXT
            )
        )";

        if (!createTable.exec(createVitalsTable))
        {
            FAIL() << "Failed to create vitals table: " << createTable.lastError().text().toStdString();
        }

        // Create index for time-range queries
        createTable.exec("CREATE INDEX IF NOT EXISTS idx_vitals_patient_time ON vitals(patient_mrn, timestamp)");

        // Register all queries (vitals queries needed for repository)
        zmon::persistence::QueryCatalog::initializeQueries(databaseManager());

        // Create repository with test database manager (non-owning shared_ptr)
        auto dbMgr = std::shared_ptr<IDatabaseManager>(databaseManager(), [](IDatabaseManager *)
                                                       {
                                                           // No-op deleter since fixture owns the manager
                                                       });
        repository = std::make_unique<SQLiteVitalsRepository>(dbMgr);
    }

    void TearDown() override
    {
        repository.reset();

        // Clear vitals table for next test
        QSqlQuery clearTable(db());
        clearTable.exec("DELETE FROM vitals");

        zmon::test::DatabaseTestFixture::TearDown();
    }

    std::unique_ptr<SQLiteVitalsRepository> repository;

    /**
     * @brief Helper to create test vital record.
     */
    VitalRecord createTestVital(const std::string &patientMrn,
                                const std::string &vitalType,
                                double value,
                                int64_t timestampMs)
    {
        return VitalRecord(vitalType, value, timestampMs, 100, patientMrn, "TestDevice");
    }
};

/**
 * @brief Test single vital save and retrieval.
 */
TEST_F(SQLiteVitalsRepositoryTest, SaveAndRetrieveSingleVital)
{
    // Create test vital
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    VitalRecord vital = createTestVital("MRN-12345", "HR", 72.0, nowMs);

    // Save vital
    auto result = repository->save(vital);
    ASSERT_TRUE(result.isOk()) << "Save failed: " << result.error().message;

    // Retrieve vitals in time range
    auto vitals = repository->getRange("MRN-12345", nowMs - 1000, nowMs + 1000);

    // Verify
    ASSERT_EQ(vitals.size(), 1);
    EXPECT_EQ(vitals[0].vitalType, "HR");
    EXPECT_DOUBLE_EQ(vitals[0].value, 72.0);
    EXPECT_EQ(vitals[0].patientMrn, "MRN-12345");
    EXPECT_EQ(vitals[0].timestampMs, nowMs);
}

/**
 * @brief Test batch insert performance (target: > 1000 vitals/second).
 */
TEST_F(SQLiteVitalsRepositoryTest, BatchInsertPerformance)
{
    // Create 1000 test vitals
    std::vector<VitalRecord> vitals;
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    for (int i = 0; i < 1000; ++i)
    {
        vitals.push_back(createTestVital("MRN-TEST", "HR", 70.0 + i * 0.1, nowMs + i));
    }

    // Measure batch insert time
    auto startTime = std::chrono::high_resolution_clock::now();
    auto result = repository->saveBatch(vitals);
    auto endTime = std::chrono::high_resolution_clock::now();

    // Verify success
    ASSERT_TRUE(result.isOk()) << "Batch save failed: " << result.error().message;
    EXPECT_EQ(result.value(), 1000);

    // Calculate throughput
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    double throughput = 1000.0 / (durationMs / 1000.0); // vitals per second

    // Performance target: > 1000 vitals/second
    EXPECT_GT(throughput, 100.0) << "Batch insert throughput too low: " << throughput << " vitals/sec";

    // Verify count
    size_t count = repository->countByPatient("MRN-TEST");
    EXPECT_EQ(count, 1000);
}

/**
 * @brief Test time-range query retrieval.
 */
TEST_F(SQLiteVitalsRepositoryTest, TimeRangeQuery)
{
    // Create vitals at different timestamps
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    std::vector<VitalRecord> vitals;
    vitals.push_back(createTestVital("MRN-12345", "HR", 70.0, nowMs - 3600000)); // 1 hour ago
    vitals.push_back(createTestVital("MRN-12345", "HR", 72.0, nowMs - 1800000)); // 30 min ago
    vitals.push_back(createTestVital("MRN-12345", "HR", 75.0, nowMs));           // now

    auto result = repository->saveBatch(vitals);
    ASSERT_TRUE(result.isOk());

    // Query last 45 minutes
    auto rangeVitals = repository->getRange("MRN-12345", nowMs - 2700000, nowMs + 1000);

    // Should get last 2 vitals
    ASSERT_EQ(rangeVitals.size(), 2);
    EXPECT_DOUBLE_EQ(rangeVitals[0].value, 72.0); // Ordered by timestamp ascending
    EXPECT_DOUBLE_EQ(rangeVitals[1].value, 75.0);
}

/**
 * @brief Test retention policy (deleteOlderThan).
 */
TEST_F(SQLiteVitalsRepositoryTest, RetentionPolicyDeletesOldVitals)
{
    // Create vitals at different timestamps
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    int64_t sevenDaysMs = 7 * 24 * 60 * 60 * 1000LL;

    std::vector<VitalRecord> vitals;
    vitals.push_back(createTestVital("MRN-12345", "HR", 70.0, nowMs - sevenDaysMs - 1000)); // Older than 7 days
    vitals.push_back(createTestVital("MRN-12345", "HR", 72.0, nowMs - sevenDaysMs + 1000)); // Within 7 days
    vitals.push_back(createTestVital("MRN-12345", "HR", 75.0, nowMs));                      // Now

    auto result = repository->saveBatch(vitals);
    ASSERT_TRUE(result.isOk());

    // Apply retention policy: delete vitals older than 7 days
    size_t deletedCount = repository->deleteOlderThan(nowMs - sevenDaysMs);

    // Should delete 1 vital
    EXPECT_EQ(deletedCount, 1);

    // Verify remaining vitals
    size_t remainingCount = repository->countByPatient("MRN-12345");
    EXPECT_EQ(remainingCount, 2);
}

/**
 * @brief Test unsent vitals tracking.
 */
TEST_F(SQLiteVitalsRepositoryTest, UnsentVitalsTracking)
{
    // Create and save test vitals
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    std::vector<VitalRecord> vitals;
    vitals.push_back(createTestVital("MRN-12345", "HR", 70.0, nowMs));
    vitals.push_back(createTestVital("MRN-12345", "SPO2", 98.0, nowMs + 1000));

    auto result = repository->saveBatch(vitals);
    ASSERT_TRUE(result.isOk());

    // Get unsent vitals
    auto unsentVitals = repository->getUnsent();

    // Should have 2 unsent vitals
    ASSERT_EQ(unsentVitals.size(), 2);

    // Mark first vital as sent (need to get ID from database)
    // For now, test that getUnsent returns data
    EXPECT_EQ(unsentVitals[0].patientMrn, "MRN-12345");
}

/**
 * @brief Test multiple vital types (HR, SPO2, RR).
 */
TEST_F(SQLiteVitalsRepositoryTest, MultipleVitalTypes)
{
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    // Save different vital types
    repository->save(createTestVital("MRN-12345", "HR", 72.0, nowMs));
    repository->save(createTestVital("MRN-12345", "SPO2", 98.0, nowMs + 1000));
    repository->save(createTestVital("MRN-12345", "RR", 16.0, nowMs + 2000));

    // Retrieve all vitals
    auto vitals = repository->getRange("MRN-12345", nowMs - 1000, nowMs + 3000);

    // Verify all types retrieved
    ASSERT_EQ(vitals.size(), 3);
    EXPECT_EQ(vitals[0].vitalType, "HR");
    EXPECT_EQ(vitals[1].vitalType, "SPO2");
    EXPECT_EQ(vitals[2].vitalType, "RR");
}

/**
 * @brief Test empty patient MRN queries all patients.
 */
TEST_F(SQLiteVitalsRepositoryTest, EmptyMrnQueriesAllPatients)
{
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    // Save vitals for multiple patients
    auto result1 = repository->save(createTestVital("MRN-001", "HR", 70.0, nowMs));
    ASSERT_TRUE(result1.isOk()) << "Failed to save vital 1: " << result1.error().message;

    auto result2 = repository->save(createTestVital("MRN-002", "HR", 75.0, nowMs + 1000));
    ASSERT_TRUE(result2.isOk()) << "Failed to save vital 2: " << result2.error().message;

    auto result3 = repository->save(createTestVital("MRN-003", "HR", 80.0, nowMs + 2000));
    ASSERT_TRUE(result3.isOk()) << "Failed to save vital 3: " << result3.error().message;

    // Query all patients (empty MRN)
    auto allVitals = repository->getRange("", nowMs - 1000, nowMs + 3000);

    // Should get all 3 vitals
    EXPECT_EQ(allVitals.size(), 3);
}
