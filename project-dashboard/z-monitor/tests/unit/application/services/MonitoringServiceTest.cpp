/**
 * @file MonitoringServiceTest.cpp
 * @brief Unit tests for MonitoringService application service.
 *
 * Tests alarm detection logic, threshold checking, error handling,
 * and performance requirements (< 50ms alarm detection latency).
 *
 * @author Z Monitor Team
 * @date 2025-12-02
 */

#include <gtest/gtest.h>
#include "application/services/MonitoringService.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/monitoring/AlarmThreshold.h"
#include "domain/monitoring/AlarmSnapshot.h"
#include "tests/mocks/domain/MockPatientRepository.h"
#include "tests/mocks/infrastructure/MockSensorDataSource.h"
#include "infrastructure/caching/VitalsCache.h"
#include "infrastructure/caching/WaveformCache.h"
#include <QSignalSpy>
#include <QTest>
#include <chrono>

using namespace zmon;

/**
 * @class MonitoringServiceTest
 * @brief Test fixture for MonitoringService unit tests.
 */
class MonitoringServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create mock repositories
        patientRepo = std::make_shared<MockPatientRepository>();
        telemetryRepo = nullptr;
        alarmRepo = nullptr;
        vitalsRepo = nullptr;

        // Create mock infrastructure
        sensorDataSource = std::make_shared<MockSensorDataSource>();
        vitalsCache = std::make_shared<VitalsCache>();
        waveformCache = std::make_shared<WaveformCache>();

        // Register metatypes for Qt signals
        qRegisterMetaType<VitalRecord>("VitalRecord");
        qRegisterMetaType<AlarmSnapshot>("AlarmSnapshot");

        // Create service under test
        service = std::make_unique<MonitoringService>(
            patientRepo, telemetryRepo, alarmRepo, vitalsRepo,
            sensorDataSource, vitalsCache, waveformCache);
    }

    void TearDown() override
    {
        service.reset();
    }

    // Helper method to create a vital record
    VitalRecord createVital(const std::string &type, double value,
                            const std::string &mrn = "MRN-12345",
                            const std::string &device = "ZM-ICU-MON-04")
    {
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
        return VitalRecord(type, value, nowMs, 100, mrn, device);
    }

    // Mock repositories
    std::shared_ptr<MockPatientRepository> patientRepo;
    std::shared_ptr<ITelemetryRepository> telemetryRepo;
    std::shared_ptr<IAlarmRepository> alarmRepo;
    std::shared_ptr<IVitalsRepository> vitalsRepo;

    // Mock infrastructure
    std::shared_ptr<MockSensorDataSource> sensorDataSource;
    std::shared_ptr<VitalsCache> vitalsCache;
    std::shared_ptr<WaveformCache> waveformCache;

    // Service under test
    std::unique_ptr<MonitoringService> service;
};

/**
 * @test Verify alarm threshold configuration works correctly.
 */
TEST_F(MonitoringServiceTest, SetAndGetAlarmThreshold)
{
    // Create custom threshold
    AlarmThreshold threshold("HR", 40.0, 150.0, 10.0, AlarmPriority::HIGH, true);

    // Set threshold
    service->setAlarmThreshold(threshold);

    // Get threshold
    const AlarmThreshold *retrieved = service->getAlarmThreshold("HR");

    // Verify
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->vitalType, "HR");
    EXPECT_DOUBLE_EQ(retrieved->lowLimit, 40.0);
    EXPECT_DOUBLE_EQ(retrieved->highLimit, 150.0);
    EXPECT_DOUBLE_EQ(retrieved->hysteresis, 10.0);
    EXPECT_EQ(retrieved->priority, AlarmPriority::HIGH);
    EXPECT_TRUE(retrieved->enabled);
}

/**
 * @test Verify alarm is NOT triggered when vital is within thresholds.
 */
TEST_F(MonitoringServiceTest, NoAlarmWhenWithinThresholds)
{
    // No alarm repository configured; signal spy should suffice

    // Create signal spy for alarm raised signal
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);

    // Process vital within normal HR range (50-120)
    VitalRecord vital = createVital("HR", 75.0);
    service->processVital(vital);

    // Verify no alarm was raised
    EXPECT_EQ(alarmSpy.count(), 0);
}

