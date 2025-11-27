# Benchmark and Performance Measurement Strategy

**Document ID:** DESIGN-040  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document defines the benchmark and performance measurement strategy for the Z Monitor, including framework design, measurement targets, CI/CD integration, and nightly execution workflow.

## 1. Overview

Performance benchmarks are critical for the Z Monitor due to safety-critical requirements, particularly alarm detection latency (< 50ms). This document establishes:

- **Benchmark Framework:** Tools, structure, and patterns for writing benchmarks
- **Performance Targets:** Measurable targets derived from non-functional requirements
- **Measurement Strategy:** How to measure, collect, and analyze performance data
- **CI/CD Integration:** Automated nightly benchmark execution and regression detection
- **Trend Analysis:** Historical tracking and performance degradation alerts

## 2. Related Documents

- **Performance Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](../../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md) - Section 3: Performance Requirements
- **Testing Workflow:** [18_TESTING_WORKFLOW.md](./18_TESTING_WORKFLOW.md) - Benchmark policy and integration
- **Thread Model:** [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Latency targets and thread priorities
- **Data Caching Strategy:** [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) - Critical path performance

## 3. Performance Requirements Summary

| Requirement ID | Target | Criticality | Benchmark |
|----------------|--------|-------------|-----------|
| REQ-NFR-PERF-001 | UI response < 500ms | Must Have | `bench_ui_response_time` |
| REQ-NFR-PERF-100 | Alarm detection < 50ms | **CRITICAL** | `bench_alarm_detection_latency` |
| REQ-NFR-PERF-101 | Display refresh 60 FPS | Must Have | `bench_display_refresh_rate` |
| REQ-NFR-PERF-110 | DB query < 100ms (95th percentile) | Must Have | `bench_database_query_performance` |
| REQ-NFR-PERF-111 | DB write throughput ≥ 10 rec/sec | Must Have | `bench_database_write_throughput` |
| REQ-NFR-PERF-200 | Network latency < 500ms | Should Have | `bench_network_latency` |
| REQ-NFR-HW-001 | Memory usage < 1.5 GB | Must Have | `bench_memory_usage` |

## 4. Benchmark Framework

### 4.1. Tool Selection: Google Benchmark

**Rationale:**
- Industry-standard C++ benchmarking library
- Statistical analysis (mean, median, standard deviation, percentiles)
- CPU throttling detection and warnings
- JSON/CSV output for CI/CD integration
- Cross-platform (Linux, macOS, Windows)
- Integrates with CMake build system

**Installation:**
```cmake
# CMakeLists.txt
find_package(benchmark REQUIRED)
target_link_libraries(benchmarks PRIVATE benchmark::benchmark)
```

### 4.2. Benchmark Structure

```
tests/benchmarks/
  core/
    alarm_detection.cpp          # REQ-NFR-PERF-100: Alarm latency
    database_queries.cpp          # REQ-NFR-PERF-110: Query performance
    database_writes.cpp           # REQ-NFR-PERF-111: Write throughput
    memory_usage.cpp              # REQ-NFR-HW-001: Memory profiling
  ui/
    response_time.cpp             # REQ-NFR-PERF-001: UI responsiveness
    display_refresh.cpp           # REQ-NFR-PERF-101: Frame rate
  network/
    telemetry_latency.cpp         # REQ-NFR-PERF-200: Network latency
  integration/
    end_to_end_latency.cpp        # Full workflow latency
    data_flow_latency.cpp         # Sensor → Cache → DB → Server
```

### 4.3. Benchmark Pattern

**Example: Alarm Detection Latency (Critical Path)**

```cpp
#include <benchmark/benchmark.h>
#include "AlarmManager.h"
#include "VitalsCache.h"
#include "VitalRecord.h"

/**
 * @brief Benchmark alarm detection latency (REQ-NFR-PERF-100)
 * 
 * Target: < 50ms (CRITICAL)
 * Measures: Time from vital sign threshold violation to alarm trigger
 */
static void BM_AlarmDetectionLatency(benchmark::State& state) {
    // Setup: Create AlarmManager with pre-configured thresholds
    AlarmManager alarmManager;
    alarmManager.setThreshold("heart_rate", 120, "bpm");
    
    // Pre-allocate vital records (no allocations in critical path)
    std::vector<VitalRecord> vitals;
    vitals.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
        VitalRecord v;
        v.heartRate = 100 + (i % 50);  // Mix of normal and threshold violations
        v.timestamp = QDateTime::currentDateTime().addMSecs(i * 100);
        vitals.push_back(v);
    }
    
    int alarmCount = 0;
    for (auto _ : state) {
        // Measure: Process vitals and detect alarms
        for (const auto& vital : vitals) {
            auto start = std::chrono::high_resolution_clock::now();
            bool triggered = alarmManager.checkThresholds(vital);
            auto end = std::chrono::high_resolution_clock::now();
            
            if (triggered) {
                auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                state.SetIterationTime(latency.count() / 1000.0);  // Convert to milliseconds
                alarmCount++;
            }
        }
    }
    
    // Report: Median latency (most relevant for safety-critical)
    state.SetLabel("Alarm Detection");
    state.counters["alarms_triggered"] = alarmCount;
    state.counters["median_latency_ms"] = benchmark::Counter(
        state.iterations() > 0 ? state.iterations() : 1,
        benchmark::Counter::kAvgThreads
    );
}

// Register benchmark with iterations and time limit
BENCHMARK(BM_AlarmDetectionLatency)
    ->Iterations(1000)                    // 1000 alarm checks
    ->Unit(benchmark::kMillisecond)        // Report in milliseconds
    ->Repetitions(10)                       // 10 repetitions for statistics
    ->ReportAggregatesOnly(false)          // Show all statistics
    ->ComputeStatistics("median", [](const std::vector<double>& v) {
        std::vector<double> sorted = v;
        std::sort(sorted.begin(), sorted.end());
        return sorted[sorted.size() / 2];
    });

BENCHMARK_MAIN();
```

**Key Patterns:**
1. **Pre-allocate data:** No memory allocations during benchmark
2. **Measure critical path only:** Exclude setup/teardown from timing
3. **Statistical reporting:** Use median (not mean) for safety-critical metrics
4. **Realistic data:** Use production-like data volumes and patterns
5. **Multiple iterations:** Run enough iterations for statistical significance

### 4.4. Benchmark Categories

#### 4.4.1. Micro-benchmarks
- **Purpose:** Measure individual function/algorithm performance
- **Examples:** String operations, container lookups, hash calculations
- **Frequency:** On-demand during development
- **Target:** Identify performance bottlenecks

#### 4.4.2. Component Benchmarks
- **Purpose:** Measure service/component performance in isolation
- **Examples:** AlarmManager latency, DatabaseManager query time
- **Frequency:** Pre-commit, CI gate
- **Target:** Detect component-level regressions

#### 4.4.3. Integration Benchmarks
- **Purpose:** Measure end-to-end workflows
- **Examples:** Sensor → Cache → Alarm → UI, Patient admission workflow
- **Frequency:** Nightly CI
- **Target:** Detect system-level performance degradation

#### 4.4.4. Resource Benchmarks
- **Purpose:** Measure memory, CPU, disk usage
- **Examples:** Memory footprint, database size growth
- **Frequency:** Nightly CI
- **Target:** Detect resource leaks and growth

## 5. Performance Measurement Targets

### 5.1. Alarm Detection Latency (CRITICAL)

**Requirement:** REQ-NFR-PERF-100  
**Target:** < 50ms (95th percentile)  
**Measurement:** Time from vital sign threshold violation to alarm trigger

**Benchmark Configuration:**
- **Iterations:** 1000 alarm checks
- **Repetitions:** 10 (for statistical significance)
- **Data:** Mix of normal vitals and threshold violations
- **Environment:** Release build, dedicated CI runner (no noisy neighbors)
- **Pass Criteria:** Median latency < 50ms, 95th percentile < 50ms

**Regression Threshold:** Fail if median increases by > 10% vs baseline

### 5.2. UI Response Time

**Requirement:** REQ-NFR-PERF-001  
**Target:** < 500ms (95th percentile)  
**Measurement:** Time from user interaction (tap/click) to visual feedback

**Benchmark Configuration:**
- **Scenarios:** Button tap, screen navigation, form submission
- **Iterations:** 100 interactions per scenario
- **Environment:** Headless Xvfb or VNC for CI
- **Pass Criteria:** 95th percentile < 500ms

### 5.3. Database Query Performance

**Requirement:** REQ-NFR-PERF-110  
**Target:** < 100ms (95th percentile)  
**Measurement:** Time to execute common queries (patient lookup, vitals query, trend data)

**Benchmark Configuration:**
- **Queries:** Patient by MRN, vitals by time range, trend aggregation
- **Data Volume:** 7 days of vitals (~2.6M records)
- **Iterations:** 100 queries per type
- **Pass Criteria:** 95th percentile < 100ms

### 5.4. Database Write Throughput

**Requirement:** REQ-NFR-PERF-111  
**Target:** ≥ 10 records/second sustained  
**Measurement:** Sustained write rate without blocking

**Benchmark Configuration:**
- **Duration:** 60 seconds sustained writes
- **Batch Size:** 10 records per transaction
- **Pass Criteria:** Average throughput ≥ 10 rec/sec, no blocking

### 5.5. Display Refresh Rate

**Requirement:** REQ-NFR-PERF-101  
**Target:** 60 FPS (frame drops < 1%)  
**Measurement:** Actual frame rate during normal operation

**Benchmark Configuration:**
- **Duration:** 60 seconds continuous rendering
- **Scenarios:** Normal monitoring, alarm display, trend view
- **Pass Criteria:** Average FPS ≥ 60, frame drops < 1%

### 5.6. Network Latency

**Requirement:** REQ-NFR-PERF-200  
**Target:** < 500ms end-to-end  
**Measurement:** Time from data creation to server acknowledgment

**Benchmark Configuration:**
- **Scenarios:** Telemetry sync, patient lookup, alarm notification
- **Iterations:** 100 requests per scenario
- **Network:** Simulated hospital network (10-50ms RTT)
- **Pass Criteria:** 95th percentile < 500ms

### 5.7. Memory Usage

**Requirement:** REQ-NFR-HW-001  
**Target:** < 1.5 GB total RAM  
**Measurement:** Peak memory usage during normal operation

**Benchmark Configuration:**
- **Duration:** 1 hour continuous operation
- **Scenarios:** Normal monitoring, alarm handling, patient admission
- **Measurement:** Peak RSS (Resident Set Size)
- **Pass Criteria:** Peak memory < 1.5 GB

## 6. Benchmark Execution

### 6.1. Local Development

**Run All Benchmarks:**
```bash
./scripts/run_tests.sh bench
```

**Run Specific Benchmark:**
```bash
cd build
./tests/benchmarks/core/alarm_detection --benchmark_filter=BM_AlarmDetectionLatency
```

**Run with Custom Iterations:**
```bash
./tests/benchmarks/core/alarm_detection \
  --benchmark_iterations=5000 \
  --benchmark_repetitions=20 \
  --benchmark_out=results.json \
  --benchmark_out_format=json
```

### 6.2. CI/CD Integration

#### 6.2.1. Pre-commit (Fast Benchmarks)

**Purpose:** Catch obvious regressions before commit  
**Benchmarks:** Micro-benchmarks only (fast, < 30 seconds)  
**Failure:** Block commit if regression > 20%

```yaml
# .github/workflows/pre-commit.yml
- name: Run Fast Benchmarks
  run: |
    ./scripts/run_tests.sh bench --filter="micro"
    ./scripts/compare_benchmarks.py --threshold=0.20
```

#### 6.2.2. Pull Request (Component Benchmarks)

**Purpose:** Validate component performance before merge  
**Benchmarks:** Component benchmarks (moderate, < 5 minutes)  
**Failure:** Comment on PR with benchmark comparison, fail if regression > 10%

```yaml
# .github/workflows/pr.yml
- name: Run Component Benchmarks
  run: |
    ./scripts/run_tests.sh bench --filter="component"
    ./scripts/compare_benchmarks.py \
      --baseline=main \
      --threshold=0.10 \
      --comment-on-pr
```

#### 6.2.3. Nightly (Full Benchmark Suite)

**Purpose:** Comprehensive performance monitoring and trend analysis  
**Benchmarks:** All benchmarks (full suite, ~30 minutes)  
**Actions:** Store results, generate reports, alert on regressions

```yaml
# .github/workflows/nightly.yml
name: Nightly Benchmarks

on:
  schedule:
    - cron: '0 2 * * *'  # 2 AM UTC daily
  workflow_dispatch:     # Manual trigger

jobs:
  benchmarks:
    runs-on: self-hosted-benchmark-runner  # Dedicated hardware
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup Benchmark Environment
        run: |
          # Install dependencies, setup isolated environment
          ./scripts/setup_benchmark_env.sh
      
      - name: Run Full Benchmark Suite
        run: |
          ./scripts/run_tests.sh bench --all \
            --output=benchmark_results_$(date +%Y%m%d).json \
            --format=json
      
      - name: Store Benchmark Results
        uses: actions/upload-artifact@v3
        with:
          name: benchmark-results-${{ github.run_number }}
          path: benchmark_results_*.json
          retention-days: 90
      
      - name: Compare with Baseline
        run: |
          ./scripts/compare_benchmarks.py \
            --baseline=main \
            --current=benchmark_results_*.json \
            --threshold=0.10 \
            --generate-report
      
      - name: Upload Performance Report
        uses: actions/upload-artifact@v3
        with:
          name: performance-report-${{ github.run_number }}
          path: performance_report.html
      
      - name: Alert on Regression
        if: failure()
        run: |
          ./scripts/alert_performance_regression.py \
            --report=performance_report.html \
            --slack-webhook=${{ secrets.SLACK_WEBHOOK }}
```

### 6.3. Benchmark Comparison Script

**Purpose:** Compare current benchmark results with baseline  
**Location:** `scripts/compare_benchmarks.py`

**Features:**
- Load baseline results (from main branch or previous run)
- Compare current results with baseline
- Calculate percentage change
- Generate HTML report with charts
- Fail if regression exceeds threshold
- Post comment on PR with comparison

**Usage:**
```bash
./scripts/compare_benchmarks.py \
  --baseline=baseline.json \
  --current=current.json \
  --threshold=0.10 \
  --output=comparison_report.html
```

## 7. Benchmark Results Storage and Analysis

### 7.1. Storage Format

**JSON Format (for CI/CD):**
```json
{
  "benchmark_name": "BM_AlarmDetectionLatency",
  "timestamp": "2025-11-27T02:00:00Z",
  "git_commit": "abc123def456",
  "git_branch": "main",
  "environment": {
    "os": "Linux",
    "cpu": "ARM Cortex-A72",
    "ram": "4GB"
  },
  "results": {
    "iterations": 1000,
    "repetitions": 10,
    "median_ms": 45.2,
    "mean_ms": 46.8,
    "stddev_ms": 3.1,
    "p95_ms": 52.1,
    "p99_ms": 58.3,
    "min_ms": 38.1,
    "max_ms": 62.4
  },
  "counters": {
    "alarms_triggered": 1000,
    "median_latency_ms": 45.2
  }
}
```

### 7.2. Trend Analysis

**Storage:** Store benchmark results in time-series database or flat files with versioning

**Analysis:**
- **Daily Trends:** Track performance over time
- **Regression Detection:** Alert when performance degrades
- **Performance Improvements:** Track optimizations and their impact
- **Hardware Comparison:** Compare performance across different hardware

**Tools:**
- **Grafana:** Visualize benchmark trends over time
- **Custom Scripts:** Generate HTML reports with charts (Chart.js, Plotly)
- **GitHub Actions Artifacts:** Store results for 90 days

### 7.3. Performance Dashboard

**Purpose:** Visualize benchmark trends and current performance status

**Components:**
1. **Current Performance:** Latest benchmark results vs targets
2. **Trend Charts:** Performance over time (last 30 days)
3. **Regression Alerts:** Recent performance regressions
4. **Hardware Comparison:** Performance across different test environments

**Location:** `docs/performance_dashboard.html` (generated nightly)

## 8. Regression Detection and Alerts

### 8.1. Regression Thresholds

| Benchmark Category | Regression Threshold | Action |
|-------------------|---------------------|--------|
| Critical (Alarm Detection) | > 5% increase | **FAIL BUILD** + Alert |
| Must Have | > 10% increase | **FAIL BUILD** + Comment on PR |
| Should Have | > 20% increase | Warning comment on PR |
| Resource Usage | > 15% increase | Warning + Investigation |

### 8.2. Alert Mechanisms

1. **CI/CD Failure:** Build fails if critical benchmarks regress
2. **PR Comments:** Automated comment with benchmark comparison
3. **Slack/Email:** Alert team on significant regressions
4. **Performance Dashboard:** Visual indicators for regressions

### 8.3. Investigation Workflow

When regression detected:

1. **Verify:** Re-run benchmarks to confirm (may be flaky)
2. **Identify:** Check git history for recent changes
3. **Analyze:** Profile code to identify bottleneck
4. **Fix:** Implement optimization or revert change
5. **Validate:** Re-run benchmarks to confirm fix

## 9. Benchmark Environment

### 9.1. Hardware Requirements

**Dedicated CI Runner:**
- **CPU:** ARM Cortex-A72 or equivalent (matches target hardware)
- **RAM:** 4 GB minimum
- **OS:** Linux (Ubuntu 22.04 LTS)
- **Isolation:** Dedicated runner (no noisy neighbors)
- **Consistency:** Same hardware for all benchmark runs

### 9.2. Environment Setup

**Isolation:**
- Disable CPU frequency scaling (performance governor)
- Disable background services
- Set CPU affinity for benchmark process
- Disable swap (ensure consistent memory behavior)

**Script:** `scripts/setup_benchmark_env.sh`
```bash
#!/bin/bash
# Set CPU governor to performance
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Disable swap
sudo swapoff -a

# Set process priority
sudo renice -20 -p $$

# Disable background services (if possible)
sudo systemctl stop unnecessary-services
```

### 9.3. Build Configuration

**Release Build:**
- Optimizations enabled (`-O3` or `-O2`)
- Debug symbols disabled (or separate debug build)
- Link-time optimization (LTO) enabled
- Profile-guided optimization (PGO) if available

**CMake Configuration:**
```cmake
# Benchmark build configuration
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)  # LTO
```

## 10. Integration with Development Workflow

### 10.1. Pre-commit Hooks

**Fast Benchmarks:**
- Run micro-benchmarks (< 30 seconds)
- Block commit if regression > 20%
- Allow bypass with `--no-verify` (with justification)

### 10.2. Pull Request Workflow

**Component Benchmarks:**
- Run on PR creation/update
- Post benchmark comparison as PR comment
- Fail CI if regression > 10% (critical benchmarks)
- Require approval if regression > 5% (critical benchmarks)

### 10.3. Nightly Workflow

**Full Benchmark Suite:**
- Run all benchmarks (micro, component, integration, resource)
- Store results for trend analysis
- Generate performance dashboard
- Alert on regressions

### 10.4. Release Workflow

**Performance Validation:**
- Run full benchmark suite before release
- Compare with previous release baseline
- Generate performance report for release notes
- Document any known performance changes

## 11. Benchmark Maintenance

### 11.1. Adding New Benchmarks

**Process:**
1. Identify performance requirement (from REQ-NFR-PERF-###)
2. Write benchmark following established patterns
3. Set performance target and regression threshold
4. Add to appropriate benchmark category
5. Update CI/CD workflows if needed
6. Document in this document

### 11.2. Updating Baseline

**When:**
- After intentional performance optimizations
- After hardware changes
- After significant architectural changes

**Process:**
1. Run full benchmark suite
2. Verify all benchmarks pass
3. Commit results as new baseline
4. Update performance targets if needed
5. Document changes

### 11.3. Benchmark Cleanup

**Remove Benchmarks:**
- When feature is removed
- When requirement is obsolete
- When benchmark is no longer relevant

**Archive Benchmarks:**
- Keep historical results for trend analysis
- Document why benchmark was removed

## 12. Example Benchmark Implementations

### 12.1. Alarm Detection Latency

See Section 4.3 for complete example.

### 12.2. Database Query Performance

```cpp
static void BM_DatabaseQueryPatientByMRN(benchmark::State& state) {
    DatabaseManager dbManager;
    dbManager.initialize(":memory:");  // In-memory database
    
    // Setup: Insert test data
    for (int i = 0; i < 1000; ++i) {
        Patient p;
        p.mrn = QString("MRN-%1").arg(i);
        p.name = QString("Patient %1").arg(i);
        dbManager.savePatient(p);
    }
    
    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        auto patient = dbManager.findPatientByMRN("MRN-500");
        auto end = std::chrono::high_resolution_clock::now();
        
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        state.SetIterationTime(latency.count() / 1000.0);
        
        benchmark::DoNotOptimize(patient);  // Prevent optimization
    }
    
    state.SetLabel("Patient Query by MRN");
}

BENCHMARK(BM_DatabaseQueryPatientByMRN)
    ->Iterations(100)
    ->Unit(benchmark::kMillisecond)
    ->Repetitions(10);
```

### 12.3. Memory Usage

```cpp
static void BM_MemoryUsagePeak(benchmark::State& state) {
    for (auto _ : state) {
        // Simulate normal operation
        VitalsCache cache;
        cache.setCapacity(3 * 24 * 60 * 60 * 10);  // 3 days at 10 Hz
        
        // Fill cache
        for (int i = 0; i < 100000; ++i) {
            VitalRecord v;
            v.heartRate = 70 + (i % 40);
            v.timestamp = QDateTime::currentDateTime().addMSecs(i * 100);
            cache.addVital(v);
        }
        
        // Measure peak memory
        long peakRSS = getPeakRSS();  // Platform-specific function
        state.counters["peak_rss_mb"] = benchmark::Counter(
            peakRSS / 1024.0 / 1024.0,
            benchmark::Counter::kAvgThreads
        );
    }
    
    state.SetLabel("Peak Memory Usage");
}

BENCHMARK(BM_MemoryUsagePeak)
    ->Iterations(10)
    ->Repetitions(5);
```

## 13. Next Steps

1. **Implement Benchmark Framework:**
   - Add Google Benchmark to CMake
   - Create benchmark directory structure
   - Implement first benchmark (alarm detection)

2. **CI/CD Integration:**
   - Add benchmark jobs to GitHub Actions
   - Implement comparison script
   - Set up nightly workflow

3. **Performance Dashboard:**
   - Create HTML dashboard template
   - Implement trend visualization
   - Set up artifact storage

4. **Documentation:**
   - Add benchmark examples to developer guide
   - Document performance targets
   - Create benchmark writing guidelines

## 14. References

- **Google Benchmark:** https://github.com/google/benchmark
- **Performance Requirements:** [04_NON_FUNCTIONAL_REQUIREMENTS.md](../../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md)
- **Testing Workflow:** [18_TESTING_WORKFLOW.md](./18_TESTING_WORKFLOW.md)
- **Thread Model:** [12_THREAD_MODEL.md](./12_THREAD_MODEL.md)

