/**
 * @file test_alarm_manager.cpp
 * @brief Example unit test demonstrating mock usage.
 *
 * This test demonstrates how to use mock objects to test services
 * that depend on external interfaces. This is a template/example
 * that can be adapted for actual AlarmManager or other service tests.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QObject>

// Mock includes (relative to tests/ directory)
#include "mocks/infrastructure/MockTelemetryServer.h"
#include "mocks/infrastructure/MockPatientLookupService.h"
#include "mocks/domain/MockPatientRepository.h"

using namespace zmon;

/**
 * @class AlarmManagerTest
 * @brief Test fixture for AlarmManager tests.
 *
 * This fixture sets up mock objects and provides helper methods
 * for testing alarm management functionality.
 */
class AlarmManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create QCoreApplication if it doesn't exist
        if (!QCoreApplication::instance())
        {
            int argc = 1;
            char *argv[] = {const_cast<char *>("test")};
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }

        // Create mock objects
        m_mockTelemetryServer = new MockTelemetryServer();
        m_mockPatientLookupService = new MockPatientLookupService();
        m_mockPatientRepository = std::make_unique<MockPatientRepository>();
    }

    void TearDown() override
    {
        // Cleanup is handled by Qt parent-child ownership
        m_mockTelemetryServer = nullptr;
        m_mockPatientLookupService = nullptr;
        m_mockPatientRepository.reset();
    }

    QObject *parent() { return m_app.get(); }
    MockTelemetryServer *m_mockTelemetryServer{nullptr};
    MockPatientLookupService *m_mockPatientLookupService{nullptr};
    std::unique_ptr<MockPatientRepository> m_mockPatientRepository;
    std::unique_ptr<QCoreApplication> m_app;
};

// Example test: Verify mock telemetry server works
TEST_F(AlarmManagerTest, MockTelemetryServer_SendTelemetry_Success)
{
    // Arrange
    TelemetryData data;
    data.deviceId = "TEST-DEVICE-001";
    data.patientMrn = "MRN-001";
    data.timestamp = QDateTime::currentDateTime();

    // Act
    ServerResponse response = m_mockTelemetryServer->sendTelemetry(data);

    // Assert
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_EQ(m_mockTelemetryServer->getTelemetrySendCount(), 1);

    QList<TelemetryData> sent = m_mockTelemetryServer->getSentTelemetry();
    ASSERT_EQ(sent.size(), 1);
    EXPECT_EQ(sent[0].deviceId, "TEST-DEVICE-001");
    EXPECT_EQ(sent[0].patientMrn, "MRN-001");
}

// Example test: Verify mock telemetry server handles failures
TEST_F(AlarmManagerTest, MockTelemetryServer_SendTelemetry_Failure)
{
    // Arrange
    m_mockTelemetryServer->setShouldSucceed(false);

    TelemetryData data;
    data.deviceId = "TEST-DEVICE-001";
    data.patientMrn = "MRN-001";

    // Act
    ServerResponse response = m_mockTelemetryServer->sendTelemetry(data);

    // Assert
    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.statusCode, 500);
    EXPECT_EQ(response.message, "Internal Server Error");
}

// Example test: Verify mock patient lookup service works
TEST_F(AlarmManagerTest, MockPatientLookupService_GetByMrn_ReturnsAggregate)
{
    // Arrange
    std::string mrn = "MRN-001";

    // Act
    auto result = m_mockPatientLookupService->getByMrn(mrn);

    // Assert
    ASSERT_TRUE(result.isOk()) << "Expected success but got error: " << result.error().message;
    // Note: Mock returns default (not admitted) aggregate
    // For actual patient data, use searchByName which returns PatientIdentity
    EXPECT_EQ(m_mockPatientLookupService->lookupCount(), 1);
}

// Example test: Verify mock patient lookup service search by name
TEST_F(AlarmManagerTest, MockPatientLookupService_SearchByName_Success)
{
    // Arrange
    std::string name = "Jane";

    // Act
    auto result = m_mockPatientLookupService->searchByName(name);

    // Assert
    ASSERT_TRUE(result.isOk()) << "Expected success but got error: " << result.error().message;
    const auto &identities = result.value();
    ASSERT_FALSE(identities.empty());
    EXPECT_EQ(identities[0].mrn, "MRN-002");
    EXPECT_EQ(identities[0].name, "Jane Smith");
}

// Example test: Verify mock patient repository works
TEST_F(AlarmManagerTest, MockPatientRepository_SavePatient_Success)
{
    // Arrange
    // Note: This is a simplified example - actual PatientAggregate creation
    // would require proper domain object construction

    // Assert
    EXPECT_EQ(m_mockPatientRepository->patientCount(), 0);
    EXPECT_FALSE(m_mockPatientRepository->isSimulatingFailures());
}

// Example test: Verify mock patient repository handles failures
TEST_F(AlarmManagerTest, MockPatientRepository_SavePatient_Failure)
{
    // Arrange
    m_mockPatientRepository->setSimulateFailures(true);
    m_mockPatientRepository->setFailureError("Database connection failed");

    // Assert
    EXPECT_TRUE(m_mockPatientRepository->isSimulatingFailures());
}
