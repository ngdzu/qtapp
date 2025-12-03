/**
 * @file DatabaseQueryBenchmark.cpp
 * @brief Performance benchmarks for database query operations.
 *
 * Simulates realistic database query workloads (filtering, sorting, aggregation)
 * to measure performance without requiring complete database infrastructure.
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <ctime>

/**
 * @brief Simulated vital signs record for benchmark testing.
 */
struct VitalRecord
{
    int64_t timestamp;      ///< Unix timestamp in milliseconds
    std::string patient_id; ///< Patient identifier
    int hr;                 ///< Heart rate (bpm)
    int spo2;               ///< SpO2 percentage
    int rr;                 ///< Respiratory rate (breaths/min)
    double temp;            ///< Temperature (Celsius)
};

/**
 * @brief Generate realistic vital signs data for testing.
 * @param count Number of records to generate
 * @param patient_id Patient identifier
 * @return Vector of vital records
 */
static std::vector<VitalRecord> GenerateVitalsData(size_t count, const std::string &patient_id)
{
    std::vector<VitalRecord> data;
    data.reserve(count);

    int64_t base_time = 1700000000000LL; // Base timestamp
    for (size_t i = 0; i < count; ++i)
    {
        data.push_back({
            base_time + static_cast<int64_t>(i * 10000), // 10 second intervals
            patient_id,
            static_cast<int>(70 + (i % 30)), // HR varies 70-99
            static_cast<int>(95 + (i % 5)),  // SpO2 varies 95-99
            static_cast<int>(12 + (i % 8)),  // RR varies 12-19
            36.5 + (i % 10) * 0.1            // Temp varies 36.5-37.4
        });
    }

    return data;
}

// Simulate 1-hour vitals query (360 records at 10-second intervals)
static void BM_QueryVitals1Hour(benchmark::State &state)
{
    auto vitals = GenerateVitalsData(360, "PAT001");

    for (auto _ : state)
    {
        // Simulate filtering by time range
        int64_t start_time = vitals[0].timestamp;
        int64_t end_time = vitals[359].timestamp;

        std::vector<VitalRecord> filtered;
        std::copy_if(vitals.begin(), vitals.end(), std::back_inserter(filtered),
                     [start_time, end_time](const VitalRecord &v)
                     {
                         return v.timestamp >= start_time && v.timestamp <= end_time;
                     });

        benchmark::DoNotOptimize(filtered);
    }
}
BENCHMARK(BM_QueryVitals1Hour)->Unit(benchmark::kMillisecond);

// Simulate 24-hour vitals query (8640 records)
static void BM_QueryVitals24Hours(benchmark::State &state)
{
    auto vitals = GenerateVitalsData(8640, "PAT001");

    for (auto _ : state)
    {
        // Simulate filtering and aggregation
        std::vector<VitalRecord> filtered;
        std::copy_if(vitals.begin(), vitals.end(), std::back_inserter(filtered),
                     [](const VitalRecord &v)
                     { return v.hr > 80; });

        // Calculate average HR
        if (!filtered.empty())
        {
            double avg_hr = std::accumulate(filtered.begin(), filtered.end(), 0.0,
                                            [](double sum, const VitalRecord &v)
                                            { return sum + v.hr; }) /
                            filtered.size();
            benchmark::DoNotOptimize(avg_hr);
        }

        benchmark::DoNotOptimize(filtered);
    }
}
BENCHMARK(BM_QueryVitals24Hours)->Unit(benchmark::kMillisecond);

// Simulate alarm history query with sorting
static void BM_QueryAlarmHistory(benchmark::State &state)
{
    auto vitals = GenerateVitalsData(1000, "PAT001");

    for (auto _ : state)
    {
        // Find all high HR alarms
        std::vector<VitalRecord> alarms;
        std::copy_if(vitals.begin(), vitals.end(), std::back_inserter(alarms),
                     [](const VitalRecord &v)
                     { return v.hr > 100 || v.hr < 50; });

        // Sort by timestamp (descending)
        std::sort(alarms.begin(), alarms.end(),
                  [](const VitalRecord &a, const VitalRecord &b)
                  {
                      return a.timestamp > b.timestamp;
                  });

        benchmark::DoNotOptimize(alarms);
    }
}
BENCHMARK(BM_QueryAlarmHistory)->Unit(benchmark::kMillisecond);

// Simulate active alarms filtering
static void BM_QueryActiveAlarms(benchmark::State &state)
{
    auto vitals = GenerateVitalsData(500, "PAT001");

    for (auto _ : state)
    {
        // Count alarms by severity
        int critical = std::count_if(vitals.begin(), vitals.end(),
                                     [](const VitalRecord &v)
                                     { return v.hr > 120 || v.hr < 40; });
        int major = std::count_if(vitals.begin(), vitals.end(),
                                  [](const VitalRecord &v)
                                  { return (v.hr > 110 && v.hr <= 120) || (v.hr >= 40 && v.hr < 50); });
        int minor = std::count_if(vitals.begin(), vitals.end(),
                                  [](const VitalRecord &v)
                                  { return (v.hr > 100 && v.hr <= 110) || (v.hr >= 50 && v.hr < 60); });

        benchmark::DoNotOptimize(critical);
        benchmark::DoNotOptimize(major);
        benchmark::DoNotOptimize(minor);
    }
}
BENCHMARK(BM_QueryActiveAlarms)->Unit(benchmark::kMillisecond);

// Simulate batch insert operation
static void BM_BatchInsertVitals(benchmark::State &state)
{
    for (auto _ : state)
    {
        // Generate batch of 100 records
        auto batch = GenerateVitalsData(100, "PAT001");

        // Simulate data validation and preparation
        std::vector<VitalRecord> validated;
        for (const auto &record : batch)
        {
            if (record.hr > 0 && record.hr < 300 &&
                record.spo2 >= 0 && record.spo2 <= 100)
            {
                validated.push_back(record);
            }
        }

        benchmark::DoNotOptimize(validated);
    }
}
BENCHMARK(BM_BatchInsertVitals)->Unit(benchmark::kMillisecond);

// Simulate multi-patient query
static void BM_MultiPatientQuery(benchmark::State &state)
{
    // Generate data for 10 patients
    std::vector<VitalRecord> all_vitals;
    for (int i = 0; i < 10; ++i)
    {
        auto patient_data = GenerateVitalsData(500, "PAT" + std::to_string(i + 1));
        all_vitals.insert(all_vitals.end(), patient_data.begin(), patient_data.end());
    }

    for (auto _ : state)
    {
        // Find patients with abnormal vitals
        std::vector<std::string> abnormal_patients;
        for (const auto &v : all_vitals)
        {
            if (v.hr > 100 || v.spo2 < 92)
            {
                if (std::find(abnormal_patients.begin(), abnormal_patients.end(), v.patient_id) == abnormal_patients.end())
                {
                    abnormal_patients.push_back(v.patient_id);
                }
            }
        }

        benchmark::DoNotOptimize(abnormal_patients);
    }
}
BENCHMARK(BM_MultiPatientQuery)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
