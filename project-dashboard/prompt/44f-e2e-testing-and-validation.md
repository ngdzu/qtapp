# Task: End-to-End Testing and Validation

## Context

**Documentation:**
- Architecture: [37_SENSOR_INTEGRATION.md](../doc/z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md) - Troubleshooting section
- Implementation Guide: [44_SIMULATOR_INTEGRATION_GUIDE.md](../doc/z-monitor/architecture_and_design/44_SIMULATOR_INTEGRATION_GUIDE.md) - Phase 6
- Testing Workflow: [18_TESTING_WORKFLOW.md](../doc/z-monitor/architecture_and_design/18_TESTING_WORKFLOW.md)
- E2E Tests: [sensor-simulator/tests/e2e_test_instructions.md](../sensor-simulator/tests/e2e_test_instructions.md)

**Previous Work:**
- ✅ Simulator built and running
- ✅ SharedMemorySensorDataSource implemented
- ✅ MonitoringService wired to controllers
- ✅ QML UI displaying live data
- ⏳ Need comprehensive testing and validation

**Dependencies:**
- Complete integration stack (simulator → z-monitor → UI)
- GoogleTest framework (for unit tests)
- Qt Test framework (for integration tests)
- Profiling tools (Qt QML Profiler, Instruments/perf)

---

## Objective

Perform comprehensive end-to-end testing and validation of the complete simulator/z-monitor integration. Execute unit tests (SharedMemoryControlChannel, SharedMemoryRingBuffer, SharedMemorySensorDataSource), integration tests (E2E with simulator), latency measurement (verify < 16ms simulator→signal, < 50ms total), stall detection test (kill simulator → detected within 300ms), and performance profiling. Document test results, capture diagnostic data, and create final validation report.

**Success Criteria:** All tests pass, latency targets met, no regressions introduced.

---

## Testing Strategy

### Test Categories

1. **Unit Tests** - Test individual components in isolation
2. **Integration Tests** - Test component interactions with real simulator
3. **Performance Tests** - Measure latency, throughput, resource usage
4. **Reliability Tests** - Test error handling, recovery, edge cases
5. **Visual Regression Tests** - Compare UI screenshots with baselines

---

## Test Suite Breakdown

### Unit Tests

#### Test 1: SharedMemoryControlChannel

**File:** `z-monitor/tests/unit/infrastructure/sensors/shared_memory_control_channel_test.cpp`

```cpp
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "infrastructure/sensors/SharedMemoryControlChannel.h"

class SharedMemoryControlChannelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure simulator is running
        ASSERT_TRUE(isSimulatorRunning()) << "Simulator must be running for tests";
    }
    
    bool isSimulatorRunning() {
        QLocalSocket socket;
        socket.connectToServer("/tmp/z-monitor-sensor.sock");
        return socket.waitForConnected(1000);
    }
};

TEST_F(SharedMemoryControlChannelTest, ConnectsToSimulator) {
    SharedMemoryControlChannel channel;
    QSignalSpy completedSpy(&channel, &SharedMemoryControlChannel::handshakeCompleted);
    QSignalSpy failedSpy(&channel, &SharedMemoryControlChannel::handshakeFailed);
    
    ASSERT_TRUE(channel.connect("/tmp/z-monitor-sensor.sock"));
    
    // Wait for handshake (should complete within 1 second)
    ASSERT_TRUE(completedSpy.wait(1000)) << "Handshake timeout";
    EXPECT_EQ(failedSpy.count(), 0) << "Handshake should not fail";
    
    // Verify file descriptor received
    QList<QVariant> args = completedSpy.takeFirst();
    int memfdFd = args.at(0).toInt();
    size_t ringBufferSize = args.at(1).toULongLong();
    
    EXPECT_GT(memfdFd, 0) << "Invalid file descriptor";
    EXPECT_EQ(ringBufferSize, 8392704) << "Unexpected ring buffer size";  // 2048 * 4096
}

TEST_F(SharedMemoryControlChannelTest, HandlesConnectionFailure) {
    SharedMemoryControlChannel channel;
    QSignalSpy failedSpy(&channel, &SharedMemoryControlChannel::handshakeFailed);
    
    // Connect to non-existent socket
    ASSERT_TRUE(channel.connect("/tmp/non-existent-socket.sock"));
    
    // Should fail within 1 second
    ASSERT_TRUE(failedSpy.wait(1000));
    
    QList<QVariant> args = failedSpy.takeFirst();
    QString error = args.at(0).toString();
    EXPECT_FALSE(error.isEmpty()) << "Error message should be provided";
}
```

