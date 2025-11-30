# Sensor Simulator Integration Guide - Step-by-Step

**Document ID:** DESIGN-044  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-29

---

## Overview

This guide provides **step-by-step instructions** for integrating the Z Monitor application with the Sensor Simulator using shared memory transport. This is a **practical implementation guide** that walks you through the entire process from setup to verification.

> **📋 Related Documents:**
> - [Sensor Integration Architecture (37_SENSOR_INTEGRATION.md)](./37_SENSOR_INTEGRATION.md) - **Complete architecture reference** ⭐
> - [Data Caching Strategy (36_DATA_CACHING_STRATEGY.md)](./36_DATA_CACHING_STRATEGY.md) - In-memory caching details
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Threading and latency targets
> - [Low Latency Techniques (42_LOW_LATENCY_TECHNIQUES.md)](./42_LOW_LATENCY_TECHNIQUES.md) - Performance optimization
> - [Sensor Simulator README](../../../sensor-simulator/README.md) - Simulator build and usage

---

## Prerequisites

Before starting integration, ensure you have:

### System Requirements
- **OS:** macOS 12+ or Linux (Ubuntu 20.04+)
- **Qt:** 6.9.2 installed (required for both simulator and z-monitor)
- **CMake:** 3.20+ (build system)
- **Compiler:** Clang 14+ (macOS) or GCC 11+ (Linux)
- **Memory:** 8 GB RAM minimum (for 3-day in-memory cache)
- **Disk:** 500 MB free space (for 7-day local storage)

### Knowledge Requirements
- **C++17** - Move semantics, atomics, smart pointers
- **Qt Signals/Slots** - Inter-object communication
- **Shared Memory IPC** - memfd (Linux) or shm_open (macOS)
- **Unix Domain Sockets** - Socket programming, SCM_RIGHTS
- **Threading** - QThread, lock-free queues, atomic operations

### Documentation Reading
Before proceeding, **read these documents completely:**
1. [37_SENSOR_INTEGRATION.md](./37_SENSOR_INTEGRATION.md) - Understand architecture
2. [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) - Understand caching
3. [sensor-simulator/README.md](../../../sensor-simulator/README.md) - Understand simulator
4. [sensor-simulator/tests/handshake_compatibility.md](../../../sensor-simulator/tests/handshake_compatibility.md) - **Critical for socket handshake**

---

## Integration Workflow Overview

```
┌──────────────────────────────────────────────────────────────┐
│ Phase 1: Environment Setup (30 minutes)                      │
├──────────────────────────────────────────────────────────────┤
│ 1. Install Qt 6.9.2                                          │
│ 2. Build sensor simulator locally                            │
│ 3. Verify simulator runs and creates shared memory           │
└──────────────────────────────────────────────────────────────┘
                          ↓
┌──────────────────────────────────────────────────────────────┐
│ Phase 2: Core Infrastructure (2-4 hours)                     │
├──────────────────────────────────────────────────────────────┤
│ 1. Implement SharedMemoryControlChannel                      │
│ 2. Implement SharedMemoryRingBuffer                          │
│ 3. Test handshake and ring buffer reading                    │
└──────────────────────────────────────────────────────────────┘
                          ↓
┌──────────────────────────────────────────────────────────────┐
│ Phase 3: Sensor Data Source (3-5 hours)                      │
├──────────────────────────────────────────────────────────────┤
│ 1. Implement SharedMemorySensorDataSource                    │
│ 2. Implement frame parsing (Vitals, Waveform, Heartbeat)     │
│ 3. Implement stall detection                                 │
│ 4. Test data flow: simulator → z-monitor                     │
└──────────────────────────────────────────────────────────────┘
                          ↓
┌──────────────────────────────────────────────────────────────┐
│ Phase 4: Application Integration (2-3 hours)                 │
├──────────────────────────────────────────────────────────────┤
│ 1. Implement VitalsCache and WaveformCache                   │
│ 2. Update MonitoringService to use ISensorDataSource         │
│ 3. Wire signals: SensorDataSource → MonitoringService        │
│ 4. Test: Data flows into caches                              │
└──────────────────────────────────────────────────────────────┘
                          ↓
┌──────────────────────────────────────────────────────────────┐
│ Phase 5: UI Integration (2-3 hours)                          │
├──────────────────────────────────────────────────────────────┤
│ 1. Update DashboardController (vitals Q_PROPERTY binding)    │
│ 2. Update WaveformController (waveform data for Canvas)      │
│ 3. Update QML UI (bind to controllers)                       │
│ 4. Test: Live data appears in UI                             │
└──────────────────────────────────────────────────────────────┘
                          ↓
┌──────────────────────────────────────────────────────────────┐
│ Phase 6: Testing & Validation (2-4 hours)                    │
├──────────────────────────────────────────────────────────────┤
│ 1. Unit tests (SharedMemoryControlChannel, RingBuffer)       │
│ 2. Integration tests (E2E with simulator)                    │
│ 3. Latency measurement (verify < 16ms target)                │
│ 4. Stall detection test (kill simulator)                     │
│ 5. Performance profiling (optimize hot paths)                │
└──────────────────────────────────────────────────────────────┘
                          ↓
┌──────────────────────────────────────────────────────────────┐
│ Phase 7: Documentation & Cleanup (1-2 hours)                 │
├──────────────────────────────────────────────────────────────┤
│ 1. Update architecture diagrams                              │
│ 2. Document troubleshooting steps                            │
│ 3. Create integration checklist                              │
│ 4. Update ZTODO.md with completion status                    │
└──────────────────────────────────────────────────────────────┘

Total Estimated Time: 12-21 hours (1.5 - 2.5 working days)
```

