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

| Approach | Latency | Throughput | CPU Overhead |
|----------|---------|------------|--------------|
| **WebSocket + JSON** | > 60ms | ~100 KB/s | High (serialization, network stack) |
| **Unix Socket + Binary** | ~20-30ms | ~1 MB/s | Medium (syscalls per frame) |
| **Shared Memory (memfd)** | < 16ms ✅ | ~10+ MB/s | Low (direct memory access) |

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
