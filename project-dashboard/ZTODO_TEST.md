# Z Monitor Development Tasks - TEST

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

---

## Testing Tasks

- [ ] TASK-TEST-001: Implement Build Verification Script (verify-build.sh)
  - What: Create `scripts/verify-build.sh` shell script that takes a list of modified/created files as arguments, automatically identifies affected CMake targets and their dependents, and builds all affected targets to ensure changes don't break the build. Script must: (1) Parse each file path to find nearest CMakeLists.txt, (2) Identify which target(s) include that file (parse CMakeLists.txt or use CMake API), (3) Build each affected target, (4) Query CMake dependency graph to find all targets that depend on affected targets (transitive dependencies), (5) Build all dependent targets, (6) Report success/failure with clear output.
  - Why: Automates build verification during ZTODO task verification; ensures code changes don't break compilation; catches build issues early; required for all verification workflows.
  - Files: `scripts/verify-build.sh` (main script), `scripts/cmake-helpers/find-affected-targets.sh` (helper to parse CMakeLists.txt), `scripts/cmake-helpers/find-dependent-targets.sh` (helper to query dependency graph), documentation in `.github/ztodo_verification.md` (already updated).
  - Acceptance: Script correctly identifies affected targets for various file types (C++, headers, QML, CMakeLists.txt); builds targets incrementally; reports clear success/failure; handles edge cases (file not in any target, multiple targets, header-only libraries); performance acceptable (<30s for typical changes).
  - Subtasks:
    - [ ] Implement file-to-CMakeLists.txt path resolution (walk up directory tree)
    - [ ] Implement CMakeLists.txt parsing to find target containing file (regex or CMake --graphviz)
    - [ ] Implement CMake dependency graph query (cmake --graphviz + parse dot file, or use cmake --build --target <target> --verbose)
    - [ ] Implement incremental target build (cmake --build build --target <target>)
    - [ ] Add error handling and clear reporting (colors, progress indicators)
    - [ ] Add dry-run mode (--dry-run) to show what would be built without building
    - [ ] Add verbose mode (--verbose) for debugging
    - [ ] Document script usage in .github/ztodo_verification.md
    - [ ] Add unit tests for script (test with sample CMake projects)
  - Verification Steps:
    1. Functional: Script correctly identifies targets for test files; builds affected targets; detects build failures; handles edge cases (new files, deleted files, CMakeLists.txt changes).
    2. Code Quality: Shell script follows best practices (set -euo pipefail, quotes, error handling); documented functions; clear variable names.
    3. Documentation: Usage documented in ztodo_verification.md; README.md mentions script; script has --help flag.
    4. Integration: Works with z-monitor CMake structure; integrates with verification workflow; CI can use script.
    5. Tests: Test cases cover various scenarios (single file, multiple files, header files, QML files, CMakeLists.txt changes); edge cases tested.
  - Prompt: `project-dashboard/prompt/verify-build-script.md`

- [ ] TASK-TEST-002: Implement Device Simulator
  - What: Create a standalone simulator application or module that generates realistic vital signs (ECG, SpO2, NIBP) and alarm conditions.
  - Why: Essential for testing the MonitoringService and UI without physical hardware. Allows stress testing and edge case simulation.
  - Files:
    - `z-monitor/src/simulation/DeviceSimulator.h/cpp`
    - `z-monitor/src/simulation/WaveformGenerator.h/cpp`
    - `z-monitor/src/simulation/VitalSignGenerator.h/cpp`
  - Acceptance: Simulator generates realistic data streams, configurable vital ranges, triggers alarms.
  - Verification Steps:
    1. Functional: Generates valid waveforms/vitals, configurable parameters work
    2. Code Quality: Doxygen comments, clean code
    3. Documentation: Simulator usage documented
    4. Integration: Feeds data to MonitoringService
    5. Tests: Unit tests for generators
  - Prompt: `project-dashboard/prompt/47-implement-device-simulator.md`

- [ ] TASK-TEST-003: Implement Sensor Simulator
  - What: Create a software simulation of hardware sensors (ECG leads, SpO2 probe, NIBP cuff) to test the hardware abstraction layer.
  - Why: Allows development of hardware-dependent code without physical sensors. Verifies error handling (sensor disconnect, noise).
  - Files:
    - `z-monitor/src/infrastructure/hardware/SensorSimulator.h/cpp`
    - `z-monitor/src/infrastructure/hardware/ISensorDriver.h` (interface)
  - Acceptance: Simulates sensor connection/disconnection, noise injection, raw data output.
  - Verification Steps:
    1. Functional: Simulates connection states, generates raw data
    2. Code Quality: Doxygen comments, clean code
    3. Documentation: Sensor simulation documented
    4. Integration: Works with HardwareManager
    5. Tests: Unit tests for sensor logic
  - Prompt: `project-dashboard/prompt/48-implement-sensor-simulator.md`

