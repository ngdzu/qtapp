# Sensor Integration Summary

**Document ID:** DESIGN-037  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

---

## Overview

For local development we run the Sensor Simulator on the **same machine** as Z-Monitor. To achieve < 16 ms transport latency (60 Hz vitals + 250 Hz waveforms), the simulator and device exchange data through a shared-memory ring buffer backed by `memfd`/POSIX shared memory. The `ISensorDataSource` abstraction is still honored, but the concrete implementation is now `SharedMemorySensorDataSource`, which reads directly from the ring buffer instead of a WebSocket.

---

## Architecture

### Component Diagram

> **📊 Sensor Integration Diagram**  
> [View Sensor Integration (Mermaid)](./37_SENSOR_INTEGRATION.mmd)

The simulator now writes binary frames into a lock-free shared-memory ring buffer. Z-Monitor maps the same buffer, and `SharedMemorySensorDataSource` pulls frames, converts them to `VitalRecord` / `WaveformSample`, and emits the usual Qt signals. The diagram shows the writer, shared buffer, and reader chain, all staying within the < 16 ms target from simulator write to UI update.

---

## Sensor Simulator Details

### Location
```
project-dashboard/sensor-simulator/
```

### Shared Memory Transport

- **Control Socket:** `unix://run/zmonitor-sim.sock` – simulator publishes new `memfd` handles and status.
- **Data Buffer:** `memfd://zmonitor-sim-ring` (mirrored at `/dev/shm/zmonitor-sim-ring` for diagnostic tooling).
- **Synchronization:** lock-free ring buffer with atomic write index + heartbeat timestamp; reader polls using CPU-friendly backoff (< 50 µs).
- **Data Rate:** 60 Hz vitals (one frame every 16.67 ms) and 250 Hz ECG waveform samples (batched into 10-sample chunks per frame).

### Ring Buffer Layout

```
┌──────────────────────────────────────────────┐
│ struct SharedMemoryHeader {                  │
│   uint32_t magic = 0x534D5252;               │
│   uint16_t version = 1;                      │
│   uint16_t slotCount = 2048;                 │
│   std::atomic<uint32_t> writeIndex;          │
│   std::atomic<uint64_t> heartbeatNs;         │
│   uint32_t frameSizeBytes;                   │
│   uint32_t reserved[11];                     │
│ };                                           │
│                                              │
│ struct SensorFrame {                         │
│   uint64_t timestampNs;                      │
│   uint16_t sampleCount;                      │
│   uint16_t channelMask;                      │
│   VitalPayload vital;                        │
│   WaveformPayload waveforms[MAX_CHANNELS];   │
│   uint32_t crc32;                            │
│ };                                           │
└──────────────────────────────────────────────┘
```

- **Writer (Simulator):** increments `writeIndex` after copying a new `SensorFrame` into `slots[writeIndex % slotCount]`, then updates `heartbeatNs`.
- **Reader (Z-Monitor):** tracks `readIndex`, waits until `writeIndex != readIndex`, copies frame, validates `crc32`, then emits Qt signals.

### Simulator Responsibilities

1. On startup, create `memfd`, size it (`ftruncate`) to `sizeof(Header) + slotCount * frameSize`.
2. Publish the `memfd` file descriptor over the Unix domain control socket.
3. Stream data into the ring buffer at 60 Hz / 250 Hz, ensuring writes complete within 2 ms.
4. Update `heartbeatNs` every frame so Z-Monitor can detect stalled writers.
5. Provide tooling (`simctl`) to dump the buffer for debugging.

### Running the Simulator

**Local:**
```bash
cd project-dashboard/sensor-simulator
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./sensor_simulator
```

**Docker:**
```bash
cd project-dashboard
docker compose -f docker-compose.simulator.yml up --build
```

---

## Z-Monitor Integration

### Interface: ISensorDataSource

**Location:** `doc/z-monitor/architecture_and_design/interfaces/ISensorDataSource.md`

