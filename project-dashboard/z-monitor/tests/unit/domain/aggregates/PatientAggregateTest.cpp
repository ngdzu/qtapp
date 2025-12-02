/**
 * @file PatientAggregateTest.cpp
 * @brief Unit tests for PatientAggregate domain aggregate.
 *
 * This file contains unit tests that verify PatientAggregate business rules
 * and state transitions including:
 * - Cannot admit when already admitted
 * - Cannot discharge when not admitted
 * - Cannot transfer when not admitted
 * - Vitals only recorded for admitted patients
 * - MRN validation in vitals
 * - State transitions (NotAdmitted → Admitted → Discharged)
 *
 * @author Z Monitor Team
 * @date 2025-12-01
 */

#include <gtest/gtest.h>
#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/common/Result.h"

using namespace zmon;

/**
 * @class PatientAggregateTest
 * @brief Test fixture for PatientAggregate tests.
 */
class PatientAggregateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Test data will be created inline in each test
        // (value objects are immutable with const members)
    }
};

/**
 * @test Verify initial state is NotAdmitted
 */
TEST_F(PatientAggregateTest, InitialStateIsNotAdmitted)
{
    PatientAggregate aggregate;

    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::NotAdmitted);
    EXPECT_FALSE(aggregate.isAdmitted());
    EXPECT_EQ(aggregate.getPatientMrn(), "");
    EXPECT_EQ(aggregate.getAdmittedAt(), 0);
    EXPECT_EQ(aggregate.getDischargedAt(), 0);
}

/**
 * @test Verify successful patient admission
 */
TEST_F(PatientAggregateTest, AdmitPatientSucceeds)
{
    PatientAggregate aggregate;

    PatientIdentity identity("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");

    auto result = aggregate.admit(identity, bedLocation, "manual");

    ASSERT_TRUE(result.isOk()) << "Admission should succeed";
    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::Admitted);
    EXPECT_TRUE(aggregate.isAdmitted());
    EXPECT_EQ(aggregate.getPatientMrn(), "MRN12345");
    EXPECT_GT(aggregate.getAdmittedAt(), 0) << "Admitted timestamp should be set";
    EXPECT_EQ(aggregate.getDischargedAt(), 0) << "Not yet discharged";
    EXPECT_EQ(aggregate.getPatientIdentity().mrn, "MRN12345");
    EXPECT_EQ(aggregate.getBedLocation().location, "ICU-101");
}

/**
 * @test Verify business rule: Cannot admit when already admitted
 */
TEST_F(PatientAggregateTest, CannotAdmitWhenAlreadyAdmitted)
{
    PatientAggregate aggregate;

    // First admission succeeds
    PatientIdentity identity1("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");
    auto result1 = aggregate.admit(identity1, bedLocation, "manual");
    ASSERT_TRUE(result1.isOk());

    // Second admission fails (business rule violation)
    PatientIdentity differentPatient("MRN99999", "Jane Smith", 946684800000, "F", {});
    auto result2 = aggregate.admit(differentPatient, bedLocation, "manual");

    ASSERT_TRUE(result2.isError()) << "Should not allow second admission";
    EXPECT_EQ(result2.error().code, ErrorCode::Conflict);
    EXPECT_EQ(aggregate.getPatientMrn(), "MRN12345") << "Original patient should remain";
}

/**
 * @test Verify business rule: Cannot discharge when not admitted
 */
TEST_F(PatientAggregateTest, CannotDischargeWhenNotAdmitted)
{
    PatientAggregate aggregate;

    auto result = aggregate.discharge();

    ASSERT_TRUE(result.isError()) << "Cannot discharge when not admitted";
    EXPECT_EQ(result.error().code, ErrorCode::NotFound);
    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::NotAdmitted);
}

/**
 * @test Verify successful patient discharge
 */
TEST_F(PatientAggregateTest, DischargePatientSucceeds)
{
    PatientAggregate aggregate;

    // Admit patient first
    PatientIdentity identity("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");
    auto admitResult = aggregate.admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk());

    // Discharge patient
    auto dischargeResult = aggregate.discharge();

    ASSERT_TRUE(dischargeResult.isOk()) << "Discharge should succeed";
    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::Discharged);
    EXPECT_FALSE(aggregate.isAdmitted());
    EXPECT_GT(aggregate.getDischargedAt(), 0) << "Discharged timestamp should be set";
    EXPECT_GE(aggregate.getDischargedAt(), aggregate.getAdmittedAt())
        << "Discharge time should be >= admission time";
}

/**
 * @test Verify business rule: Cannot transfer when not admitted
 */
TEST_F(PatientAggregateTest, CannotTransferWhenNotAdmitted)
{
    PatientAggregate aggregate;

    auto result = aggregate.transfer("DEVICE-002");

    ASSERT_TRUE(result.isError()) << "Cannot transfer when not admitted";
    EXPECT_EQ(result.error().code, ErrorCode::NotFound);
}

/**
 * @test Verify successful patient transfer
 */
