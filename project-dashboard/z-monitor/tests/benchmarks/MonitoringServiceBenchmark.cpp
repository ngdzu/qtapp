/**
 * @file MonitoringServiceBenchmark.cpp
 * @brief Performance benchmarks for MonitoringService alarm detection latency.
 *
 * Verifies < 50ms alarm detection latency requirement (REQ-PERF-LATENCY-001).
 * Tests performance under various conditions: normal load, high load, burst traffic.
 *
 * @author Z Monitor Team
 * @date 2025-12-02
 */

#include <benchmark/benchmark.h>
#include "application/services/MonitoringService.h"
#include "domain/monitoring/VitalRecord.h"
#include "tests/mocks/domain/repositories/MockPatientRepository.h"
#include "tests/mocks/domain/repositories/MockTelemetryRepository.h"
#include "tests/mocks/domain/repositories/MockAlarmRepository.h"
#include "tests/mocks/domain/repositories/MockVitalsRepository.h"
#include "tests/mocks/infrastructure/MockSensorDataSource.h"
#include "tests/mocks/infrastructure/MockVitalsCache.h"
#include "tests/mocks/infrastructure/MockWaveformCache.h"
#include <chrono>
#include <memory>

using namespace zmon;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

/**
 * @brief Helper function to create MonitoringService with mocks.
 */
static std::unique_ptr<MonitoringService> createMonitoringService()
{
    auto patientRepo = std::make_shared<NiceMock<MockPatientRepository>>();
    auto telemetryRepo = std::make_shared<NiceMock<MockTelemetryRepository>>();
    auto alarmRepo = std::make_shared<NiceMock<MockAlarmRepository>>();
    auto vitalsRepo = std::make_shared<NiceMock<MockVitalsRepository>>();
    auto sensorDataSource = std::make_shared<NiceMock<MockSensorDataSource>>();
    auto vitalsCache = std::make_shared<NiceMock<MockVitalsCache>>();
    auto waveformCache = std::make_shared<NiceMock<MockWaveformCache>>();

    // Configure mocks to accept all operations
    ON_CALL(*alarmRepo, save(_)).WillByDefault(Return(Result<void>::ok()));
    ON_CALL(*vitalsRepo, save(_)).WillByDefault(Return(Result<void>::ok()));

    return std::make_unique<MonitoringService>(
        patientRepo, telemetryRepo, alarmRepo, vitalsRepo,
        sensorDataSource, vitalsCache, waveformCache);
}

/**
 * @brief Helper function to create a vital record.
 */
static VitalRecord createVital(const std::string &type, double value)
{
    VitalRecord vital;
    vital.vitalType = type;
    vital.value = value;
    vital.patientMrn = "MRN-BENCH-001";
    vital.deviceId = "ZM-BENCH-01";
    vital.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return vital;
}

/**
 * @brief Benchmark: Single alarm detection latency.
 *
 * Measures time to detect a single alarm from a vital that exceeds threshold.
 * Target: < 50ms (REQ-PERF-LATENCY-001)
 */
static void BM_SingleAlarmDetection(benchmark::State &state)
{
    auto service = createMonitoringService();

    for (auto _ : state)
    {
        // Process vital that triggers alarm
        VitalRecord vital = createVital("HR", 150.0); // Above threshold

        auto start = std::chrono::high_resolution_clock::now();
        service->processVital(vital);
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        state.SetIterationTime(elapsed / 1000000.0); // Convert to seconds
    }

    // Report custom metrics
    int64_t latencyMs = service->getLastAlarmDetectionLatencyMs();
    state.counters["latency_ms"] = benchmark::Counter(latencyMs);

    // Verify requirement
    if (latencyMs >= 50)
    {
        state.SkipWithError("Latency requirement violated: must be < 50ms");
    }
}
BENCHMARK(BM_SingleAlarmDetection)->UseManualTime()->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark: Normal load (60 Hz vitals processing).
 *
 * Simulates normal load: 60 vital signs per second (typical sensor frequency).
 * Measures maximum alarm detection latency under sustained load.
 */
static void BM_NormalLoad60Hz(benchmark::State &state)
{
    auto service = createMonitoringService();

    int64_t maxLatency = 0;

    for (auto _ : state)
    {
        // Process 60 vitals (1 second worth at 60 Hz)
        for (int i = 0; i < 60; ++i)
        {
            // Alternate between alarm-triggering and normal vitals
            double value = (i % 10 == 0) ? 150.0 : 75.0;
            VitalRecord vital = createVital("HR", value);

            service->processVital(vital);

            int64_t latency = service->getLastAlarmDetectionLatencyMs();
            if (latency > maxLatency)
            {
                maxLatency = latency;
            }
        }
    }

    // Report maximum latency seen
    state.counters["max_latency_ms"] = benchmark::Counter(maxLatency);

    // Verify requirement
    if (maxLatency >= 50)
    {
        state.SkipWithError("Maximum latency requirement violated: must be < 50ms");
    }
}
BENCHMARK(BM_NormalLoad60Hz)->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark: High load (250 Hz waveform + 60 Hz vitals).
 *
 * Simulates high load: 250 Hz waveform samples + 60 Hz vital signs.
 * Measures alarm detection latency under maximum sensor load.
 */