- [ ] TASK-TEST-004: Verify real-time vitals update and waveform rendering
  - What: Verify that vitals update in real-time and are visible on screen. Start simulator and z-monitor, observe that heart rate, SpO2, respiratory rate, temperature, and blood pressure values change dynamically every second. Confirm values match simulator output. Verify waveforms (ECG, Pleth, Resp) render smoothly at 60 FPS with no stuttering or frame drops. Confirm waveforms scroll right-to-left as expected in medical monitors.
  - Why: Critical functional verification that the complete data pipeline works end-to-end. This confirms data flows from simulator → shared memory → SharedMemorySensorDataSource → MonitoringService → Caches → Controllers → QML UI. Visual verification ensures the UI is usable for clinical monitoring.
  - Files:
    - Verify: `z-monitor/resources/qml/views/MonitorView.qml` (vitals display)
    - Verify: `z-monitor/resources/qml/components/VitalTile.qml` (value updates)
    - Verify: `z-monitor/resources/qml/components/WaveformPanel.qml` (Canvas rendering)
    - Verify: `z-monitor/resources/qml/components/ConnectionStatus.qml` (connection indicator)
  - Dependencies:
    - Simulator running and generating data (`sensor_simulator`)
    - z-monitor running and connected to shared memory
    - Previous task 44e completed (QML UI wired to controllers)
  - Acceptance:
    - Vitals update visibly every second (heart rate changes from 70→72→75 etc.)
    - All 5 vital types display live data (HR, SpO2, RR, Temp, NIBP)
    - Waveforms render smoothly at 60 FPS (no visible stuttering)
    - ECG waveform shows recognizable PQRST complex
    - Waveforms scroll right-to-left continuously
    - Connection status shows "Connected" with green indicator
    - No QML errors in console output
  - Verification Steps:
    1. Functional: Launch simulator and z-monitor, observe live data for 30 seconds, verify vitals change, verify waveforms render, check connection status **Status:** ⏳ Pending
    2. Visual Inspection: Confirm waveforms smooth (no jitter), vitals readable and updating, UI responsive to user interaction **Status:** ⏳ Pending
    3. Console Check: No QML errors or warnings, no "property binding loop" errors **Status:** ⏳ Pending
  - Prompt: `project-dashboard/prompt/44f-verify-realtime-vitals-waveforms.md`

- [ ] TASK-TEST-005: End-to-end integration test and sign-off
  - What: Perform comprehensive end-to-end test of complete simulator→z-monitor integration. Verify all acceptance criteria from tasks 44b-44e are met. Run extended soak test (30 minutes continuous operation), check for memory leaks, verify error handling (disconnect/reconnect scenarios), and confirm all documentation is complete. Sign off on simulator integration phase.
  - Why: Final validation that the simulator integration is production-ready. Ensures system stability under extended operation, proper error recovery, and complete documentation. This task gates transition to next development phase.
  - Files:
    - Create: `project-dashboard/tests/e2e/test_simulator_integration.md` (test plan and results)
    - Update: `project-dashboard/doc/simulator/DOC-SIM-002_simulator_integration_guide.md` (mark all phases complete, add lessons learned)
    - Create: `project-dashboard/screenshots/z-monitor-soak-test-results.png` (30-min run statistics)
  - Dependencies:
    - All previous tasks 44b-44f-4 completed and verified
    - Simulator and z-monitor both stable
    - All acceptance criteria documented
  - Test Scenarios:
    1. **Happy Path:** Start simulator, start z-monitor, verify data flows, run 30 minutes, check stability
    2. **Disconnect/Reconnect:** Stop simulator, verify UI shows "Disconnected", restart simulator, verify reconnection
    3. **Data Validation:** Compare simulator output to UI display, verify accuracy (±1% tolerance)
    4. **Memory Stability:** Monitor memory usage over 30 minutes, verify no leaks (< 5% growth)
    5. **Error Handling:** Corrupt shared memory, verify error recovery
    6. **Performance:** Verify latency < 50ms, FPS = 60 sustained
  - Acceptance:
    - All test scenarios pass
    - 30-minute soak test completes without crashes or errors
    - Memory usage stable (< 5% growth over 30 minutes)
    - Latency meets target (< 50ms average)
    - FPS sustained at 60 throughout test
    - All documentation complete and reviewed
    - Screenshots captured for all scenarios
    - Lessons learned documented
  - Verification Steps:
    1. Execute: Run all 6 test scenarios, document results **Status:** ⏳ Pending
    2. Soak Test: 30-minute continuous run, monitor metrics **Status:** ⏳ Pending
    3. Review: Check all acceptance criteria, verify documentation complete **Status:** ⏳ Pending
    4. Sign-off: Mark tasks 44b-44f complete, update ZTODO **Status:** ⏳ Pending
  - Prompt: `project-dashboard/prompt/44f-end-to-end-integration-test.md`

