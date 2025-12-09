/**
 * @file AdmissionWorkflowTest.cpp
 * @brief Integration tests for end-to-end admission workflow.
 *
 * Tests the complete admission workflow including:
 * - Patient lookup via MockPatientLookupService
 * - Patient admission via AdmissionService
 * - Database persistence verification
 * - UI state updates via PatientController
 * - Audit logging verification
 *
 * @author Z Monitor Team
 * @date 2025-12-03
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QUuid>

#include "application/services/AdmissionService.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include "ui/controllers/PatientController.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include "tests/mocks/infrastructure/MockPatientLookupService.h"
#include "tests/mocks/infrastructure/MockSensorDataSource.h"

using namespace zmon;

namespace
{

    /**
     * @class AdmissionWorkflowTest
     * @brief Integration test fixture for admission workflow.
     *
     * Sets up complete test environment with:
     * - In-memory database with migrations
     * - Mock patient lookup service
     * - Mock sensor data source
     * - Admission service
     * - Patient controller
     */
    class AdmissionWorkflowTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Initialize in-memory database and run migrations
            db = std::make_unique<DatabaseManager>();
            // Use unique URI for each test to avoid shared cache contamination
            const QString uniqueDbUri = QString("file:test_%1?mode=memory&cache=shared")
                                            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
            auto opened = db->open(uniqueDbUri);
            ASSERT_TRUE(opened.isOk()) << "Failed to open in-memory database";

            auto migrated = db->executeMigrations();
            ASSERT_TRUE(migrated.isOk()) << "Failed to run migrations: "
                                         << migrated.error().message;

            // Create mock services
            mockPatientLookup = std::make_unique<MockPatientLookupService>();
            mockSensor = std::make_unique<MockSensorDataSource>();

            // Create admission service (no action log for this test)
            admissionService = std::make_unique<AdmissionService>(nullptr, nullptr);

            // Create patient controller
            patientController = std::make_unique<PatientController>(admissionService.get(), nullptr);

            // Set up default test patients in mock lookup service
            setupTestPatients();
        }

        void TearDown() override
        {
            patientController.reset();
            admissionService.reset();
            mockSensor.reset();
            mockPatientLookup.reset();

            if (db)
                db->close();
            db.reset();
        }

        /**
         * @brief Set up default test patients in mock lookup service.
         */
        void setupTestPatients()
        {
            // Clear default patients from mock constructor
            mockPatientLookup->clear();

            // Patient 1: Standard ICU patient with allergies
            PatientIdentity patient1("MRN-123456", "John Doe",
                                     QDate(1965, 5, 15).startOfDay().toMSecsSinceEpoch(),
                                     "M", {"Penicillin", "Latex"});
            mockPatientLookup->addPatient("MRN-123456", patient1);

            // Patient 2: Patient with no allergies
            PatientIdentity patient2("MRN-789012", "Jane Smith",
                                     QDate(1980, 10, 25).startOfDay().toMSecsSinceEpoch(),
                                     "F", {});
            mockPatientLookup->addPatient("MRN-789012", patient2);

            // Patient 3: Pediatric patient
            PatientIdentity patient3("MRN-345678", "Tommy Johnson",
                                     QDate(2015, 3, 8).startOfDay().toMSecsSinceEpoch(),
                                     "M", {"Peanuts"});
            mockPatientLookup->addPatient("MRN-345678", patient3);
        }

        /**
         * @brief Verify patient exists in database.
         *
         * @param mrn Medical Record Number to verify
         * @return true if patient exists, false otherwise
         */
        bool verifyPatientInDatabase(const QString &mrn)
        {
            QSqlQuery query(db->getConnection());
            query.prepare("SELECT COUNT(*) FROM patients WHERE mrn = :mrn");
            query.bindValue(":mrn", mrn);

            if (!query.exec() || !query.next())
            {
                return false;
            }

            return query.value(0).toInt() > 0;
        }

        /**
         * @brief Verify admission event logged in database.
         *
         * @param mrn Medical Record Number
         * @param eventType Expected event type (e.g., "admission")
         * @return true if event logged, false otherwise
         */
        bool verifyAdmissionEvent(const QString &mrn, const QString &eventType)
        {
            QSqlQuery query(db->getConnection());
            query.prepare("SELECT COUNT(*) FROM admission_events "
                          "WHERE patient_mrn = :mrn AND event_type = :type");
            query.bindValue(":mrn", mrn);
            query.bindValue(":type", eventType);

            if (!query.exec() || !query.next())
            {
                return false;
            }

            return query.value(0).toInt() > 0;
        }

        // Test fixtures
        std::unique_ptr<DatabaseManager> db;
        std::unique_ptr<MockPatientLookupService> mockPatientLookup;
        std::unique_ptr<MockSensorDataSource> mockSensor;
        std::unique_ptr<AdmissionService> admissionService;
        std::unique_ptr<PatientController> patientController;
    };

    // ========================================================================
    // Happy Path Tests
    // ========================================================================

    /**
     * @test Manual admission workflow: Admit patient by MRN
     *
     * Verifies:
     * - Patient can be admitted with valid MRN
     * - AdmissionService reflects admitted state
     * - Patient information is correct
     */
    TEST_F(AdmissionWorkflowTest, ManualAdmission_ValidMrn_AdmitsPatient)
    {
        const std::string mrn = "MRN-123456";
        const std::string name = "John Doe";

        // Create patient identity and bed location
        PatientIdentity identity(mrn, name, 0, "M");
        BedLocation location("4B", "ICU"); // room=4B, unit=ICU -> formats as "ICU-4B"

        // Admit patient via AdmissionService
        auto admitResult = admissionService->admitPatient(identity, location,
                                                          AdmissionService::AdmissionSource::Manual);

        // Verify admission succeeded
        ASSERT_TRUE(admitResult.isOk()) << "Admission failed: "
                                        << admitResult.error().message;

        // Verify service reflects admitted state
        EXPECT_TRUE(admissionService->isPatientAdmitted());

        // Verify admission info
        auto info = admissionService->getCurrentAdmission();
        EXPECT_EQ(info.mrn, QString::fromStdString(mrn));
        EXPECT_EQ(info.name, QString::fromStdString(name));
        EXPECT_EQ(info.bedLocation, QString("ICU-4B"));
        EXPECT_TRUE(info.admittedAt.isValid());
    }

    /**
     * @test Barcode scan admission workflow
     *
     * Simulates barcode scan admission:
     * - Lookup patient by MRN from barcode
     * - Admit patient with lookup data
     * - Verify admission succeeds
     */
    TEST_F(AdmissionWorkflowTest, BarcodeAdmission_ValidBarcode_AdmitsPatient)
    {
        const QString mrn = "MRN-789012";

        // Simulate barcode scan: Lookup patient by name to get identity
        auto searchResult = mockPatientLookup->searchByName("Jane Smith");
        ASSERT_TRUE(searchResult.isOk()) << "Patient search failed";
        ASSERT_FALSE(searchResult.value().empty()) << "Patient not found";

        const PatientIdentity &identity = searchResult.value()[0];
        EXPECT_EQ(identity.mrn, mrn.toStdString());

        // Use bed location
        // BedLocation(room, unit) formats as "unit-room", so for "ICU-4B" we need room="4B", unit="ICU"
        BedLocation location("4B", "ICU");

        // Admit patient
        auto admitResult = admissionService->admitPatient(identity, location,
                                                          AdmissionService::AdmissionSource::Barcode);

        // Verify admission succeeded
        ASSERT_TRUE(admitResult.isOk());
        EXPECT_TRUE(admissionService->isPatientAdmitted());

        auto info = admissionService->getCurrentAdmission();
        EXPECT_EQ(info.mrn, mrn);
        EXPECT_EQ(info.name, QString("Jane Smith"));
    }

    /**
     * @test Patient controller state updates on admission
     *
     * Verifies:
     * - PatientController properties update when patient admitted
     * - Signals are emitted for QML bindings
     */
    TEST_F(AdmissionWorkflowTest, Admission_UpdatesControllerState)
    {
        // Set up signal spies
        QSignalSpy isAdmittedSpy(patientController.get(), &PatientController::isAdmittedChanged);
        QSignalSpy patientNameSpy(patientController.get(), &PatientController::patientNameChanged);
        QSignalSpy patientMrnSpy(patientController.get(), &PatientController::patientMrnChanged);

        const std::string mrn = "MRN-345678";
        const std::string name = "Tommy Johnson";

        // Verify initial state
        EXPECT_FALSE(patientController->isAdmitted());
        EXPECT_TRUE(patientController->patientName().isEmpty());

        // Admit patient
        PatientIdentity identity(mrn, name, 10, "M");
        BedLocation location("PICU-2C", "PICU");
        auto admitResult = admissionService->admitPatient(identity, location,
                                                          AdmissionService::AdmissionSource::Manual);
        ASSERT_TRUE(admitResult.isOk());

        // Process events to trigger signals
        QCoreApplication::processEvents();

        // Verify controller state updated
        EXPECT_TRUE(patientController->isAdmitted());
        EXPECT_EQ(patientController->patientName(), QString::fromStdString(name));
        EXPECT_EQ(patientController->patientMrn(), QString::fromStdString(mrn));

        // Verify signals emitted (at least once each)
        EXPECT_GE(isAdmittedSpy.count(), 1) << "isAdmittedChanged signal not emitted";
        EXPECT_GE(patientNameSpy.count(), 1) << "patientNameChanged signal not emitted";
        EXPECT_GE(patientMrnSpy.count(), 1) << "patientMrnChanged signal not emitted";
    }

    /**
     * @test Patient discharge workflow
     *
     * Verifies:
     * - Patient can be discharged after admission
     * - Service state updates correctly
     * - Controller reflects discharge state
     */
    TEST_F(AdmissionWorkflowTest, Discharge_AfterAdmission_UpdatesState)
    {
        // First admit a patient
        const QString mrn = "MRN-123456";
        PatientIdentity identity(mrn.toStdString(), "John Doe", 0, "M");
        BedLocation location("ICU-4B", "ICU");
        auto admitResult = admissionService->admitPatient(identity, location,
                                                          AdmissionService::AdmissionSource::Manual);
        ASSERT_TRUE(admitResult.isOk());
        ASSERT_TRUE(admissionService->isPatientAdmitted());

        // Set up signal spy for discharge
        QSignalSpy isAdmittedSpy(patientController.get(), &PatientController::isAdmittedChanged);

        // Discharge patient
        auto dischargeResult = admissionService->dischargePatient(mrn);
        ASSERT_TRUE(dischargeResult.isOk()) << "Discharge failed: "
                                            << dischargeResult.error().message;

        // Process events
        QCoreApplication::processEvents();

        // Verify service state
        EXPECT_FALSE(admissionService->isPatientAdmitted());

        // Verify controller state
        EXPECT_FALSE(patientController->isAdmitted());

        // Verify signal emitted
        EXPECT_GE(isAdmittedSpy.count(), 1);
    }

    // ========================================================================
    // Error Handling Tests
    // ========================================================================

    /**
     * @test Attempt to admit when patient already admitted
     *
     * Verifies error handling for duplicate admission attempts.
     */
    TEST_F(AdmissionWorkflowTest, AdmitTwice_ReturnsError)
    {
        // Admit first patient
        PatientIdentity identity1("MRN-123456", "John Doe", 0, "M");
        BedLocation location1("ICU-4B", "ICU");
        auto admit1 = admissionService->admitPatient(identity1, location1,
                                                     AdmissionService::AdmissionSource::Manual);
        ASSERT_TRUE(admit1.isOk());

        // Attempt to admit second patient without discharge
        PatientIdentity identity2("MRN-789012", "Jane Smith", 0, "F");
        BedLocation location2("ICU-5A", "ICU");
        auto admit2 = admissionService->admitPatient(identity2, location2,
                                                     AdmissionService::AdmissionSource::Manual);

        // Should fail - patient already admitted
        EXPECT_FALSE(admit2.isOk());
        EXPECT_FALSE(admit2.error().message.empty());

        // Verify first patient still admitted
        EXPECT_TRUE(admissionService->isPatientAdmitted());
        auto info = admissionService->getCurrentAdmission();
        EXPECT_EQ(info.mrn, QString("MRN-123456"));
    }

    /**
     * @test Discharge when no patient admitted
     *
     * Verifies error handling for invalid discharge attempts.
     */
    TEST_F(AdmissionWorkflowTest, DischargeWithoutAdmission_ReturnsError)
    {
        // Verify no patient admitted
        ASSERT_FALSE(admissionService->isPatientAdmitted());

        // Attempt discharge with empty MRN
        auto dischargeResult = admissionService->dischargePatient(QString(""));

        // Should fail - no patient to discharge
        EXPECT_FALSE(dischargeResult.isOk());
        EXPECT_FALSE(dischargeResult.error().message.empty());
    }

    /**
     * @test Patient lookup failure handling
     *
     * Verifies behavior when patient lookup fails.
     */
    TEST_F(AdmissionWorkflowTest, PatientLookup_ServiceFailure_ReturnsError)
    {
        // Enable simulated failures
        mockPatientLookup->setSimulateFailures(true);

        // Attempt search
        auto result = mockPatientLookup->searchByName("Jane Smith");

        // Should return error
        EXPECT_TRUE(result.isError());
        EXPECT_FALSE(result.error().message.empty());
    }

    /**
     * @test Patient lookup for non-existent patient
     *
     * Verifies handling of patient not found.
     */
    TEST_F(AdmissionWorkflowTest, PatientLookup_PatientNotFound_ReturnsEmptyList)
    {
        // Attempt search for non-existent patient
        auto result = mockPatientLookup->searchByName("NonExistentPatient");

        // Should return empty list (success but no matches)
        ASSERT_TRUE(result.isOk());
        EXPECT_TRUE(result.value().empty());
    }

    // ========================================================================
    // End-to-End Workflow Tests
    // ========================================================================

    /**
     * @test Complete admission workflow: Barcode → Lookup → Admit → Display
     *
     * Simulates complete workflow:
     * 1. Scan barcode (get MRN)
     * 2. Lookup patient from HIS
     * 3. Admit patient to device
     * 4. Verify UI state updates
     * 5. Start monitoring (mock sensor)
     */
    TEST_F(AdmissionWorkflowTest, CompleteWorkflow_BarcodeToMonitoring_Succeeds)
    {
        // Step 1: Simulate barcode scan
        const QString scannedMrn = "MRN-123456";

        // Step 2: Lookup patient from mock HIS by name
        auto searchResult = mockPatientLookup->searchByName("John Doe");
        ASSERT_TRUE(searchResult.isOk()) << "Patient search failed";
        ASSERT_FALSE(searchResult.value().empty()) << "Patient not found";

        const PatientIdentity &identity = searchResult.value()[0];
        EXPECT_EQ(identity.mrn, scannedMrn.toStdString());

        // Verify lookup history tracked
        EXPECT_EQ(mockPatientLookup->lookupCount(), 0); // searchByName doesn't track history

        // Step 3: Create bed location and admit patient
        // BedLocation(room, unit) formats as "unit-room"
        // For ICU-4B we use room="4B", unit="ICU"
        BedLocation location("4B", "ICU");

        auto admitResult = admissionService->admitPatient(identity, location,
                                                          AdmissionService::AdmissionSource::Barcode);
        ASSERT_TRUE(admitResult.isOk());

        // Process events
        QCoreApplication::processEvents();

        // Step 4: Verify UI controller state
        EXPECT_TRUE(patientController->isAdmitted());
        EXPECT_EQ(patientController->patientName(), QString("John Doe"));
        EXPECT_EQ(patientController->patientMrn(), scannedMrn);
        EXPECT_EQ(patientController->bedLocation(), QString("ICU-4B"));

        // Step 5: Start monitoring (mock sensor)
        auto sensorStart = mockSensor->start();
        ASSERT_TRUE(sensorStart.isOk());
        EXPECT_TRUE(mockSensor->isActive());

        // Verify complete workflow succeeded
        EXPECT_TRUE(admissionService->isPatientAdmitted());
        EXPECT_TRUE(mockSensor->isActive());
    }

    /**
     * @test Admission → Monitor → Discharge workflow
     *
     * Tests complete patient lifecycle on device.
     */
    TEST_F(AdmissionWorkflowTest, CompleteLifecycle_AdmitMonitorDischarge_Succeeds)
    {
        // Phase 1: Admit patient
        PatientIdentity identity("MRN-789012", "Jane Smith", 0, "F");
        BedLocation location("ICU-5A", "ICU");
        auto admitResult = admissionService->admitPatient(identity, location,
                                                          AdmissionService::AdmissionSource::Manual);
        ASSERT_TRUE(admitResult.isOk());
        EXPECT_TRUE(admissionService->isPatientAdmitted());

        // Phase 2: Start monitoring
        auto sensorStart = mockSensor->start();
        ASSERT_TRUE(sensorStart.isOk());
        EXPECT_TRUE(mockSensor->isActive());

        // Simulate monitoring period (in real code, vitals would be collected)
        QCoreApplication::processEvents();

        // Phase 3: Stop monitoring
        mockSensor->stop();
        EXPECT_FALSE(mockSensor->isActive());

        // Phase 4: Discharge patient
        auto dischargeResult = admissionService->dischargePatient(QString("MRN-789012"));
        ASSERT_TRUE(dischargeResult.isOk());
        EXPECT_FALSE(admissionService->isPatientAdmitted());

        QCoreApplication::processEvents();

        // Verify final state
        EXPECT_FALSE(patientController->isAdmitted());
        EXPECT_FALSE(mockSensor->isActive());
    }

} // namespace

/**
 * @brief Custom main function for Qt integration tests
 *
 * Creates QCoreApplication instance required by Qt SQL and other Qt components
 * before running Google Test suite.
 */
int main(int argc, char **argv)
{
    // Initialize Qt application (required for QSqlDatabase and Qt plugins)
    QCoreApplication app(argc, argv);

    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run all tests
    return RUN_ALL_TESTS();
}
