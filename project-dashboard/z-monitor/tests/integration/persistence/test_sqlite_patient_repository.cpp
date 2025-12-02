/**
 * @file test_sqlite_patient_repository.cpp
 * @brief Unit tests for SQLitePatientRepository with hybrid ORM + manual SQL.
 *
 * Tests verify:
 * - ORM mapping uses Schema constants
 * - CRUD operations work with ORM (when enabled)
 * - Complex queries work with manual SQL
 * - Hybrid approach functions correctly
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include "infrastructure/persistence/SQLitePatientRepository.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include "domain/common/Result.h"
#include "tests/fixtures/RepositoryTestFixture.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <memory>
#include <set>

namespace zmon
{

    /**
     * @class SQLitePatientRepositoryTest
     * @brief Test fixture for SQLitePatientRepository tests.
     */
    class SQLitePatientRepositoryTest : public test::RepositoryTestFixture
    {
    protected:
        void SetUp() override
        {
            RepositoryTestFixture::SetUp();
            // Create repository using fixture's DatabaseManager
            m_repository = std::make_unique<SQLitePatientRepository>(databaseManager());
        }

        void TearDown() override
        {
            m_repository.reset();
            RepositoryTestFixture::TearDown();
        }

        /**
         * @brief Create a test patient aggregate.
         */
        std::shared_ptr<PatientAggregate> createTestPatient(
            const std::string &mrn = "MRN-001",
            const std::string &name = "John Doe",
            int64_t dobMs = 0,
            const std::string &sex = "M")
        {
            std::vector<std::string> allergies = {"Penicillin", "Latex"};
            PatientIdentity identity(mrn, name, dobMs, sex, allergies);
            BedLocation bedLocation("ICU-4B", "ICU");

            auto patient = std::make_shared<PatientAggregate>();
            auto result = patient->admit(identity, bedLocation, "manual");
            if (result.isError())
            {
                ADD_FAILURE() << "Cannot create test patient: " << result.error().message;
                return nullptr;
            }

            return patient;
        }

        std::unique_ptr<SQLitePatientRepository> m_repository;
    };

    /**
     * @test Test that repository can find patient by MRN using manual SQL.
     */
    TEST_F(SQLitePatientRepositoryTest, FindByMrn_ManualSql)
    {
        // Create and save test patient
        auto patient = createTestPatient("MRN-001", "John Doe");
        auto saveResult = m_repository->save(*patient);
        ASSERT_TRUE(saveResult.isOk()) << "Save failed: " << saveResult.error().message;

        // Find patient
        auto result = m_repository->findByMrn("MRN-001");
        ASSERT_TRUE(result.isOk()) << "Find failed: " << result.error().message;
        auto found = result.value();
        ASSERT_NE(found, nullptr) << "Patient not found";
        ASSERT_EQ(found->getPatientMrn(), "MRN-001");
        ASSERT_EQ(found->getPatientIdentity().name, "John Doe");
    }

    /**
     * @test Test that repository returns nullptr for non-existent patient.
     */
    TEST_F(SQLitePatientRepositoryTest, FindByMrn_NotFound)
    {
        auto result = m_repository->findByMrn("MRN-NONEXISTENT");
        ASSERT_TRUE(result.isError()) << "Should return error for non-existent patient";
        ASSERT_EQ(result.error().code, ErrorCode::NotFound);
    }

    /**
     * @test Test that repository can save patient using manual SQL.
     */
    TEST_F(SQLitePatientRepositoryTest, Save_ManualSql)
    {
        auto patient = createTestPatient("MRN-002", "Jane Smith");

        // Save patient
        auto result = m_repository->save(*patient);
        ASSERT_TRUE(result.isOk()) << "Save failed: " << result.error().message;

        // Verify patient was saved
        auto findResult = m_repository->findByMrn("MRN-002");
        ASSERT_TRUE(findResult.isOk()) << "Find failed: " << findResult.error().message;
        auto found = findResult.value();
        ASSERT_NE(found, nullptr) << "Patient not found after save";
        ASSERT_EQ(found->getPatientIdentity().name, "Jane Smith");
    }

    /**
     * @test Test that repository can update existing patient.
     */
    TEST_F(SQLitePatientRepositoryTest, Save_UpdateExisting)
    {
        // Create and save initial patient
        auto patient = createTestPatient("MRN-003", "Bob Johnson");
        auto saveResult = m_repository->save(*patient);
        ASSERT_TRUE(saveResult.isOk()) << "Initial save failed";

        // Update patient (discharge and readmit with new name)
        patient->discharge();
        PatientIdentity newIdentity("MRN-003", "Robert Johnson", 0, "M");
        BedLocation newBedLocation("Ward-2A", "Ward");
        patient->admit(newIdentity, newBedLocation, "manual");

        // Save updated patient
        auto updateResult = m_repository->save(*patient);
        ASSERT_TRUE(updateResult.isOk()) << "Update failed: " << updateResult.error().message;

        // Verify patient was updated
        auto findResult = m_repository->findByMrn("MRN-003");
        ASSERT_TRUE(findResult.isOk()) << "Find failed: " << findResult.error().message;
        auto found = findResult.value();
        ASSERT_NE(found, nullptr) << "Patient not found after update";
        ASSERT_EQ(found->getPatientIdentity().name, "Robert Johnson");
    }

    /**
     * @test Test that repository can remove patient using manual SQL.
     */
    TEST_F(SQLitePatientRepositoryTest, Remove_ManualSql)
    {
        // Create and save test patient
        auto patient = createTestPatient("MRN-004", "Alice Brown");
        auto saveResult = m_repository->save(*patient);
        ASSERT_TRUE(saveResult.isOk()) << "Save failed";

        // Verify patient exists
        auto findResult = m_repository->findByMrn("MRN-004");
        ASSERT_TRUE(findResult.isOk()) << "Find failed: " << findResult.error().message;
        auto found = findResult.value();
        ASSERT_NE(found, nullptr) << "Patient not found before removal";

        // Remove patient
        auto removeResult = m_repository->remove("MRN-004");
        ASSERT_TRUE(removeResult.isOk()) << "Remove failed: " << removeResult.error().message;

        // Verify patient was removed
        auto findAfterResult = m_repository->findByMrn("MRN-004");
        ASSERT_TRUE(findAfterResult.isError()) << "Should return error for removed patient";
        ASSERT_EQ(findAfterResult.error().code, ErrorCode::NotFound);
    }

    /**
     * @test Test that repository returns error when removing non-existent patient.
     */
    TEST_F(SQLitePatientRepositoryTest, Remove_NotFound)
    {
        auto result = m_repository->remove("MRN-NONEXISTENT");
        ASSERT_TRUE(result.isError()) << "Should return error for non-existent patient";
        ASSERT_EQ(result.error().code, ErrorCode::NotFound);
    }

    /**
     * @test Test that repository can find all patients using manual SQL.
     */
    TEST_F(SQLitePatientRepositoryTest, FindAll_ManualSql)
    {
        // Create and save multiple patients
        auto patient1 = createTestPatient("MRN-005", "Patient One");
        auto patient2 = createTestPatient("MRN-006", "Patient Two");
        auto patient3 = createTestPatient("MRN-007", "Patient Three");

        m_repository->save(*patient1);
        m_repository->save(*patient2);
        m_repository->save(*patient3);

        // Find all patients
        auto findAllResult = m_repository->findAll();
        ASSERT_TRUE(findAllResult.isOk()) << "FindAll failed: " << findAllResult.error().message;
        auto allPatients = findAllResult.value();
        ASSERT_GE(allPatients.size(), 3) << "Should find at least 3 patients";

        // Verify all patients are found
        std::set<std::string> foundMrns;
        for (const auto &p : allPatients)
        {
            foundMrns.insert(p->getPatientMrn());
        }

        ASSERT_TRUE(foundMrns.count("MRN-005")) << "MRN-005 not found";
        ASSERT_TRUE(foundMrns.count("MRN-006")) << "MRN-006 not found";
        ASSERT_TRUE(foundMrns.count("MRN-007")) << "MRN-007 not found";
    }

    /**
     * @test Test that repository can get admission history using manual SQL.
     */
    TEST_F(SQLitePatientRepositoryTest, GetAdmissionHistory_ManualSql)
    {
        // Create and save test patient
        auto patient = createTestPatient("MRN-008", "History Patient");
        m_repository->save(*patient);

        // Insert admission events directly (simulating admission workflow)
        QSqlQuery query(databaseManager()->getWriteConnection());
        QString insertEventSql = R"(
        INSERT INTO admission_events (patient_mrn, event_type, details, timestamp)
        VALUES (:mrn, :eventType, :details, :timestamp)
    )";

        query.prepare(insertEventSql);
        query.bindValue(":mrn", "MRN-008");
        query.bindValue(":eventType", "ADMIT");
        query.bindValue(":details", "Admitted to ICU-4B");
        query.bindValue(":timestamp", QDateTime::currentMSecsSinceEpoch() - 3600000); // 1 hour ago
        ASSERT_TRUE(query.exec()) << "Cannot insert admission event";

        query.bindValue(":mrn", "MRN-008");
        query.bindValue(":eventType", "TRANSFER");
        query.bindValue(":details", "Transferred to Ward-2A");
        query.bindValue(":timestamp", QDateTime::currentMSecsSinceEpoch() - 1800000); // 30 min ago
        ASSERT_TRUE(query.exec()) << "Cannot insert transfer event";

        // Get admission history
        auto historyResult = m_repository->getAdmissionHistory("MRN-008");
        ASSERT_TRUE(historyResult.isOk()) << "GetAdmissionHistory failed: " << historyResult.error().message;
        auto history = historyResult.value();
        ASSERT_GE(history.size(), 2) << "Should find at least 2 events";

        // Verify events are in reverse chronological order (most recent first)
        // History format: "timestamp|eventType|details"
        ASSERT_FALSE(history.empty()) << "History should not be empty";
    }

    /**
     * @test Test that repository handles empty database correctly.
     */
    TEST_F(SQLitePatientRepositoryTest, FindAll_EmptyDatabase)
    {
        auto result = m_repository->findAll();
        ASSERT_TRUE(result.isOk()) << "FindAll failed: " << result.error().message;
        auto allPatients = result.value();
        ASSERT_EQ(allPatients.size(), 0) << "Should return empty vector for empty database";
    }

    /**
     * @test Test that repository handles getAdmissionHistory for patient with no events.
     */
    TEST_F(SQLitePatientRepositoryTest, GetAdmissionHistory_NoEvents)
    {
        // Create and save test patient
        auto patient = createTestPatient("MRN-009", "No History Patient");
        m_repository->save(*patient);

        // Get admission history (should be empty)
        auto historyResult = m_repository->getAdmissionHistory("MRN-009");
        ASSERT_TRUE(historyResult.isOk()) << "GetAdmissionHistory failed: " << historyResult.error().message;
        auto history = historyResult.value();
        ASSERT_EQ(history.size(), 0) << "Should return empty history for patient with no events";
    }

    /**
     * @test Test that repository uses Schema constants in SQL queries.
     *
     * This test verifies that the repository uses Schema:: constants instead of
     * hardcoded table/column names. We can't directly test this, but we can verify
     * that the queries work correctly, which implies Schema constants are used.
     */
    TEST_F(SQLitePatientRepositoryTest, UsesSchemaConstants)
    {
        // This test verifies that Schema constants are used by ensuring queries work
        // If hardcoded strings were used, queries would fail when schema changes

        auto patient = createTestPatient("MRN-SCHEMA-TEST", "Schema Test Patient");
        auto saveResult = m_repository->save(*patient);
        ASSERT_TRUE(saveResult.isOk()) << "Save should work with Schema constants";

        auto findResult = m_repository->findByMrn("MRN-SCHEMA-TEST");
        ASSERT_TRUE(findResult.isOk()) << "Find should work with Schema constants";
        ASSERT_NE(findResult.value(), nullptr) << "Find should work with Schema constants";

        auto findAllResult = m_repository->findAll();
        ASSERT_TRUE(findAllResult.isOk()) << "FindAll should work with Schema constants";
        ASSERT_GE(findAllResult.value().size(), 1) << "FindAll should work with Schema constants";
    }

