# Task: Implement SharedMemorySensorDataSource in Z-Monitor

## Context

**Documentation:**
- Architecture: [37_SENSOR_INTEGRATION.md](../doc/z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md) - Complete reference
- Implementation Guide: [44_SIMULATOR_INTEGRATION_GUIDE.md](../doc/z-monitor/architecture_and_design/44_SIMULATOR_INTEGRATION_GUIDE.md) - Phases 2-3
- Socket Handshake: [handshake_compatibility.md](../sensor-simulator/tests/handshake_compatibility.md) - **CRITICAL**

**Previous Work:**
- ✅ Simulator built and running locally
- ✅ ISensorDataSource interface defined
- ⏳ Need SharedMemorySensorDataSource implementation

**Dependencies:**
- Simulator running and writing to shared memory
- `ISensorDataSource` interface exists at `z-monitor/src/domain/interfaces/ISensorDataSource.h`
- Unix domain socket support (POSIX)
- CRC32 validation library (Qt `qChecksum` or standalone)
- JSON parsing (Qt's `QJsonDocument`)

---

## Objective

Implement `SharedMemorySensorDataSource` class that implements the `ISensorDataSource` interface. This class connects to the simulator's Unix control socket (`/tmp/z-monitor-sensor.sock`), receives the memfd file descriptor via `SCM_RIGHTS`, maps the shared-memory ring buffer, and reads sensor data frames (60 Hz vitals + 250 Hz waveforms). Parse frames (validate CRC32, deserialize JSON payloads), convert to `VitalRecord` and `WaveformSample` objects, and emit Qt signals for consumption by `MonitoringService` and UI controllers.

**Critical Success Factor:** Achieve < 16ms end-to-end latency from simulator write to signal emission.

---

## Architecture Overview

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

**Data Transfer (Continuous, via Shared Memory):**
```
Sensor Simulator (local process)
  ↓ (memfd write @ 60 Hz / 250 Hz - DIRECT MEMORY WRITE)
SharedMemoryRingBuffer::writeFrame()
  ↓ (atomic writeIndex - NO SOCKET I/O)
SharedMemorySensorDataSource::pollFrames()
  ↓ (binary decode - NO SOCKET I/O)
emit ISensorDataSource::vitalsReceived(VitalRecord)
emit ISensorDataSource::waveformSampleReady(WaveformSample)
  ↓ (Qt Signal - < 16ms from write)
MonitoringService::onVitalsReceived(VitalRecord)
```

**Key Point:** After handshake, socket is disconnected. **All data flows through shared memory only**.

---

## Component Breakdown

### Component 1: SharedMemoryControlChannel

**Purpose:** Handle Unix socket handshake and file descriptor exchange.

**File:** `z-monitor/src/infrastructure/sensors/SharedMemoryControlChannel.h`

**Critical Implementation Detail:**
```cpp
bool SharedMemoryControlChannel::receiveFileDescriptor(int& fd, size_t& ringBufferSize) {
    // **CRITICAL:** Must use recvmsg() NOT recv() for first receive
    // recv() will consume data but lose ancillary data (file descriptor)
    
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
    
    // Use recvmsg() to get both data and SCM_RIGHTS
    ssize_t received = ::recvmsg(socketFd, &msg, 0);
    
    // Extract file descriptor from ancillary data
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
        ringBufferSize = message.ringBufferSize;
        return true;
    }
    return false;
}
```

**See full implementation in 44_SIMULATOR_INTEGRATION_GUIDE.md Phase 2.1**

---

### Component 2: SharedMemoryRingBuffer

**Purpose:** Wrap shared memory region and provide safe frame reading.

**File:** `z-monitor/src/infrastructure/sensors/SharedMemoryRingBuffer.h`

**Data Structures:**
```cpp
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

struct SensorFrame {
    uint8_t type;                // 0x01 = Vitals, 0x02 = Waveform, 0x03 = Heartbeat
    uint8_t reserved[3];
    uint64_t timestampNs;        // Nanoseconds since epoch
    uint32_t sequenceNumber;
    uint32_t dataSize;           // Size of JSON payload
    uint8_t data[4064];          // JSON payload
    uint32_t crc32;
};
```

**Key Methods:**
```cpp
class SharedMemoryRingBuffer {
public:
    SharedMemoryRingBuffer(void* mappedMemory, size_t mappedSize);
    
    bool validateHeader() const;  // Check magic=0x534D5242, version=1
    uint64_t getWriteIndex() const;  // Atomic read with acquire semantics
    uint64_t getHeartbeatTimestamp() const;  // Atomic read
    bool readFrame(uint64_t readIndex, SensorFrame& outFrame) const;  // Copy + CRC validate
    
    uint32_t getFrameCount() const { return m_header->frameCount; }
    uint32_t getFrameSize() const { return m_header->frameSizeBytes; }
    
private:
    uint32_t calculateCRC32(const void* data, size_t size) const;
    
    RingBufferHeader* m_header;
    uint8_t* m_slots;
    size_t m_mappedSize;
};
```

---

### Component 3: SharedMemorySensorDataSource

**Purpose:** Poll ring buffer, parse frames, emit Qt signals.

**File:** `z-monitor/src/infrastructure/sensors/SharedMemorySensorDataSource.h`

**Class Definition:**
```cpp
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

---

## Implementation Steps

### Step 1: Implement SharedMemoryControlChannel

**Files to Create:**
- `z-monitor/src/infrastructure/sensors/SharedMemoryControlChannel.h`
- `z-monitor/src/infrastructure/sensors/SharedMemoryControlChannel.cpp`

**Implementation:**
- See complete code example in `44_SIMULATOR_INTEGRATION_GUIDE.md` Phase 2.1
- **CRITICAL:** Use `recvmsg()` NOT `recv()` to receive file descriptor
- Extract FD from `SCM_RIGHTS` ancillary data

**Test:**
```cpp
TEST(SharedMemoryControlChannel, ConnectsToSimulator) {
    SharedMemoryControlChannel channel;
    QSignalSpy completedSpy(&channel, &SharedMemoryControlChannel::handshakeCompleted);
    
    ASSERT_TRUE(channel.connect("/tmp/z-monitor-sensor.sock"));
    QVERIFY(completedSpy.wait(1000));
    
    QList<QVariant> args = completedSpy.takeFirst();
    int memfdFd = args.at(0).toInt();
    EXPECT_GT(memfdFd, 0);  // Valid file descriptor
}
```

---

### Step 2: Implement SharedMemoryRingBuffer

**Files to Create:**
- `z-monitor/src/infrastructure/sensors/SharedMemoryRingBuffer.h`
- `z-monitor/src/infrastructure/sensors/SharedMemoryRingBuffer.cpp`

**Key Implementation Points:**
```cpp
bool SharedMemoryRingBuffer::validateHeader() const {
    if (m_header->magic != 0x534D5242) {  // 'SMRB'
        qCritical() << "Invalid magic number:" << hex << m_header->magic;
        return false;
    }
    if (m_header->version != 1) {
        qCritical() << "Unsupported version:" << m_header->version;
        return false;
    }
    return true;
}

uint64_t SharedMemoryRingBuffer::getWriteIndex() const {
    return m_header->writeIndex.load(std::memory_order_acquire);
}

bool SharedMemoryRingBuffer::readFrame(uint64_t readIndex, SensorFrame& outFrame) const {
    uint32_t slot = readIndex % m_header->frameCount;
    uint8_t* framePtr = m_slots + (slot * m_header->frameSizeBytes);
    
    memcpy(&outFrame, framePtr, sizeof(SensorFrame));
    
    // Validate CRC32
    uint32_t expectedCRC = outFrame.crc32;
    outFrame.crc32 = 0;  // Zero out before calculating
    uint32_t actualCRC = calculateCRC32(&outFrame, sizeof(SensorFrame) - 4);
    outFrame.crc32 = expectedCRC;
    
    if (actualCRC != expectedCRC) {
        qWarning() << "CRC mismatch: expected" << hex << expectedCRC << "got" << actualCRC;
        return false;
    }
    return true;
}
```

---

### Step 3: Implement SharedMemorySensorDataSource

**Files to Create:**
- `z-monitor/src/infrastructure/sensors/SharedMemorySensorDataSource.h`
- `z-monitor/src/infrastructure/sensors/SharedMemorySensorDataSource.cpp`

**start() Method:**
```cpp
bool SharedMemorySensorDataSource::start() {
    if (m_active) {
        qWarning() << "SharedMemorySensorDataSource: Already active";
        return true;
    }
    
    // Connect to control socket and wait for handshake
    m_controlChannel = new SharedMemoryControlChannel(this);
    connect(m_controlChannel, &SharedMemoryControlChannel::handshakeCompleted,
            this, &SharedMemorySensorDataSource::onHandshakeCompleted);
    connect(m_controlChannel, &SharedMemoryControlChannel::handshakeFailed,
            this, &SharedMemorySensorDataSource::onHandshakeFailed);
    
    if (!m_controlChannel->connect(m_socketPath)) {
        qCritical() << "Failed to connect to control socket:" << m_socketPath;
        return false;
    }
    
    qInfo() << "SharedMemorySensorDataSource: Waiting for handshake...";
    return true;  // Async - wait for handshakeCompleted signal
}
```

**onHandshakeCompleted() Slot:**
```cpp
void SharedMemorySensorDataSource::onHandshakeCompleted(int memfdFd, size_t ringBufferSize) {
    qInfo() << "Handshake completed, mapping shared memory...";
    
    // Map shared memory
    if (!mapSharedMemory(memfdFd, ringBufferSize)) {
        emit sensorError(SensorError("Failed to map shared memory"));
        return;
    }
    
    // Create ring buffer wrapper
    m_ringBuffer = new SharedMemoryRingBuffer(m_mappedMemory, m_mappedSize);
    if (!m_ringBuffer->validateHeader()) {
        emit sensorError(SensorError("Invalid ring buffer header"));
        return;
    }
    
    // Start polling timer (< 50µs interval)
    m_pollTimer = new QTimer(this);
    m_pollTimer->setTimerType(Qt::PreciseTimer);
    m_pollTimer->setInterval(0);  // Poll as fast as possible
    connect(m_pollTimer, &QTimer::timeout, this, &SharedMemorySensorDataSource::pollRingBuffer);
    m_pollTimer->start();
    
    // Start heartbeat watchdog (check every 100ms)
    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(100);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &SharedMemorySensorDataSource::checkHeartbeat);
    m_heartbeatTimer->start();
    
    m_active = true;
    m_readIndex = 0;
    m_lastHeartbeat = m_ringBuffer->getHeartbeatTimestamp();
    
    emit connectionStatusChanged(true, "Shared Memory Simulator");
    qInfo() << "SharedMemorySensorDataSource: Started successfully";
}
```

**pollRingBuffer() Slot:**
```cpp
void SharedMemorySensorDataSource::pollRingBuffer() {
    if (!m_active || !m_ringBuffer) {
        return;
    }
    
    uint64_t writeIndex = m_ringBuffer->getWriteIndex();
    
    // Process all available frames
    while (m_readIndex < writeIndex) {
        SensorFrame frame;
        if (m_ringBuffer->readFrame(m_readIndex, frame)) {
            parseFrame(frame);
        } else {
            qWarning() << "Failed to read frame at index" << m_readIndex;
        }
        m_readIndex++;
    }
}
```

**parseFrame() Method:**
```cpp
void SharedMemorySensorDataSource::parseFrame(const SensorFrame& frame) {
    switch (frame.type) {
        case 0x01:  // Vitals
            parseVitalsFrame(frame);
            break;
        case 0x02:  // Waveform
            parseWaveformFrame(frame);
            break;
        case 0x03:  // Heartbeat
            parseHeartbeatFrame(frame);
            break;
        default:
            qWarning() << "Unknown frame type:" << hex << frame.type;
    }
}

