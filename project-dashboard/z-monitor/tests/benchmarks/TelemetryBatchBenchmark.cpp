/**
 * @file TelemetryBatchBenchmark.cpp
 * @brief Performance benchmarks for telemetry batch processing.
 *
 * Simulates realistic telemetry operations (batch processing, compression,
 * serialization) to measure performance without complete service dependencies.
 *
 * @author Z Monitor Team
 * @date 2025-12-03
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>

/**
 * @brief Simulated telemetry event record.
 */
struct TelemetryRecord
{
    int64_t timestamp;      ///< Unix timestamp in milliseconds
    std::string event_type; ///< Event type ("vital", "alarm", "action")
    std::string patient_id; ///< Patient identifier
    double value;           ///< Numeric value
};

/**
 * @brief Generate realistic telemetry batch data.
 * @param count Number of records
 * @return Vector of telemetry records
 */
static std::vector<TelemetryRecord> GenerateTelemetryBatch(size_t count)
{
    std::vector<TelemetryRecord> batch;
    batch.reserve(count);

    int64_t base_time = 1700000000000LL;
    const std::vector<std::string> event_types = {"vital", "alarm", "action"};

    for (size_t i = 0; i < count; ++i)
    {
        batch.push_back({base_time + static_cast<int64_t>(i * 1000),
                         event_types[i % event_types.size()],
                         "PAT" + std::to_string((i % 5) + 1),
                         70.0 + (i % 30)});
    }

    return batch;
}

/**
 * @brief Simple run-length encoding compression.
 */
static std::string CompressData(const std::vector<TelemetryRecord> &data)
{
    std::ostringstream compressed;

    if (data.empty())
        return "";

    std::string current_type = data[0].event_type;
    int count = 1;

    for (size_t i = 1; i < data.size(); ++i)
    {
        if (data[i].event_type == current_type)
        {
            count++;
        }
        else
        {
            compressed << current_type << ":" << count << "|";
            current_type = data[i].event_type;
            count = 1;
        }
    }
    compressed << current_type << ":" << count;

    return compressed.str();
}

// Simulate 10-minute batch processing (600 events at 1/second)
static void BM_Process10MinBatch(benchmark::State &state)
{
    auto batch = GenerateTelemetryBatch(600);

    for (auto _ : state)
    {
        // Aggregate events by type
        int vital_count = std::count_if(batch.begin(), batch.end(),
                                        [](const TelemetryRecord &r)
                                        { return r.event_type == "vital"; });
        int alarm_count = std::count_if(batch.begin(), batch.end(),
                                        [](const TelemetryRecord &r)
                                        { return r.event_type == "alarm"; });
        int action_count = std::count_if(batch.begin(), batch.end(),
                                         [](const TelemetryRecord &r)
                                         { return r.event_type == "action"; });

        benchmark::DoNotOptimize(vital_count);
        benchmark::DoNotOptimize(alarm_count);
        benchmark::DoNotOptimize(action_count);
    }
}
BENCHMARK(BM_Process10MinBatch)->Unit(benchmark::kMillisecond);

// Simulate compression (run-length encoding)
static void BM_CompressTelemetryData(benchmark::State &state)
{
    auto batch = GenerateTelemetryBatch(600);

    for (auto _ : state)
    {
        std::string compressed = CompressData(batch);
        benchmark::DoNotOptimize(compressed);
    }
}
BENCHMARK(BM_CompressTelemetryData)->Unit(benchmark::kMillisecond);

// Simulate JSON-like serialization
static void BM_SerializeTelemetryBatch(benchmark::State &state)
{
    auto batch = GenerateTelemetryBatch(600);

    for (auto _ : state)
    {
        std::ostringstream json;
        json << "{\"events\":[";

        for (size_t i = 0; i < batch.size(); ++i)
        {
            if (i > 0)
                json << ",";
            json << "{\"ts\":" << batch[i].timestamp
                 << ",\"type\":\"" << batch[i].event_type << "\""
                 << ",\"patient\":\"" << batch[i].patient_id << "\""
                 << ",\"value\":" << batch[i].value << "}";
        }

        json << "]}";
        std::string result = json.str();

        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SerializeTelemetryBatch)->Unit(benchmark::kMillisecond);

// Simulate end-to-end processing (aggregate + compress + serialize)
static void BM_EndToEndTelemetry(benchmark::State &state)
{
    auto batch = GenerateTelemetryBatch(600);

    for (auto _ : state)
    {
        // Step 1: Filter critical events
        std::vector<TelemetryRecord> critical;
        std::copy_if(batch.begin(), batch.end(), std::back_inserter(critical),
                     [](const TelemetryRecord &r)
                     { return r.event_type == "alarm"; });

        // Step 2: Compress
        std::string compressed = CompressData(critical);

        // Step 3: Calculate statistics
        double avg = critical.empty() ? 0.0 : std::accumulate(critical.begin(), critical.end(), 0.0, [](double sum, const TelemetryRecord &r)
                                                              { return sum + r.value; }) /
                                                  critical.size();

        benchmark::DoNotOptimize(compressed);
        benchmark::DoNotOptimize(avg);
    }
}
BENCHMARK(BM_EndToEndTelemetry)->Unit(benchmark::kMillisecond);

// Simulate high-frequency telemetry (15000 samples, ECG data)
static void BM_HighFrequencyTelemetry(benchmark::State &state)
{
    auto batch = GenerateTelemetryBatch(15000); // 1 minute of 250 Hz data

    for (auto _ : state)
    {
        // Downsample to 60 Hz (take every 4th sample)
        std::vector<TelemetryRecord> downsampled;
        for (size_t i = 0; i < batch.size(); i += 4)
        {
            downsampled.push_back(batch[i]);
        }

        benchmark::DoNotOptimize(downsampled);
    }
}
BENCHMARK(BM_HighFrequencyTelemetry)->Unit(benchmark::kMillisecond);

// Simulate multi-patient telemetry aggregation
static void BM_MultiPatientTelemetry(benchmark::State &state)
{
    auto batch = GenerateTelemetryBatch(3000); // 5 patients, 600 events each

    for (auto _ : state)
    {
        // Group by patient
        std::vector<std::pair<std::string, std::vector<TelemetryRecord>>> patient_groups;

        for (const auto &record : batch)
        {
            auto it = std::find_if(patient_groups.begin(), patient_groups.end(),
                                   [&record](const auto &pair)
                                   { return pair.first == record.patient_id; });

            if (it != patient_groups.end())
            {
                it->second.push_back(record);
            }
            else
            {
                patient_groups.push_back({record.patient_id, {record}});
            }
        }

        benchmark::DoNotOptimize(patient_groups);
    }
}
BENCHMARK(BM_MultiPatientTelemetry)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
