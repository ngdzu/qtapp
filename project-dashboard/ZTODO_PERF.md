# Z Monitor Development Tasks - PERF

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

---

## Performance Tasks

- [ ] TASK-PERF-001: Measure end-to-end latency from simulator to UI
  - What: Measure the complete latency from when simulator writes data to shared memory until it appears on screen. Target: < 50ms end-to-end. Instrument code at key points: (1) simulator write timestamp, (2) SharedMemorySensorDataSource read timestamp, (3) MonitoringService processing timestamp, (4) Controller property update timestamp, (5) QML render timestamp. Calculate deltas and report average/max latency.
  - Why: Latency directly impacts clinical usability. Medical monitors require near-real-time display (<50ms) to enable timely intervention. High latency can delay arrhythmia detection or vital sign changes, impacting patient safety.
  - Files:
    - Update: `sensor-simulator/src/SharedMemoryRingBuffer.cpp` (add write timestamp logging)
    - Update: `z-monitor/src/infrastructure/sensors/SharedMemorySensorDataSource.cpp` (add read timestamp logging)
    - Update: `z-monitor/src/application/services/MonitoringService.cpp` (add processing timestamp logging)
    - Update: `z-monitor/src/interface/controllers/DashboardController.cpp` (add property update timestamp logging)
    - Create: `scripts/measure_latency.py` (parse logs and calculate latency statistics)
  - Dependencies:
    - Simulator and z-monitor running with live data
    - High-resolution timestamps available (std::chrono::high_resolution_clock)
    - Logging enabled at INFO level
  - Implementation Details:
    - Add timestamps at each pipeline stage:
      ```cpp
      // Simulator write
      auto writeTime = std::chrono::high_resolution_clock::now();
      frame.timestampNs = std::chrono::duration_cast<std::chrono::nanoseconds>(writeTime.time_since_epoch()).count();
      
      // SharedMemorySensorDataSource read
      auto readTime = std::chrono::high_resolution_clock::now();
      qInfo() << "Latency: frame written at" << frame.timestampNs << "read at" << readTime;
      ```
    - Parse logs to extract timestamps and calculate deltas:
      ```python
      import re, statistics
      write_times = []
      read_times = []
      latencies = [read - write for write, read in zip(write_times, read_times)]
      print(f"Avg latency: {statistics.mean(latencies):.2f}ms")
      print(f"Max latency: {max(latencies):.2f}ms")
      ```
  - Acceptance:
    - Average end-to-end latency < 50ms (target)
    - Max latency < 100ms (acceptable)
    - 99th percentile latency < 75ms
    - Latency breakdown by stage documented (identify bottlenecks)
    - No latency spikes > 200ms during 5-minute test
  - Verification Steps:
    1. Instrumentation: Add timestamps at all 5 pipeline stages, verify logging works **Status:** ⏳ Pending
    2. Measurement: Run for 5 minutes, collect 300+ samples, calculate statistics **Status:** ⏳ Pending
    3. Analysis: Identify slowest stage, verify meets target, document results **Status:** ⏳ Pending
  - Prompt: `project-dashboard/prompt/44f-measure-end-to-end-latency.md`

