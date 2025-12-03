# TASK-TEST-015: Implement Performance Benchmarks for Critical Paths

**Status:** 🏗️ In Progress
**Priority:** High (Performance validation critical for medical device)
**Estimated Time:** 4-6 hours
**Task ID:** TASK-TEST-015

---

## Overview

Create comprehensive performance benchmarks using Google Benchmark to measure critical performance paths in the Z Monitor application. Benchmarks ensure performance requirements are met and prevent regressions through CI integration.

---

## Performance Requirements

From `project-dashboard/doc/legacy/architecture_and_design/40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md` and requirements:

| Metric                     | Target                | Requirement ID         | Critical Path               |
| -------------------------- | --------------------- | ---------------------- | --------------------------- |
| Alarm Detection Latency    | < 50ms                | REQ-PERF-LATENCY-001   | Sensor data → alarm raised  |
| Waveform Rendering         | 60 FPS (< 16ms/frame) | REQ-PERF-UI-001        | Data processing → UI update |
| Database Query             | < 100ms               | REQ-PERF-DB-001        | Time-series vitals queries  |
| Telemetry Batch Processing | < 5s for 10min batch  | REQ-PERF-TELEMETRY-001 | Batch compression + upload  |

---

## What to Implement

### 1. Google Benchmark Infrastructure Setup

**File:** `tests/benchmarks/CMakeLists.txt`

- Add Google Benchmark dependency (via vcpkg or FetchContent)
- Create benchmark executable targets
- Link with application libraries (z_monitor_application, z_monitor_infrastructure)
- Configure benchmark output format (JSON for CI parsing)

**Example CMake:**
```cmake
# Fetch Google Benchmark
include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

# Create benchmark executables
add_executable(AlarmDetectionBenchmark AlarmDetectionBenchmark.cpp)
target_link_libraries(AlarmDetectionBenchmark PRIVATE
  benchmark::benchmark
  z_monitor_application
  z_monitor_infrastructure
  zmon_test_mocks_infrastructure
  Qt6::Core
  Qt6::Test
)
```

---

### 2. Alarm Detection Benchmark

**File:** `tests/benchmarks/AlarmDetectionBenchmark.cpp`

**What to Measure:**
- Time from sensor data received → alarm raised event emitted
- Includes: threshold checking, alarm aggregate creation, repository save, event dispatch

**Benchmark Structure:**
```cpp
/**
 * @file AlarmDetectionBenchmark.cpp
 * @brief Performance benchmark for alarm detection latency
 *
 * Measures end-to-end latency from sensor data input to alarm raised event.
 * Target: < 50ms (REQ-PERF-LATENCY-001)
 */

#include <benchmark/benchmark.h>
#include "application/services/MonitoringService.h"
#include "domain/aggregates/alarm/AlarmAggregate.h"
#include "tests/mocks/infrastructure/MockSensorDataSource.h"
#include "tests/mocks/infrastructure/MockAlarmRepository.h"
#include <QCoreApplication>

using namespace zmonitor;

/**
 * @brief Benchmark alarm detection from vitals update to alarm raised
 * 
 * Simulates real-time vital sign update that exceeds threshold.
 * Measures complete alarm detection pipeline latency.
 */
static void BM_AlarmDetection_ThresholdExceeded(benchmark::State& state) {
    // Setup (not measured)
    QCoreApplication app(argc, argv);
    auto mockSensorSource = std::make_shared<MockSensorDataSource>();
    auto mockAlarmRepo = std::make_shared<MockAlarmRepository>();
    MonitoringService service(mockSensorSource, mockAlarmRepo);
    
    // Prepare threshold-exceeding vital
    VitalRecord vital;
    vital.heartRate = 150.0; // Exceeds high threshold (typically 120 bpm)
    vital.timestamp = QDateTime::currentDateTime();
    
    // Benchmark loop
    for (auto _ : state) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Trigger alarm detection
        service.processVitalUpdate(vital);
        
        // Wait for alarm raised signal
        QSignalSpy spy(&service, &MonitoringService::alarmRaised);
        spy.wait(100); // Max 100ms timeout
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        );
        
        state.SetIterationTime(elapsed.count() / 1000.0); // Convert to seconds
    }
    
    // Report statistics
    state.SetLabel("Alarm Detection Latency (ms)");
}
BENCHMARK(BM_AlarmDetection_ThresholdExceeded)->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark alarm detection with multiple simultaneous threshold violations
 */
static void BM_AlarmDetection_MultipleVitals(benchmark::State& state) {
    // Similar setup, but send 5 vitals simultaneously (HR, SpO2, NIBP, Resp, Temp)
    // Measure time to detect and raise all 5 alarms
}

BENCHMARK_MAIN();
```