---

## Phase 1: Environment Setup

### Step 1.1: Verify Qt Installation

**Objective:** Ensure Qt 6.9.2 is installed and accessible.

```bash
# Check Qt installation
ls -la /Users/dustinwind/Qt/6.9.2/macos  # macOS
# or
ls -la ~/Qt/6.9.2/gcc_64  # Linux

# Verify qmake
/Users/dustinwind/Qt/6.9.2/macos/bin/qmake --version
# Expected: "QMake version 3.1, Using Qt version 6.9.2"

# Set environment variable (add to ~/.zshrc or ~/.bashrc)
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"
```

**Verification:**
- [ ] Qt 6.9.2 directory exists
- [ ] `qmake --version` shows 6.9.2
- [ ] `CMAKE_PREFIX_PATH` set correctly

---

### Step 1.2: Build Sensor Simulator

**Objective:** Build and run the sensor simulator to verify it works.

```bash
cd project-dashboard/sensor-simulator

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos"

# Build
cmake --build . --config Release -j4

# Run simulator
./sensor_simulator
```

**Expected Output:**
```
Simulator: Initializing...
Simulator: Shared memory initialized (size: 8392704 bytes, frames: 2048, frame size: 4096 bytes)
ControlServer: Listening on /tmp/z-monitor-sensor.sock
Simulator: Started successfully
```

**UI Verification:**
- [ ] Simulator window opens (title: "Telemetry Simulator")
- [ ] Vitals display shows: Heart Rate, SpO2, Respiration Rate updating
- [ ] ECG waveform displays with PQRST complex pattern
- [ ] Log console shows telemetry frames being written

**Troubleshooting:**
- **Error: "Qt not found"** → Set `CMAKE_PREFIX_PATH` correctly
- **Error: "memfd_create not found"** (macOS) → This is expected, simulator should use `shm_open` fallback
- **Simulator crashes** → Check console for Qt dependency issues

---

### Step 1.3: Verify Shared Memory Creation

**Objective:** Confirm simulator creates shared memory and control socket.

**Linux:**
```bash
# Check shared memory (should exist while simulator running)
ls -la /dev/shm/zmonitor-sim-ring
# Expected: -rw------- 1 user user 8392704 Nov 29 12:34 /dev/shm/zmonitor-sim-ring

# Check socket
ls -la /tmp/z-monitor-sensor.sock
# Expected: srwxr-xr-x 1 user user 0 Nov 29 12:34 /tmp/z-monitor-sensor.sock

# Dump first 64 bytes of shared memory (ring buffer header)
xxd -l 64 /dev/shm/zmonitor-sim-ring
# Expected: First 4 bytes should be 53 4d 52 42 (magic 'SMRB')
```