- [ ] TASK-PERF-002: Performance profiling and optimization
  - What: Profile z-monitor with Qt QML Profiler and identify performance bottlenecks. Measure frame rate (should be 60 FPS), Canvas rendering time (< 10ms per frame), property binding overhead, and memory usage. Optimize any code paths that exceed targets. Document optimization results.
  - Why: Ensures the UI remains responsive under continuous data load. Poor performance (frame drops, high latency) degrades user experience and can impact clinical usability. Profiling identifies specific bottlenecks (e.g., excessive Canvas redraws, slow property bindings) that can be optimized.
  - Files:
    - Create: `project-dashboard/profiling/z-monitor-baseline-profile.qtd` (QML Profiler data)
    - Create: `project-dashboard/profiling/z-monitor-performance-report.md` (profiling results and optimizations)
    - Update: `z-monitor/resources/qml/components/WaveformPanel.qml` (optimization if needed)
  - Dependencies:
    - Qt QML Profiler installed (part of Qt SDK)
    - z-monitor running with live data
    - Simulator generating full data load (60 Hz vitals + 250 Hz waveforms)
  - Implementation Details:
    - Launch QML Profiler: `qmlprofiler -attach localhost:9999 -o profile.qtd`
    - Start z-monitor with profiling enabled: `QML_PROFILER_PORT=9999 ./z-monitor`
    - Run for 60 seconds to capture steady-state behavior
    - Analyze profile in Qt Creator: Scene Graph, JavaScript, Memory
    - Identify bottlenecks:
      - Canvas onPaint taking > 10ms → reduce point count or simplify drawing
      - Property bindings updating too frequently → use Connections with throttling
      - Memory leaks → check for QML object lifecycle issues
  - Acceptance:
    - Sustained 60 FPS during normal operation (no frame drops)
    - Canvas rendering < 10ms per frame (WaveformPanel.qml)
    - Property binding updates < 5ms total per cycle
    - Memory usage stable (no leaks over 5-minute run)
    - CPU usage < 30% on target hardware
    - Profiling report documents baseline and any optimizations applied
  - Verification Steps:
    1. Profile: Capture 60-second profile with QML Profiler **Status:** ⏳ Pending
    2. Analysis: Identify bottlenecks, prioritize by impact **Status:** ⏳ Pending
    3. Optimization: Apply optimizations, re-profile, verify improvements **Status:** ⏳ Pending
    4. Documentation: Document baseline, optimizations, final metrics **Status:** ⏳ Pending
  - Prompt: `project-dashboard/prompt/44f-performance-profiling-optimization.md`

- [ ] TASK-PERF-003: Implement Benchmark Framework and Performance Measurement
  - What: Add Google Benchmark library to CMake, create benchmark directory structure (`tests/benchmarks/core/`, `tests/benchmarks/ui/`, `tests/benchmarks/network/`, `tests/benchmarks/integration/`), implement critical benchmarks (alarm detection latency, database query performance, UI response time, memory usage). Create benchmark comparison script (`scripts/compare_benchmarks.py`) and setup script (`scripts/setup_benchmark_env.sh`). Integrate into CI/CD with nightly execution workflow.
  - Why: Performance benchmarks are critical for safety-critical requirements, especially alarm detection latency (< 50ms). Automated nightly benchmarks detect performance regressions early and ensure system meets all performance targets.
  - Files:
    - `z-monitor/tests/benchmarks/core/alarm_detection.cpp` (REQ-NFR-PERF-100)
    - `z-monitor/tests/benchmarks/core/database_queries.cpp` (REQ-NFR-PERF-110)
    - `z-monitor/tests/benchmarks/core/database_writes.cpp` (REQ-NFR-PERF-111)
    - `z-monitor/tests/benchmarks/core/memory_usage.cpp` (REQ-NFR-HW-001)
    - `z-monitor/tests/benchmarks/ui/response_time.cpp` (REQ-NFR-PERF-001)
    - `z-monitor/tests/benchmarks/ui/display_refresh.cpp` (REQ-NFR-PERF-101)
    - `z-monitor/tests/benchmarks/network/telemetry_latency.cpp` (REQ-NFR-PERF-200)
    - `z-monitor/tests/benchmarks/integration/end_to_end_latency.cpp`
    - `z-monitor/scripts/compare_benchmarks.py` (compare with baseline, generate reports)
    - `z-monitor/scripts/setup_benchmark_env.sh` (isolate benchmark environment)
    - `.github/workflows/nightly-benchmarks.yml` (nightly CI workflow)
    - `.github/workflows/pr-benchmarks.yml` (PR benchmark comparison)
  - Acceptance:
    - Google Benchmark integrated into CMake build system
    - All critical benchmarks implemented (alarm detection, database, UI, memory, network)
    - Benchmarks run successfully and produce JSON/CSV output
    - Comparison script compares current vs baseline and generates HTML reports
    - Nightly workflow runs all benchmarks and stores results
    - PR workflow runs component benchmarks and posts comparison comments
    - Regression detection works (fail if > 10% regression for critical benchmarks)
    - Performance dashboard generated from benchmark results
  - Verification Steps:
    1. Functional: All benchmarks run successfully, produce valid JSON output, comparison script works, regression detection works, nightly workflow executes, PR workflow posts comments
    2. Code Quality: Benchmarks follow established patterns (pre-allocate data, measure critical path only, statistical reporting), Doxygen comments on all benchmarks, linter passes
    3. Documentation: `project-dashboard/doc/legacy/architecture_and_design/40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md` complete, benchmark examples documented, CI/CD integration documented, performance targets documented
    4. Integration: CMake builds benchmarks, CI workflows execute successfully, results stored correctly, comparison reports generated
    5. Tests: Benchmark framework tests, comparison script tests, verify benchmarks meet performance targets, verify regression detection works
  - Documentation: See `project-dashboard/doc/legacy/architecture_and_design/40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md` for complete benchmark strategy, framework design, CI/CD integration, and nightly execution workflow. See `project-dashboard/doc/legacy/architecture_and_design/18_TESTING_WORKFLOW.md` for testing workflow integration.
  - Prompt: `project-dashboard/prompt/40-implement-benchmark-framework.md`