**Acceptance Criteria:**
- ✅ Latency consistently < 50ms (p99 percentile)
- ✅ Measures complete pipeline (data → threshold check → alarm creation → persistence → event)
- ✅ Uses real MonitoringService with mock dependencies
- ✅ Benchmark repeatable (consistent results across runs)

---

### 3. Waveform Rendering Benchmark

**File:** `tests/benchmarks/WaveformRenderingBenchmark.cpp`

**What to Measure:**
- Time to process waveform data for rendering (decimation, ring buffer update, signal generation)
- Target: < 16ms (60 FPS requirement)

**Note:** This benchmarks the **data processing** for waveforms, not QML rendering (which is GPU-dependent). Focus on WaveformController data transformation.

**Benchmark Structure:**
```cpp
/**
 * @brief Benchmark waveform data processing for 60 FPS rendering
 * 
 * Measures WaveformController processing latency: read from cache → decimate → update properties.
 * Target: < 16ms per frame (REQ-PERF-UI-001)
 */
static void BM_WaveformProcessing_SingleFrame(benchmark::State& state) {
    WaveformCache cache;
    WaveformController controller;
    
    // Populate cache with 250 Hz waveform samples (typical ECG rate)
    std::vector<WaveformSample> samples;
    for (int i = 0; i < 250; ++i) { // 1 second of data
        samples.push_back({QDateTime::currentDateTime().addMSecs(i * 4), 
                          WaveformType::ECG, 0.5 + 0.5 * std::sin(i * 0.1)});
    }
    cache.addSamples(samples);
    
    for (auto _ : state) {
        // Benchmark single frame processing (60 FPS = every 16ms)
        controller.updateWaveformData(); // Reads from cache, decimates, updates Q_PROPERTY
    }
    
    state.SetLabel("Waveform Frame Processing (ms)");
}
BENCHMARK(BM_WaveformProcessing_SingleFrame)->Unit(benchmark::kMillisecond);
```

**Acceptance Criteria:**
- ✅ Processing time < 16ms (p99 percentile)
- ✅ Simulates realistic waveform data volume (250 Hz ECG, 60 Hz pleth)
- ✅ Measures ring buffer operations, decimation, data transformation

---

### 4. Database Query Benchmark

**File:** `tests/benchmarks/DatabaseQueryBenchmark.cpp`

**What to Measure:**
- Time-series vitals query performance (critical for TrendsView)
- Alarm history query performance
- Target: < 100ms (REQ-PERF-DB-001)

**Benchmark Structure:**
```cpp
/**
 * @brief Benchmark time-series vitals query for trends display
 * 
 * Measures SQLiteVitalsRepository query performance for 24-hour time range.
 * Target: < 100ms (REQ-PERF-DB-001)
 */
static void BM_DatabaseQuery_Vitals24Hours(benchmark::State& state) {
    // Setup database with realistic data volume
    DatabaseManager dbManager(":memory:");
    SQLiteVitalsRepository vitalsRepo(dbManager);
    
    // Insert 24 hours of vitals (60 samples/hour * 24 hours = 1440 samples per vital type)
    for (int hour = 0; hour < 24; ++hour) {
        for (int minute = 0; minute < 60; ++minute) {
            VitalRecord vital;
            vital.patientMrn = "TEST-12345";
            vital.timestamp = QDateTime::currentDateTime().addSecs(-hour * 3600 - minute * 60);
            vital.heartRate = 70.0 + (rand() % 20 - 10);
            vital.spo2 = 95.0 + (rand() % 5);
            vitalsRepo.save(vital);
        }
    }
    
    for (auto _ : state) {
        // Benchmark 24-hour time-range query
        auto startTime = QDateTime::currentDateTime().addDays(-1);
        auto endTime = QDateTime::currentDateTime();
        auto result = vitalsRepo.getRange("TEST-12345", startTime, endTime);
    }
    
    state.SetLabel("Vitals Query 24h (ms)");
}
BENCHMARK(BM_DatabaseQuery_Vitals24Hours)->Unit(benchmark::kMillisecond);
```

**Acceptance Criteria:**
- ✅ Query time < 100ms for 24-hour range (1440 samples)
- ✅ Tests realistic data volumes
- ✅ Measures query with indices enabled (verify index usage)

---

### 5. Telemetry Batch Processing Benchmark

**File:** `tests/benchmarks/TelemetryBatchBenchmark.cpp`

**What to Measure:**
- Time to compress and serialize 10-minute telemetry batch
- Target: < 5 seconds (REQ-PERF-TELEMETRY-001)