**Purpose:**
- Abstract sensor data source (simulator vs hardware vs mock)
- Enable dependency inversion (MonitoringService depends on interface, not implementation)
- Support testing with mock data sources
- Allow future hardware sensor integration

**Key Methods:**
```cpp
class ISensorDataSource : public QObject {
    Q_OBJECT
public:
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isActive() const = 0;
    virtual DataSourceInfo getInfo() const = 0;

signals:
    void vitalSignsReceived(const VitalRecord& vital);
    void waveformSampleReceived(const WaveformSample& waveform);
    void connectionStatusChanged(bool connected, const QString& sensorType);
    void sensorError(const SensorError& error);
};
```

### Implementation: SharedMemorySensorDataSource

**Location:** `z-monitor/src/infrastructure/sensors/SharedMemorySensorDataSource.cpp/h`

**Responsibilities:**
1. Connect to the Unix-domain control socket, receive the latest `memfd` handle.
2. `mmap` the header + slots into process space (read-only).
3. Poll `writeIndex` / `heartbeatNs`, detect new frames, and copy them into local buffers.
4. Convert binary payloads into `VitalRecord` and `WaveformSample` structs (no JSON parsing).
5. Emit Qt signals to `MonitoringService`.
6. Detect stalled simulator (no heartbeat for >250 ms) and raise `sensorError`.

**Key Features:**
- **Microsecond latency:** Reader copies directly from shared memory, no syscalls on the hot path.
- **Zero allocations:** Pre-allocated scratch buffers for waveform arrays.
- **Backpressure handling:** If reader lags, `readIndex` simply jumps to latest and raises a `FrameDropped` warning.
- **Watchdog:** If simulator stops updating `heartbeatNs`, data source enters `Paused` state and awaits next control message.

### Data Flow

```
Sensor Simulator (local process)
  ↓ (memfd write @ 60 Hz / 250 Hz)
SharedMemoryRingBuffer::writeFrame()
  ↓ (atomic writeIndex)
SharedMemorySensorDataSource::pollFrames()
  ↓ (binary decode)
emit ISensorDataSource::vitalSignsReceived(VitalRecord)
emit ISensorDataSource::waveformSampleReceived(WaveformSample)
  ↓ (Qt Signal)
MonitoringService::onVitalsReceived(VitalRecord)
  ↓ (< 16ms transport budget)
  ├─ VitalsCache::append()
  ├─ AlarmManager::processVitalSigns()
  ├─ UI update
  └─ TelemetryBatch::addVital()
```

---

## Alternative Implementations

### 1. SimulatorDataSource (Legacy/Fallback)
- **Purpose:** Internal Qt Timer-based simulator (no external process)
- **Use Case:** When sensor-simulator is not available
- **Location:** `z-monitor/src/infrastructure/sensors/SimulatorDataSource.cpp/h`

### 2. MockSensorDataSource (Testing)
- **Purpose:** Deterministic testing with predefined data
- **Use Case:** Unit tests, integration tests
- **Location:** `z-monitor/src/infrastructure/sensors/MockSensorDataSource.cpp/h`

### 3. HardwareSensorAdapter (Future)
- **Purpose:** Real hardware sensors (serial/USB)
- **Use Case:** Production deployment with actual medical sensors
- **Location:** `z-monitor/src/infrastructure/sensors/HardwareSensorAdapter.cpp/h`

### 4. ReplayDataSource (Development)
- **Purpose:** Replay recorded data from file
- **Use Case:** Debugging specific scenarios
- **Location:** `z-monitor/src/infrastructure/sensors/ReplayDataSource.cpp/h`

---

## Dependency Injection

