/**
 * @file monitoring_service_sensor_integration_test.cpp
 * @brief Integration test for MonitoringService with SharedMemorySensorDataSource.
 *
 * Tests the complete data pipeline:
 * Simulator → SharedMemorySensorDataSource → MonitoringService → Caches
 *
 * @author Z Monitor Team
 * @date 2025-11-30
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTimer>
#include <memory>
#include <atomic>

#include "application/services/MonitoringService.h"
#include "infrastructure/sensors/SharedMemorySensorDataSource.h"
#include "infrastructure/caching/VitalsCache.h"
#include "infrastructure/caching/WaveformCache.h"

using namespace zmon;

class MonitoringServiceIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create infrastructure layer components
        m_sensorDataSource = std::make_shared<SharedMemorySensorDataSource>(
            "/tmp/z-monitor-sensor.sock",
            nullptr);

        m_vitalsCache = std::make_shared<VitalsCache>(259200);    // 3 days @ 60 Hz
        m_waveformCache = std::make_shared<WaveformCache>(22500); // 30 seconds @ 250 Hz × 3 channels

        // Create application service layer
        m_monitoringService = new MonitoringService(
            nullptr, // patient repo (not needed for this test)
            nullptr, // telemetry repo (not needed for this test)
            nullptr, // alarm repo (not needed for this test)
            nullptr, // vitals repo (not needed for this test)
            m_sensorDataSource,
            m_vitalsCache,
            m_waveformCache,
            nullptr); // No parent - we'll delete manually

        // Connect to signals for verification
        QObject::connect(m_monitoringService, &MonitoringService::vitalsUpdated,
                         [this]()
                         { m_vitalsUpdateCount++; });
    }

    void TearDown() override
    {
        delete m_monitoringService;
    }

    std::shared_ptr<SharedMemorySensorDataSource> m_sensorDataSource;
    std::shared_ptr<VitalsCache> m_vitalsCache;
    std::shared_ptr<WaveformCache> m_waveformCache;
    MonitoringService *m_monitoringService;
    std::atomic<int> m_vitalsUpdateCount{0};
};

/**
 * @test Verify data flows from simulator to MonitoringService and caches.
 *
 * Prerequisites: sensor_simulator must be running on /tmp/z-monitor-sensor.sock
 */
TEST_F(MonitoringServiceIntegrationTest, DataFlowFromSimulatorToMonitoringServiceAndCaches)
{
    // Start monitoring service
    bool started = m_monitoringService->start();

    // Skip test gracefully if simulator is not available
    if (!started)
    {
        GTEST_SKIP() << "Sensor simulator not available at /tmp/z-monitor-sensor.sock. "
                     << "Start the simulator to run this integration test.";
        return;
    }

    // Wait for data to arrive (5 seconds)
    QTimer::singleShot(5000, []()
                       { QCoreApplication::quit(); });
    QCoreApplication::exec();

    // Stop monitoring service
    m_monitoringService->stop();

    // Verify vitals were received and cached
    EXPECT_GT(m_vitalsCache->size(), 0) << "No vitals in cache";
    EXPECT_GT(m_waveformCache->size(), 0) << "No waveforms in cache";
    EXPECT_GT(m_vitalsUpdateCount, 0) << "No vitals updates emitted";

    // Check specific vital types in cache
    bool found = false;
    auto hrVital = m_vitalsCache->getLatest("HR", found);
    EXPECT_TRUE(found) << "No HR vitals in cache";
    if (found)
    {
        EXPECT_GT(hrVital.value, 0) << "HR value is zero";
        EXPECT_LE(hrVital.value, 200) << "HR value is unrealistic";
    }

    auto spo2Vital = m_vitalsCache->getLatest("SPO2", found);
    EXPECT_TRUE(found) << "No SPO2 vitals in cache";
    if (found)
    {
        EXPECT_GE(spo2Vital.value, 70) << "SPO2 value too low";
        EXPECT_LE(spo2Vital.value, 100) << "SPO2 value too high";
    }

    // Check waveform channels
    auto ecgSamples = m_waveformCache->getChannelSamples("ecg", 1);
    EXPECT_GT(ecgSamples.size(), 0) << "No ECG samples in cache";

    auto plethSamples = m_waveformCache->getChannelSamples("pleth", 1);
    // Pleth may not be implemented in simulator yet - log warning but don't fail
    if (plethSamples.size() == 0)
    {
        std::cout << "  WARNING: No Pleth samples in cache (may not be implemented in simulator)" << std::endl;
    }

    // Log results
    std::cout << "\n========================================" << std::endl;
    std::cout << "Integration Test Results:" << std::endl;
    std::cout << "  Vitals in cache: " << m_vitalsCache->size() << std::endl;
    std::cout << "  Waveforms in cache: " << m_waveformCache->size() << std::endl;
    std::cout << "  Vitals updates emitted: " << m_vitalsUpdateCount << std::endl;
    std::cout << "  ECG samples (last 1s): " << ecgSamples.size() << std::endl;
    std::cout << "  Pleth samples (last 1s): " << plethSamples.size() << std::endl;
    if (found)
    {
        std::cout << "  Latest HR: " << static_cast<int>(hrVital.value) << " BPM" << std::endl;
        std::cout << "  Latest SPO2: " << static_cast<int>(spo2Vital.value) << "%" << std::endl;
    }
    std::cout << "========================================\n"
              << std::endl;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
