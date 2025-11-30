# Sensor Integration Summary

**Document ID:** DESIGN-037  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

## Overview

For local development we run the Sensor Simulator on the **same machine** as Z-Monitor. To achieve < 16 ms transport latency (60 Hz vitals + 250 Hz waveforms), the simulator and device exchange data through a shared-memory ring buffer backed by `memfd`/POSIX shared memory. The `ISensorDataSource` abstraction is still honored, but the concrete implementation is now `SharedMemorySensorDataSource`, which reads directly from the ring buffer instead of a WebSocket.

**Key Architecture Point:** The Unix domain socket is **ONLY** used for the initial handshake to exchange the `memfd` file descriptor. **All actual data transfer happens through shared memory** (zero-copy, < 16ms latency). The socket is NOT used for data transfer.

---

## Architecture

### Component Diagram

> **📊 Sensor Integration Diagram**  
> [View Sensor Integration (Mermaid)](./37_SENSOR_INTEGRATION.mmd)

The simulator now writes binary frames into a lock-free shared-memory ring buffer. Z-Monitor maps the same buffer, and `SharedMemorySensorDataSource` pulls frames, converts them to `VitalRecord` / `WaveformSample`, and emits the usual Qt signals. The diagram shows the writer, shared buffer, and reader chain, all staying within the < 16 ms target from simulator write to UI update.

---

## Sensor Simulator Details

### Location
```
project-dashboard/sensor-simulator/
```

### Shared Memory Transport

**Architecture Overview:**
- **Control Socket (Unix Domain Socket):** `unix://run/zmonitor-sim.sock` – Used **ONLY** for the initial handshake to exchange the `memfd` file descriptor. This is a one-time operation during connection setup. The socket is **NOT used for data transfer** - all sensor data flows through shared memory.
- **Data Buffer (Shared Memory):** `memfd://zmonitor-sim-ring` (mirrored at `/dev/shm/zmonitor-sim-ring` for diagnostic tooling) – **All actual sensor data** (60 Hz vitals, 250 Hz waveforms) is transferred through this shared memory ring buffer for zero-copy, low-latency performance.

**Why Socket + Shared Memory?**
- File descriptors (like `memfd`) cannot be easily passed through shared memory itself
- Unix domain sockets support `SCM_RIGHTS` ancillary data to pass file descriptors
- Once the file descriptor is exchanged, all data transfer happens through shared memory
- This pattern avoids socket I/O overhead for every frame (which would add > 60ms latency)

**Transport Details:**
- **Synchronization:** lock-free ring buffer with atomic write index + heartbeat timestamp; reader polls using CPU-friendly backoff (< 50 µs).
- **Data Rate:** 60 Hz vitals (one frame every 16.67 ms) and 250 Hz ECG waveform samples (batched into 10-sample chunks per frame).

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
2. Publish the `memfd` file descriptor over the Unix domain control socket (one-time handshake).
3. Stream data into the ring buffer at 60 Hz / 250 Hz, ensuring writes complete within 2 ms.
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
1. Connect to the Unix-domain control socket **ONLY for initial handshake**, receive the `memfd` file descriptor (one-time operation). **The socket is NOT used for data transfer.**
2. `mmap` the header + slots into process space (read-only) using the received file descriptor.
3. **Disconnect from socket** - no further socket I/O needed. All data transfer happens through shared memory.
4. Poll `writeIndex` / `heartbeatNs` in shared memory (direct memory access, no socket I/O), detect new frames, and copy them into local buffers.
5. Convert binary payloads into `VitalRecord` and `WaveformSample` structs (no JSON parsing).
6. Emit Qt signals to `MonitoringService`.
7. Detect stalled simulator (no heartbeat for >250 ms) and raise `sensorError`.

**Why Socket + Shared Memory?**
- **File descriptors cannot be passed through shared memory itself** - you need a mechanism to exchange the `memfd` file descriptor between processes
- Unix domain sockets support `SCM_RIGHTS` ancillary data to pass file descriptors between processes
- Once the file descriptor is exchanged, **all data transfer happens through shared memory** (zero-copy, < 16ms latency)
- This pattern avoids socket I/O overhead for every frame (which would add > 60ms latency)
- The socket is only used during connection setup/teardown, not during normal data transfer

**Key Features:**
- **Microsecond latency:** Reader copies directly from shared memory, no syscalls on the hot path.
- **Zero allocations:** Pre-allocated scratch buffers for waveform arrays.
- **Backpressure handling:** If reader lags, `readIndex` simply jumps to latest and raises a `FrameDropped` warning.
- **Watchdog:** If simulator stops updating `heartbeatNs`, data source enters `Paused` state and awaits next control message.

### Data Flow

**Connection Setup (One-Time, via Socket):**
```
Sensor Simulator creates memfd
  ↓ (Unix domain socket handshake)
SharedMemoryControlChannel::connect()
  ↓ (SCM_RIGHTS - pass file descriptor)
SharedMemorySensorDataSource receives memfd
  ↓ (mmap shared memory)
SharedMemorySensorDataSource::mapSharedMemory()
  ↓ (Socket disconnected - no longer used)
```

**Data Transfer (Continuous, via Shared Memory - Zero Socket I/O):**
```
Sensor Simulator (local process)
  ↓ (memfd write @ 60 Hz / 250 Hz - DIRECT MEMORY WRITE)
SharedMemoryRingBuffer::writeFrame()
  ↓ (atomic writeIndex - NO SOCKET I/O)
SharedMemorySensorDataSource::pollFrames()
  ↓ (binary decode - NO SOCKET I/O)
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

**Key Point:** After the initial handshake, the socket is disconnected and **all data flows through shared memory only**. No socket I/O occurs during normal operation, achieving < 16ms latency.

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

## Platform-Specific Implementation Notes

### macOS Implementation (memfd Compatibility)

**Challenge:** Linux `memfd_create()` is not available on macOS. Must use POSIX `shm_open()` as alternative.

#### Option 1: POSIX Shared Memory (`shm_open`)

**Simulator Side:**
```cpp
#ifdef __APPLE__
    // macOS: Use POSIX shared memory
    const char* shmName = "/zmonitor-sim-ring";
    int shmFd = shm_open(shmName, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shmFd < 0) {
        qCritical() << "shm_open failed:" << strerror(errno);
        return false;
    }
    
    // Set size
    if (ftruncate(shmFd, ringBufferSize) < 0) {
        qCritical() << "ftruncate failed:" << strerror(errno);
        shm_unlink(shmName);
        close(shmFd);
        return false;
    }
    
    // Map memory
    void* mappedMemory = mmap(nullptr, ringBufferSize, PROT_READ | PROT_WRITE,
                              MAP_SHARED, shmFd, 0);
