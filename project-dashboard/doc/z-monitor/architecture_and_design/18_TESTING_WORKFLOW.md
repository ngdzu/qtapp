# Testing Workflow and Strategy

This document defines the testing strategy, tooling, and workflow for the Z Monitor project. It covers unit, integration, end-to-end, benchmark, and coverage testing, and explains how testing integrates with day-to-day development.

## 1. Goals

1. Catch regressions early with fast, deterministic unit tests.
2. Validate component interactions with integration tests.
3. Exercise real-world workflows through end-to-end (E2E) tests.
4. Measure performance characteristics with repeatable benchmarks.
5. Maintain high confidence through automated coverage reporting.
6. Provide a single developer-friendly entry point for running the full test suite.

## 2. Test Pyramid and Coverage

| Level         | Scope                                        | Frequency           | Tools                                 |
|---------------|----------------------------------------------|---------------------|---------------------------------------|
| Unit          | Single class/function                        | Pre-commit, CI      | GoogleTest + Qt Test                  |
| Integration   | Multiple components/services                 | On-demand, CI       | GoogleTest, custom fixtures/mocks     |
| End-to-End    | Full device → UI → server workflow           | Nightly / release   | Squish (UI), QML Test Runner, pytest  |
| Benchmark     | Performance-critical code paths              | On demand, CI gate  | Google Benchmark                      |
| Static/Format | Lint, formatting, security scans             | Pre-commit, CI      | clang-format, clang-tidy, cppcheck    |

## 3. Tooling Choices and Rationale

### 3.1. Unit Tests
- **GoogleTest**: Rich assertions, death tests, parameterized tests, wide ecosystem. Works well with CMake and Qt.
- **Qt Test (QTestLib)**: Ideal for QObject/QML-centric tests (signals/slots, QML components).

**Why both?** Core business logic uses GoogleTest for familiarity and speed; QML or QObject-heavy code uses Qt Test for native signal/slot helpers.

### 3.2. Integration Tests
- **GoogleTest + custom fixtures**: Allows spinning up multiple services (e.g., NetworkManager + MockTelemetryServer) inside the same test process.
- **QML Test Runner** (`qmltestrunner`): Validates QML components integrated with controllers.

### 3.3. End-to-End Tests
- **Squish for Qt/QML** (or alternative open-source tools like `maestro` if Squish license unavailable). Automates full UI flows.
- **Python + pytest** driving the simulated central server and device CLI for black-box validation.

### 3.4. Benchmarks
- **Google Benchmark**: Industry-standard for micro/meso benchmarks with statistical reporting and CPU throttling controls.

### 3.5. Coverage
- **gcov/lcov/genhtml** on Linux, **llvm-cov** on macOS. The build enables coverage via `-DENABLE_COVERAGE=ON` (CMake option).

### 3.6. Static Analysis (supporting)
- **clang-tidy**, **cppcheck**, **include-what-you-use** integrated into CMake presets for optional static analysis runs.

## 4. Directory Layout

```
tests/
  unit/
    core/            # GoogleTest suites for core services
    qml/             # Qt Test suites for QML components
  integration/
    network/         # NetworkManager + mock server tests
    security/        # Certificate workflow integration tests
  e2e/
    scenarios/       # Squish/pytest scenarios
  benchmarks/
    core/            # Google Benchmark suites
scripts/
  run_tests.sh       # Unified test runner (see Section 6)
```

## 5. Workflow

### 5.1. Developer Workflow
1. **Before coding:** Run `./scripts/run_tests.sh unit` to ensure a clean baseline.
2. **During development:** Write tests alongside features. Use TDD for controllers/services.
3. **Pre-commit:** Run `./scripts/run_tests.sh unit integration lint`.
4. **Pre-push:** Run `./scripts/run_tests.sh all` (unit + integration + coverage smoke).

### 5.2. CI Workflow
| Pipeline Stage   | Jobs                                                                                   |
|------------------|----------------------------------------------------------------------------------------|
| `lint`           | clang-format, clang-tidy, cppcheck                                                     |
| `unit`           | GoogleTest + Qt Test suites                                                            |
| `integration`    | Full integration suites with DatabaseManager, NetworkManager, mock servers             |
| `e2e`            | Squish/QML UI tests using headless Xvfb or VNC                                         |
| `bench`          | Benchmarks (fail build if regression > defined % threshold)                            |
| `coverage`       | Build with coverage flags, run unit+integration, publish lcov/llvm-cov report          |
| `artifacts`      | Upload coverage HTML, benchmark CSV, Squish logs                                       |

## 6. Unified Test Runner (`scripts/run_tests.sh`)

The script orchestrates test execution:

```bash
./scripts/run_tests.sh unit           # Run GoogleTest + Qt Test suites
./scripts/run_tests.sh integration    # Integration suites only
./scripts/run_tests.sh e2e            # Squish/pytest scenarios (requires simulator)
./scripts/run_tests.sh bench          # Benchmarks
./scripts/run_tests.sh coverage       # Enable coverage, run unit+integration, generate report
./scripts/run_tests.sh all            # Runs lint + unit + integration + coverage smoke
```

Coverage artifacts land in `build/coverage/index.html`.

## 7. Coverage Configuration

- Enable with `cmake -DENABLE_COVERAGE=ON -S . -B build/coverage`.
- Script uses `lcov` to capture `build/coverage/coverage.info` and `genhtml` to render HTML.
- Thresholds enforced in CI (e.g., minimum 80% line coverage on `src/core/`).

## 8. Benchmark Policy

- Benchmarks run on dedicated CI runner to avoid noisy neighbors.
- Results stored as CSV/JSON for trend analysis.
- Regression threshold: failure if median runtime increases by >10% vs baseline.

## 9. Thinking Process / Selection Rationale

1. **GoogleTest vs Catch2**: Selected GoogleTest due to native support in Qt ecosystem, existing developer familiarity, and richer tooling integration (e.g., gMock). Catch2 is lighter but would require more onboarding.
2. **Qt Test**: Essential for QML and QObject testing; saves time compared with re-implementing event loop helpers.
3. **Squish**: Proven tool for Qt/QML E2E automation with object names, less flaky than image-based solutions. Alternative (maestro, Playwright) lack deep Qt accessibility.
4. **Google Benchmark**: Mature library with statistical analysis, more reliable than ad-hoc QElapsedTimer measurements.
5. **lcov/llvm-cov**: Standard coverage pipeline compatible with both GCC and Clang, widely supported by CI tools.
6. **pytest for server-side integration**: Python’s rapid prototyping plus rich HTTP tooling makes it ideal for driving the simulated central server.

## 10. Next Steps

1. Implement GoogleTest + Qt Test scaffolding under `tests/`.
2. Add Squish/pytest fixtures in `tests/e2e/`.
3. Integrate `scripts/run_tests.sh` into CI pipelines.
4. Configure coverage thresholds and publish reports.
5. Document per-component test cases in respective `.md` files.

Refer to `ZTODO.md` for implementation tasks tied to this testing workflow.