- [ ] TASK-TEST-006: Implement unified testing workflow
  - What: Create GoogleTest + Qt Test scaffolding under `z-monitor/tests/unit/`, integration suites under `z-monitor/tests/integration/`, E2E suites under `z-monitor/tests/e2e/`, and benchmark suites under `z-monitor/tests/benchmarks/`. Organize tests by DDD layer: `tests/unit/domain/`, `tests/unit/application/`, `tests/unit/infrastructure/`, `tests/unit/interface/`. Apply `ctest` labels (`unit`, `integration`, `benchmark`) and follow the process documented in `project-dashboard/doc/legacy/architecture_and_design/18_TESTING_WORKFLOW.md`.
  - Why: Testing groundwork must be established early to support iterative development. DDD-aligned test structure matches code organization.
  - Acceptance: Test framework integrated, test structure matches DDD layers, ctest labels work, tests can run independently, CI integration works.
  - Verification Steps:
    1. Functional: Tests compile and run, test structure matches code structure, ctest labels work
    2. Code Quality: Test code follows guidelines, test organization clear, no test framework warnings
    3. Documentation: Testing workflow documented, test structure documented
    4. Integration: CI runs tests, test reports generated, coverage integration works
    5. Tests: Test framework tests, test organization verified

- [ ] TASK-TEST-007: Add coverage pipeline
  - What: Enable coverage builds with `-DENABLE_COVERAGE=ON`, integrate `lcov`/`genhtml`, and enforce minimum 80% line coverage on critical components (`src/domain/`, `src/application/`, `src/infrastructure/persistence/`, `src/infrastructure/network/`). Publish reports from `build_coverage/coverage/index.html`.
  - Why: Maintains confidence in critical code paths. Per REQ-NFR-MAIN-002, critical components require 90%+ coverage, normal components 80%+.
  - Acceptance: Coverage builds work, reports generated, CI enforces coverage thresholds, coverage tracked per component.
  - Verification Steps:
    1. Functional: Coverage builds succeed, reports generated, thresholds enforced
    2. Code Quality: Coverage targets met, no regressions
    3. Documentation: Coverage workflow documented, thresholds documented
    4. Integration: CI runs coverage builds, reports published, thresholds enforced
    5. Tests: Coverage measurement verified, threshold enforcement tested

- [ ] TASK-TEST-008: Automate lint/static analysis
  - What: Extend `scripts/run_tests.sh lint` to invoke clang-format, clang-tidy, and cppcheck; gate CI on lint success.
  - Why: Keeps codebase consistent and surfaces issues before compilation.

- [ ] TASK-TEST-009: Enforce testing workflow in CI
  - What: Update CI workflows to call `./scripts/run_tests.sh all`, publish coverage + benchmark artifacts, and fail builds when thresholds/regressions occur.
  - Why: Ensures automation matches the documented workflow.

- [ ] TASK-TEST-010: Add E2E containerized test harness
  - What: Compose the Z Monitor (headless) and the server simulator in docker-compose test environment and run basic E2E scenarios.
  - Why: Validates connectivity, DB writes, and archival behavior in a reproducible environment.
  - Prompt: `project-dashboard/prompt/21-e2e-containerized-harness.md`

- [ ] TASK-TEST-011: Implement Central Server Simulator
  - What: Create a mock central server that accepts telemetry connections, receives batches, and sends configuration updates.
  - Why: Required to test network transmission, buffering, and retry logic in TelemetryService.
  - Files:
    - `tools/central-server-sim/main.cpp`
    - `tools/central-server-sim/Server.h/cpp`
  - Acceptance: Accepts TCP/TLS connections, parses telemetry batches, logs received data, sends ACKs.
  - Verification Steps:
    1. Functional: Receives data, sends ACKs, handles multiple connections
    2. Code Quality: Clean code
    3. Documentation: Server usage documented
    4. Integration: Works with TelemetryService
    5. Tests: Integration tests with client
  - Prompt: `project-dashboard/prompt/49-implement-central-server-simulator.md`
