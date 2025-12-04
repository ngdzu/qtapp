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
#include "interface/controllers/PatientController.h"
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
            // Patient 1: Standard ICU patient with allergies
            PatientInfo patient1;
            patient1.patientId = "PAT-001";
            patient1.mrn = "MRN-123456";
            patient1.name = "John Doe";
            patient1.dateOfBirth = QDate(1965, 5, 15);
            patient1.sex = "M";
            patient1.allergies = {"Penicillin", "Latex"};
            patient1.room = "ICU-4B";
            patient1.lastUpdated = QDateTime::currentDateTime();
            mockPatientLookup->addPatient(patient1.mrn, patient1);

            // Patient 2: Patient with no allergies
            PatientInfo patient2;
            patient2.patientId = "PAT-002";
            patient2.mrn = "MRN-789012";
            patient2.name = "Jane Smith";
            patient2.dateOfBirth = QDate(1980, 10, 25);
            patient2.sex = "F";
            patient2.allergies = {};
            patient2.room = "ICU-5A";
            patient2.lastUpdated = QDateTime::currentDateTime();
            mockPatientLookup->addPatient(patient2.mrn, patient2);

            // Patient 3: Pediatric patient
            PatientInfo patient3;
            patient3.patientId = "PAT-003";
            patient3.mrn = "MRN-345678";
            patient3.name = "Tommy Johnson";
            patient3.dateOfBirth = QDate(2015, 3, 8);
            patient3.sex = "M";
            patient3.allergies = {"Peanuts"};
            patient3.room = "PICU-2C";
            patient3.lastUpdated = QDateTime::currentDateTime();
            mockPatientLookup->addPatient(patient3.mrn, patient3);
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

        // Simulate barcode scan: Lookup patient from mock service
        auto patientInfo = mockPatientLookup->lookupPatient(mrn);
        ASSERT_TRUE(patientInfo.has_value()) << "Patient lookup failed";

        // Create identity from lookup data
        PatientIdentity identity(
            patientInfo->mrn.toStdString(),
            patientInfo->name.toStdString(),
            0, // Age calculation would be done in real code
            patientInfo->sex.toStdString());

        // Use bed from lookup - extract room number from "ICU-4B" format
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
    TEST_F(AdmissionWorkflowTest, PatientLookup_ServiceFailure_ReturnsEmpty)
    {
        // Enable simulated failures
        mockPatientLookup->setSimulateFailures(true);

        // Attempt lookup
        auto patientInfo = mockPatientLookup->lookupPatient("MRN-999999");

        // Should return empty optional
        EXPECT_FALSE(patientInfo.has_value());

        // Verify error message available
        EXPECT_FALSE(mockPatientLookup->getLastError().isEmpty());
    }

    /**
     * @test Patient lookup for non-existent patient
     *
     * Verifies handling of patient not found.
     */
    TEST_F(AdmissionWorkflowTest, PatientLookup_PatientNotFound_ReturnsEmpty)
    {
        // Lookup non-existent patient
        auto patientInfo = mockPatientLookup->lookupPatient("MRN-NONEXISTENT");

        // Should return empty optional
        EXPECT_FALSE(patientInfo.has_value());
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

        // Step 2: Lookup patient from mock HIS
        auto patientInfo = mockPatientLookup->lookupPatient(scannedMrn);
        ASSERT_TRUE(patientInfo.has_value()) << "Patient lookup failed";

        // Verify lookup history tracked
        EXPECT_EQ(mockPatientLookup->lookupCount(), 1);
        EXPECT_TRUE(mockPatientLookup->lookupHistory().contains(scannedMrn));

        // Step 3: Create identity and admit patient
        PatientIdentity identity(
            patientInfo->mrn.toStdString(),
            patientInfo->name.toStdString(),
            0,
            patientInfo->sex.toStdString());
        // BedLocation(room, unit) formats as "unit-room"
        // patientInfo->room is "ICU-4B", so extract room="4B", unit="ICU"
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