#### Test 2: SharedMemoryRingBuffer

**File:** `z-monitor/tests/unit/infrastructure/sensors/shared_memory_ring_buffer_test.cpp`

```cpp
TEST(SharedMemoryRingBufferTest, ValidatesHeader) {
    // Create test shared memory with valid header
    size_t size = sizeof(RingBufferHeader) + (2048 * 4096);
    void* memory = malloc(size);
    
    RingBufferHeader* header = reinterpret_cast<RingBufferHeader*>(memory);
    header->magic = 0x534D5242;  // 'SMRB'
    header->version = 1;
    header->frameSizeBytes = 4096;
    header->frameCount = 2048;
    
    SharedMemoryRingBuffer ringBuffer(memory, size);
    EXPECT_TRUE(ringBuffer.validateHeader());
    
    free(memory);
}

TEST(SharedMemoryRingBufferTest, RejectsInvalidMagic) {
    size_t size = sizeof(RingBufferHeader);
    void* memory = malloc(size);
    
    RingBufferHeader* header = reinterpret_cast<RingBufferHeader*>(memory);
    header->magic = 0x12345678;  // Invalid magic
    header->version = 1;
    
    SharedMemoryRingBuffer ringBuffer(memory, size);
    EXPECT_FALSE(ringBuffer.validateHeader());
    
    free(memory);
}

TEST(SharedMemoryRingBufferTest, ReadFrameValidatesCRC) {
    // Create test ring buffer
    size_t size = sizeof(RingBufferHeader) + (10 * 4096);
    void* memory = calloc(1, size);
    
    RingBufferHeader* header = reinterpret_cast<RingBufferHeader*>(memory);
    header->magic = 0x534D5242;
    header->version = 1;
    header->frameSizeBytes = 4096;
    header->frameCount = 10;
    
    // Write test frame with valid CRC
    uint8_t* slots = reinterpret_cast<uint8_t*>(memory) + sizeof(RingBufferHeader);
    SensorFrame* frame = reinterpret_cast<SensorFrame*>(slots);
    frame->type = 0x01;
    frame->timestampNs = 123456789;
    frame->dataSize = 32;
    strcpy((char*)frame->data, "{\"hr\":72}");
    frame->crc32 = calculateCRC32(frame, sizeof(SensorFrame) - 4);
    
    // Read and validate
    SharedMemoryRingBuffer ringBuffer(memory, size);
    SensorFrame readFrame;
    EXPECT_TRUE(ringBuffer.readFrame(0, readFrame));
    EXPECT_EQ(readFrame.type, 0x01);
    EXPECT_EQ(readFrame.timestampNs, 123456789);
    
    free(memory);
}
```

#### Test 3: SharedMemorySensorDataSource

**File:** `z-monitor/tests/unit/infrastructure/sensors/shared_memory_sensor_test.cpp`

```cpp
TEST(SharedMemorySensorDataSourceTest, ParsesVitalsFrame) {
    // Create test frame
    SensorFrame frame;
    frame.type = 0x01;  // Vitals
    frame.timestampNs = QDateTime::currentMSecsSinceEpoch() * 1000000;
    frame.dataSize = 32;
    strcpy((char*)frame.data, "{\"hr\":72,\"spo2\":98,\"rr\":16}");
    frame.crc32 = calculateCRC32(&frame, sizeof(SensorFrame) - 4);
    
    // Parse
    SharedMemorySensorDataSource dataSource("/tmp/test.sock", "test-ring");
    QSignalSpy vitalsSpy(&dataSource, &ISensorDataSource::vitalsReceived);
    
    dataSource.parseFrame(frame);
    
    ASSERT_EQ(vitalsSpy.count(), 1);
    QList<QVariant> args = vitalsSpy.takeFirst();
    VitalRecord vital = args.at(0).value<VitalRecord>();
    
    EXPECT_EQ(vital.heartRate, 72);
    EXPECT_EQ(vital.spo2, 98);
    EXPECT_EQ(vital.respirationRate, 16);
}
```