**Benchmark Structure:**
```cpp
/**
 * @brief Benchmark telemetry batch processing (compression + serialization)
 * 
 * Measures TelemetryService batch processing for 10-minute interval.
 * Target: < 5s (REQ-PERF-TELEMETRY-001)
 */
static void BM_TelemetryBatch_10MinuteInterval(benchmark::State& state) {
    TelemetryService service;
    
    // Create 10 minutes of telemetry data (600 vital records + 100 alarms)
    std::vector<VitalRecord> vitals;
    for (int i = 0; i < 600; ++i) { // 1 vital/second * 600 seconds
        VitalRecord vital;
        vital.timestamp = QDateTime::currentDateTime().addSecs(-600 + i);
        vital.heartRate = 70.0 + (rand() % 20 - 10);
        vitals.push_back(vital);
    }
    
    for (auto _ : state) {
        // Benchmark batch compression and serialization
        service.createBatch(vitals);
        auto compressedBatch = service.compressBatch();
    }
    
    state.SetLabel("Telemetry Batch Processing (s)");
}
BENCHMARK(BM_TelemetryBatch_10MinuteInterval)->Unit(benchmark::kSecond);
```

**Acceptance Criteria:**
- ✅ Batch processing < 5 seconds
- ✅ Includes gzip compression overhead
- ✅ Realistic data volume (10 minutes of vitals + alarms)

---

## Integration with CMake

Update `tests/CMakeLists.txt`:

```cmake
# Add benchmarks subdirectory
add_subdirectory(benchmarks)
```

Create `tests/benchmarks/CMakeLists.txt`:

```cmake
# Google Benchmark dependency
include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark tests")
FetchContent_MakeAvailable(benchmark)

# Common libraries for all benchmarks
set(BENCHMARK_LIBS
  benchmark::benchmark
  z_monitor_application
  z_monitor_infrastructure
  zmon_test_mocks_infrastructure
  Qt6::Core
  Qt6::Test
  Qt6::Sql
)

# Alarm Detection Benchmark
add_executable(AlarmDetectionBenchmark AlarmDetectionBenchmark.cpp)
target_link_libraries(AlarmDetectionBenchmark PRIVATE ${BENCHMARK_LIBS})

# Waveform Rendering Benchmark
add_executable(WaveformRenderingBenchmark WaveformRenderingBenchmark.cpp)
target_link_libraries(WaveformRenderingBenchmark PRIVATE ${BENCHMARK_LIBS})

# Database Query Benchmark
add_executable(DatabaseQueryBenchmark DatabaseQueryBenchmark.cpp)
target_link_libraries(DatabaseQueryBenchmark PRIVATE ${BENCHMARK_LIBS})

# Telemetry Batch Benchmark
add_executable(TelemetryBatchBenchmark TelemetryBatchBenchmark.cpp)
target_link_libraries(TelemetryBatchBenchmark PRIVATE ${BENCHMARK_LIBS})

# Add all benchmarks to a custom target
add_custom_target(benchmarks)
add_dependencies(benchmarks
  AlarmDetectionBenchmark
  WaveformRenderingBenchmark
  DatabaseQueryBenchmark
  TelemetryBatchBenchmark
)
```

---

## Running Benchmarks

### Local Execution
```bash
cd /Users/dustinwind/Development/Qt/qtapp/project-dashboard/z-monitor
cmake --build build --target benchmarks -j8

# Run individual benchmarks
./build/tests/benchmarks/AlarmDetectionBenchmark --benchmark_format=json
./build/tests/benchmarks/WaveformRenderingBenchmark
./build/tests/benchmarks/DatabaseQueryBenchmark
./build/tests/benchmarks/TelemetryBatchBenchmark

# Run all benchmarks with JSON output for CI
./build/tests/benchmarks/AlarmDetectionBenchmark --benchmark_format=json > alarm_results.json
```

### CI Integration (Future)
- Add benchmark step to GitHub Actions workflow
- Compare results against baseline (detect regressions > 10%)
- Fail build if performance targets not met

---

## Acceptance Criteria

- [x] Google Benchmark infrastructure set up (CMakeLists.txt configured)
- [x] AlarmDetectionBenchmark implemented and measures < 50ms latency
- [x] WaveformRenderingBenchmark implemented and measures < 16ms processing time
- [x] DatabaseQueryBenchmark implemented and measures < 100ms query time
- [x] TelemetryBatchBenchmark implemented and measures < 5s batch processing
- [x] All benchmarks build successfully
- [x] All benchmarks execute successfully with realistic data
- [x] Performance targets met (verified by benchmark output)
- [x] Documentation updated with benchmark results

---

## Verification Steps

### 1. Functional Verification
- ✅ All benchmarks execute successfully
- ✅ Performance targets met (alarm < 50ms, waveform < 16ms, database < 100ms, telemetry < 5s)
- ✅ Benchmarks use realistic data volumes
- ✅ Results reproducible across multiple runs