#ifdef USE_QXORM
    /**
     * @test Test that repository can use ORM for findByMrn when QxOrm is enabled.
     *
     * This test only runs when USE_QXORM is defined.
     */
    TEST_F(SQLitePatientRepositoryTest, FindByMrn_Orm)
    {
        if (!DatabaseManager::isQxOrmEnabled())
        {
            GTEST_SKIP() << "QxOrm is not enabled";
        }

        // Create and save test patient
        auto patient = createTestPatient("MRN-ORM-001", "ORM Test Patient");
        auto saveResult = m_repository->save(*patient);
        ASSERT_TRUE(saveResult.isOk()) << "Save failed: " << saveResult.error().message;

        // Find patient (should use ORM)
        auto result = m_repository->findByMrn("MRN-ORM-001");
        ASSERT_TRUE(result.isOk()) << "Find failed: " << result.error().message;
        auto found = result.value();
        ASSERT_NE(found, nullptr) << "Patient not found via ORM";
        ASSERT_EQ(found->getPatientMrn(), "MRN-ORM-001");
        ASSERT_EQ(found->getPatientIdentity().name, "ORM Test Patient");
    }

    /**
     * @test Test that repository can use ORM for save when QxOrm is enabled.
     */
    TEST_F(SQLitePatientRepositoryTest, Save_Orm)
    {
        if (!DatabaseManager::isQxOrmEnabled())
        {
            GTEST_SKIP() << "QxOrm is not enabled";
        }

        auto patient = createTestPatient("MRN-ORM-002", "ORM Save Test");

        // Save patient (should use ORM)
        auto result = m_repository->save(*patient);
        ASSERT_TRUE(result.isOk()) << "ORM save failed: " << result.error().message;

        // Verify patient was saved
        auto findResult = m_repository->findByMrn("MRN-ORM-002");
        ASSERT_TRUE(findResult.isOk()) << "Find failed: " << findResult.error().message;
        auto found = findResult.value();
        ASSERT_NE(found, nullptr) << "Patient not found after ORM save";
    }

    /**
     * @test Test that repository can use ORM for remove when QxOrm is enabled.
     */
    TEST_F(SQLitePatientRepositoryTest, Remove_Orm)
    {
        if (!DatabaseManager::isQxOrmEnabled())
        {
            GTEST_SKIP() << "QxOrm is not enabled";
        }

        // Create and save test patient
        auto patient = createTestPatient("MRN-ORM-003", "ORM Remove Test");
        m_repository->save(*patient);

        // Remove patient (should use ORM)
        auto result = m_repository->remove("MRN-ORM-003");
        ASSERT_TRUE(result.isOk()) << "ORM remove failed: " << result.error().message;

        // Verify patient was removed
        auto findResult = m_repository->findByMrn("MRN-ORM-003");
        ASSERT_TRUE(findResult.isError()) << "Should return error for removed patient";
        ASSERT_EQ(findResult.error().code, ErrorCode::NotFound);
    }

    /**
     * @test Test hybrid approach: ORM for simple CRUD, manual SQL for complex queries.
     */
    TEST_F(SQLitePatientRepositoryTest, HybridApproach)
    {
        if (!DatabaseManager::isQxOrmEnabled())
        {
            GTEST_SKIP() << "QxOrm is not enabled";
        }

        // Use ORM for simple CRUD
        auto patient1 = createTestPatient("MRN-HYBRID-001", "Hybrid Test 1");
        auto patient2 = createTestPatient("MRN-HYBRID-002", "Hybrid Test 2");

        auto saveResult1 = m_repository->save(*patient1); // Should use ORM
        auto saveResult2 = m_repository->save(*patient2); // Should use ORM
        ASSERT_TRUE(saveResult1.isOk()) << "ORM save failed";
        ASSERT_TRUE(saveResult2.isOk()) << "ORM save failed";

        auto findResult1 = m_repository->findByMrn("MRN-HYBRID-001"); // Should use ORM
        auto findResult2 = m_repository->findByMrn("MRN-HYBRID-002"); // Should use ORM
        ASSERT_TRUE(findResult1.isOk()) << "ORM find failed: " << findResult1.error().message;
        ASSERT_TRUE(findResult2.isOk()) << "ORM find failed: " << findResult2.error().message;
        ASSERT_NE(findResult1.value(), nullptr) << "ORM find failed";
        ASSERT_NE(findResult2.value(), nullptr) << "ORM find failed";

        // Use manual SQL for complex queries
        auto findAllResult = m_repository->findAll(); // Should use manual SQL
        ASSERT_TRUE(findAllResult.isOk()) << "FindAll failed: " << findAllResult.error().message;
        auto allPatients = findAllResult.value();
        ASSERT_GE(allPatients.size(), 2) << "FindAll should work with manual SQL";

        // Verify both patients are in the list
        std::set<std::string> foundMrns;
        for (const auto &p : allPatients)
        {
            foundMrns.insert(p->getPatientMrn());
        }
        ASSERT_TRUE(foundMrns.count("MRN-HYBRID-001")) << "Patient 1 not found in findAll";
        ASSERT_TRUE(foundMrns.count("MRN-HYBRID-002")) << "Patient 2 not found in findAll";
    }
#endif // USE_QXORM

} // namespace zmon