void SharedMemorySensorDataSource::parseVitalsFrame(const SensorFrame& frame) {
    // Parse JSON payload
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray((char*)frame.data, frame.dataSize));
    if (doc.isNull()) {
        qWarning() << "Failed to parse vitals JSON";
        return;
    }
    
    QJsonObject obj = doc.object();
    VitalRecord vital;
    vital.heartRate = obj["hr"].toInt();
    vital.spo2 = obj["spo2"].toInt();
    vital.respirationRate = obj["rr"].toInt();
    vital.timestamp = QDateTime::fromMSecsSinceEpoch(frame.timestampNs / 1000000);
    
    emit vitalsReceived(vital);
}
```

**checkHeartbeat() Slot:**
```cpp
void SharedMemorySensorDataSource::checkHeartbeat() {
    if (!m_active || !m_ringBuffer) {
        return;
    }
    
    uint64_t currentHeartbeat = m_ringBuffer->getHeartbeatTimestamp();
    uint64_t timeSinceLastHeartbeat = currentHeartbeat - m_lastHeartbeat;
    
    if (timeSinceLastHeartbeat > 250) {  // 250ms threshold
        qWarning() << "Writer stalled: no heartbeat for" << timeSinceLastHeartbeat << "ms";
        emit sensorError(SensorError("Writer stalled"));
        m_active = false;
        m_pollTimer->stop();
        emit connectionStatusChanged(false, "Shared Memory Simulator");
    }
    
    m_lastHeartbeat = currentHeartbeat;
}
```

---

## Troubleshooting

### Handshake Fails - "Failed to receive file descriptor"

**Root Cause:** Using `recv()` instead of `recvmsg()` for first receive.

**Solution:** Update `SharedMemoryControlChannel::receiveFileDescriptor()` to use `recvmsg()`.

**Verify:**
```bash
# Check socket exists
ls -la /tmp/z-monitor-sensor.sock