**macOS:**
```bash
# Check shared memory (may be in /tmp/ or accessible via simulator process)
lsof -c sensor_simulator | grep shm
# Expected: Shows memory-mapped file descriptor

# Check socket
ls -la /tmp/z-monitor-sensor.sock
# Expected: srwxr-xr-x 1 user user 0 Nov 29 12:34 /tmp/z-monitor-sensor.sock

# Test socket connection
echo "test" | nc -U /tmp/z-monitor-sensor.sock
# Expected: Connection succeeds (may get binary data back)
```

**Verification:**
- [ ] Shared memory exists (Linux: `/dev/shm/`, macOS: via lsof)
- [ ] Socket exists at `/tmp/z-monitor-sensor.sock`
- [ ] Socket is connectable (nc succeeds)
- [ ] Magic number in shared memory is `0x534D5242` ('SMRB')

---

## Phase 2: Core Infrastructure Implementation

### Step 2.1: Implement SharedMemoryControlChannel

**Objective:** Handle Unix socket handshake and receive file descriptor.

**File:** `z-monitor/src/infrastructure/sensors/SharedMemoryControlChannel.h`

```cpp
/**
 * @class SharedMemoryControlChannel
 * @brief Handles Unix domain socket connection to simulator and memfd exchange.
 * 
 * This class is responsible for:
 * - Connecting to simulator's Unix domain control socket
 * - Receiving memfd file descriptor via SCM_RIGHTS
 * - Emitting signals when handshake completes or fails
 * 
 * After successful handshake, the socket is no longer needed (disconnected).
 * All data transfer happens through shared memory.
 */
class SharedMemoryControlChannel : public QObject {
    Q_OBJECT
public:
    explicit SharedMemoryControlChannel(QObject* parent = nullptr);
    ~SharedMemoryControlChannel();
    
    /**
     * @brief Connect to simulator's control socket.
     * @param socketPath Path to Unix domain socket (e.g., "/tmp/z-monitor-sensor.sock")
     * @return true if connection initiated, false on immediate failure
     * 
     * This is an asynchronous operation. Listen for handshakeCompleted or handshakeFailed signals.
     */
    bool connect(const QString& socketPath);
    
    /**
     * @brief Disconnect from control socket.
     * 
     * Should be called after handshake completes (socket no longer needed).
     */
    void disconnect();
    
signals:
    /**
     * @brief Emitted when memfd file descriptor is successfully received.
     * @param memfdFd File descriptor for shared memory (to be mmap'd)
     * @param ringBufferSize Total size of shared memory region
     */
    void handshakeCompleted(int memfdFd, size_t ringBufferSize);
    
    /**
     * @brief Emitted when handshake fails.
     * @param error Error description
     */
    void handshakeFailed(const QString& error);
    
private slots:
    void onSocketConnected();
    void onSocketDataAvailable();
    void onSocketError(QLocalSocket::LocalSocketError error);
    
private:
    bool receiveFileDescriptor(int& fd, size_t& ringBufferSize);
    
    QLocalSocket* m_socket;
    int m_memfdFd;
    size_t m_ringBufferSize;
};
```

**Implementation:** `z-monitor/src/infrastructure/sensors/SharedMemoryControlChannel.cpp`

