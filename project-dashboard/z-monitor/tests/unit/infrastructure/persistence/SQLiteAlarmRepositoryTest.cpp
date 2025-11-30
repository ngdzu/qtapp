/**
 * @file SQLiteAlarmRepositoryTest.cpp
 * @brief Unit tests for SQLiteAlarmRepository
 *
 * NOTE: These are UNIT tests using a mock DatabaseManager.
 * They test the repository logic without actual database I/O.
 * For integration tests with real database, create a separate integration test file.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "infrastructure/persistence/SQLiteAlarmRepository.h"
#include "MockDatabaseManager.h"
#include "domain/monitoring/AlarmSnapshot.h"
#include <chrono>

using namespace zmon;
using namespace zmon::testing;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

/**
 * @brief Test fixture for SQLiteAlarmRepository unit tests
 *
 * Uses MockDatabaseManager to test repository logic without database I/O.
 * This ensures fast, isolated unit tests focused on business logic.
 */
class SQLiteAlarmRepositoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create mock database manager
        mockDbManager = std::make_shared<NiceMock<MockDatabaseManager>>();

        // Setup default mock behavior - database is open
        ON_CALL(*mockDbManager, isOpen())
            .WillByDefault(Return(true));

        // Create repository with mock
        repository = std::make_unique<SQLiteAlarmRepository>(mockDbManager);
    }

    void TearDown() override
    {
        repository.reset();
        mockDbManager.reset();
    }

    std::shared_ptr<NiceMock<MockDatabaseManager>> mockDbManager;
    std::unique_ptr<SQLiteAlarmRepository> repository;
};

/**
 * @brief Test that save() fails when database is not open
 */
TEST_F(SQLiteAlarmRepositoryTest, SaveFailsWhenDatabaseNotOpen)
{
    // Setup: database is closed
    ON_CALL(*mockDbManager, isOpen())
        .WillByDefault(Return(false));

    // Create test alarm
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    AlarmSnapshot alarm{
        "ALM001",            // alarmId
        "HR_HIGH",           // alarmType
        AlarmPriority::HIGH, // priority
        AlarmStatus::Active, // status
        120.0,               // value
        100.0,               // thresholdValue
        nowMs,               // timestampMs
        "P001",              // patientMrn
        "DEV001",            // deviceId
        "",                  // acknowledgedBy (empty)
        0                    // acknowledgedAtMs (0)
    };

    // Execute
    auto result = repository->save(alarm);

    // Verify: should fail with database error
    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code, ErrorCode::DatabaseError);
}

/**
 * @brief Test that getActive() returns empty vector when database is closed
 */
TEST_F(SQLiteAlarmRepositoryTest, GetActiveReturnsEmptyWhenDatabaseClosed)
{
    // Setup: database is closed
    ON_CALL(*mockDbManager, isOpen())
        .WillByDefault(Return(false));

    // Execute
    auto activeAlarms = repository->getActive();

    // Verify: should return empty vector (graceful degradation)
    EXPECT_TRUE(activeAlarms.empty());
}

/**
 * @brief Test that findById() returns empty snapshot when database is closed
 */
TEST_F(SQLiteAlarmRepositoryTest, FindByIdReturnsEmptyWhenDatabaseClosed)
{
    // Setup: database is closed
    ON_CALL(*mockDbManager, isOpen())
        .WillByDefault(Return(false));

    // Execute
    auto result = repository->findById("ALM999");

    // Verify: should return empty snapshot
    EXPECT_TRUE(result.alarmId.empty());
}

/**
 * @brief Test that updateStatus() fails when database is not open
 */
TEST_F(SQLiteAlarmRepositoryTest, UpdateStatusFailsWhenDatabaseNotOpen)
{
    // Setup: database is closed
    ON_CALL(*mockDbManager, isOpen())
        .WillByDefault(Return(false));

    // Execute
    auto result = repository->updateStatus("ALM001", AlarmStatus::Acknowledged, "USER123");

    // Verify: should fail with database error
    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code, ErrorCode::DatabaseError);
}

/**
 * @brief Test that getHistory() returns empty vector when database is closed
 */
TEST_F(SQLiteAlarmRepositoryTest, GetHistoryReturnsEmptyWhenDatabaseClosed)
{
    // Setup: database is closed
    ON_CALL(*mockDbManager, isOpen())
        .WillByDefault(Return(false));

    auto now = std::chrono::system_clock::now();
    auto oneHourAgo = now - std::chrono::hours(1);

    auto startMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                       oneHourAgo.time_since_epoch())
                       .count();
    auto endMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch())
                     .count();

    // Execute
    auto history = repository->getHistory("P001", startMs, endMs);

    // Verify: should return empty vector
    EXPECT_TRUE(history.empty());
}

// Main function for Google Test
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