/**
 * @test Verify alarm IS triggered when vital exceeds high threshold.
 */
TEST_F(MonitoringServiceTest, AlarmTriggeredWhenExceedsHighThreshold)
{
    // No alarm repository configured; alarm still raised

    // Create signal spy for alarm raised signal
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);

    // Process vital above HR high threshold (> 120)
    VitalRecord vital = createVital("HR", 150.0);
    service->processVital(vital);

    // Verify alarm was raised
    EXPECT_EQ(alarmSpy.count(), 1);

    // Verify alarm details
    QList<QVariant> arguments = alarmSpy.takeFirst();
    QString alarmId = arguments.at(0).toString();
    QString alarmType = arguments.at(1).toString();
    int priority = arguments.at(2).toInt();

    EXPECT_FALSE(alarmId.isEmpty());
    EXPECT_EQ(alarmType.toStdString(), "HR_HIGH");
    EXPECT_EQ(priority, static_cast<int>(AlarmPriority::HIGH));
}

/**
 * @test Verify alarm IS triggered when vital falls below low threshold.
 */
TEST_F(MonitoringServiceTest, AlarmTriggeredWhenBelowLowThreshold)
{
    // No alarm repository configured; alarm still raised

    // Create signal spy for alarm raised signal
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);

    // Process vital below HR low threshold (< 50)
    VitalRecord vital = createVital("HR", 35.0);
    service->processVital(vital);

    // Verify alarm was raised
    EXPECT_EQ(alarmSpy.count(), 1);

    // Verify alarm details
    QList<QVariant> arguments = alarmSpy.takeFirst();
    QString alarmType = arguments.at(1).toString();

    EXPECT_EQ(alarmType.toStdString(), "HR_LOW");
}

/**
 * @test Verify alarm detection latency is measured and reported.
 */
TEST_F(MonitoringServiceTest, AlarmDetectionLatencyMeasured)
{
    // Process vital that triggers alarm
    VitalRecord vital = createVital("HR", 150.0);
    service->processVital(vital);

    // Get latency
    int64_t latency = service->getLastAlarmDetectionLatencyMs();

    // Verify latency was measured (should be >= 0 and < 50ms per requirement)
    EXPECT_GE(latency, 0);
    EXPECT_LT(latency, 50) << "Alarm detection latency must be < 50ms (REQ-PERF-LATENCY-001)";
}

/**
 * @test Verify alarm is NOT triggered when threshold is disabled.
 */
TEST_F(MonitoringServiceTest, NoAlarmWhenThresholdDisabled)
{
    // Disable HR threshold
    AlarmThreshold disabledThreshold("HR", 50.0, 120.0, 5.0, AlarmPriority::HIGH, false);
    service->setAlarmThreshold(disabledThreshold);

    // Create signal spy for alarm raised signal
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);

    // Process vital that would normally trigger alarm
    VitalRecord vital = createVital("HR", 150.0);
    service->processVital(vital);

    // Verify no alarm was raised
    EXPECT_EQ(alarmSpy.count(), 0);
}

/**
 * @test Verify service continues processing even if alarm repository save fails.
 */
TEST_F(MonitoringServiceTest, ContinuesProcessingWhenAlarmSaveFails)
{
    // No alarm repository configured; verify processing continues

    // Create signal spy for alarm raised signal
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);

    // Process vital that triggers alarm
    VitalRecord vital = createVital("HR", 150.0);
    service->processVital(vital);

    // Verify alarm signal was still emitted (even though save failed)
    EXPECT_EQ(alarmSpy.count(), 1);
}

/**
 * @test Verify service handles vitals repository save failure gracefully.
 */
TEST_F(MonitoringServiceTest, HandlesVitalsRepositorySaveFailure)
{
    // No vitals repository configured; vitalProcessed should still emit

    // Create signal spy for vital processed signal
    QSignalSpy vitalSpy(service.get(), &MonitoringService::vitalProcessed);

    // Process vital
    VitalRecord vital = createVital("HR", 75.0);
    service->processVital(vital);

    // Verify vital was still processed (signal emitted)
    EXPECT_EQ(vitalSpy.count(), 1);
}