```cpp
#include "SharedMemoryControlChannel.h"
#include <QLocalSocket>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

// Control message structure (must match simulator)
struct ControlMessage {
    uint8_t type;            // 0x01 = Handshake
    uint8_t reserved[3];
    uint32_t version;        // Protocol version
    uint64_t ringBufferSize;
    char socketPath[108];    // For diagnostics
};

SharedMemoryControlChannel::SharedMemoryControlChannel(QObject* parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this))
    , m_memfdFd(-1)
    , m_ringBufferSize(0)
{
    connect(m_socket, &QLocalSocket::connected, this, &SharedMemoryControlChannel::onSocketConnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &SharedMemoryControlChannel::onSocketDataAvailable);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &SharedMemoryControlChannel::onSocketError);
}

SharedMemoryControlChannel::~SharedMemoryControlChannel() {
    disconnect();
}

bool SharedMemoryControlChannel::connect(const QString& socketPath) {
    qInfo() << "SharedMemoryControlChannel: Connecting to" << socketPath;
    m_socket->connectToServer(socketPath);
    return true;  // Async operation, wait for signals
}

void SharedMemoryControlChannel::disconnect() {
    if (m_socket->state() == QLocalSocket::ConnectedState) {
        m_socket->disconnectFromServer();
    }
}

void SharedMemoryControlChannel::onSocketConnected() {
    qInfo() << "SharedMemoryControlChannel: Socket connected, waiting for file descriptor";
}

void SharedMemoryControlChannel::onSocketDataAvailable() {
    // **CRITICAL:** Must use recvmsg() to get both ControlMessage and FD together
    // DO NOT use QLocalSocket::read() - it won't receive ancillary data (SCM_RIGHTS)
    
    int fd = -1;
    size_t ringBufferSize = 0;
    
    if (receiveFileDescriptor(fd, ringBufferSize)) {
        qInfo() << "SharedMemoryControlChannel: Received file descriptor" << fd
                << "ring buffer size:" << ringBufferSize;
        
        m_memfdFd = fd;
        m_ringBufferSize = ringBufferSize;
        
        // Handshake complete - disconnect socket (no longer needed)
        disconnect();
        
        emit handshakeCompleted(m_memfdFd, m_ringBufferSize);
    } else {
        emit handshakeFailed("Failed to receive file descriptor via SCM_RIGHTS");
    }
}

bool SharedMemoryControlChannel::receiveFileDescriptor(int& fd, size_t& ringBufferSize) {
    // Get raw socket file descriptor (Qt abstraction doesn't expose recvmsg)
    int socketFd = m_socket->socketDescriptor();
    if (socketFd < 0) {
        qCritical() << "Invalid socket file descriptor";
        return false;
    }
    
    // Prepare to receive ControlMessage structure + file descriptor
    struct msghdr msg = {0};
    struct iovec iov[1];
    ControlMessage message;
    char cmsg_buffer[CMSG_SPACE(sizeof(int))];
    
    iov[0].iov_base = &message;
    iov[0].iov_len = sizeof(message);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg_buffer;
    msg.msg_controllen = sizeof(cmsg_buffer);
    
    // **CRITICAL:** Use recvmsg() to get both data and ancillary data (SCM_RIGHTS)
    ssize_t received = ::recvmsg(socketFd, &msg, 0);
    if (received < 0) {
        qCritical() << "recvmsg failed:" << strerror(errno);
        return false;
    }
    
    if (received != sizeof(message)) {
        qCritical() << "Received incomplete ControlMessage:" << received << "bytes (expected" << sizeof(message) << ")";
        return false;
    }
    
    // Verify message type
    if (message.type != 0x01) {
        qCritical() << "Invalid message type:" << message.type << "(expected 0x01 for handshake)";
        return false;
    }
    
    // Extract file descriptor from ancillary data
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg || cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS) {
        qCritical() << "No SCM_RIGHTS ancillary data found";
        return false;
    }
    
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
    ringBufferSize = message.ringBufferSize;
    
    qInfo() << "Received file descriptor:" << fd << "ring buffer size:" << ringBufferSize;
    return true;
}

void SharedMemoryControlChannel::onSocketError(QLocalSocket::LocalSocketError error) {
    QString errorMsg = m_socket->errorString();
    qCritical() << "SharedMemoryControlChannel: Socket error:" << error << errorMsg;
    emit handshakeFailed(errorMsg);
}
```

**Testing:**

```cpp
// Unit test
TEST(SharedMemoryControlChannel, ConnectsToSimulator) {
    // Ensure simulator is running
    SharedMemoryControlChannel channel;
    QSignalSpy completedSpy(&channel, &SharedMemoryControlChannel::handshakeCompleted);
    QSignalSpy failedSpy(&channel, &SharedMemoryControlChannel::handshakeFailed);
    
    ASSERT_TRUE(channel.connect("/tmp/z-monitor-sensor.sock"));
    
    // Wait for handshake (should complete within 1 second)
    QVERIFY(completedSpy.wait(1000));
    QCOMPARE(failedSpy.count(), 0);
    
    // Verify file descriptor received
    QList<QVariant> args = completedSpy.takeFirst();
    int memfdFd = args.at(0).toInt();
    size_t ringBufferSize = args.at(1).toULongLong();
    
    EXPECT_GT(memfdFd, 0);  // Valid file descriptor
    EXPECT_GT(ringBufferSize, 0);  // Valid size
}
```

