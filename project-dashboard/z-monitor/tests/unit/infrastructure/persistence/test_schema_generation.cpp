/**
 * @file test_schema_generation.cpp
 * @brief Unit tests for schema generation and SchemaInfo.h constants.
 */

#include <gtest/gtest.h>
#include "infrastructure/persistence/generated/SchemaInfo.h"
#include <string>

using namespace Schema;

/**
 * @test Verify table name constants are defined
 */
TEST(SchemaInfoTest, TableConstantsExist) {
    EXPECT_NE(Tables::PATIENTS, nullptr);
    EXPECT_NE(Tables::VITALS, nullptr);
    EXPECT_NE(Tables::ACTION_LOG, nullptr);
    EXPECT_NE(Tables::ALARMS, nullptr);
    EXPECT_NE(Tables::SETTINGS, nullptr);
    EXPECT_NE(Tables::USERS, nullptr);
}

/**
 * @test Verify table name constants have correct values
 */
TEST(SchemaInfoTest, TableConstantsValues) {
    EXPECT_STREQ(Tables::PATIENTS, "patients");
    EXPECT_STREQ(Tables::VITALS, "vitals");
    EXPECT_STREQ(Tables::ACTION_LOG, "action_log");
    EXPECT_STREQ(Tables::ALARMS, "alarms");
    EXPECT_STREQ(Tables::SETTINGS, "settings");
    EXPECT_STREQ(Tables::USERS, "users");
}

/**
 * @test Verify ActionLog column constants exist
 */
TEST(SchemaInfoTest, ActionLogColumnConstantsExist) {
    EXPECT_NE(Columns::ActionLog::ID, nullptr);
    EXPECT_NE(Columns::ActionLog::TIMESTAMP_MS, nullptr);
    EXPECT_NE(Columns::ActionLog::ACTION_TYPE, nullptr);
    EXPECT_NE(Columns::ActionLog::USER_ID, nullptr);
    EXPECT_NE(Columns::ActionLog::TARGET_TYPE, nullptr);
    EXPECT_NE(Columns::ActionLog::TARGET_ID, nullptr);
    EXPECT_NE(Columns::ActionLog::RESULT, nullptr);
    EXPECT_NE(Columns::ActionLog::DEVICE_ID, nullptr);
}

/**
 * @test Verify ActionLog column constants have correct values
 */
TEST(SchemaInfoTest, ActionLogColumnConstantsValues) {
    EXPECT_STREQ(Columns::ActionLog::ID, "id");
    EXPECT_STREQ(Columns::ActionLog::TIMESTAMP_MS, "timestamp_ms");
    EXPECT_STREQ(Columns::ActionLog::ACTION_TYPE, "action_type");
    EXPECT_STREQ(Columns::ActionLog::USER_ID, "user_id");
    EXPECT_STREQ(Columns::ActionLog::TARGET_TYPE, "target_type");
    EXPECT_STREQ(Columns::ActionLog::TARGET_ID, "target_id");
    EXPECT_STREQ(Columns::ActionLog::RESULT, "result");
    EXPECT_STREQ(Columns::ActionLog::DEVICE_ID, "device_id");
}

/**
 * @test Verify Patients column constants exist
 */
TEST(SchemaInfoTest, PatientsColumnConstantsExist) {
    EXPECT_NE(Columns::Patients::MRN, nullptr);
    EXPECT_NE(Columns::Patients::NAME, nullptr);
    EXPECT_NE(Columns::Patients::DOB, nullptr);
    EXPECT_NE(Columns::Patients::SEX, nullptr);
}

/**
 * @test Verify Patients column constants have correct values
 */
TEST(SchemaInfoTest, PatientsColumnConstantsValues) {
    EXPECT_STREQ(Columns::Patients::MRN, "mrn");
    EXPECT_STREQ(Columns::Patients::NAME, "name");
    EXPECT_STREQ(Columns::Patients::DOB, "dob");
    EXPECT_STREQ(Columns::Patients::SEX, "sex");
}