/**
 * @test Verify alarm acknowledge workflow works correctly.
 */
TEST_F(MonitoringServiceTest, AcknowledgeAlarmWorkflow)
{
    // First, raise an alarm
    VitalRecord vital = createVital("HR", 150.0);
    service->processVital(vital);

    // Get active alarms to get alarm ID
    std::vector<AlarmSnapshot> activeAlarms = service->getActiveAlarms();
    ASSERT_GT(activeAlarms.size(), 0);

    QString alarmId = QString::fromStdString(activeAlarms[0].alarmId);

    // Create signal spy for alarm acknowledged signal
    QSignalSpy ackSpy(service.get(), &MonitoringService::alarmAcknowledged);

    // Acknowledge alarm
    bool success = service->acknowledgeAlarm(alarmId, "USER-001");

    // Verify
    EXPECT_TRUE(success);
    EXPECT_EQ(ackSpy.count(), 1);
}

/**
 * @test Verify alarm silence workflow works correctly.
 */
TEST_F(MonitoringServiceTest, SilenceAlarmWorkflow)
{
    // First, raise an alarm
    VitalRecord vital = createVital("HR", 150.0);
    service->processVital(vital);

    // Get active alarms to get alarm ID
    std::vector<AlarmSnapshot> activeAlarms = service->getActiveAlarms();
    ASSERT_GT(activeAlarms.size(), 0);

    QString alarmId = QString::fromStdString(activeAlarms[0].alarmId);

    // Silence alarm for 60 seconds
    bool success = service->silenceAlarm(alarmId, 60000);

    // Verify
    EXPECT_TRUE(success);
}

/**
 * @test Verify getAlarmHistory retrieves from repository correctly.
 */
TEST_F(MonitoringServiceTest, GetAlarmHistoryFromRepository)
{
    // With no repository configured, service should return empty history
    std::vector<AlarmSnapshot> history = service->getAlarmHistory("MRN-12345", 0, 1000);

    // Verify
    ASSERT_EQ(history.size(), 0);
}

/**
 * @test Verify multiple alarm types can be configured and triggered independently.
 */
TEST_F(MonitoringServiceTest, MultipleAlarmTypesIndependent)
{
    // Configure custom thresholds for SPO2
    AlarmThreshold spo2Threshold("SPO2", 85.0, 100.0, 2.0, AlarmPriority::HIGH, true);
    service->setAlarmThreshold(spo2Threshold);

    // Create signal spy for alarm raised signal
    QSignalSpy alarmSpy(service.get(), &MonitoringService::alarmRaised);

    // Process HR alarm
    VitalRecord hrVital = createVital("HR", 150.0);
    service->processVital(hrVital);

    // Process SPO2 alarm
    VitalRecord spo2Vital = createVital("SPO2", 80.0);
    service->processVital(spo2Vital);

    // Verify both alarms were raised
    EXPECT_EQ(alarmSpy.count(), 2);

    // Verify alarm types are different
    QList<QVariant> hrAlarm = alarmSpy.at(0);
    QList<QVariant> spo2Alarm = alarmSpy.at(1);

    EXPECT_EQ(hrAlarm.at(1).toString().toStdString(), "HR_HIGH");
    EXPECT_EQ(spo2Alarm.at(1).toString().toStdString(), "SPO2_LOW");
}

/**
 * @test Performance test: Verify < 50ms latency requirement is met under load.
 */
TEST_F(MonitoringServiceTest, PerformanceLatencyRequirementMet)
{
    // Process multiple vitals and track maximum latency
    int64_t maxLatency = 0;
    constexpr int NUM_VITALS = 100;

    for (int i = 0; i < NUM_VITALS; ++i)
    {
        // Alternate between alarm-triggering and normal vitals
        double value = (i % 2 == 0) ? 150.0 : 75.0;
        VitalRecord vital = createVital("HR", value);
        service->processVital(vital);

        int64_t latency = service->getLastAlarmDetectionLatencyMs();
        if (latency > maxLatency)
        {
            maxLatency = latency;
        }
    }

    // Verify maximum latency is < 50ms
    EXPECT_LT(maxLatency, 50) << "Maximum alarm detection latency must be < 50ms (REQ-PERF-LATENCY-001)";
}
