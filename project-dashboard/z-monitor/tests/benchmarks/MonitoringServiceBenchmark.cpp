/**
 * @file MonitoringServiceBenchmark.cpp
 * @brief Performance benchmarks for alarm detection latency.
 *
 * Simulates realistic vital sign monitoring and alarm detection operations
 * to measure performance without complete service dependencies.
 *
 * @author Z Monitor Team
 * @date 2025-12-03
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>
#include <deque>

/**
 * @brief Simulated vital signs snapshot.
 */
struct VitalSnapshot
{
    int hr;      ///< Heart rate (bpm)
    int spo2;    ///< SpO2 percentage
    int rr;      ///< Respiratory rate (breaths/min)
    double temp; ///< Temperature (Celsius)
};

/**
 * @brief Vital sign thresholds for alarm detection.
 */
struct VitalThresholds
{
    int hr_low = 50;
    int hr_high = 120;
    int spo2_low = 90;
    int rr_low = 8;
    int rr_high = 30;
    double temp_low = 36.0;
    double temp_high = 38.5;
};

/**
 * @brief Check if vitals violate any threshold.
 * @return Number of alarms detected
 */
static int DetectAlarms(const VitalSnapshot &vitals, const VitalThresholds &thresholds)
{
    int alarm_count = 0;

    if (vitals.hr < thresholds.hr_low || vitals.hr > thresholds.hr_high)
        alarm_count++;
    if (vitals.spo2 < thresholds.spo2_low)
        alarm_count++;
    if (vitals.rr < thresholds.rr_low || vitals.rr > thresholds.rr_high)
        alarm_count++;
    if (vitals.temp < thresholds.temp_low || vitals.temp > thresholds.temp_high)
        alarm_count++;

    return alarm_count;
}

// Simulate single alarm detection (< 50ms target)
static void BM_SingleAlarmDetection(benchmark::State &state)
{
    VitalSnapshot vitals = {75, 98, 14, 37.2};
    VitalThresholds thresholds;

    for (auto _ : state)
    {
        int alarms = DetectAlarms(vitals, thresholds);
        benchmark::DoNotOptimize(alarms);
    }
}
BENCHMARK(BM_SingleAlarmDetection)->Unit(benchmark::kMillisecond);

// Simulate normal load (60 Hz vital updates)
static void BM_NormalLoad60Hz(benchmark::State &state)
{
    VitalThresholds thresholds;

    for (auto _ : state)
    {
        // Process 60 vital updates (1 second of data)
        int total_alarms = 0;
        for (int i = 0; i < 60; ++i)
        {
            VitalSnapshot vitals = {
                70 + (i % 30),
                95 + (i % 5),
                12 + (i % 8),
                36.5 + (i % 10) * 0.1};
            total_alarms += DetectAlarms(vitals, thresholds);
        }
        benchmark::DoNotOptimize(total_alarms);
    }
}
BENCHMARK(BM_NormalLoad60Hz)->Unit(benchmark::kMillisecond);

// Simulate high load (250 Hz ECG processing with peak detection)
static void BM_HighLoad250Hz(benchmark::State &state)
{
    std::vector<double> ecg_samples(250);

    for (auto _ : state)
    {
        // Generate ECG waveform
        for (size_t i = 0; i < ecg_samples.size(); ++i)
        {
            ecg_samples[i] = std::sin(2 * M_PI * i / 250.0) + 0.3 * std::sin(2 * M_PI * 60 * i / 250.0);
        }

        // Simple peak detection
        int peak_count = 0;
        for (size_t i = 1; i < ecg_samples.size() - 1; ++i)
        {
            if (ecg_samples[i] > ecg_samples[i - 1] && ecg_samples[i] > ecg_samples[i + 1] && ecg_samples[i] > 0.5)
            {
                peak_count++;
            }
        }

        benchmark::DoNotOptimize(peak_count);
    }
}
BENCHMARK(BM_HighLoad250Hz)->Unit(benchmark::kMillisecond);

// Simulate burst traffic (100 vital updates at once)
static void BM_BurstTraffic(benchmark::State &state)
{
    VitalThresholds thresholds;

    for (auto _ : state)
    {
        std::vector<VitalSnapshot> burst(100);
        for (size_t i = 0; i < burst.size(); ++i)
        {
            burst[i] = {
                70 + static_cast<int>(i % 30),
                95 + static_cast<int>(i % 5),
                12 + static_cast<int>(i % 8),
                36.5 + (i % 10) * 0.1};
        }

        int total_alarms = 0;
        for (const auto &vitals : burst)
        {
            total_alarms += DetectAlarms(vitals, thresholds);
        }

        benchmark::DoNotOptimize(total_alarms);
    }
}
BENCHMARK(BM_BurstTraffic)->Unit(benchmark::kMillisecond);

// Simulate multiple alarm types (6 different threshold checks)
static void BM_MultipleAlarmTypes(benchmark::State &state)
{
    VitalSnapshot vitals = {75, 98, 14, 37.2};
    VitalThresholds thresholds;

    for (auto _ : state)
    {
        // Check all threshold types
        bool hr_low_alarm = vitals.hr < thresholds.hr_low;
        bool hr_high_alarm = vitals.hr > thresholds.hr_high;
        bool spo2_alarm = vitals.spo2 < thresholds.spo2_low;
        bool rr_low_alarm = vitals.rr < thresholds.rr_low;
        bool rr_high_alarm = vitals.rr > thresholds.rr_high;
        bool temp_alarm = vitals.temp < thresholds.temp_low || vitals.temp > thresholds.temp_high;

        benchmark::DoNotOptimize(hr_low_alarm);
        benchmark::DoNotOptimize(hr_high_alarm);
        benchmark::DoNotOptimize(spo2_alarm);
        benchmark::DoNotOptimize(rr_low_alarm);
        benchmark::DoNotOptimize(rr_high_alarm);
        benchmark::DoNotOptimize(temp_alarm);
    }
}
BENCHMARK(BM_MultipleAlarmTypes)->Unit(benchmark::kMillisecond);

// Simulate worst-case with persistence check (sliding window)
static void BM_WorstCaseWithPersistence(benchmark::State &state)
{
    VitalThresholds thresholds;
    std::deque<VitalSnapshot> history;
    const size_t window_size = 10; // 10-sample persistence window

    for (auto _ : state)
    {
        // Add new vital
        VitalSnapshot vitals = {75, 98, 14, 37.2};
        history.push_back(vitals);

        if (history.size() > window_size)
        {
            history.pop_front();
        }

        // Check if alarm persists over window
        int persistent_hr_alarms = 0;
        for (const auto &v : history)
        {
            if (v.hr > thresholds.hr_high)
            {
                persistent_hr_alarms++;
            }
        }

        bool persistent_alarm = persistent_hr_alarms >= static_cast<int>(window_size * 0.8); // 80% persistence

        benchmark::DoNotOptimize(persistent_alarm);
    }
}
BENCHMARK(BM_WorstCaseWithPersistence)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