# Test connection
echo "test" | nc -U /tmp/z-monitor-sensor.sock
```

---

### No Data Received

**Root Cause:** Ring buffer structure mismatch or CRC validation failing.

**Solution:**
1. Verify magic number: `0x534D5242`
2. Verify version: `1`
3. Add debug logging to frame parsing
4. Check CRC32 algorithm matches simulator

---

### High Latency > 16ms

**Root Cause:** Polling interval too slow or frame processing overhead.

**Solution:**
1. Set `QTimer::setTimerType(Qt::PreciseTimer)`
2. Profile frame parsing (should be < 200µs per frame)
3. Pre-allocate buffers (avoid allocations in hot path)

---

## Acceptance Criteria

- [ ] `SharedMemorySensorDataSource` implements all `ISensorDataSource` methods
- [ ] Connects to simulator's Unix socket successfully
- [ ] Receives memfd file descriptor via `SCM_RIGHTS`
- [ ] Maps shared memory ring buffer correctly
- [ ] Reads vitals frames (60 Hz) and emits `vitalsReceived` signal
- [ ] Reads waveform frames (250 Hz samples) and emits `waveformSampleReady` signal
- [ ] Validates CRC32 for all frames (detects corruption)
- [ ] Detects writer stalls (no heartbeat > 250ms) and emits error
- [ ] Handles connection errors gracefully
- [ ] End-to-end latency < 16ms (simulator write → signal emission)

---

## Verification Checklist

### 1. Functional
- [ ] Successfully connects to simulator
- [ ] Receives vitals at 60 Hz
- [ ] Receives waveforms at 250 Hz
- [ ] Signals emitted correctly
- [ ] Data matches simulator output
- [ ] Stall detection works (kill simulator → error within 300ms)

### 2. Code Quality
- [ ] Doxygen comments for all public APIs
- [ ] Proper error handling (`Result<T,E>` pattern)
- [ ] No memory leaks
- [ ] Thread-safe (if needed)
- [ ] CRC validation implemented correctly

### 3. Documentation
- [ ] Implementation documented in `37_SENSOR_INTEGRATION.md`
- [ ] Integration guide updated with z-monitor setup steps

### 4. Integration
- [ ] Works with running simulator
- [ ] MonitoringService can consume signals
- [ ] Latency measured and documented (< 16ms target)

### 5. Tests
- [ ] Unit tests for frame parsing
- [ ] Unit tests for CRC validation
- [ ] Unit tests for error handling
- [ ] Integration test with simulator running (vitals flow, waveform flow, stall detection)

---

## Performance Targets

- **Sensor read → sample enqueued:** < 1 ms
- **Frame parsing (including CRC):** < 2 ms
- **Signal emission:** < 1 ms
- **Total latency (simulator write → signal):** < 16 ms ✅

---

## Next Steps

After implementation complete:
1. Proceed to **44d-wire-sensor-to-monitoring-service.md** (MonitoringService integration)
2. Test end-to-end data flow
3. Measure and optimize latency

---

**Estimated Time:** 3-5 hours (includes implementation, testing, debugging)
