# Performance Benchmarks using Google Benchmark

# Fetch Google Benchmark
include(FetchContent)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark tests")
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Disable benchmark install")
FetchContent_MakeAvailable(benchmark)

# Common libraries for all benchmarks
set(BENCHMARK_LIBS
    benchmark::benchmark
    z_monitor_domain
    Qt6::Core
)

# Alarm Detection Benchmark (already exists as MonitoringServiceBenchmark.cpp)
add_executable(AlarmDetectionBenchmark MonitoringServiceBenchmark.cpp)
target_link_libraries(AlarmDetectionBenchmark PRIVATE ${BENCHMARK_LIBS} gmock)
target_include_directories(AlarmDetectionBenchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Waveform Rendering Benchmark
add_executable(WaveformRenderingBenchmark WaveformRenderingBenchmark.cpp)
target_link_libraries(WaveformRenderingBenchmark PRIVATE ${BENCHMARK_LIBS})
target_include_directories(WaveformRenderingBenchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Database Query Benchmark
add_executable(DatabaseQueryBenchmark DatabaseQueryBenchmark.cpp)
target_link_libraries(DatabaseQueryBenchmark PRIVATE ${BENCHMARK_LIBS})
target_include_directories(DatabaseQueryBenchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Telemetry Batch Benchmark
add_executable(TelemetryBatchBenchmark TelemetryBatchBenchmark.cpp)
target_link_libraries(TelemetryBatchBenchmark PRIVATE ${BENCHMARK_LIBS})
target_include_directories(TelemetryBatchBenchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Add all benchmarks to a custom target
add_custom_target(benchmarks)
add_dependencies(benchmarks
    AlarmDetectionBenchmark
    WaveformRenderingBenchmark
    DatabaseQueryBenchmark
    TelemetryBatchBenchmark
)

# Install benchmark targets (optional - for CI)
install(TARGETS
    AlarmDetectionBenchmark
    WaveformRenderingBenchmark
    DatabaseQueryBenchmark
    TelemetryBatchBenchmark
    RUNTIME DESTINATION benchmarks
)