```cpp
// Configuration (based on settings)
void ServiceContainer::configure() {
    if (Settings::instance()->useSharedMemorySimulator()) {
        registerSingleton<ISensorDataSource>([]() {
            return new SharedMemorySensorDataSource(
                Settings::instance()->sharedMemorySocketPath(),   // e.g. unix://run/zmonitor-sim.sock
                Settings::instance()->sharedMemoryRingName());    // e.g. zmonitor-sim-ring
        });
    } else if (Settings::instance()->useInternalSimulator()) {
        registerSingleton<ISensorDataSource>([]() {
            return new SimulatorDataSource();
        });
    } else {
        registerSingleton<ISensorDataSource>([]() {
            return new MockSensorDataSource();
        });
    }
    
    registerSingleton<MonitoringService>([](ServiceContainer* container) {
        return new MonitoringService(
            container->resolve<ISensorDataSource>(),
            container->resolve<VitalsCache>(),
            container->resolve<AlarmManager>()
        );
    });
}
```

---

## Testing Strategy

### Unit Tests
```cpp
TEST(SharedMemorySensorDataSource, ConnectsToRingBuffer) {
    SharedMemoryTestHarness harness(/*slotCount=*/8);
    SharedMemorySensorDataSource dataSource(harness.socketPath(), harness.ringName());
    QSignalSpy spy(&dataSource, &ISensorDataSource::vitalSignsReceived);
    
    ASSERT_TRUE(dataSource.start());
    harness.writeFrame(makeTestFrame());
    
    QVERIFY(spy.wait(100));   // Expect a frame within 100 ms
}

TEST(SharedMemorySensorDataSource, DetectsStalledWriter) {
    SharedMemoryTestHarness harness(/*slotCount=*/4);
    SharedMemorySensorDataSource dataSource(harness.socketPath(), harness.ringName());
    QSignalSpy errorSpy(&dataSource, &ISensorDataSource::sensorError);
    
    ASSERT_TRUE(dataSource.start());
    harness.stopHeartbeat();
    
    QVERIFY(errorSpy.wait(300));  // Reader should emit stalled error
}
```

### Integration Tests
```cpp
TEST(MonitoringService, ReceivesVitalsFromSharedMemory) {
    SharedMemoryTestHarness harness(/*slotCount=*/16);
    SharedMemorySensorDataSource* dataSource =
        new SharedMemorySensorDataSource(harness.socketPath(), harness.ringName());
    VitalsCache cache;
    AlarmManager alarmMgr;
    MonitoringService service(dataSource, &cache, &alarmMgr);
    
    // Act
    service.start();
    harness.writeFrames(120);  // 2 seconds of data at 60 Hz
    
    // Assert
    EXPECT_GT(cache.size(), 0);  // Cache has vitals
}
```

---

## Performance Considerations

### Data Rate
- **Vitals:** 60 Hz (16.67 ms intervals), 32 bytes per payload.
- **Waveforms:** 250 Hz (~4 ms intervals) batched as 10-sample chunks (ECG Lead II + SpO₂ pleth).
- **Total Shared Memory Throughput:** ≈ 1.2 MB/s (fits comfortably in L3 cache).

### Critical Path Timing
```
Simulator writes frame to shared memory (0µs)
  ↓
SharedMemorySensorDataSource polls (≤ 50µs)
  ↓
Frame decode + signal emit (< 200µs)
  ↓
MonitoringService processing (< 15ms budget)
  ├─ VitalsCache::append() (< 2ms)
  ├─ AlarmManager::processVitals() (< 8ms)
  ├─ UI update (< 3ms)
  └─ Telemetry batch (< 2ms)
```

**Total: < 16 ms from simulator write to UI update** ✅

---

## Security Considerations

### Shared Memory Security
- **Access Control:** Simulator creates the `memfd` with `0600` permissions and only shares the descriptor with local processes that pass an authentication token over the control socket.
- **Sandboxing:** Z-Monitor validates `magic`, `version`, and `frameSizeBytes` before trusting the buffer.
- **DoS Mitigation:** Reader detects overruns and logs frame drops; watchdog restarts simulator if heartbeat stops.

### Data Validation
- **Binary Schema:** CRC32 on each frame, plus per-field range validation (heart rate 0–300 BPM, etc.).
- **Range Checking:** Ensure vitals within physiological limits
- **Error Handling:** Corrupted frames are skipped, and diagnostics are logged.

---