#else
    // Linux: Use memfd
    int shmFd = memfd_create("zmonitor-sim-ring", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    // ... rest of memfd implementation
#endif
```

**Z-Monitor Side:**
```cpp
// Receive file descriptor via SCM_RIGHTS (same on both platforms)
int receivedFd = -1;
if (receiveFileDescriptor(receivedFd)) {
    // Map shared memory using received file descriptor
    // Works identically on macOS and Linux after handshake
    m_mappedMemory = mmap(nullptr, size, PROT_READ, MAP_SHARED, receivedFd, 0);
}
```

**Cleanup (macOS-specific):**
```cpp
#ifdef __APPLE__
    // Must explicitly unlink POSIX shared memory
    shm_unlink("/zmonitor-sim-ring");
#endif
// close(fd) automatically cleans up memfd on Linux
```

**Key Differences:**
| Feature                     | Linux (memfd)                   | macOS (shm_open)                            |
| --------------------------- | ------------------------------- | ------------------------------------------- |
| **Creation**                | `memfd_create()`                | `shm_open()` with name                      |
| **Namespace**               | Anonymous (no filesystem entry) | Named (`/zmonitor-sim-ring` in `/dev/shm/`) |
| **Cleanup**                 | Automatic on close              | Requires `shm_unlink()`                     |
| **Sealing**                 | Supported (`F_ADD_SEALS`)       | Not supported                               |
| **File Descriptor Passing** | Via `SCM_RIGHTS`                | Via `SCM_RIGHTS` (same)                     |
| **Performance**             | Identical after mmap()          | Identical after mmap()                      |

**Recommended Approach:**
- Use preprocessor directives (`#ifdef __APPLE__`) to select implementation
- File descriptor passing via Unix socket works identically on both platforms
- After `mmap()`, all ring buffer operations are identical

#### Option 2: memfd Polyfill Library

**Alternative:** Use a cross-platform memfd polyfill library (e.g., `memfd_create_compat`):
```cpp
#ifdef __APPLE__
    #include "memfd_compat.h"  // Polyfill that uses shm_open internally
#else
    #include <sys/memfd.h>
#endif

int fd = memfd_create("zmonitor-sim-ring", MFD_CLOEXEC);
// Same code on both platforms
```

**Trade-offs:**
- **Pros:** Single codebase, no `#ifdef` in business logic
- **Cons:** Extra dependency, polyfill may not support all features (sealing)

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

## Z-Monitor Integration Checklist

### What Z-Monitor MUST Implement

This checklist defines all components that z-monitor must implement to successfully integrate with the sensor simulator.

#### 1. ISensorDataSource Interface ✅

**Status:** Interface defined in `doc/interfaces/ISensorDataSource.md`

**Requirements:**
- [ ] Define pure virtual interface with all required methods:
  - `bool start()` - Start data acquisition
  - `void stop()` - Stop data acquisition
  - `bool isActive() const` - Check if active
  - `DataSourceInfo getInfo() const` - Get data source metadata
- [ ] Define Qt signals:
  - `vitalsReceived(const VitalRecord& vital)` - New vitals available
  - `waveformSampleReady(const WaveformSample& sample)` - New waveform sample
  - `connectionStatusChanged(bool connected, const QString& sensorType)` - Connection state
  - `sensorError(const SensorError& error)` - Error occurred

**Files:**
- `z-monitor/src/domain/interfaces/ISensorDataSource.h`
- `doc/interfaces/ISensorDataSource.md`

---

#### 2. SharedMemorySensorDataSource Implementation ⏳

**Status:** Not yet implemented

**Requirements:**
- [ ] Implement `ISensorDataSource` interface
- [ ] Constructor accepts:
  - `socketPath` (e.g., "/tmp/z-monitor-sensor.sock")
  - `ringBufferName` (e.g., "zmonitor-sim-ring")
- [ ] `start()` method:
  - Connects to Unix domain socket
  - Receives file descriptor via `SCM_RIGHTS`
  - Maps shared memory using `mmap()`
  - Validates ring buffer header (magic, version)
  - Starts polling timer (< 50µs interval, `Qt::PreciseTimer`)
  - Returns `true` on success, `false` on failure
- [ ] `stop()` method:
  - Stops polling timer
  - Unmaps shared memory (`munmap()`)
  - Closes file descriptor
  - Disconnects from socket
- [ ] Polling loop:
  - Read `writeIndex` from ring buffer header (atomic load, acquire semantics)
  - If `writeIndex != readIndex`, process frames
  - Copy frame from `slots[readIndex % frameCount]`
  - Validate CRC32
  - Parse frame based on type (Vitals, Waveform, Heartbeat)
  - Emit appropriate Qt signal
  - Increment `readIndex`
- [ ] Stall detection:
  - Read `heartbeatTimestamp` every poll
  - If no update for > 250ms, emit `sensorError("Writer stalled")`
- [ ] Error handling:
  - CRC mismatch → Skip frame, log error, continue
  - Invalid frame type → Skip frame, log warning
  - Connection lost → Emit `connectionStatusChanged(false)`, attempt reconnect

**Files:**
- `z-monitor/src/infrastructure/sensors/SharedMemorySensorDataSource.h`
- `z-monitor/src/infrastructure/sensors/SharedMemorySensorDataSource.cpp`

---

#### 3. SharedMemoryControlChannel ⏳

**Status:** Not yet implemented

**Requirements:**
- [ ] Handles Unix domain socket connection and file descriptor exchange
- [ ] `connect(const QString& socketPath)` method:
  - Creates Unix domain socket (`socket(AF_UNIX, SOCK_STREAM, 0)`)
  - Connects to simulator's control socket
  - Returns `true` on success
- [ ] `receiveFileDescriptor(int& fd)` method:
  - **CRITICAL:** Uses `recvmsg()` NOT `recv()` for first receive
  - Receives `ControlMessage` structure
  - Extracts file descriptor from ancillary data (`SCM_RIGHTS`)
  - Returns `true` on success
- [ ] Signal: `handshakeCompleted(int memfdFd, size_t ringBufferSize)`
- [ ] Signal: `handshakeFailed(const QString& error)`

**Files:**
- `z-monitor/src/infrastructure/sensors/SharedMemoryControlChannel.h`
- `z-monitor/src/infrastructure/sensors/SharedMemoryControlChannel.cpp`

**Platform Note:**
- Code must work on both Linux (memfd) and macOS (shm_open)
- File descriptor passing via `SCM_RIGHTS` is identical on both platforms

---

#### 4. SharedMemoryRingBuffer ⏳

**Status:** Not yet implemented

**Requirements:**
- [ ] Wraps shared memory region, provides safe frame reading
- [ ] Constructor accepts:
  - `void* mappedMemory` - Pointer to mapped shared memory
  - `size_t mappedSize` - Size of mapped region
- [ ] `validateHeader()` method:
  - Check magic number: `0x534D5242` ('SMRB')
  - Check version: `1`
  - Check frame size: `4096` bytes
  - Check frame count: `2048` frames
  - Returns `true` if valid
- [ ] `readFrame(uint64_t readIndex, SensorFrame& outFrame)` method:
  - Calculate slot: `readIndex % frameCount`
  - Copy frame from `slots[slot]` into `outFrame`
  - Validate CRC32
  - Returns `true` if valid frame
- [ ] `getWriteIndex()` method:
  - Atomic load of `writeIndex` with acquire semantics
  - Returns current write index
- [ ] `getHeartbeatTimestamp()` method:
  - Atomic load of `heartbeatTimestamp`
  - Returns last heartbeat time (ms since epoch)

**Files:**
- `z-monitor/src/infrastructure/sensors/SharedMemoryRingBuffer.h`
- `z-monitor/src/infrastructure/sensors/SharedMemoryRingBuffer.cpp`

---

#### 5. Frame Parsing (Vitals, Waveform, Heartbeat) ⏳

**Status:** Not yet implemented

**Requirements:**
- [ ] Parse Vitals Frame (type `0x01`):
  - Deserialize JSON payload: `{"hr":72,"spo2":98,"rr":16}`
  - Create `VitalRecord` object
  - Populate: `heartRate`, `spo2`, `respirationRate`, `timestamp`
  - Emit `vitalsReceived(VitalRecord)`
- [ ] Parse Waveform Frame (type `0x02`):
  - Deserialize JSON payload: `{"channel":"ecg","sample_rate":250,"start_timestamp_ms":...,"values":[...]}`
  - Create `WaveformSample` objects for each value
  - Populate: `channel`, `sampleRate`, `timestamp`, `value`
  - Emit `waveformSampleReady(WaveformSample)` for each sample
- [ ] Parse Heartbeat Frame (type `0x03`):
  - Update connection status (writer is alive)
  - Reset stall watchdog timer

**Files:**
- Implement in `SharedMemorySensorDataSource::parseFrame(const SensorFrame& frame)`

---

#### 6. MonitoringService Integration ⏳

**Status:** Partial (MonitoringService exists, needs integration)

**Requirements:**
- [ ] Update `MonitoringService` to accept `ISensorDataSource*` in constructor (dependency injection)
- [ ] Connect signals:
  - `ISensorDataSource::vitalsReceived` → `MonitoringService::onVitalsReceived(VitalRecord)`
  - `ISensorDataSource::waveformSampleReady` → `MonitoringService::onWaveformSample(WaveformSample)`
  - `ISensorDataSource::sensorError` → `MonitoringService::onSensorError(SensorError)`
- [ ] `onVitalsReceived(VitalRecord)` method:
  - Append to `VitalsCache` (in-memory, critical path)
  - Evaluate alarm rules via `AlarmAggregate`
  - Emit `vitalsUpdated` signal to UI controllers
  - Enqueue to `TelemetryBatch` for network transmission
- [ ] `onWaveformSample(WaveformSample)` method:
  - Append to `WaveformCache` (circular buffer, display-only)
  - Emit `waveformUpdated` signal to `WaveformController`
- [ ] Start/stop sensor data source in service lifecycle:
  - `MonitoringService::start()` → `m_sensorDataSource->start()`
  - `MonitoringService::stop()` → `m_sensorDataSource->stop()`

**Files:**
- `z-monitor/src/application/services/MonitoringService.h`
- `z-monitor/src/application/services/MonitoringService.cpp`

---

#### 7. VitalsCache and WaveformCache ⏳

**Status:** Not yet implemented

**Requirements:**
- [ ] **VitalsCache** (see `doc/36_DATA_CACHING_STRATEGY.md`):
  - In-memory cache for 3-day capacity (~39 MB)
  - Thread-safe (`QReadWriteLock`)
  - `append(VitalRecord)` - Add vital to cache
  - `getRange(start, end)` - Get vitals in time range
  - `getUnpersistedVitals()` - Get vitals not yet saved to database
  - `markAsPersisted(upToTimestamp)` - Mark vitals as saved
- [ ] **WaveformCache** (see `doc/36_DATA_CACHING_STRATEGY.md`):
  - Circular buffer for 30 seconds (~0.1 MB)
  - Display-only (not persisted to database)
  - `append(WaveformSample)` - Add sample (overwrites oldest)
  - `getLastSeconds(seconds)` - Get last N seconds of data

**Files:**
- `z-monitor/src/infrastructure/caching/VitalsCache.h`
- `z-monitor/src/infrastructure/caching/VitalsCache.cpp`
- `z-monitor/src/infrastructure/caching/WaveformCache.h`
- `z-monitor/src/infrastructure/caching/WaveformCache.cpp`

---

#### 8. UI Controllers (DashboardController, WaveformController) ⏳

**Status:** Partial (controllers exist, need data binding)

**Requirements:**
- [ ] **DashboardController:**
  - Connect to `MonitoringService::vitalsUpdated` signal
  - Update Q_PROPERTY values: `heartRate`, `spo2`, `respirationRate`, `nibp`, `temperature`
  - QML binds to these properties for real-time display
- [ ] **WaveformController:**
  - Connect to `MonitoringService::waveformUpdated` signal
  - Provide waveform data as `QVariantList` for QML Canvas rendering
  - Support 60 FPS update rate (16ms frame budget)

**Files:**
- `z-monitor/src/interface/controllers/DashboardController.h`
- `z-monitor/src/interface/controllers/DashboardController.cpp`
- `z-monitor/src/interface/controllers/WaveformController.h`
- `z-monitor/src/interface/controllers/WaveformController.cpp`

---

#### 9. Configuration ⏳

**Status:** Not yet implemented

**Requirements:**
- [ ] Add settings for sensor data source selection:
  - `sensor.source` - "shared_memory", "internal_simulator", "mock", or "hardware"
  - `sensor.shared_memory.socket_path` - Unix socket path (default: "/tmp/z-monitor-sensor.sock")
  - `sensor.shared_memory.ring_buffer_name` - Shared memory name (default: "zmonitor-sim-ring")
- [ ] Dependency injection container configures correct `ISensorDataSource` implementation based on settings

**Files:**
- `z-monitor/config/default.conf` or similar
- `z-monitor/src/infrastructure/config/Settings.h`

---

#### 10. Testing ⏳

**Status:** Not yet implemented

**Requirements:**
- [ ] **Unit Tests:**
  - `SharedMemoryControlChannel` - Socket connection, FD exchange
  - `SharedMemoryRingBuffer` - Header validation, frame reading, CRC validation
  - `SharedMemorySensorDataSource` - Frame parsing, signal emission, stall detection
- [ ] **Integration Tests:**
  - E2E test with simulator running: verify data flow from simulator → z-monitor → UI
  - Latency measurement: verify < 16ms end-to-end
  - Stall detection: kill simulator, verify z-monitor detects within 300ms
  - Multiple readers: verify multiple z-monitor instances can read simultaneously

**Files:**
- `z-monitor/tests/unit/infrastructure/sensors/shared_memory_sensor_test.cpp`
- `z-monitor/tests/integration/sensor_simulator_integration_test.cpp`

---

### Implementation Priority

**Phase 1: Core Infrastructure (High Priority)**
1. SharedMemoryControlChannel (socket + FD exchange)
2. SharedMemoryRingBuffer (ring buffer reader)
3. SharedMemorySensorDataSource (polling loop, frame parsing)

**Phase 2: Application Integration (High Priority)**
4. VitalsCache and WaveformCache
5. MonitoringService integration

**Phase 3: UI Integration (Medium Priority)**
6. DashboardController and WaveformController updates
7. QML UI data binding

**Phase 4: Testing & Validation (Medium Priority)**
8. Unit tests
9. Integration tests
10. Latency measurement and optimization

---

## Performance Considerations

### Data Rate
- **Vitals:** 60 Hz (16.67 ms intervals), 32 bytes per payload.
- **Waveforms:** 250 Hz (~4 ms intervals) batched as 10-sample chunks (ECG Lead II + SpO₂ pleth).
- **Total Shared Memory Throughput:** ≈ 1.2 MB/s (fits comfortably in L3 cache).

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

**Total: < 16 ms from simulator write to UI update** ✅

---

## Troubleshooting and Diagnostics

### Common Issues and Solutions

#### Issue 1: Handshake Fails - "Failed to receive file descriptor"

**Symptoms:**
- Z-Monitor log: "Failed to receive file descriptor via SCM_RIGHTS"
- Z-Monitor log: "SharedMemoryControlChannel: Connection failed"
- Simulator shows client connected but no data transfer

**Root Causes:**
1. **Z-Monitor using `recv()` instead of `recvmsg()` for first receive**
   - Problem: `recv()` consumes data portion but loses ancillary data (file descriptor)
   - Solution: Update `SharedMemoryControlChannel::onSocketDataAvailable()` to use `recvmsg()` for first receive
   - Reference: See `project-dashboard/sensor-simulator/tests/handshake_compatibility.md`

2. **Socket path mismatch**
   - Simulator: `/tmp/z-monitor-sensor.sock`
   - Z-Monitor: `/tmp/zmonitor-sim.sock` (different!)
   - Solution: Verify socket path configuration matches on both sides

3. **Permissions issue**
   - Socket file created with restrictive permissions
   - Solution: Create socket with `0666` permissions or ensure both processes run as same user

**Diagnostic Commands:**
```bash
# Check if socket exists
ls -la /tmp/z-monitor-sensor.sock

# Check socket permissions
stat /tmp/z-monitor-sensor.sock

# Test socket connection (should show "Connected")
echo "test" | nc -U /tmp/z-monitor-sensor.sock

# Check processes using socket
lsof | grep z-monitor-sensor.sock
```

**Fix Checklist:**
- [ ] Verify socket paths match in both configs
- [ ] Update Z-Monitor to use `recvmsg()` for first receive
- [ ] Check socket file permissions (should be readable/writable)
- [ ] Verify both processes run as same user (or socket has 0666 perms)
- [ ] Check simulator logs: "ControlServer: Listening on [path]"
- [ ] Check z-monitor logs: "Connecting to [path]"

---

#### Issue 2: No Data Received - Connection succeeds but no vitals/waveforms

**Symptoms:**
- Handshake completes successfully
- Z-Monitor log: "SharedMemorySensorDataSource: Started successfully"
- No vitals or waveforms appear in UI
- No `vitalsUpdated` or `waveformSampleReady` signals emitted

**Root Causes:**
1. **Ring buffer structure mismatch**
   - Magic number mismatch (simulator: `0x534D5242`, z-monitor expects different)
   - Version mismatch (simulator: v1, z-monitor expects v2)
   - Field offset mismatch (different header layouts)

2. **Frame parsing errors**
   - CRC32 validation failing (data corruption or different CRC algorithm)
   - JSON deserialization failing (different payload schema)
   - Frame type not recognized (simulator sends 0x01, z-monitor expects 0x10)

3. **Polling not working**
   - `writeIndex` not being updated by simulator
   - Z-Monitor not polling (timer not started)
   - Polling interval too slow (missing data)

**Diagnostic Commands:**
```bash
# Dump shared memory buffer (Linux)
hexdump -C /dev/shm/zmonitor-sim-ring | head -n 20

# Check if simulator is writing (watch writeIndex)
watch -n 0.1 'xxd -s 16 -l 8 /dev/shm/zmonitor-sim-ring'

# Verify magic number (should be 52 42 4D 53 = 'SMRB')
xxd -s 0 -l 4 /dev/shm/zmonitor-sim-ring

# Monitor simulator process CPU (should be ~2-5% if writing)
top -p $(pgrep sensor_simulator)
```

**Fix Checklist:**
- [ ] Run integration test to verify structure compatibility
- [ ] Verify magic number: `0x534D5242` ('SMRB')
- [ ] Verify version: `1`
- [ ] Check CRC32 algorithm (both use same implementation)
- [ ] Add debug logging to frame parsing (log every frame read)
- [ ] Verify polling timer is started (`QTimer` running at 50µs interval)
- [ ] Check simulator logs: "Frame written, writeIndex: X"
- [ ] Check z-monitor logs: "Frame read, readIndex: X"

---

#### Issue 3: High Latency - Latency > 16ms

**Symptoms:**
- Data appears in UI but with noticeable delay
- Measured latency > 16ms (target: < 16ms)
- Waveforms appear "stuttery" or delayed

**Root Causes:**
1. **System load**
   - CPU at 100% (other processes consuming resources)
   - Thermal throttling (CPU frequency reduced)

2. **Timer precision issues**
   - Simulator timer interval too long (> 16.67ms for 60Hz vitals)
   - Z-Monitor polling interval too long (> 50µs)
   - QTimer not using precise timing (`Qt::PreciseTimer`)

3. **Frame processing overhead**
   - CRC validation taking too long
   - JSON parsing taking too long
   - Memory allocations in hot path

**Diagnostic Commands:**
```bash
# Measure end-to-end latency (requires instrumentation)
# Add timestamps to simulator frames, measure in z-monitor

# Check CPU frequency (should not be throttled)
sysctl -a | grep machdep.cpu.brand_string  # macOS
lscpu | grep MHz  # Linux

# Check system load
top -l 1 | grep "CPU usage"  # macOS
top -bn1 | grep "Cpu(s)"  # Linux

# Profile z-monitor (find hot spots)
instruments -t "Time Profiler" ./z-monitor  # macOS
perf record -g ./z-monitor  # Linux
```

**Fix Checklist:**
- [ ] Verify simulator timer: 16.67ms for vitals (60Hz), 4ms for waveforms (250Hz)
- [ ] Verify z-monitor polling: < 50µs (use `QTimer::setTimerType(Qt::PreciseTimer)`)
- [ ] Profile frame processing (should be < 200µs per frame)
- [ ] Remove memory allocations in hot path (pre-allocate buffers)
- [ ] Optimize CRC32 (use hardware acceleration if available)
- [ ] Optimize JSON parsing (consider binary format for waveforms)
- [ ] Check thread priority (RT thread should be high priority)
- [ ] Verify CPU affinity (pin RT thread to dedicated core)

---

#### Issue 4: Stall Detection Not Working

**Symptoms:**
- Simulator stops writing but Z-Monitor doesn't detect stall
- No `sensorError` signal emitted
- UI shows last vitals indefinitely (stale data)

**Root Causes:**
1. **Heartbeat not being checked**
   - Z-Monitor not reading `heartbeatTimestamp` from header
   - Watchdog timer not running

2. **Threshold too long**
   - Stall threshold set to > 250ms (too tolerant)
   - UI appears frozen before detection

**Fix Checklist:**
- [ ] Verify Z-Monitor reads `heartbeatTimestamp` every poll
- [ ] Verify stall detection threshold: 250ms (no heartbeat update)
- [ ] Add debug logging: "Last heartbeat: X ms ago"
- [ ] Test by killing simulator process (should detect within 300ms)
- [ ] Verify `sensorError` signal is connected to UI

---

### Diagnostic Tools

#### Tool 1: Shared Memory Inspector (`shmem_inspect`)

**Purpose:** Dump ring buffer header and frames for manual inspection

```bash
#!/bin/bash
# shmem_inspect.sh - Inspect shared memory ring buffer

SHM_PATH="/dev/shm/zmonitor-sim-ring"  # Linux
# SHM_PATH="/tmp/zmonitor-sim-ring"  # macOS

if [ ! -e "$SHM_PATH" ]; then
    echo "Error: Shared memory not found at $SHM_PATH"
    exit 1
fi

echo "=== Ring Buffer Header ==="
echo "Magic:       $(xxd -s 0 -l 4 -p $SHM_PATH)"  # Should be 534d5242
echo "Version:     $(xxd -s 4 -l 2 -p $SHM_PATH)"
echo "Frame Size:  $(xxd -s 6 -l 4 -p $SHM_PATH)"
echo "Frame Count: $(xxd -s 10 -l 4 -p $SHM_PATH)"
echo "Write Index: $(xxd -s 16 -l 8 -p $SHM_PATH)"
echo "Heartbeat:   $(xxd -s 24 -l 8 -p $SHM_PATH)"
echo ""
echo "=== First Frame ==="
xxd -s 64 -l 256 $SHM_PATH  # Dump first frame (skip header)
```

**Usage:**
```bash
chmod +x shmem_inspect.sh
./shmem_inspect.sh
```

**Expected Output:**
```
=== Ring Buffer Header ===
Magic:       534d5242
Version:     0001
Frame Size:  00001000  (4096 bytes)
Frame Count: 00000800  (2048 frames)
Write Index: 000000000000002a  (42)
Heartbeat:   000001934f8e7c80  (timestamp)

=== First Frame ===
0000040: 01 00 00 00 00 01 93 4f  8e 7c 40 00 00 00 2a 00  .......O.|@...*.
0000050: 7b 22 68 72 22 3a 37 32  2c 22 73 70 6f 32 22 3a  {"hr":72,"spo2":
```

---

#### Tool 2: Latency Measurement (`latency_test`)

**Purpose:** Measure end-to-end latency from simulator write to z-monitor signal

**Implementation:**
```cpp
// In simulator: Add write timestamp to frame
void Simulator::writeFrame(const SensorFrame& frame) {
    auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();
    frame.timestampNs = nowNs;  // Write time
    writeToRingBuffer(frame);
}

// In z-monitor: Measure latency on receive
void SharedMemorySensorDataSource::onFrameReceived(const SensorFrame& frame) {
    auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();
    auto latencyNs = nowNs - frame.timestampNs;
    auto latencyMs = latencyNs / 1'000'000.0;
    
    if (latencyMs > 16.0) {
        qWarning() << "High latency:" << latencyMs << "ms";
    }
    
    // Emit to metrics collector
    emit latencyMeasured(latencyMs);
}
```

**Usage:**
1. Build z-monitor with latency measurement enabled
2. Connect to `latencyMeasured` signal in UI
3. Display real-time latency graph
4. Alert if latency > 16ms

---

#### Tool 3: Frame Validator (`frame_validator`)

**Purpose:** Validate frame structure compatibility between simulator and z-monitor

```cpp
// Integration test that runs both simulator and z-monitor code
TEST(FrameCompatibility, StructureMatches) {
    // Simulator creates frame
    SensorFrame simFrame;
    simFrame.type = 0x01;  // Vitals
    simFrame.timestampNs = 123456789;
    simFrame.sequenceNumber = 42;
    simFrame.dataSize = 32;
    strcpy((char*)simFrame.data, "{\"hr\":72,\"spo2\":98,\"rr\":16}");
    simFrame.crc32 = calculateCRC32(&simFrame, sizeof(simFrame) - 4);
    
    // Z-Monitor reads frame
    VitalRecord vital = parseVitalsFrame(simFrame);
    
    // Verify
    EXPECT_EQ(vital.heartRate, 72);
    EXPECT_EQ(vital.spo2, 98);
    EXPECT_EQ(vital.respirationRate, 16);
}
```

---

### Debugging Checklist for Integration

**Before Starting:**
- [ ] Read `doc/37_SENSOR_INTEGRATION.md` (architecture overview)
- [ ] Read `doc/44_SIMULATOR_INTEGRATION_GUIDE.md` (step-by-step guide)
- [ ] Read `project-dashboard/sensor-simulator/README.md` (simulator details)
- [ ] Read `project-dashboard/sensor-simulator/tests/handshake_compatibility.md` (socket handshake)
- [ ] Understand memfd vs shm_open difference (Linux vs macOS)

**Simulator Setup:**
- [ ] Simulator builds without errors
- [ ] Simulator creates shared memory (check `/dev/shm/` or `/tmp/`)
- [ ] Simulator creates Unix socket (check `/tmp/z-monitor-sensor.sock`)
- [ ] Simulator log shows: "Listening on /tmp/z-monitor-sensor.sock"
- [ ] Simulator UI displays vitals updating at 60 Hz
- [ ] Simulator UI displays waveform updating at 250 Hz

**Z-Monitor Setup:**
- [ ] Z-Monitor builds without errors
- [ ] SharedMemorySensorDataSource class exists
- [ ] SharedMemoryControlChannel class exists (socket handshake)
- [ ] SharedMemoryRingBuffer class exists (ring buffer reader)
- [ ] Configuration points to correct socket path

**Handshake:**
- [ ] Z-Monitor connects to socket (check logs)
- [ ] Z-Monitor receives file descriptor via `recvmsg()` with `SCM_RIGHTS`
- [ ] Z-Monitor maps shared memory successfully (`mmap()` succeeds)
- [ ] Z-Monitor validates ring buffer header (magic, version)
- [ ] Socket disconnects after handshake (no longer needed)

**Data Flow:**
- [ ] Simulator writes frames to ring buffer
- [ ] `writeIndex` increments every 16.67ms (vitals) and 4ms (waveforms)
- [ ] `heartbeatTimestamp` updates every frame
- [ ] Z-Monitor polls `writeIndex` (< 50µs interval)
- [ ] Z-Monitor reads frames when `writeIndex != readIndex`
- [ ] CRC32 validation passes for all frames
- [ ] JSON deserialization succeeds
- [ ] `vitalsUpdated` signal emitted (connects to MonitoringService)
- [ ] `waveformSampleReady` signal emitted (connects to WaveformController)

**UI Verification:**
- [ ] Vitals display shows data matching simulator
- [ ] Waveforms render smoothly (60 FPS)
- [ ] Connection status shows "Connected"
- [ ] No QML errors in console
- [ ] Latency < 16ms (measure with instrumentation)

**Error Handling:**
- [ ] Stop simulator → Z-Monitor detects stall within 300ms
- [ ] Invalid CRC → Frame skipped, error logged
- [ ] Socket disconnects → Z-Monitor attempts reconnect
- [ ] Shared memory unmapped → Error logged, graceful shutdown

---

## Understanding memfd and Socket Handshake Architecture

> **📚 Foundational Knowledge:** For a general overview of shared memory IPC patterns, see [Shared Memory IPC with memfd and Unix Domain Sockets](../../foundation/05_memory_and_performance/07_shared_memory_ipc.md) in the foundational knowledge base.

This section provides a comprehensive explanation of why we use both `memfd` (shared memory) and a Unix domain socket for sensor data integration. Understanding this architecture is crucial for developers working on the sensor integration layer.

### What is memfd?

**`memfd_create`** is a Linux system call (since kernel 3.17) that creates an anonymous file descriptor backed by RAM. Unlike traditional POSIX shared memory (`shm_open`), `memfd`:

1. **No Filesystem Namespace Pollution:** Traditional `shm_open` creates entries in `/dev/shm/` that persist until explicitly removed. `memfd` creates an anonymous file descriptor that exists only in memory and is automatically cleaned up when all references are closed.

2. **Better Security:** `memfd` file descriptors are not accessible via the filesystem, reducing attack surface. They can only be accessed by processes that have the file descriptor (passed via IPC mechanisms).

3. **Automatic Cleanup:** When the last process closes the `memfd` file descriptor, the memory is automatically freed by the kernel. No manual cleanup of `/dev/shm/` entries required.

4. **Sealing Support:** `memfd` supports "sealing" (`F_SEAL_WRITE`, `F_SEAL_SHRINK`, etc.) to prevent modifications after initialization, providing additional security guarantees.

**Example: Creating a memfd**
```cpp
// In sensor simulator (Simulator.cpp)
#include <sys/mman.h>
#include <sys/memfd.h>
#include <unistd.h>
#include <fcntl.h>

int memfdFd = memfd_create("zmonitor-sim-ring", MFD_CLOEXEC | MFD_ALLOW_SEALING);
if (memfdFd < 0) {
    qCritical() << "Failed to create memfd:" << strerror(errno);
    return false;
}

// Set size of shared memory region
size_t ringBufferSize = sizeof(RingBufferHeader) + (2048 * 4096); // Header + 2048 frames × 4KB
if (ftruncate(memfdFd, ringBufferSize) < 0) {
    qCritical() << "Failed to set memfd size:" << strerror(errno);
    close(memfdFd);
    return false;
}

// Map into process address space
void* mappedMemory = mmap(nullptr, ringBufferSize, PROT_READ | PROT_WRITE, 
                          MAP_SHARED, memfdFd, 0);
if (mappedMemory == MAP_FAILED) {
    qCritical() << "Failed to mmap memfd:" << strerror(errno);
    close(memfdFd);
    return false;
}
```

### Why Do We Need a Socket Connection?

**Critical Point:** File descriptors (like `memfd`) **cannot be passed through shared memory itself**. You need a separate IPC mechanism to exchange the file descriptor between processes.

**Unix Domain Sockets with SCM_RIGHTS:**
- Unix domain sockets (`AF_UNIX`) support **ancillary data** via `SCM_RIGHTS` control messages
- `SCM_RIGHTS` allows passing file descriptors between processes over a socket connection
- This is the standard Linux mechanism for file descriptor passing

**The Handshake Flow:**

```
┌─────────────────────────────────────────────────────────────┐
│ Phase 1: Connection Setup (One-Time, via Socket)           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Simulator Process                    Z-Monitor Process    │
│  ────────────────                    ────────────────     │
│                                                             │
│  1. Create memfd                                            │
│     memfd_create()                                          │
│         │                                                   │
│  2. Map shared memory                                       │
│     mmap(memfdFd)                                           │
│         │                                                   │
│  3. Start control socket                                    │
│     listen("/tmp/z-monitor-sensor.sock")                    │
│         │                                                   │
│         │                   4. Connect to socket            │
│         │◄────────────────── connect()                     │
│         │                                                   │
│  5. Send memfd via SCM_RIGHTS                               │
│     sendmsg(socketFd, {ControlMessage, memfdFd})            │
│         │──────────────────►                                │
│         │                   6. Receive memfd                │
│         │                   recvmsg() extracts FD           │
│         │                                                   │
│  7. Socket can be closed (handshake complete)              │
│                                                             │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ Phase 2: Data Transfer (Continuous, via Shared Memory)     │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Simulator Process                    Z-Monitor Process    │
│  ────────────────                    ────────────────       │
│                                                             │
│  8. Write frame to shared memory                            │
│     memcpy(mappedMemory + offset, frameData)                │
│     atomic_store(&writeIndex, newIndex)                      │
│         │                                                   │
│         │                   9. Poll shared memory            │
│         │                   atomic_load(&writeIndex)        │
│         │                   if (writeIndex != readIndex)   │
│         │                     memcpy(frameData)            │
│         │                                                   │
│  10. Repeat @ 60 Hz (vitals) / 250 Hz (waveforms)          │
│      NO SOCKET I/O - Direct memory access only             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Code Example: Socket Handshake (Simulator Side)**
```cpp
// In ControlServer.cpp - Sending memfd via socket
bool ControlServer::sendFileDescriptor(int clientFd) {
    // Prepare ControlMessage structure
    struct ControlMessage {
        uint8_t type;            // 0x01 = Handshake
        uint8_t reserved[3];
        uint32_t memfdFd;        // Not used (FD is in ancillary data)
        uint64_t ringBufferSize;
        char socketPath[108];
    } message;
    
    message.type = 0x01;  // Handshake
    message.ringBufferSize = m_ringBufferSize;
    
    // Send ControlMessage + memfd file descriptor together
    struct msghdr msg = {0};
    struct iovec iov[1];
    char control_buf[CMSG_SPACE(sizeof(int))];
    
    iov[0].iov_base = &message;
    iov[0].iov_len = sizeof(message);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    // Prepare ancillary data for file descriptor
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);
    
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;  // Pass file descriptor
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    memcpy(CMSG_DATA(cmsg), &m_memfdFd, sizeof(int));
    
    // Send message (ControlMessage + FD in one sendmsg call)
    ssize_t sent = sendmsg(clientFd, &msg, 0);
    return sent >= 0;
}
```

**Code Example: Socket Handshake (Z-Monitor Side)**
```cpp
// In SharedMemoryControlChannel.cpp - Receiving memfd via socket
void SharedMemoryControlChannel::onSocketDataAvailable() {
    // Receive ControlMessage
    ControlMessage message;
    ssize_t received = recv(m_socketFd, &message, sizeof(message), 0);
    
    // Receive file descriptor via SCM_RIGHTS
    int fd = -1;
    if (receiveFileDescriptor(fd)) {
        // Map shared memory using received file descriptor
        m_memfdFd = fd;
        m_ringBufferSize = message.ringBufferSize;
        emit handshakeCompleted(m_memfdFd, m_ringBufferSize);
    }
}

