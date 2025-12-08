/**
 * @file SQLiteAuditRepositoryTest.cpp
 * @brief Unit tests for SQLiteAuditRepository.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "infrastructure/persistence/SQLiteAuditRepository.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include "infrastructure/persistence/QueryRegistry.h"
#include "tests/fixtures/RepositoryTestFixture.h"

using namespace zmon;
using namespace zmon::persistence;
using namespace zmon::test;

class SQLiteAuditRepositoryTest : public RepositoryTestFixture
{
protected:
    void SetUp() override
    {
        RepositoryTestFixture::SetUp();

        // Register queries
        zmon::persistence::QueryCatalog::initializeQueries(databaseManager());

        // Create a shared_ptr that doesn't delete the pointer, as ownership remains with the fixture
        std::shared_ptr<DatabaseManager> sharedDbManager(databaseManager(), [](DatabaseManager *) {});
        repository = std::make_unique<SQLiteAuditRepository>(sharedDbManager);
    }

    std::unique_ptr<SQLiteAuditRepository> repository;
};

TEST_F(SQLiteAuditRepositoryTest, SaveAndRetrieveEntry)
{
    IAuditRepository::AuditEntry entry;
    entry.timestampMs = 1000;
    entry.userId = "user1";
    entry.actionType = "LOGIN";
    entry.targetType = "AUTHENTICATION";
    entry.details = "details";
    entry.previousHash = "prev_hash";

    auto result = repository->save(entry);
    ASSERT_TRUE(result.isOk());

    auto entries = repository->getRange(0, 2000);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].userId, "user1");
    EXPECT_EQ(entries[0].actionType, "LOGIN");
}

TEST_F(SQLiteAuditRepositoryTest, GetByUser)
{
    IAuditRepository::AuditEntry entry1;
    entry1.timestampMs = 1000;
    entry1.userId = "user1";
    entry1.actionType = "LOGIN";
    entry1.targetType = "AUTHENTICATION";
    repository->save(entry1);

    IAuditRepository::AuditEntry entry2;
    entry2.timestampMs = 2000;
    entry2.userId = "user2";
    entry2.actionType = "LOGIN";
    entry2.targetType = "AUTHENTICATION";
    repository->save(entry2);

    auto entries = repository->getByUser("user1", 0, 3000);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].userId, "user1");
}

TEST_F(SQLiteAuditRepositoryTest, GetByTarget)
{
    IAuditRepository::AuditEntry entry1;
    entry1.timestampMs = 1000;
    entry1.userId = "user1";
    entry1.actionType = "LOGIN";
    entry1.targetType = "AUTHENTICATION";
    repository->save(entry1);

    IAuditRepository::AuditEntry entry2;
    entry2.timestampMs = 2000;
    entry2.userId = "user1";
    entry2.actionType = "UPDATE";
    entry2.targetType = "SETTINGS";
    repository->save(entry2);

    auto entries = repository->getByTarget("AUTHENTICATION", "", 0, 3000);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].targetType, "AUTHENTICATION");
}

TEST_F(SQLiteAuditRepositoryTest, GetLastEntry)
{
    IAuditRepository::AuditEntry entry1;
    entry1.timestampMs = 1000;
    entry1.userId = "user1";
    repository->save(entry1);

    IAuditRepository::AuditEntry entry2;
    entry2.timestampMs = 2000;
    entry2.userId = "user2";
    repository->save(entry2);

    auto lastEntry = repository->getLastEntry();
    EXPECT_EQ(lastEntry.timestampMs, 2000);
    EXPECT_EQ(lastEntry.userId, "user2");
}

TEST_F(SQLiteAuditRepositoryTest, Archive)
{
    IAuditRepository::AuditEntry entry1;
    entry1.timestampMs = 1000;
    repository->save(entry1);

    IAuditRepository::AuditEntry entry2;
    entry2.timestampMs = 3000;
    repository->save(entry2);

    size_t archived = repository->archive(2000);
    EXPECT_EQ(archived, 1);

    auto entries = repository->getRange(0, 4000);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].timestampMs, 3000);
}