- [ ] TASK-PERF-004: Integrate benchmarking harness
  - What: Add Google Benchmark targets for AlarmManager, SignalProcessor, telemetry serialization, and certificate validation routines. Store results (CSV/JSON) for regression tracking.
  - Why: Detects performance regressions early.

- [ ] TASK-MONITOR-002: Implement Application Metrics Collection with Prometheus
  - What: Integrate Prometheus client library to collect application metrics: alarm detection latency (histogram), waveform rendering FPS (gauge), database query duration (histogram), telemetry upload success/failure (counter), active patient count (gauge), memory usage (gauge). Expose metrics endpoint at /metrics for Prometheus scraper. Create Grafana dashboard for visualization.
  - Why: Metrics enable observability and performance monitoring. Prometheus provides industry-standard metrics collection. Grafana dashboards enable real-time monitoring and alerting.
  - Files:
    - src/infrastructure/monitoring/MetricsCollector.h/cpp
    - src/infrastructure/monitoring/PrometheusExporter.h/cpp
    - grafana/dashboards/z-monitor-dashboard.json
    - prometheus/prometheus.yml (scraper configuration)
    - Update CMakeLists.txt (add prometheus-cpp dependency)
  - Acceptance: Metrics collected for all critical paths, Prometheus endpoint exposed, metrics scraped successfully, Grafana dashboard displays metrics, alerting configured for critical metrics (alarm latency > 50ms).
  - Verification Steps:
    1. Functional: Metrics collected, Prometheus scrapes successfully, Grafana displays dashboards
    2. Code Quality: Doxygen comments, minimal performance overhead (< 1% CPU)
    3. Documentation: Update project-dashboard/doc/legacy/architecture_and_design/40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md with metrics infrastructure
    4. Integration: Works with Prometheus and Grafana, metrics available in production
    5. Tests: Metrics collection test, Prometheus endpoint test
    6. Performance: Metrics collection overhead measured (< 1% CPU, < 10 MB memory)
  - Dependencies: prometheus-cpp library, Prometheus server, Grafana
  - Documentation: See project-dashboard/doc/legacy/architecture_and_design/40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md for performance monitoring design.
  - Prompt: project-dashboard/prompt/TASK-MONITOR-002-prometheus-metrics.md