**Verification:**
- [ ] Class compiles without errors
- [ ] Unit test passes with simulator running
- [ ] `handshakeCompleted` signal emitted with valid FD
- [ ] Socket disconnects after handshake

---

### Step 2.2: Implement SharedMemoryRingBuffer

**Objective:** Wrap shared memory region and provide safe frame reading.

**File:** `z-monitor/src/infrastructure/sensors/SharedMemoryRingBuffer.h`

```cpp
/**
 * @struct RingBufferHeader
 * @brief Ring buffer header structure (must match simulator).
 */
struct RingBufferHeader {
    uint32_t magic;              // 0x534D5242 ('SMRB')
    uint16_t version;            // 1
    uint16_t reserved1;
    uint32_t frameSizeBytes;     // 4096
    uint32_t frameCount;         // 2048
    std::atomic<uint64_t> writeIndex;
    std::atomic<uint64_t> heartbeatTimestamp;  // ms since epoch
    uint32_t crc32;
    uint32_t reserved2[11];
};

/**
 * @struct SensorFrame
 * @brief Single sensor frame structure (must match simulator).
 */
struct SensorFrame {
    uint8_t type;                // 0x01 = Vitals, 0x02 = Waveform, 0x03 = Heartbeat
    uint8_t reserved[3];
    uint64_t timestampNs;        // Nanoseconds since epoch
    uint32_t sequenceNumber;
    uint32_t dataSize;           // Size of JSON payload
    uint8_t data[4064];          // JSON payload (4096 - 32 header bytes)
    uint32_t crc32;
};

/**
 * @class SharedMemoryRingBuffer
 * @brief Reader for shared memory ring buffer.
 * 
 * This class provides safe access to the ring buffer:
 * - Validates header structure
 * - Reads frames with CRC validation
 * - Tracks read index
 */
class SharedMemoryRingBuffer {
public:
    /**
     * @brief Constructor.
     * @param mappedMemory Pointer to mmap'd shared memory
     * @param mappedSize Size of mapped region
     */
    SharedMemoryRingBuffer(void* mappedMemory, size_t mappedSize);
    
    /**
     * @brief Validate ring buffer header.
     * @return true if header is valid
     */
    bool validateHeader() const;
    
    /**
     * @brief Get current write index (atomic read).
     * @return Current write index
     */
    uint64_t getWriteIndex() const;
    
    /**
     * @brief Get last heartbeat timestamp (atomic read).
     * @return Timestamp in milliseconds since epoch
     */
    uint64_t getHeartbeatTimestamp() const;
    
    /**
     * @brief Read frame at specified index.
     * @param readIndex Index to read
     * @param outFrame Output frame (if successful)
     * @return true if frame valid (CRC passed)
     */
    bool readFrame(uint64_t readIndex, SensorFrame& outFrame) const;
    
    /**
     * @brief Get frame count.
     */
    uint32_t getFrameCount() const;
    
    /**
     * @brief Get frame size.
     */
    uint32_t getFrameSize() const;
    
private:
    uint32_t calculateCRC32(const void* data, size_t size) const;
    
    RingBufferHeader* m_header;
    uint8_t* m_slots;
    size_t m_mappedSize;
};
```

**Implementation:** See architecture document for complete implementation details.

**Verification:**
- [ ] Header validation passes with simulator ring buffer
- [ ] Frame reading works correctly
- [ ] CRC validation detects corrupted frames
- [ ] Atomic operations work correctly

---

## Phase 3: Sensor Data Source Implementation

### Step 3.1: Implement SharedMemorySensorDataSource

**Objective:** Poll ring buffer and emit Qt signals for vitals/waveforms.

**File:** `z-monitor/src/infrastructure/sensors/SharedMemorySensorDataSource.h`