---

### Integration Tests

#### Test 1: End-to-End Data Flow

**File:** `z-monitor/tests/integration/sensor_simulator_integration_test.cpp`

```cpp
class SensorSimulatorIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure simulator is running
        ASSERT_TRUE(startSimulator());
        QTest::qWait(2000);  // Wait for simulator to initialize
    }
    
    void TearDown() override {
        stopSimulator();
    }
    
    bool startSimulator() {
        m_simulatorProcess = new QProcess();
        m_simulatorProcess->start("./sensor_simulator");
        return m_simulatorProcess->waitForStarted(5000);
    }
    
    void stopSimulator() {
        if (m_simulatorProcess) {
            m_simulatorProcess->terminate();
            m_simulatorProcess->waitForFinished(5000);
            delete m_simulatorProcess;
        }
    }
    
    QProcess* m_simulatorProcess = nullptr;
};

TEST_F(SensorSimulatorIntegrationTest, ReceivesVitalsFromSimulator) {
    // Setup
    SharedMemorySensorDataSource dataSource("/tmp/z-monitor-sensor.sock", "zmonitor-sim-ring");
    QSignalSpy vitalsSpy(&dataSource, &ISensorDataSource::vitalsReceived);
    QSignalSpy connectedSpy(&dataSource, &ISensorDataSource::connectionStatusChanged);
    
    // Start
    ASSERT_TRUE(dataSource.start());
    
    // Wait for connection
    ASSERT_TRUE(connectedSpy.wait(2000));
    
    // Wait for vitals (at 60 Hz, should receive ~120 vitals in 2 seconds)
    QTest::qWait(2000);
    
    // Assert
    EXPECT_GE(vitalsSpy.count(), 100) << "Should receive ~120 vitals in 2 seconds";
    
    // Verify data
    if (vitalsSpy.count() > 0) {
        QList<QVariant> args = vitalsSpy.first();
        VitalRecord vital = args.at(0).value<VitalRecord>();
        EXPECT_GT(vital.heartRate, 0);
        EXPECT_GT(vital.spo2, 0);
        EXPECT_GT(vital.respirationRate, 0);
    }
}

TEST_F(SensorSimulatorIntegrationTest, ReceivesWaveformsFromSimulator) {
    SharedMemorySensorDataSource dataSource("/tmp/z-monitor-sensor.sock", "zmonitor-sim-ring");
    QSignalSpy waveformSpy(&dataSource, &ISensorDataSource::waveformSampleReady);
    
    ASSERT_TRUE(dataSource.start());
    QTest::qWait(2000);
    
    // At 250 Hz, should receive ~500 waveform samples in 2 seconds
    EXPECT_GE(waveformSpy.count(), 400);
}
```

#### Test 2: Stall Detection

```cpp
TEST_F(SensorSimulatorIntegrationTest, DetectsStalledWriter) {
    SharedMemorySensorDataSource dataSource("/tmp/z-monitor-sensor.sock", "zmonitor-sim-ring");
    QSignalSpy errorSpy(&dataSource, &ISensorDataSource::sensorError);
    QSignalSpy statusSpy(&dataSource, &ISensorDataSource::connectionStatusChanged);
    
    // Start and connect
    ASSERT_TRUE(dataSource.start());
    ASSERT_TRUE(statusSpy.wait(2000));
    
    // Kill simulator (simulate stall)
    stopSimulator();
    
    // Should detect stall within 300ms
    ASSERT_TRUE(errorSpy.wait(500));
    
    // Verify error message
    QList<QVariant> args = errorSpy.first();
    SensorError error = args.at(0).value<SensorError>();
    EXPECT_TRUE(error.message.contains("stall") || error.message.contains("timeout"));
}
```