bool SharedMemoryControlChannel::receiveFileDescriptor(int& fd) {
    struct msghdr msg;
    struct iovec iov;
    char buffer[1];
    char cmsg_buffer[CMSG_SPACE(sizeof(int))];
    
    iov.iov_base = buffer;
    iov.iov_len = sizeof(buffer);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg_buffer;
    msg.msg_controllen = sizeof(cmsg_buffer);
    
    ssize_t received = recvmsg(m_socketFd, &msg, 0);
    if (received < 0) return false;
    
    // Extract file descriptor from ancillary data
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_level == SOL_SOCKET && 
        cmsg->cmsg_type == SCM_RIGHTS) {
        memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
        return true;
    }
    return false;
}
```

**Code Example: Mapping Shared Memory (Z-Monitor Side)**
```cpp
// In SharedMemorySensorDataSource.cpp - After receiving memfd
bool SharedMemorySensorDataSource::mapSharedMemory(int fd, size_t size) {
    // Map shared memory using the received file descriptor
    m_mappedMemory = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
    if (m_mappedMemory == MAP_FAILED) {
        qCritical() << "Failed to mmap shared memory:" << strerror(errno);
        return false;
    }
    
    m_mappedSize = size;
    
    // Create ring buffer reader
    m_ringBuffer = std::make_unique<SharedMemoryRingBuffer>(m_mappedMemory, m_mappedSize);
    
    // At this point, socket is no longer needed
    // All data transfer happens through shared memory
    return true;
}
```

### Architecture Pattern: Control Channel + Data Channel

This is a **standard high-performance IPC pattern** used in many systems:

1. **Control Channel (Socket):** Used for setup/teardown, passing metadata, file descriptors, and control messages. Low-frequency operations.

2. **Data Channel (Shared Memory):** Used for high-frequency data transfer. Zero-copy, direct memory access, minimal latency.

**Why This Pattern?**
- **Separation of Concerns:** Control operations (connection setup, errors, shutdown) are separate from data transfer
- **Performance:** Data transfer avoids syscall overhead (no `send()`/`recv()` for every frame)
- **Scalability:** Shared memory supports multiple readers efficiently
- **Latency:** Direct memory access achieves < 16ms latency vs. > 60ms for socket-based transfer

### Performance Comparison

| Approach                  | Latency  | Throughput | CPU Overhead                        |
| ------------------------- | -------- | ---------- | ----------------------------------- |
| **WebSocket + JSON**      | > 60ms   | ~100 KB/s  | High (serialization, network stack) |
| **Unix Socket + Binary**  | ~20-30ms | ~1 MB/s    | Medium (syscalls per frame)         |
| **Shared Memory (memfd)** | < 16ms ✅ | ~10+ MB/s  | Low (direct memory access)          |

**Why Shared Memory Achieves < 16ms Latency:**
1. **Zero-Copy:** Data is written directly to shared memory, no copying between processes
2. **No Syscalls on Hot Path:** After initial `mmap()`, data access is pure memory operations
3. **CPU Cache Friendly:** Shared memory stays in L3 cache, avoiding main memory access
4. **Lock-Free Ring Buffer:** Atomic operations for synchronization, no mutex contention

**WebSocket Overhead Breakdown (Why > 60ms):**
- JSON serialization: ~5-10ms
- Network stack (TCP/IP): ~10-20ms
- Socket I/O syscalls: ~5-10ms per frame
- JSON deserialization: ~5-10ms
- **Total: > 60ms** ❌

**Shared Memory Overhead Breakdown (Why < 16ms):**
- Direct memory write: ~0.1µs
- Atomic index update: ~0.1µs
- Memory read (cache hit): ~0.1µs
- Frame decode: ~200µs
- **Total: < 16ms** ✅

### Security Considerations

**memfd Security:**
1. **File Descriptor Isolation:** `memfd` file descriptors are not accessible via filesystem paths. Only processes with the file descriptor can access the memory.

2. **Permission Model:** When creating `memfd`, you can set file permissions (e.g., `0600` for read/write by owner only). However, the primary security mechanism is **file descriptor passing** - only processes that receive the FD via `SCM_RIGHTS` can access the memory.

3. **Sealing:** `memfd` supports sealing (`F_SEAL_WRITE`, `F_SEAL_SHRINK`) to prevent modifications after initialization:
   ```cpp
   fcntl(memfdFd, F_ADD_SEALS, F_SEAL_WRITE | F_SEAL_SHRINK);
   ```

4. **Automatic Cleanup:** When all file descriptors are closed, the memory is automatically freed, preventing memory leaks.

**Socket Security:**
1. **Unix Domain Socket Permissions:** Socket files have filesystem permissions (e.g., `0666` for all users, `0600` for owner only). In production, use `0600` to restrict access.

2. **SCM_RIGHTS Limitations:** File descriptors passed via `SCM_RIGHTS` inherit the permissions of the original file descriptor. If the `memfd` was created with `0600` permissions, the receiving process will have the same access.

3. **Process Isolation:** Only processes that can connect to the Unix domain socket can receive the file descriptor. This provides a basic access control mechanism.

**Best Practices:**
- Create `memfd` with `0600` permissions (owner read/write only)
- Use Unix domain socket with `0600` permissions
- Validate `magic` number and `version` in ring buffer header before trusting data
- Use CRC32 validation on each frame
- Implement heartbeat mechanism to detect stalled/disconnected writers

### Why Not Just Use Shared Memory Without Socket?

**Question:** If we're using shared memory to avoid socket overhead, why do we need a socket at all?

**Answer:** File descriptors cannot be passed through shared memory. You need an IPC mechanism to exchange the `memfd` file descriptor between processes. Unix domain sockets with `SCM_RIGHTS` are the standard Linux mechanism for this.

**Alternatives Considered:**
1. **Named Shared Memory (`shm_open`):** Requires a known name, filesystem namespace pollution, manual cleanup. `memfd` is cleaner.

2. **Environment Variables:** Cannot pass file descriptors through environment variables.

3. **Process Inheritance:** File descriptors can be inherited by child processes, but Z-Monitor and simulator are separate processes, not parent/child.

4. **File System:** Could write the file descriptor number to a file, but this is insecure and unreliable (FD numbers are process-local).

**Conclusion:** Unix domain socket with `SCM_RIGHTS` is the **only reliable and secure way** to pass file descriptors between unrelated processes on Linux.

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