**Verification Commands:**
```bash
# Run all benchmarks
cmake --build build --target benchmarks -j8
for bench in AlarmDetectionBenchmark WaveformRenderingBenchmark DatabaseQueryBenchmark TelemetryBatchBenchmark; do
  ./build/tests/benchmarks/$bench --benchmark_min_time=1s
done

# Verify targets met (check output for p99 latencies)
```

### 2. Code Quality Verification
- ✅ Doxygen comments on all benchmark functions
- ✅ No hardcoded values (use constants for thresholds, data volumes)
- ✅ Benchmark code follows C++ guidelines (RAII, smart pointers)
- ✅ Builds without warnings

**Verification Commands:**
```bash
# Check for hardcoded values
grep -r "150\|250\|1440" tests/benchmarks/ --include="*.cpp"

# Build with warnings enabled
cmake --build build --target benchmarks -- VERBOSE=1 2>&1 | grep -i warning
```

### 3. Documentation Verification
- ✅ Update `project-dashboard/doc/legacy/architecture_and_design/40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md` with:
  - Benchmark results (latencies, throughput)
  - Performance targets verification status
  - Optimization recommendations (if any targets missed)
- ✅ Update this prompt file with completion notes
- ✅ Add benchmark execution instructions to BUILD.md or README.md

### 4. Integration Verification
- ✅ Benchmarks build with CMake
- ✅ All dependencies linked correctly
- ✅ Benchmarks use same code paths as production (via shared libraries)
- ✅ Mock objects used where appropriate (sensor data, repositories)

**Verification Commands:**
```bash
# Verify benchmark targets exist
cmake --build build --target help | grep -i benchmark

# Verify dependencies linked
otool -L build/tests/benchmarks/AlarmDetectionBenchmark  # macOS
```

### 5. Tests Verification
- ✅ All 4 benchmark suites implemented (AlarmDetection, WaveformRendering, DatabaseQuery, TelemetryBatch)
- ✅ Each suite has multiple benchmark variations (single threshold, multiple vitals, etc.)
- ✅ Benchmarks cover critical paths identified in requirements
- ✅ Benchmark output includes statistics (mean, median, p99)

### 6. Performance Verification
- ✅ Alarm detection: < 50ms (p99)
- ✅ Waveform processing: < 16ms (p99)
- ✅ Database queries: < 100ms (p99)
- ✅ Telemetry batch: < 5s (p99)

**Document results in format:**
```
Benchmark Results (2025-12-03):
- AlarmDetection_ThresholdExceeded: 23.4ms (p99: 45.2ms) ✅
- WaveformProcessing_SingleFrame: 8.1ms (p99: 12.3ms) ✅
- DatabaseQuery_Vitals24Hours: 67.8ms (p99: 89.2ms) ✅
- TelemetryBatch_10MinuteInterval: 2.3s (p99: 3.8s) ✅
```

---

## Dependencies

- Google Benchmark (v1.8.3 or later)
- MonitoringService (alarm detection)
- WaveformController (waveform processing)
- SQLiteVitalsRepository, SQLiteAlarmRepository (database queries)
- TelemetryService (batch processing)
- Mock objects (MockSensorDataSource, MockAlarmRepository, etc.)

---

## Documentation References

- `project-dashboard/doc/legacy/architecture_and_design/40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md` - Performance requirements
- `project-dashboard/doc/legacy/architecture_and_design/42_LOW_LATENCY_TECHNIQUES.md` - Optimization strategies
- `project-dashboard/doc/legacy/architecture_and_design/12_THREAD_MODEL.md` - Thread architecture
- `.cursor/rules/cpp_guidelines.mdc` - C++ coding standards

---

## Implementation Notes

- **QCoreApplication Required:** Benchmarks using Qt signals/slots require `QCoreApplication::instance()`. Create app in `main()` or each benchmark.
- **Database Isolation:** Use in-memory databases (`:memory:`) for database benchmarks to avoid filesystem I/O overhead.
- **Mock Sensor Data:** Generate realistic sensor data patterns (ECG waveforms, vital trends) for accurate benchmarks.
- **Warmup Runs:** Google Benchmark automatically performs warmup runs - don't disable this for accurate results.
- **Statistical Significance:** Run benchmarks with `--benchmark_min_time=2s` to ensure statistical significance.

---

## Success Criteria

Task complete when:
1. All 4 benchmark suites implemented and building
2. All benchmarks execute successfully
3. All performance targets met (verified by output)
4. Documentation updated with results
5. ZTODO.md verification steps marked complete with details

---

**Prompt File Version:** 1.0
**Last Updated:** 2025-12-03
**Task Status:** Ready for implementation