```cpp
/**
 * @class SharedMemorySensorDataSource
 * @brief ISensorDataSource implementation using shared memory ring buffer.
 * 
 * This class:
 * - Connects to simulator via Unix socket (handshake only)
 * - Maps shared memory ring buffer
 * - Polls ring buffer for new frames (< 50µs interval)
 * - Parses frames and emits Qt signals
 * - Detects stalled writer (no heartbeat > 250ms)
 */
class SharedMemorySensorDataSource : public QObject, public ISensorDataSource {
    Q_OBJECT
public:
    explicit SharedMemorySensorDataSource(const QString& socketPath,
                                          const QString& ringBufferName,
                                          QObject* parent = nullptr);
    ~SharedMemorySensorDataSource() override;
    
    // ISensorDataSource interface
    bool start() override;
    void stop() override;
    bool isActive() const override;
    DataSourceInfo getInfo() const override;
    
signals:
    void vitalsReceived(const VitalRecord& vital) override;
    void waveformSampleReady(const WaveformSample& sample) override;
    void connectionStatusChanged(bool connected, const QString& sensorType) override;
    void sensorError(const SensorError& error) override;
    
private slots:
    void onHandshakeCompleted(int memfdFd, size_t ringBufferSize);
    void onHandshakeFailed(const QString& error);
    void pollRingBuffer();
    void checkHeartbeat();
    
private:
    bool mapSharedMemory(int fd, size_t size);
    void parseFrame(const SensorFrame& frame);
    void parseVitalsFrame(const SensorFrame& frame);
    void parseWaveformFrame(const SensorFrame& frame);
    void parseHeartbeatFrame(const SensorFrame& frame);
    
    QString m_socketPath;
    QString m_ringBufferName;
    SharedMemoryControlChannel* m_controlChannel;
    SharedMemoryRingBuffer* m_ringBuffer;
    
    void* m_mappedMemory;
    size_t m_mappedSize;
    
    QTimer* m_pollTimer;           // Polling timer (< 50µs)
    QTimer* m_heartbeatTimer;      // Heartbeat watchdog (check every 100ms)
    
    uint64_t m_readIndex;          // Current read index
    uint64_t m_lastHeartbeat;      // Last heartbeat timestamp
    bool m_active;
};
```

**Implementation:** See full implementation in architecture document.

**Verification:**
- [ ] Connects to simulator successfully
- [ ] Maps shared memory correctly
- [ ] Polls at < 50µs interval
- [ ] Emits `vitalsReceived` signal for Vitals frames
- [ ] Emits `waveformSampleReady` signal for Waveform frames
- [ ] Detects stalled writer within 300ms

---

## Phase 4: Application Integration

### Step 4.1: Update MonitoringService

**Objective:** Wire SharedMemorySensorDataSource to MonitoringService.

```cpp
// In MonitoringService constructor (dependency injection)
MonitoringService::MonitoringService(ISensorDataSource* sensorDataSource,
                                     VitalsCache* vitalsCache,
                                     WaveformCache* waveformCache,
                                     AlarmManager* alarmManager,
                                     QObject* parent)
    : QObject(parent)
    , m_sensorDataSource(sensorDataSource)
    , m_vitalsCache(vitalsCache)
    , m_waveformCache(waveformCache)
    , m_alarmManager(alarmManager)
{
    // Connect signals from sensor data source
    connect(m_sensorDataSource, &ISensorDataSource::vitalsReceived,
            this, &MonitoringService::onVitalsReceived);
    connect(m_sensorDataSource, &ISensorDataSource::waveformSampleReady,
            this, &MonitoringService::onWaveformSample);
    connect(m_sensorDataSource, &ISensorDataSource::sensorError,
            this, &MonitoringService::onSensorError);
}

void MonitoringService::start() {
    if (!m_sensorDataSource->start()) {
        qCritical() << "MonitoringService: Failed to start sensor data source";
        return;
    }
    qInfo() << "MonitoringService: Started successfully";
}

void MonitoringService::onVitalsReceived(const VitalRecord& vital) {
    // CRITICAL PATH - Keep this fast (< 5ms)
    
    // 1. Append to in-memory cache (non-blocking)
    m_vitalsCache->append(vital);
    
    // 2. Evaluate alarm rules
    m_alarmManager->processVitalSigns(vital);
    
    // 3. Emit signal to UI controllers
    emit vitalsUpdated(vital);
    
    // 4. Enqueue to telemetry batch (background thread handles persistence)
    enqueueTelemetry(vital);
}

void MonitoringService::onWaveformSample(const WaveformSample& sample) {
    // Append to waveform cache (display-only, circular buffer)
    m_waveformCache->append(sample);
    
    // Emit signal to WaveformController
    emit waveformUpdated(sample);
}
```