#### Test 3: MonitoringService Integration

```cpp
TEST(MonitoringServiceIntegrationTest, IntegratesWithSensorDataSource) {
    // Setup full stack
    SharedMemorySensorDataSource dataSource("/tmp/z-monitor-sensor.sock", "zmonitor-sim-ring");
    VitalsCache vitalsCache;
    WaveformCache waveformCache;
    AlarmManager alarmManager;
    MonitoringService service(&dataSource, &vitalsCache, &waveformCache, &alarmManager);
    
    QSignalSpy vitalsSpy(&service, &MonitoringService::vitalsUpdated);
    
    // Start
    service.start();
    
    // Wait for data flow
    QTest::qWait(2000);
    
    // Assert
    EXPECT_GT(vitalsSpy.count(), 100);
    EXPECT_GT(vitalsCache.size(), 100);
}
```

---

### Performance Tests

#### Test 1: Latency Measurement

**File:** `z-monitor/tests/performance/latency_test.cpp`

```cpp
TEST(PerformanceTest, MeasuresEndToEndLatency) {
    // Requires instrumented simulator that writes timestamp to frames
    SharedMemorySensorDataSource dataSource("/tmp/z-monitor-sensor.sock", "zmonitor-sim-ring");
    
    std::vector<double> latencies;
    
    connect(&dataSource, &ISensorDataSource::vitalsReceived, [&](const VitalRecord& vital) {
        auto nowNs = QDateTime::currentMSecsSinceEpoch() * 1000000;
        auto latencyMs = (nowNs - vital.timestampNs) / 1000000.0;
        latencies.push_back(latencyMs);
    });
    
    dataSource.start();
    QTest::qWait(10000);  // Collect 10 seconds of data
    
    // Calculate statistics
    double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    double maxLatency = *std::max_element(latencies.begin(), latencies.end());
    
    std::cout << "Average Latency: " << avgLatency << " ms" << std::endl;
    std::cout << "Max Latency: " << maxLatency << " ms" << std::endl;
    std::cout << "Samples: " << latencies.size() << std::endl;
    
    // Assert targets
    EXPECT_LT(avgLatency, 16.0) << "Average latency exceeds 16ms target";
    EXPECT_LT(maxLatency, 50.0) << "Max latency exceeds 50ms";
}
```

#### Test 2: Throughput Test

```cpp
TEST(PerformanceTest, MeasuresThroughput) {
    SharedMemorySensorDataSource dataSource("/tmp/z-monitor-sensor.sock", "zmonitor-sim-ring");
    
    int vitalsCount = 0;
    int waveformCount = 0;
    
    connect(&dataSource, &ISensorDataSource::vitalsReceived, [&]() { vitalsCount++; });
    connect(&dataSource, &ISensorDataSource::waveformSampleReady, [&]() { waveformCount++; });
    
    dataSource.start();
    QTest::qWait(10000);
    
    std::cout << "Vitals Rate: " << vitalsCount / 10.0 << " Hz" << std::endl;
    std::cout << "Waveform Rate: " << waveformCount / 10.0 << " Hz" << std::endl;
    
    EXPECT_NEAR(vitalsCount / 10.0, 60.0, 5.0);  // 60 Hz ± 5 Hz
    EXPECT_NEAR(waveformCount / 10.0, 250.0, 20.0);  // 250 Hz ± 20 Hz
}
```

---

### Visual Regression Tests

#### Test 1: Screenshot Comparison

**File:** `z-monitor/tests/visual/ui_regression_test.cpp`

```cpp
TEST(VisualRegressionTest, UIMatchesBaseline) {
    // Start simulator and z-monitor
    startSimulator();
    QApplication app;
    QQmlApplicationEngine engine;
    engine.load("qrc:/qml/Main.qml");
    
    // Wait for data flow
    QTest::qWait(5000);
    
    // Capture screenshot
    QPixmap screenshot = QPixmap::grabWindow(engine.rootObjects()[0]->winId());
    screenshot.save("test_output/current_ui.png");
    
    // Compare with baseline (using image comparison library)
    double similarity = compareImages("test_output/current_ui.png", 
                                     "baseline/ui_with_live_data.png");
    
    EXPECT_GT(similarity, 0.95) << "UI differs significantly from baseline";
}
```

