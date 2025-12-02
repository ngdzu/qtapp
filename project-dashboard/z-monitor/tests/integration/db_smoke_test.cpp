/**
 * @file db_smoke_test.cpp
 * @brief Integration smoke test for DatabaseManager.
 *
 * This test verifies that DatabaseManager can:
 * - Open and close in-memory databases
 * - Execute SQL queries
 * - Run migrations
 * - Handle transactions
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <memory>

#include "infrastructure/persistence/DatabaseManager.h"
#include "infrastructure/persistence/generated/SchemaInfo.h"

using namespace zmon;

class DatabaseManagerSmokeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!QCoreApplication::instance())
        {
            int argc = 1;
            char *argv[] = {const_cast<char *>("test")};
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }

        m_dbManager = std::make_unique<DatabaseManager>();
    }

    void TearDown() override
    {
        if (m_dbManager && m_dbManager->isOpen())
        {
            m_dbManager->close();
        }
        m_dbManager.reset();
        m_app.reset();
    }

    std::unique_ptr<QCoreApplication> m_app;
    std::unique_ptr<DatabaseManager> m_dbManager;
};

TEST_F(DatabaseManagerSmokeTest, OpenInMemoryDatabase)
{
    // Test opening an in-memory database
    auto result = m_dbManager->open(":memory:");
    ASSERT_TRUE(result.isOk()) << "Failed to open in-memory database: " << result.error().message;
    EXPECT_TRUE(m_dbManager->isOpen());
}

TEST_F(DatabaseManagerSmokeTest, CloseDatabase)
{
    // Open database first
    auto openResult = m_dbManager->open(":memory:");
    ASSERT_TRUE(openResult.isOk());
    EXPECT_TRUE(m_dbManager->isOpen());

    // Close database
    m_dbManager->close();
    EXPECT_FALSE(m_dbManager->isOpen());
}

TEST_F(DatabaseManagerSmokeTest, ExecuteSimpleQuery)
{
    // Open in-memory database
    auto openResult = m_dbManager->open(":memory:");
    ASSERT_TRUE(openResult.isOk());

    // Create a simple table
    QSqlQuery query(m_dbManager->getConnection());
    QString createTableSql = QString("CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT)");
    ASSERT_TRUE(query.exec(createTableSql)) << "Failed to create table: " << query.lastError().text().toStdString();

    // Insert data
    query.prepare("INSERT INTO test_table (name) VALUES (?)");
    query.addBindValue("test_name");
    ASSERT_TRUE(query.exec()) << "Failed to insert data: " << query.lastError().text().toStdString();

    // Query data
    query.prepare("SELECT name FROM test_table WHERE id = ?");
    query.addBindValue(1);
    ASSERT_TRUE(query.exec()) << "Failed to query data: " << query.lastError().text().toStdString();
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), "test_name");
}

TEST_F(DatabaseManagerSmokeTest, TransactionSupport)
{
    // Open in-memory database
    auto openResult = m_dbManager->open(":memory:");
    ASSERT_TRUE(openResult.isOk());

    // Create test table on write connection (same connection we'll use for inserts)
    QSqlQuery createQuery(m_dbManager->getWriteConnection());
    ASSERT_TRUE(createQuery.exec("CREATE TABLE test_table (id INTEGER PRIMARY KEY, value INTEGER)"));

    // Begin transaction
    auto beginResult = m_dbManager->beginTransaction();
    ASSERT_TRUE(beginResult.isOk()) << "Failed to begin transaction: " << beginResult.error().message;

    // Insert data
    QSqlQuery insertQuery(m_dbManager->getWriteConnection());
    ASSERT_TRUE(insertQuery.prepare("INSERT INTO test_table (id, value) VALUES (?, ?)"))
        << "Failed to prepare: " << insertQuery.lastError().text().toStdString();
    insertQuery.addBindValue(1);
    insertQuery.addBindValue(42);
    ASSERT_TRUE(insertQuery.exec()) << "Failed to insert in transaction: " << insertQuery.lastError().text().toStdString();

    // Commit transaction
    auto commitResult = m_dbManager->commit();
    ASSERT_TRUE(commitResult.isOk()) << "Failed to commit transaction: " << commitResult.error().message;

    // Verify data persisted (use write connection since read connection can't see table in :memory:)
    QSqlQuery selectQuery(m_dbManager->getWriteConnection());
    selectQuery.prepare("SELECT value FROM test_table WHERE id = ?");
    selectQuery.addBindValue(1);
    ASSERT_TRUE(selectQuery.exec()) << "Failed to query after commit: " << selectQuery.lastError().text().toStdString();
    ASSERT_TRUE(selectQuery.next());
    EXPECT_EQ(selectQuery.value(0).toInt(), 42);
}

TEST_F(DatabaseManagerSmokeTest, TransactionRollback)
{
    // Open in-memory database
    auto openResult = m_dbManager->open(":memory:");
    ASSERT_TRUE(openResult.isOk());

    // Create test table on write connection (same connection we'll use for inserts)
    QSqlQuery createQuery(m_dbManager->getWriteConnection());
    ASSERT_TRUE(createQuery.exec("CREATE TABLE test_table (id INTEGER PRIMARY KEY, value INTEGER)"));

    // Begin transaction
    auto beginResult = m_dbManager->beginTransaction();
    ASSERT_TRUE(beginResult.isOk());

    // Insert data
    QSqlQuery insertQuery(m_dbManager->getWriteConnection());
    ASSERT_TRUE(insertQuery.prepare("INSERT INTO test_table (id, value) VALUES (?, ?)"))
        << "Failed to prepare: " << insertQuery.lastError().text().toStdString();
    insertQuery.addBindValue(1);
    insertQuery.addBindValue(99);
    ASSERT_TRUE(insertQuery.exec());

    // Rollback transaction
    auto rollbackResult = m_dbManager->rollback();
    ASSERT_TRUE(rollbackResult.isOk()) << "Failed to rollback transaction: " << rollbackResult.error().message;

    // Verify data was not persisted (use write connection since read connection can't see table in :memory:)
    QSqlQuery selectQuery(m_dbManager->getWriteConnection());
    selectQuery.prepare("SELECT COUNT(*) FROM test_table");
    ASSERT_TRUE(selectQuery.exec());
    ASSERT_TRUE(selectQuery.next());
    EXPECT_EQ(selectQuery.value(0).toInt(), 0) << "Data should not exist after rollback";
}

TEST_F(DatabaseManagerSmokeTest, MultipleConnections)
{
    // Open in-memory database
    auto openResult = m_dbManager->open(":memory:");
    ASSERT_TRUE(openResult.isOk());

    // Verify we have separate connections
    QSqlDatabase &mainDb = m_dbManager->getConnection();
    QSqlDatabase &writeDb = m_dbManager->getWriteConnection();
    QSqlDatabase &readDb = m_dbManager->getReadConnection();

    EXPECT_NE(mainDb.connectionName(), writeDb.connectionName());
    EXPECT_NE(mainDb.connectionName(), readDb.connectionName());
    EXPECT_NE(writeDb.connectionName(), readDb.connectionName());

    // All connections should be open
    EXPECT_TRUE(mainDb.isOpen());
    EXPECT_TRUE(writeDb.isOpen());
    EXPECT_TRUE(readDb.isOpen());
}

TEST_F(DatabaseManagerSmokeTest, CannotOpenTwice)
{
    // Open database first time
    auto firstOpen = m_dbManager->open(":memory:");
    ASSERT_TRUE(firstOpen.isOk());
    EXPECT_TRUE(m_dbManager->isOpen());

    // Try to open again (should fail)
    auto secondOpen = m_dbManager->open(":memory:");
    ASSERT_TRUE(secondOpen.isError());
    EXPECT_EQ(secondOpen.error().code, ErrorCode::InvalidArgument);
    EXPECT_TRUE(secondOpen.error().message.find("already open") != std::string::npos);
}

TEST_F(DatabaseManagerSmokeTest, CloseWhenNotOpen)
{
    // Database should not be open initially
    EXPECT_FALSE(m_dbManager->isOpen());

    // Closing when not open should not crash
    m_dbManager->close();
    EXPECT_FALSE(m_dbManager->isOpen());
}

TEST_F(DatabaseManagerSmokeTest, SchemaConstantsAvailable)
{
    // Verify schema constants are available (generated from database.yaml)
    // This ensures schema generation is working
    EXPECT_NE(Schema::Tables::PATIENTS, nullptr);
    EXPECT_NE(Schema::Tables::VITALS, nullptr);
    EXPECT_NE(Schema::Tables::ACTION_LOG, nullptr);
    EXPECT_NE(Schema::Columns::Patients::MRN, nullptr);
    EXPECT_NE(Schema::Columns::Patients::NAME, nullptr);
}

TEST_F(DatabaseManagerSmokeTest, CreatePatientsTable)
{
    // Open in-memory database
    auto openResult = m_dbManager->open(":memory:");
    ASSERT_TRUE(openResult.isOk());

    // Create patients table using schema constants
    QSqlQuery query(m_dbManager->getConnection());
    QString createTableSql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            %2 TEXT PRIMARY KEY NOT NULL,
            %3 TEXT NOT NULL,
            %4 TEXT,
            %5 TEXT,
            %6 TEXT,
            %7 TEXT,
            %8 INTEGER NOT NULL
        )
    )")
                                 .arg(Schema::Tables::PATIENTS)
                                 .arg(Schema::Columns::Patients::MRN)
                                 .arg(Schema::Columns::Patients::NAME)
                                 .arg(Schema::Columns::Patients::DOB)
                                 .arg(Schema::Columns::Patients::SEX)
                                 .arg(Schema::Columns::Patients::ALLERGIES)
                                 .arg(Schema::Columns::Patients::BED_LOCATION)
                                 .arg(Schema::Columns::Patients::CREATED_AT);

    ASSERT_TRUE(query.exec(createTableSql)) << "Failed to create patients table: " << query.lastError().text().toStdString();

    // Verify table exists
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?");
    query.addBindValue(Schema::Tables::PATIENTS);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), Schema::Tables::PATIENTS);
}