**Verification:**
- [ ] MonitoringService starts sensor data source
- [ ] Vitals flow into VitalsCache
- [ ] Waveforms flow into WaveformCache
- [ ] Alarm detection works with cached data
- [ ] UI controllers receive signals

---

## Phase 5: UI Integration

### Step 5.1: Update DashboardController

```cpp
// In DashboardController
void DashboardController::initialize() {
    // Connect to MonitoringService
    connect(m_monitoringService, &MonitoringService::vitalsUpdated,
            this, &DashboardController::onVitalsUpdated);
}

void DashboardController::onVitalsUpdated(const VitalRecord& vital) {
    // Update Q_PROPERTY values (QML binds to these)
    setHeartRate(vital.heartRate);
    setSpo2(vital.spo2);
    setRespirationRate(vital.respirationRate);
    setLastUpdate(QDateTime::currentDateTime());
    
    // Trigger QML UI update
    emit vitalsChanged();
}
```

**QML Binding:**
```qml
// In MonitorView.qml
VitalTile {
    label: "HEART RATE"
    value: dashboardController.heartRate.toString()
    unit: "BPM"
    color: "#10b981"
}
```

**Verification:**
- [ ] Vitals display in QML UI
- [ ] Values update in real-time (60 Hz)
- [ ] No QML errors

---

## Phase 6: Testing & Validation

### Step 6.1: End-to-End Integration Test

**Test Script:**
```bash
#!/bin/bash
# integration_test.sh - E2E integration test

# Start simulator
cd project-dashboard/sensor-simulator/build
./sensor_simulator &
SIM_PID=$!
sleep 2  # Wait for simulator to initialize

# Start z-monitor
cd ../../z-monitor/build
./z-monitor &
ZMON_PID=$!
sleep 5  # Wait for z-monitor to connect

# Verify data flow (check logs)
echo "Checking z-monitor logs..."
if grep -q "SharedMemorySensorDataSource: Started successfully" z-monitor.log; then
    echo "✅ Z-Monitor connected to simulator"
else
    echo "❌ Z-Monitor failed to connect"
    kill $SIM_PID $ZMON_PID
    exit 1
fi

# Run for 30 seconds, capture metrics
sleep 30

# Check latency (should be < 16ms)
# (Requires instrumentation in code)

# Kill processes
kill $SIM_PID $ZMON_PID
echo "✅ Integration test completed"
```

**Verification:**
- [ ] Test passes without errors
- [ ] Data flows from simulator to z-monitor
- [ ] UI displays live data
- [ ] Latency < 16ms measured

---

## Troubleshooting

See [37_SENSOR_INTEGRATION.md - Troubleshooting Section](./37_SENSOR_INTEGRATION.md#troubleshooting-and-diagnostics) for complete troubleshooting guide.

**Quick Checks:**
- Socket path matches on both sides
- Simulator is running before z-monitor starts
- Shared memory exists (`/dev/shm/` on Linux)
- No firewall blocking Unix sockets
- File descriptor passed correctly (use `recvmsg()`)

---

## Success Criteria

Integration is complete when:

- [ ] Simulator builds and runs successfully
- [ ] Z-Monitor connects to simulator via Unix socket
- [ ] File descriptor received via `SCM_RIGHTS`
- [ ] Shared memory mapped successfully
- [ ] Vitals data flows at 60 Hz
- [ ] Waveform data flows at 250 Hz
- [ ] UI displays live data (vitals + waveforms)
- [ ] Connection status indicator works
- [ ] Stall detection works (kill simulator → detected within 300ms)
- [ ] Latency < 16ms measured
- [ ] All unit tests pass
- [ ] E2E integration test passes
- [ ] Documentation updated

---

## Next Steps

After completing integration:

1. **Performance Optimization:** Profile and optimize hot paths
2. **Error Handling:** Add robust error recovery
3. **Logging:** Add structured logging for diagnostics
4. **Monitoring:** Add metrics collection (latency, throughput, frame drops)
5. **Production Hardening:** Security review, stress testing, edge cases

---

**Document Status:** Complete  
**Last Updated:** 2025-11-29  
**Author:** GitHub Copilot  
**Reviewers:** [Pending]