---

## Integration Test Script

**File:** `z-monitor/tests/integration/integration_test.sh`

```bash
#!/bin/bash
# integration_test.sh - E2E integration test script

set -e

echo "=== Simulator/Z-Monitor Integration Test ==="

# Start simulator
echo "Starting simulator..."
cd project-dashboard/sensor-simulator/build
./sensor_simulator &
SIM_PID=$!
sleep 2

# Verify simulator is running
if ! pgrep -x "sensor_simulator" > /dev/null; then
    echo "ERROR: Simulator failed to start"
    exit 1
fi
echo "✅ Simulator started (PID: $SIM_PID)"

# Verify shared memory created
if [ -e /dev/shm/zmonitor-sim-ring ]; then
    echo "✅ Shared memory created"
else
    echo "ERROR: Shared memory not found"
    kill $SIM_PID
    exit 1
fi

# Verify socket exists
if [ -e /tmp/z-monitor-sensor.sock ]; then
    echo "✅ Control socket created"
else
    echo "ERROR: Control socket not found"
    kill $SIM_PID
    exit 1
fi

# Start z-monitor
echo "Starting z-monitor..."
cd ../../z-monitor/build
./z-monitor &
ZMON_PID=$!
sleep 5

# Check z-monitor logs for connection
if grep -q "SharedMemorySensorDataSource: Started successfully" z-monitor.log; then
    echo "✅ Z-Monitor connected to simulator"
else
    echo "ERROR: Z-Monitor failed to connect"
    kill $SIM_PID $ZMON_PID
    exit 1
fi

# Run for 30 seconds, collect data
echo "Running integration test (30 seconds)..."
sleep 30

# Check vitals were received
VITALS_COUNT=$(grep -c "vitalsReceived" z-monitor.log || true)
if [ "$VITALS_COUNT" -gt 1500 ]; then  # Should be ~1800 in 30 seconds at 60 Hz
    echo "✅ Vitals received: $VITALS_COUNT"
else
    echo "WARNING: Low vitals count: $VITALS_COUNT (expected ~1800)"
fi

# Check waveforms were received
WAVEFORM_COUNT=$(grep -c "waveformSampleReady" z-monitor.log || true)
if [ "$WAVEFORM_COUNT" -gt 6000 ]; then  # Should be ~7500 in 30 seconds at 250 Hz
    echo "✅ Waveforms received: $WAVEFORM_COUNT"
else
    echo "WARNING: Low waveform count: $WAVEFORM_COUNT (expected ~7500)"
fi

# Test stall detection (kill simulator)
echo "Testing stall detection..."
kill $SIM_PID
sleep 1

# Check if stall was detected
if grep -q "Writer stalled" z-monitor.log; then
    echo "✅ Stall detection works"
else
    echo "WARNING: Stall not detected"
fi

# Cleanup
kill $ZMON_PID 2>/dev/null || true
rm -f /dev/shm/zmonitor-sim-ring
rm -f /tmp/z-monitor-sensor.sock

echo "=== Integration Test Complete ==="
```

---

## Performance Profiling

### Qt QML Profiler

**Commands:**
```bash
# Profile QML performance
QSG_RENDER_TIMING=1 ./z-monitor

# Use Qt Creator profiler
# Tools → Profiler → QML Profiler → Start
```

**What to Look For:**
- Frame rendering time (should be < 16ms for 60 FPS)
- Canvas paint time (should be < 10ms)
- JavaScript execution time (minimize)
- Signal/slot overhead

---

### Instruments (macOS)

**Commands:**
```bash
# Profile with Time Profiler
instruments -t "Time Profiler" ./z-monitor

# Profile with Allocations
instruments -t "Allocations" ./z-monitor
```

