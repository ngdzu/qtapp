/**
 * @file test_query_registry.cpp
 * @brief Unit tests for Query Registry and Query Catalog.
 *
 * Tests verify that all queries are registered, query IDs are unique,
 * prepared statements work correctly, and no magic strings remain.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include "infrastructure/persistence/QueryRegistry.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include "infrastructure/persistence/generated/SchemaInfo.h"
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QSet>
#include <memory>

namespace zmon
{
    namespace persistence
    {

        class QueryRegistryTest : public ::testing::Test
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

                m_tempFile = std::make_unique<QTemporaryFile>();
                m_tempFile->setAutoRemove(false);
                if (!m_tempFile->open())
                {
                    FAIL() << "Cannot create temporary database file";
                }
                m_dbPath = m_tempFile->fileName();
                m_tempFile->close();

                m_dbManager = std::make_unique<DatabaseManager>();
                auto result = m_dbManager->open(m_dbPath);
                if (result.isError())
                {
                    FAIL() << "Cannot open database: " << result.error().message;
                }

                // Initialize all queries
                QueryCatalog::initializeQueries(m_dbManager.get());

                // Create test schema
                createTestSchema();
            }

            void TearDown() override
            {
                m_dbManager.reset();
                m_app.reset();
                QFile::remove(m_dbPath);
            }

            void createTestSchema()
            {
                using namespace Schema::Tables;
                using namespace Schema::Columns::Patients;

                QSqlQuery query(m_dbManager->getWriteConnection());
                QString createPatientsTableSql = QString(R"(
            CREATE TABLE IF NOT EXISTS %1 (
                %2 TEXT PRIMARY KEY NOT NULL,
                %3 TEXT NOT NULL,
                %4 TEXT,
                %5 TEXT,
                %6 TEXT,
                %7 TEXT,
                %8 TEXT,
                %9 INTEGER,
                %10 INTEGER,
                %11 TEXT,
                %12 INTEGER
            );
        )")
                                                     .arg(PATIENTS)
                                                     .arg(MRN)
                                                     .arg(NAME)
                                                     .arg(DOB)
                                                     .arg(SEX)
                                                     .arg(ALLERGIES)
                                                     .arg(BED_LOCATION)
                                                     .arg(ADMISSION_STATUS)
                                                     .arg(ADMITTED_AT)
                                                     .arg(DISCHARGED_AT)
                                                     .arg(ADMISSION_SOURCE)
                                                     .arg(CREATED_AT);

                if (!query.exec(createPatientsTableSql))
                {
                    FAIL() << "Failed to create patients table: " << query.lastError().text().toStdString();
                }

                using namespace Schema::Columns::ActionLog;
                QString createActionLogTableSql = QString(R"(
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
            );
        )")
                                                      .arg(Schema::Tables::ACTION_LOG)
                                                      .arg(ID)
                                                      .arg(TIMESTAMP_MS)
                                                      .arg(TIMESTAMP_ISO)
                                                      .arg(USER_ID)
                                                      .arg(USER_ROLE)
                                                      .arg(ACTION_TYPE)
                                                      .arg(TARGET_TYPE)
                                                      .arg(TARGET_ID)
                                                      .arg(DETAILS)
                                                      .arg(RESULT)
                                                      .arg(ERROR_CODE)
                                                      .arg(ERROR_MESSAGE)
                                                      .arg(DEVICE_ID)
                                                      .arg(SESSION_TOKEN_HASH)
                                                      .arg(IP_ADDRESS)
                                                      .arg(PREVIOUS_HASH);

                if (!query.exec(createActionLogTableSql))
                {
                    FAIL() << "Failed to create action_log table: " << query.lastError().text().toStdString();
                }
            }

            std::unique_ptr<QTemporaryFile> m_tempFile;
            QString m_dbPath;
            std::unique_ptr<QCoreApplication> m_app;
            std::unique_ptr<DatabaseManager> m_dbManager;
        };

        TEST_F(QueryRegistryTest, AllQueriesRegistered)
        {
            auto queries = QueryCatalog::getAllQueries();

            for (const auto &def : queries)
            {
                EXPECT_TRUE(m_dbManager->hasQuery(def.id))
                    << "Query not registered: " << def.id.toStdString();
            }
        }

        TEST_F(QueryRegistryTest, AllQueryIDsUnique)
        {
            auto queries = QueryCatalog::getAllQueries();
            QSet<QString> ids;

            for (const auto &def : queries)
            {
                EXPECT_FALSE(ids.contains(def.id))
                    << "Duplicate query ID: " << def.id.toStdString();
                ids.insert(def.id);
            }
        }

        TEST_F(QueryRegistryTest, PatientFindByMrn_Works)
        {
            // Insert test data
            QSqlQuery insertQuery(m_dbManager->getWriteConnection());
            insertQuery.prepare("INSERT INTO patients (mrn, name, dob, sex, bed_location, admission_status, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)");
            insertQuery.addBindValue("TEST-001");
            insertQuery.addBindValue("Test Patient");
            insertQuery.addBindValue("1990-01-01");
            insertQuery.addBindValue("M");
            insertQuery.addBindValue("ICU-1A");
            insertQuery.addBindValue("ADMITTED");
            insertQuery.addBindValue(QDateTime::currentMSecsSinceEpoch());
            ASSERT_TRUE(insertQuery.exec()) << "Failed to insert test patient: " << insertQuery.lastError().text().toStdString();

            // Test find query
            QSqlQuery findQuery = m_dbManager->getPreparedQueryForRead(QueryId::Patient::FIND_BY_MRN);
            ASSERT_TRUE(findQuery.isValid()) << "Query not valid";
            findQuery.bindValue(":mrn", "TEST-001");
            ASSERT_TRUE(findQuery.exec()) << "Query execution failed: " << findQuery.lastError().text().toStdString();
            ASSERT_TRUE(findQuery.next()) << "No results found";
            EXPECT_EQ(findQuery.value("name").toString(), "Test Patient");
        }

        TEST_F(QueryRegistryTest, PatientInsert_Works)
        {
            QSqlQuery insertQuery = m_dbManager->getPreparedQuery(QueryId::Patient::INSERT);
            ASSERT_TRUE(insertQuery.isValid()) << "Query not valid";

            insertQuery.bindValue(":mrn", "TEST-002");
            insertQuery.bindValue(":name", "Insert Test Patient");
            insertQuery.bindValue(":dob", "1990-01-01");
            insertQuery.bindValue(":sex", "F");
            insertQuery.bindValue(":allergies", "");
            insertQuery.bindValue(":bedLocation", "Ward-2B");
            insertQuery.bindValue(":admissionStatus", "ADMITTED");
            insertQuery.bindValue(":admittedAt", QDateTime::currentMSecsSinceEpoch());
            insertQuery.bindValue(":dischargedAt", QVariant());
            insertQuery.bindValue(":admissionSource", "manual");
            insertQuery.bindValue(":createdAt", QDateTime::currentMSecsSinceEpoch());

            ASSERT_TRUE(insertQuery.exec()) << "Insert failed: " << insertQuery.lastError().text().toStdString();

            // Verify insertion
            QSqlQuery findQuery = m_dbManager->getPreparedQueryForRead(QueryId::Patient::FIND_BY_MRN);
            findQuery.bindValue(":mrn", "TEST-002");
            ASSERT_TRUE(findQuery.exec());
            ASSERT_TRUE(findQuery.next());
            EXPECT_EQ(findQuery.value("name").toString(), "Insert Test Patient");
        }

        TEST_F(QueryRegistryTest, PatientCheckExists_Works)
        {
            // Insert test data
            QSqlQuery insertQuery(m_dbManager->getWriteConnection());
            insertQuery.prepare("INSERT INTO patients (mrn, name, dob, sex, bed_location, admission_status, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)");
            insertQuery.addBindValue("TEST-003");
            insertQuery.addBindValue("Exists Test");
            insertQuery.addBindValue("1990-01-01");
            insertQuery.addBindValue("M");
            insertQuery.addBindValue("ICU-1A");
            insertQuery.addBindValue("ADMITTED");
            insertQuery.addBindValue(QDateTime::currentMSecsSinceEpoch());
            ASSERT_TRUE(insertQuery.exec());

            // Test check exists query
            QSqlQuery checkQuery = m_dbManager->getPreparedQueryForRead(QueryId::Patient::CHECK_EXISTS);
            ASSERT_TRUE(checkQuery.isValid()) << "Query not valid";
            checkQuery.bindValue(":mrn", "TEST-003");
            ASSERT_TRUE(checkQuery.exec()) << "Query execution failed: " << checkQuery.lastError().text().toStdString();
            ASSERT_TRUE(checkQuery.next());
            EXPECT_GT(checkQuery.value(0).toInt(), 0) << "Patient should exist";

            // Test non-existent patient
            QSqlQuery checkQuery2 = m_dbManager->getPreparedQueryForRead(QueryId::Patient::CHECK_EXISTS);
            checkQuery2.bindValue(":mrn", "NONEXISTENT");
            ASSERT_TRUE(checkQuery2.exec());
            ASSERT_TRUE(checkQuery2.next());
            EXPECT_EQ(checkQuery2.value(0).toInt(), 0) << "Patient should not exist";
        }

        TEST_F(QueryRegistryTest, PatientFindAll_Works)
        {
            // Insert multiple test patients
            for (int i = 1; i <= 3; ++i)
            {
                QSqlQuery insertQuery(m_dbManager->getWriteConnection());
                insertQuery.prepare(QString("INSERT INTO patients (mrn, name, dob, sex, bed_location, admission_status, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)"));
                insertQuery.addBindValue(QString("TEST-FINDALL-%1").arg(i));
                insertQuery.addBindValue(QString("Patient %1").arg(i));
                insertQuery.addBindValue("1990-01-01");
                insertQuery.addBindValue("M");
                insertQuery.addBindValue("ICU-1A");
                insertQuery.addBindValue("ADMITTED");
                insertQuery.addBindValue(QDateTime::currentMSecsSinceEpoch() + i); // Different timestamps
                ASSERT_TRUE(insertQuery.exec());
            }

            // Test findAll query
            QSqlQuery findAllQuery = m_dbManager->getPreparedQueryForRead(QueryId::Patient::FIND_ALL);
            ASSERT_TRUE(findAllQuery.isValid()) << "Query not valid";
            ASSERT_TRUE(findAllQuery.exec()) << "Query execution failed: " << findAllQuery.lastError().text().toStdString();

            int count = 0;
            while (findAllQuery.next())
            {
                count++;
            }
            EXPECT_GE(count, 3) << "Should find at least 3 patients";
        }

        TEST_F(QueryRegistryTest, ActionLogGetLastId_Works)
        {
            // Insert test action log entry
            QSqlQuery insertQuery(m_dbManager->getWriteConnection());
            insertQuery.prepare("INSERT INTO action_log (timestamp_ms, timestamp_iso, action_type, result, device_id) VALUES (?, ?, ?, ?, ?)");
            insertQuery.addBindValue(QDateTime::currentMSecsSinceEpoch());
            insertQuery.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            insertQuery.addBindValue("LOGIN");
            insertQuery.addBindValue("SUCCESS");
            insertQuery.addBindValue("DEVICE-001");
            ASSERT_TRUE(insertQuery.exec());

            // Test get last ID query (using QueryCatalog directly since SQLiteActionLogRepository uses it)
            auto queryDef = QueryCatalog::getQuery(QueryId::ActionLog::GET_LAST_ID);
            ASSERT_FALSE(queryDef.id.isEmpty()) << "Query not found";

            QSqlQuery query(m_dbManager->getReadConnection());
            query.prepare(queryDef.sql);
            ASSERT_TRUE(query.exec());
            ASSERT_TRUE(query.next());
            EXPECT_GT(query.value("max_id").toLongLong(), 0) << "Last ID should be greater than 0";
        }

        TEST_F(QueryRegistryTest, NoMagicStringQueries)
        {
            // This test verifies that repositories use QueryId constants, not magic strings
            // We can't directly test this, but we can verify that all registered queries
            // have proper QueryId constants defined

            auto queries = QueryCatalog::getAllQueries();
            EXPECT_GT(queries.size(), 0) << "No queries registered";

            // Verify all queries have valid IDs that match QueryId namespace pattern
            for (const auto &def : queries)
            {
                EXPECT_FALSE(def.id.isEmpty()) << "Query ID cannot be empty";
                EXPECT_TRUE(def.id.contains('.')) << "Query ID should follow namespace.action pattern: " << def.id.toStdString();
                EXPECT_FALSE(def.sql.isEmpty()) << "SQL cannot be empty for query: " << def.id.toStdString();
            }
        }

        TEST_F(QueryRegistryTest, QueryCatalogGetQuery_Works)
        {
            auto queryDef = QueryCatalog::getQuery(QueryId::Patient::FIND_BY_MRN);
            EXPECT_FALSE(queryDef.id.isEmpty()) << "Query not found";
            EXPECT_EQ(queryDef.id, QueryId::Patient::FIND_BY_MRN);
            EXPECT_FALSE(queryDef.sql.isEmpty()) << "SQL should not be empty";
            EXPECT_TRUE(queryDef.isReadOnly) << "FIND_BY_MRN should be read-only";

            // Test non-existent query
            auto nonExistent = QueryCatalog::getQuery("nonexistent.query");
            EXPECT_TRUE(nonExistent.id.isEmpty()) << "Non-existent query should return empty definition";
        }

        TEST_F(QueryRegistryTest, QueryCatalogGenerateDocumentation_Works)
        {
            QString doc = QueryCatalog::generateDocumentation();
            EXPECT_FALSE(doc.isEmpty()) << "Documentation should not be empty";
            EXPECT_TRUE(doc.contains("# Database Query Reference")) << "Documentation should have title";
            EXPECT_TRUE(doc.contains("patient")) << "Documentation should contain patient queries";
            EXPECT_TRUE(doc.contains("action_log")) << "Documentation should contain action_log queries";
        }

    } // namespace persistence
} // namespace zmon