TEST_F(PatientAggregateTest, TransferPatientSucceeds)
{
    PatientAggregate aggregate;

    // Admit patient first
    PatientIdentity identity("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");
    auto admitResult = aggregate.admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk());

    // Transfer patient
    auto transferResult = aggregate.transfer("DEVICE-002");

    ASSERT_TRUE(transferResult.isOk()) << "Transfer should succeed";
    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::Discharged);
    EXPECT_FALSE(aggregate.isAdmitted());
    EXPECT_GT(aggregate.getDischargedAt(), 0) << "Discharged timestamp should be set on transfer";
}

/**
 * @test Verify business rule: Cannot transfer to empty device
 */
TEST_F(PatientAggregateTest, CannotTransferToEmptyDevice)
{
    PatientAggregate aggregate;

    PatientIdentity identity("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");
    auto admitResult = aggregate.admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk());

    auto transferResult = aggregate.transfer("");

    ASSERT_TRUE(transferResult.isError()) << "Cannot transfer to empty device";
    EXPECT_EQ(transferResult.error().code, ErrorCode::InvalidArgument);
    EXPECT_TRUE(aggregate.isAdmitted()) << "Patient should still be admitted";
}

/**
 * @test Verify business rule: Cannot record vitals when not admitted
 */
TEST_F(PatientAggregateTest, CannotRecordVitalsWhenNotAdmitted)
{
    PatientAggregate aggregate;

    VitalRecord vital("HR", 75.0, 1000000, 100, "MRN12345", "DEV-001");

    auto result = aggregate.updateVitals(vital);

    ASSERT_TRUE(result.isError()) << "Cannot record vitals when not admitted";
    EXPECT_EQ(result.error().code, ErrorCode::NotFound);
}

/**
 * @test Verify business rule: Vitals MRN must match admitted patient
 */
TEST_F(PatientAggregateTest, VitalsMustMatchAdmittedPatient)
{
    PatientAggregate aggregate;

    PatientIdentity identity("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");
    auto admitResult = aggregate.admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk());

    // Create vital with different MRN
    VitalRecord vital("HR", 75.0, 1000000, 100, "WRONG_MRN", "DEV-001");

    auto result = aggregate.updateVitals(vital);

    ASSERT_TRUE(result.isError()) << "Vitals MRN must match admitted patient";
    EXPECT_EQ(result.error().code, ErrorCode::Conflict);
}

/**
 * @test Verify successful vitals recording
 */
TEST_F(PatientAggregateTest, RecordVitalsSucceeds)
{
    PatientAggregate aggregate;

    PatientIdentity identity("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");
    auto admitResult = aggregate.admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk());

    // Create valid vital
    VitalRecord vital("HR", 75.0, 1000000, 100, "MRN12345", "DEV-001");

    auto result = aggregate.updateVitals(vital);

    ASSERT_TRUE(result.isOk()) << "Recording vitals should succeed";

    // Verify vitals are stored
    auto recentVitals = aggregate.getRecentVitals(10);
    ASSERT_EQ(recentVitals.size(), 1);
    EXPECT_EQ(recentVitals[0].patientMrn, "MRN12345");
    EXPECT_EQ(recentVitals[0].vitalType, "HR");
    EXPECT_EQ(recentVitals[0].value, 75.0);
}

/**
 * @test Verify vitals history is maintained
 */
TEST_F(PatientAggregateTest, VitalsHistoryMaintained)
{
    PatientAggregate aggregate;

    PatientIdentity identity("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");
    auto admitResult = aggregate.admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk());

    // Record multiple vitals
    for (int i = 0; i < 5; i++)
    {
        VitalRecord vital("HR", 70.0 + i, 1000000 + (i * 1000), 100, "MRN12345", "DEV-001");

        auto result = aggregate.updateVitals(vital);
        ASSERT_TRUE(result.isOk());
    }

    // Verify all vitals are stored
    auto recentVitals = aggregate.getRecentVitals(10);
    ASSERT_EQ(recentVitals.size(), 5);

    // Verify vitals are in order (most recent first)
    EXPECT_EQ(recentVitals[4].value, 74.0); // Last recorded
    EXPECT_EQ(recentVitals[0].value, 70.0); // First recorded
}

/**
 * @test Verify state transition: NotAdmitted → Admitted → Discharged
 */
TEST_F(PatientAggregateTest, StateTransitionWorkflow)
{
    PatientAggregate aggregate;

    // Initial state: NotAdmitted
    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::NotAdmitted);

    // Transition to Admitted
    PatientIdentity identity("MRN12345", "John Doe", 946684800000, "M", {"Penicillin"});
    BedLocation bedLocation("ICU-101", "ICU");
    auto admitResult = aggregate.admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk());
    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::Admitted);

    // Transition to Discharged
    auto dischargeResult = aggregate.discharge();
    ASSERT_TRUE(dischargeResult.isOk());
    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::Discharged);
}

/**
 * @test Verify invalid patient identity is rejected
 */
TEST_F(PatientAggregateTest, InvalidPatientIdentityRejected)
{
    PatientAggregate aggregate;

    // Create invalid identity (empty MRN)
    PatientIdentity invalidIdentity("", "John Doe", 946684800000, "M", {});
    BedLocation bedLocation("ICU-101", "ICU");

    auto result = aggregate.admit(invalidIdentity, bedLocation, "manual");

    ASSERT_TRUE(result.isError()) << "Invalid patient identity should be rejected";
    EXPECT_EQ(result.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(aggregate.getAdmissionState(), AdmissionState::NotAdmitted);
}