**What to Look For:**
- Hot functions (optimize if consuming > 10% CPU)
- Memory allocations in hot path (should be zero)
- Lock contention (minimize lock hold time)

---

### perf (Linux)

**Commands:**
```bash
# Record performance data
perf record -g ./z-monitor

# View report
perf report

# Flame graph
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
```

---

## Validation Report Template

**File:** `project-dashboard/doc/z-monitor/validation/44_SIMULATOR_INTEGRATION_VALIDATION.md`

```markdown
# Simulator Integration Validation Report

**Date:** 2025-11-29  
**Version:** 1.0  
**Tester:** [Your Name]

## Executive Summary

- ✅ All unit tests passed (30/30)
- ✅ All integration tests passed (5/5)
- ✅ Latency targets met (avg: 12.3ms, max: 48.2ms)
- ✅ Throughput targets met (vitals: 60 Hz, waveforms: 250 Hz)
- ✅ Stall detection functional
- ✅ Visual regression passed

## Test Results

### Unit Tests

| Component                    | Tests  | Passed | Failed |
| ---------------------------- | ------ | ------ | ------ |
| SharedMemoryControlChannel   | 10     | 10     | 0      |
| SharedMemoryRingBuffer       | 12     | 12     | 0      |
| SharedMemorySensorDataSource | 8      | 8      | 0      |
| **Total**                    | **30** | **30** | **0**  |

### Integration Tests

| Test                          | Status | Notes                                       |
| ----------------------------- | ------ | ------------------------------------------- |
| E2E Data Flow                 | ✅ Pass | Received 1823 vitals, 7512 waveforms in 30s |
| Stall Detection               | ✅ Pass | Detected within 287ms                       |
| MonitoringService Integration | ✅ Pass | Cache populated correctly                   |
| UI Live Data Display          | ✅ Pass | All vitals display correctly                |
| Visual Regression             | ✅ Pass | 97.3% similarity to baseline                |

### Performance Metrics

| Metric                         | Target | Actual   | Status |
| ------------------------------ | ------ | -------- | ------ |
| Avg Latency (simulator→signal) | < 16ms | 12.3ms   | ✅ Pass |
| Max Latency                    | < 50ms | 48.2ms   | ✅ Pass |
| Vitals Rate                    | 60 Hz  | 59.8 Hz  | ✅ Pass |
| Waveform Rate                  | 250 Hz | 248.1 Hz | ✅ Pass |
| Frame Drops                    | 0      | 0        | ✅ Pass |

## Issues Found

*None*

## Recommendations

1. Add continuous latency monitoring
2. Implement performance regression tests in CI
3. Document troubleshooting procedures

## Conclusion

Simulator/Z-Monitor integration is **COMPLETE** and **VALIDATED**. All acceptance criteria met.
```

---

## Acceptance Criteria

- [ ] All unit tests pass (30/30)
- [ ] All integration tests pass (5/5)
- [ ] End-to-end test script passes
- [ ] Latency measured: avg < 16ms, max < 50ms
- [ ] Throughput measured: 60 Hz vitals, 250 Hz waveforms
- [ ] Stall detection works (< 300ms)
- [ ] Visual regression passed (> 95% similarity)
- [ ] Validation report created

---

## Verification Checklist

### 1. Functional
- [ ] All test scenarios pass
- [ ] No errors in logs
- [ ] UI displays live data correctly
- [ ] Connection status accurate

### 2. Code Quality
- [ ] Test code follows best practices
- [ ] No memory leaks detected
- [ ] Test coverage > 80%

### 3. Documentation
- [ ] Validation report complete
- [ ] Test results documented
- [ ] Troubleshooting notes added

### 4. Integration
- [ ] E2E test script runs successfully
- [ ] All components integrate correctly
- [ ] No regressions introduced

### 5. Performance
- [ ] Latency targets met
- [ ] Throughput targets met
- [ ] No performance regressions

---

## Next Steps

After validation complete:
1. Mark task complete in ZTODO.md
2. Update architecture documentation with test results
3. Proceed to production hardening (security review, stress testing)

---

**Estimated Time:** 2-4 hours (includes test execution, profiling, validation report)
