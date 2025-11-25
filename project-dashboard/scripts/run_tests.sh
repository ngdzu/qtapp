#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
COVERAGE_BUILD_DIR="$ROOT_DIR/build_coverage"

usage() {
  cat <<EOF
Usage: $0 [unit|integration|e2e|bench|coverage|lint|all]

Targets:
  unit        Run GoogleTest + Qt Test suites (ctest label=unit)
  integration Run integration suites (ctest label=integration)
  e2e         Run end-to-end scenarios (requires simulator + Squish/pytest)
  bench       Run benchmarks (ctest label=benchmark)
  coverage    Build with coverage flags and generate lcov report
  lint        Run clang-format/clang-tidy/cppcheck presets (optional)
  all         Run lint + unit + integration + coverage smoke
EOF
}

cmake_configure() {
  local dir="$1"
  local extra="$2"
  cmake -S "$ROOT_DIR" -B "$dir" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" $extra
}

cmake_build() {
  local dir="$1"
  cmake --build "$dir" --parallel
}

run_ctest() {
  local dir="$1"
  local label="$2"
  ctest --test-dir "$dir" --output-on-failure -L "$label"
}

run_unit() {
  cmake_configure "$BUILD_DIR" ""
  cmake_build "$BUILD_DIR"
  run_ctest "$BUILD_DIR" unit
}

run_integration() {
  cmake_configure "$BUILD_DIR" ""
  cmake_build "$BUILD_DIR"
  run_ctest "$BUILD_DIR" integration
}

run_e2e() {
  pushd "$ROOT_DIR/tests/e2e" >/dev/null
  if command -v squishrunner >/dev/null 2>&1; then
    squishrunner --config suite.conf
  else
    echo "squishrunner not found; skipping Squish tests" >&2
  fi
  if command -v pytest >/dev/null 2>&1; then
    pytest .
  else
    echo "pytest not found; skipping pytest e2e tests" >&2
  fi
  popd >/dev/null
}

run_bench() {
  cmake_configure "$BUILD_DIR" "-DENABLE_BENCHMARKS=ON"
  cmake_build "$BUILD_DIR"
  run_ctest "$BUILD_DIR" benchmark
}

run_lint() {
  cmake_configure "$BUILD_DIR" "-DENABLE_LINT=ON"
  cmake --build "$BUILD_DIR" --target clang-format clang-tidy cppcheck || true
}

run_coverage() {
  cmake_configure "$COVERAGE_BUILD_DIR" "-DENABLE_COVERAGE=ON"
  cmake_build "$COVERAGE_BUILD_DIR"
  ctest --test-dir "$COVERAGE_BUILD_DIR" --output-on-failure -L "unit|integration"
  lcov --capture --directory "$COVERAGE_BUILD_DIR" --output-file "$COVERAGE_BUILD_DIR/coverage.info"
  lcov --remove "$COVERAGE_BUILD_DIR/coverage.info" "/usr/*" --output-file "$COVERAGE_BUILD_DIR/coverage.info"
  genhtml "$COVERAGE_BUILD_DIR/coverage.info" --output-directory "$COVERAGE_BUILD_DIR/coverage"
  echo "Coverage report: $COVERAGE_BUILD_DIR/coverage/index.html"
}

run_all() {
  run_lint
  run_unit
  run_integration
  run_coverage
}

target="${1:-}"
case "$target" in
  unit) run_unit ;;
  integration) run_integration ;;
  e2e) run_e2e ;;
  bench) run_bench ;;
  coverage) run_coverage ;;
  lint) run_lint ;;
  all) run_all ;;
  ""|-h|--help) usage ;;
  *) echo "Unknown target: $target" >&2; usage; exit 1 ;;
esac

