/**
 * @file DashboardControllerTest.cpp
 * @brief GoogleTest unit tests for DashboardController.
 *
 * These tests verify DashboardController's integration with MonitoringService,
 * VitalsCache, signal/slot connections, and Q_PROPERTY updates.
 *
 * @author Z Monitor Team
 * @date 2025-11-30
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <memory>

#include "ui/controllers/DashboardController.h"
#include "application/services/MonitoringService.h"
#include "infrastructure/caching/VitalsCache.h"
#include "infrastructure/caching/WaveformCache.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"

using namespace zmon;

/**
 * @brief Mock MonitoringService for testing.
 *
 * Provides minimal MonitoringService implementation for DashboardController tests.
 * Allows triggering signals and providing test patient data.
 */
class MockMonitoringService : public MonitoringService
{
public:
    MockMonitoringService()
        : MonitoringService(
              nullptr, nullptr, nullptr, nullptr, nullptr,
              std::make_shared<VitalsCache>(),
              std::make_shared<WaveformCache>(),
              nullptr)
    {
    }

    void emitVitalsUpdated() { emit vitalsUpdated(); }
    void emitAlarmRaised(const QString &id, const QString &type, int priority)
    {
        emit alarmRaised(id, type, priority);
    }

    void setTestPatient(std::shared_ptr<PatientAggregate> patient)
    {
        m_testPatient = patient;
    }

    std::shared_ptr<PatientAggregate> getCurrentPatient() const override
    {
        return m_testPatient;
    }

private:
    std::shared_ptr<PatientAggregate> m_testPatient;
};

/**
 * @brief Test fixture for DashboardController tests.
 */
class DashboardControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_vitalsCache = std::make_shared<VitalsCache>();
        m_monitoringService = new MockMonitoringService();
        m_controller = new DashboardController(m_monitoringService, m_vitalsCache.get());
    }

    void TearDown() override
    {
        delete m_controller;
        delete m_monitoringService;
    }

    std::shared_ptr<VitalsCache> m_vitalsCache;
    MockMonitoringService *m_monitoringService;
    DashboardController *m_controller;
};

/**
 * @brief Test that controller initializes with correct default values.
 */
TEST_F(DashboardControllerTest, InitializesWithDefaults)
{
    EXPECT_EQ(m_controller->heartRate(), 0);
    EXPECT_EQ(m_controller->spo2(), 0);
    EXPECT_EQ(m_controller->respiratoryRate(), 0);
    EXPECT_EQ(m_controller->bloodPressure(), QString(""));
    EXPECT_EQ(m_controller->temperature(), 0.0);
    EXPECT_EQ(m_controller->hasActiveAlarms(), false);
    EXPECT_EQ(m_controller->isMonitoring(), true); // Started when service exists
    EXPECT_EQ(m_controller->patientName(), QString(""));
    EXPECT_EQ(m_controller->patientMrn(), QString(""));
}

/**
 * @brief Test that vitals are updated when vitalsUpdated signal is emitted.
 */
TEST_F(DashboardControllerTest, UpdatesVitalsFromCache)
{
    // Add test vitals to cache
    VitalRecord hrVital{"HR", 75.0, 1234567890000, 100, "TEST-MRN", "TEST-DEVICE"};
    VitalRecord spo2Vital{"SPO2", 98.0, 1234567890000, 100, "TEST-MRN", "TEST-DEVICE"};
    VitalRecord rrVital{"RR", 16.0, 1234567890000, 100, "TEST-MRN", "TEST-DEVICE"};
    VitalRecord tempVital{"TEMP", 37.2, 1234567890000, 100, "TEST-MRN", "TEST-DEVICE"};
    VitalRecord sysVital{"NIBP_SYS", 120.0, 1234567890000, 100, "TEST-MRN", "TEST-DEVICE"};
    VitalRecord diaVital{"NIBP_DIA", 80.0, 1234567890000, 100, "TEST-MRN", "TEST-DEVICE"};

    m_vitalsCache->append(hrVital);
    m_vitalsCache->append(spo2Vital);
    m_vitalsCache->append(rrVital);
    m_vitalsCache->append(tempVital);
    m_vitalsCache->append(sysVital);
    m_vitalsCache->append(diaVital);

    // Create signal spies
    QSignalSpy hrSpy(m_controller, &DashboardController::heartRateChanged);
    QSignalSpy spo2Spy(m_controller, &DashboardController::spo2Changed);
    QSignalSpy rrSpy(m_controller, &DashboardController::respiratoryRateChanged);
    QSignalSpy tempSpy(m_controller, &DashboardController::temperatureChanged);
    QSignalSpy bpSpy(m_controller, &DashboardController::bloodPressureChanged);

    // Trigger vitals update
    m_monitoringService->emitVitalsUpdated();

    // Verify signals emitted
    EXPECT_EQ(hrSpy.count(), 1);
    EXPECT_EQ(spo2Spy.count(), 1);
    EXPECT_EQ(rrSpy.count(), 1);
    EXPECT_EQ(tempSpy.count(), 1);
    EXPECT_EQ(bpSpy.count(), 1);

    // Verify property values updated
    EXPECT_EQ(m_controller->heartRate(), 75);
    EXPECT_EQ(m_controller->spo2(), 98);
    EXPECT_EQ(m_controller->respiratoryRate(), 16);
    EXPECT_EQ(m_controller->temperature(), 37.2);
    EXPECT_EQ(m_controller->bloodPressure(), QString("120/80"));
}