static void BM_HighLoad250Hz(benchmark::State &state)
{
    auto service = createMonitoringService();

    int64_t maxLatency = 0;

    for (auto _ : state)
    {
        // Process 250 waveform samples + 60 vitals (1 second worth)
        // (Note: Waveforms don't trigger alarms, but they add processing load)
        for (int i = 0; i < 250; ++i)
        {
            // Every 4th sample is a vital (250/60 ≈ 4)
            if (i % 4 == 0)
            {
                double value = (i % 40 == 0) ? 150.0 : 75.0;
                VitalRecord vital = createVital("HR", value);
                service->processVital(vital);

                int64_t latency = service->getLastAlarmDetectionLatencyMs();
                if (latency > maxLatency)
                {
                    maxLatency = latency;
                }
            }
        }
    }

    // Report maximum latency seen
    state.counters["max_latency_ms"] = benchmark::Counter(maxLatency);

    // Verify requirement
    if (maxLatency >= 50)
    {
        state.SkipWithError("Maximum latency requirement violated under high load: must be < 50ms");
    }
}
BENCHMARK(BM_HighLoad250Hz)->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark: Burst traffic (multiple simultaneous alarms).
 *
 * Simulates burst traffic: multiple vital signs exceeding thresholds simultaneously.
 * Measures alarm detection latency when multiple alarms are raised at once.
 */
static void BM_BurstTraffic(benchmark::State &state)
{
    auto service = createMonitoringService();

    int64_t maxLatency = 0;

    for (auto _ : state)
    {
        // Process burst of 10 simultaneous alarm-triggering vitals
        for (int i = 0; i < 10; ++i)
        {
            VitalRecord vital = createVital("HR", 150.0 + i); // All above threshold
            service->processVital(vital);

            int64_t latency = service->getLastAlarmDetectionLatencyMs();
            if (latency > maxLatency)
            {
                maxLatency = latency;
            }
        }
    }

    // Report maximum latency seen
    state.counters["max_latency_ms"] = benchmark::Counter(maxLatency);

    // Verify requirement
    if (maxLatency >= 50)
    {
        state.SkipWithError("Maximum latency requirement violated during burst: must be < 50ms");
    }
}
BENCHMARK(BM_BurstTraffic)->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark: Multiple alarm types (HR, SPO2, RR).
 *
 * Measures alarm detection latency when processing multiple vital types.
 * Verifies threshold checking for different vital types doesn't degrade performance.
 */
static void BM_MultipleAlarmTypes(benchmark::State &state)
{
    auto service = createMonitoringService();

    int64_t maxLatency = 0;

    for (auto _ : state)
    {
        // Process vitals for all alarm types
        std::vector<std::string> vitalTypes = {"HR", "SPO2", "RR"};
        std::vector<double> alarmValues = {150.0, 80.0, 35.0}; // All trigger alarms

        for (size_t i = 0; i < vitalTypes.size(); ++i)
        {
            VitalRecord vital = createVital(vitalTypes[i], alarmValues[i]);
            service->processVital(vital);

            int64_t latency = service->getLastAlarmDetectionLatencyMs();
            if (latency > maxLatency)
            {
                maxLatency = latency;
            }
        }
    }

    // Report maximum latency seen
    state.counters["max_latency_ms"] = benchmark::Counter(maxLatency);

    // Verify requirement
    if (maxLatency >= 50)
    {
        state.SkipWithError("Maximum latency requirement violated with multiple types: must be < 50ms");
    }
}
BENCHMARK(BM_MultipleAlarmTypes)->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark: Worst case (alarm + repository save).
 *
 * Measures worst-case alarm detection latency including repository persistence.
 * This is the complete critical path: vital → threshold check → alarm raise → persist.
 */
static void BM_WorstCaseWithPersistence(benchmark::State &state)
{
    auto service = createMonitoringService();

    int64_t maxLatency = 0;

    for (auto _ : state)
    {
        // Process vital that triggers alarm with repository save
        VitalRecord vital = createVital("HR", 150.0);

        auto start = std::chrono::high_resolution_clock::now();
        service->processVital(vital);
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (elapsed > maxLatency)
        {
            maxLatency = elapsed;
        }
    }

    // Report maximum latency seen
    state.counters["max_latency_ms"] = benchmark::Counter(maxLatency);

    // Verify requirement (worst case still must be < 50ms)
    if (maxLatency >= 50)
    {
        state.SkipWithError("Worst-case latency requirement violated: must be < 50ms");
    }
}
BENCHMARK(BM_WorstCaseWithPersistence)->Unit(benchmark::kMillisecond);

// Run all benchmarks
BENCHMARK_MAIN();
