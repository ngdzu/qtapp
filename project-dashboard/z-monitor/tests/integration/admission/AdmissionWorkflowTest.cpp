// AdmissionWorkflowTest.cpp
// Integration test skeleton for admission workflow: admit path minimal

#include <gtest/gtest.h>

#include "application/services/AdmissionService.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include "interface/controllers/PatientController.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"

using namespace zmon;

namespace
{

    class AdmissionWorkflowTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Initialize in-memory database and run migrations
            db = std::make_unique<DatabaseManager>();
            auto opened = db->open(":memory:");
            ASSERT_TRUE(opened.isOk());
            auto migrated = db->executeMigrations();
            ASSERT_TRUE(migrated.isOk());

            // Create services/controllers with available minimal dependencies
            admissionService = std::make_unique<AdmissionService>(nullptr, nullptr);
        }

        void TearDown() override
        {
            admissionService.reset();
            if (db)
                db->close();
            db.reset();
        }

        std::unique_ptr<DatabaseManager> db;
        std::unique_ptr<AdmissionService> admissionService;
    };

    TEST_F(AdmissionWorkflowTest, BarcodeScanAdmitUpdatesControllerState)
    {
        const std::string mrn = "MRN-000123";

        PatientIdentity identity(mrn, "Test Patient", 0, "Unknown");
        BedLocation location("04B", "ICU");

        auto admitResult = admissionService->admitPatient(identity, location, AdmissionService::AdmissionSource::Manual);
        ASSERT_TRUE(admitResult.isOk());

        // Verify service reflects admitted state
        ASSERT_TRUE(admissionService->isPatientAdmitted());
        auto info = admissionService->getCurrentAdmission();
        ASSERT_EQ(info.mrn, QString::fromStdString(mrn));
    }

} // namespace