/**
 * @brief Test that alarm state is updated when alarmRaised signal is emitted.
 */
TEST_F(DashboardControllerTest, UpdatesAlarmState)
{
    // Initially no alarms
    EXPECT_EQ(m_controller->hasActiveAlarms(), false);

    // Create signal spy
    QSignalSpy alarmSpy(m_controller, &DashboardController::hasActiveAlarmsChanged);

    // Trigger alarm raised
    m_monitoringService->emitAlarmRaised("ALARM-001", "HR_HIGH", 2);

    // Verify signal emitted and property updated
    EXPECT_EQ(alarmSpy.count(), 1);
    EXPECT_EQ(m_controller->hasActiveAlarms(), true);

    // Trigger another alarm (should not emit signal again since already true)
    m_monitoringService->emitAlarmRaised("ALARM-002", "SPO2_LOW", 3);
    EXPECT_EQ(alarmSpy.count(), 1); // No new signal
    EXPECT_EQ(m_controller->hasActiveAlarms(), true);
}

/**
 * @brief Test that patient info is updated when patient is admitted.
 */
TEST_F(DashboardControllerTest, UpdatesPatientInfo)
{
    // Create test patient
    PatientIdentity identity{
        "TEST-MRN-123",
        "John Doe",
        0,
        "M",
        {}};
    BedLocation bedLocation{"ICU-101", "ICU"};

    auto patient = std::make_shared<PatientAggregate>();
    auto admitResult = patient->admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk()) << "Failed to admit patient: " << admitResult.error().message;

    // Set test patient in mock service
    m_monitoringService->setTestPatient(patient);

    // Create signal spies
    QSignalSpy nameSpy(m_controller, &DashboardController::patientNameChanged);
    QSignalSpy mrnSpy(m_controller, &DashboardController::patientMrnChanged);

    // Trigger patient changed manually (in real app, MonitoringService would emit patientChanged signal)
    m_controller->onPatientChanged();

    // Give Qt event loop time to process property updates
    QCoreApplication::processEvents();

    // Verify signals emitted
    EXPECT_EQ(nameSpy.count(), 1);
    EXPECT_EQ(mrnSpy.count(), 1);

    // Verify property values updated
    EXPECT_EQ(m_controller->patientName(), QString("John Doe"));
    EXPECT_EQ(m_controller->patientMrn(), QString("TEST-MRN-123"));
}

/**
 * @brief Test that patient info is cleared when patient is discharged.
 */
TEST_F(DashboardControllerTest, ClearsPatientInfoOnDischarge)
{
    // First admit a patient
    PatientIdentity identity{
        "TEST-MRN-123",
        "John Doe",
        0,
        "M",
        {}};
    BedLocation bedLocation{"ICU-101", "ICU"};

    auto patient = std::make_shared<PatientAggregate>();
    auto admitResult = patient->admit(identity, bedLocation, "manual");
    ASSERT_TRUE(admitResult.isOk()) << "Failed to admit patient: " << admitResult.error().message;

    m_monitoringService->setTestPatient(patient);
    m_controller->onPatientChanged(); // Trigger initial patient load

    EXPECT_EQ(m_controller->patientName(), QString("John Doe"));
    EXPECT_EQ(m_controller->patientMrn(), QString("TEST-MRN-123"));

    // Now discharge patient
    patient->discharge();

    // Create signal spies
    QSignalSpy nameSpy(m_controller, &DashboardController::patientNameChanged);
    QSignalSpy mrnSpy(m_controller, &DashboardController::patientMrnChanged);

    // Trigger patient changed manually (in real app, MonitoringService would emit signal on discharge)
    m_controller->onPatientChanged();

    // Verify signals emitted (patient info cleared)
    EXPECT_EQ(nameSpy.count(), 1);
    EXPECT_EQ(mrnSpy.count(), 1);

    // Verify properties cleared
    EXPECT_EQ(m_controller->patientName(), QString(""));
    EXPECT_EQ(m_controller->patientMrn(), QString(""));
}

/**
 * @brief Test null service handling.
 */
TEST_F(DashboardControllerTest, HandlesNullService)
{
    // Create controller with null service
    DashboardController controller(nullptr, nullptr);

    // Verify defaults
    EXPECT_EQ(controller.heartRate(), 0);
    EXPECT_EQ(controller.isMonitoring(), false); // No service = not monitoring

    // Trigger slots (should not crash)
    controller.onVitalsUpdated();
    controller.onPatientChanged();
    controller.onAlarmStateChanged();
}

/**
 * @brief Test that vitals don't update if cache is null.
 */
TEST_F(DashboardControllerTest, HandlesNullCache)
{
    // Create controller with null cache
    DashboardController controller(m_monitoringService, nullptr);

    // Create signal spy
    QSignalSpy hrSpy(&controller, &DashboardController::heartRateChanged);

    // Trigger vitals update
    m_monitoringService->emitVitalsUpdated();

    // Verify no signals emitted (no cache to read from)
    EXPECT_EQ(hrSpy.count(), 0);
    EXPECT_EQ(controller.heartRate(), 0);
}

/**
 * @brief Main test runner.
 */
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
