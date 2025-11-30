# Task: Build and Fix Sensor Simulator for Local Execution

## Context

**Documentation:**
- Architecture: [37_SENSOR_INTEGRATION.md](../doc/z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md)
- Implementation Guide: [44_SIMULATOR_INTEGRATION_GUIDE.md](../doc/z-monitor/architecture_and_design/44_SIMULATOR_INTEGRATION_GUIDE.md) - Phase 1
- Simulator README: [sensor-simulator/README.md](../sensor-simulator/README.md)

**Previous Work:**
- ✅ Documentation consolidated (37_SENSOR_INTEGRATION.md, 44_SIMULATOR_INTEGRATION_GUIDE.md)
- ⏳ Simulator needs to be built locally (was previously running in Docker)

**Dependencies:**
- Qt 6.9.2 installed at `/Users/dustinwind/Qt/6.9.2/macos`
- CMake 3.20+ (build system)
- macOS 12+ or Linux

---

## Objective

Build the sensor simulator (`project-dashboard/sensor-simulator/`) locally on macOS to enable shared-memory communication with z-monitor. The simulator previously ran in Docker but now needs to run natively to share memory with z-monitor (Docker containers cannot share memory with native processes). Fix all compilation errors, resolve dependency issues, ensure shared memory ring buffer writer works correctly, and verify the Unix domain socket handshake. Take screenshot of running simulator UI showing vitals display, waveform visualization, and log console.

---

## Architecture Overview

### Shared Memory Transport

**Components:**
- **Control Socket (Unix Domain Socket):** `/tmp/z-monitor-sensor.sock` - Used **ONLY** for initial handshake to exchange memfd file descriptor
- **Data Buffer (Shared Memory):** `memfd://zmonitor-sim-ring` - **All sensor data** (60 Hz vitals, 250 Hz waveforms) transferred through shared memory ring buffer

**Ring Buffer Structure:**
```cpp
struct RingBufferHeader {
    uint32_t magic = 0x534D5242;  // 'SMRB'
    uint16_t version = 1;
    uint32_t frameSizeBytes = 4096;
    uint32_t frameCount = 2048;
    std::atomic<uint64_t> writeIndex;
    std::atomic<uint64_t> heartbeatTimestamp;  // ms since epoch
};

struct SensorFrame {
    uint8_t type;              // 0x01 = Vitals, 0x02 = Waveform, 0x03 = Heartbeat
    uint64_t timestampNs;
    uint32_t sequenceNumber;
    uint32_t dataSize;
    uint8_t data[4064];        // JSON payload
    uint32_t crc32;
};
```

**Data Rate:**
- 60 Hz vitals (one frame every 16.67 ms)
- 250 Hz ECG waveform samples (batched into 10-sample chunks)

---

## Platform-Specific Considerations

### macOS Implementation (memfd Compatibility)

**Challenge:** Linux `memfd_create()` is not available on macOS. Must use POSIX `shm_open()` as alternative.

**Simulator Implementation:**
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
#endif
```

**Cleanup (macOS-specific):**
```cpp
#ifdef __APPLE__
    shm_unlink("/zmonitor-sim-ring");  // Required on macOS
#endif
close(fd);  // Automatic cleanup on Linux
```

**Key Differences:**
| Feature     | Linux (memfd)          | macOS (shm_open)             |
| ----------- | ---------------------- | ---------------------------- |
| Creation    | `memfd_create()`       | `shm_open()` with name       |
| Namespace   | Anonymous              | Named (`/zmonitor-sim-ring`) |
| Cleanup     | Automatic on close     | Requires `shm_unlink()`      |
| FD Passing  | Via `SCM_RIGHTS`       | Via `SCM_RIGHTS` (same)      |
| Performance | Identical after mmap() | Identical after mmap()       |

---

## Implementation Steps

### Step 1: Verify Qt Installation

**Commands:**
```bash
# Check Qt installation
ls -la /Users/dustinwind/Qt/6.9.2/macos

# Verify qmake
/Users/dustinwind/Qt/6.9.2/macos/bin/qmake --version
# Expected: "QMake version 3.1, Using Qt version 6.9.2"

# Set environment variable (add to ~/.zshrc)
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"
```

**Verification:**
- [ ] Qt 6.9.2 directory exists
- [ ] `qmake --version` shows 6.9.2
- [ ] `CMAKE_PREFIX_PATH` set correctly

---

### Step 2: Build Sensor Simulator

**Commands:**
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

---

### Step 3: Verify Shared Memory Creation

**macOS Commands:**
```bash
# Check shared memory (via lsof)
lsof -c sensor_simulator | grep shm
# Expected: Shows memory-mapped file descriptor

# Check socket
ls -la /tmp/z-monitor-sensor.sock
# Expected: srwxr-xr-x 1 user user 0 Nov 29 12:34 /tmp/z-monitor-sensor.sock

# Test socket connection
echo "test" | nc -U /tmp/z-monitor-sensor.sock
# Expected: Connection succeeds
```

**Verification:**
- [ ] Shared memory exists (visible via lsof)
- [ ] Socket exists at `/tmp/z-monitor-sensor.sock`
- [ ] Socket is connectable (nc succeeds)

---

### Step 4: Test UI and Data Generation

**UI Verification:**
- [ ] Simulator window opens (title: "Telemetry Simulator")
- [ ] Vitals display shows: Heart Rate, SpO2, Respiration Rate updating
- [ ] ECG waveform displays with PQRST complex pattern
- [ ] Log console shows telemetry frames being written

---

### Step 5: Capture Screenshot

**Commands:**
```bash
# Capture screenshot (macOS)
mkdir -p project-dashboard/screenshots
screencapture -w project-dashboard/screenshots/simulator-baseline-v1.0.png
# Click on simulator window to capture
```

**Screenshot Requirements:**
- Resolution: 1280x720 or native resolution
- Content: Full simulator UI including:
  - Vitals display (HR, SpO2, RR with live values)
  - Waveform panel (ECG with PQRST complex)
  - Control buttons
  - Log console (showing "Frame written" messages)

---

## Troubleshooting

### Error: "Qt not found"
**Solution:** Set `CMAKE_PREFIX_PATH` correctly:
```bash
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos"
cmake .. -DCMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos"
```

### Error: "memfd_create not found" (macOS)
**Solution:** This is expected on macOS. Simulator should automatically use `shm_open` fallback. Check `SharedMemoryWriter.cpp` for `#ifdef __APPLE__` blocks.

### Simulator crashes on startup
**Solution:**
1. Check console for Qt dependency issues
2. Verify Qt Quick modules installed
3. Check QML files exist in `qml/` directory
4. Verify `qml.qrc` includes all QML files

### Socket already in use
**Solution:**
```bash
# Remove stale socket
rm /tmp/z-monitor-sensor.sock

# Or change socket path in config
# Edit simulator config: socket_path = "/tmp/z-monitor-sensor-2.sock"
```

---

## Acceptance Criteria

- [ ] Simulator builds successfully on macOS with Qt 6.9.2
- [ ] All compilation errors fixed
- [ ] Shared memory writer uses macOS-compatible API (POSIX shm_open)
- [ ] Control server creates Unix socket at `/tmp/z-monitor-sensor.sock`
- [ ] Application launches and displays QML UI correctly
- [ ] Vitals display updates at 60 Hz (Heart Rate, SpO2, Respiration Rate)
- [ ] ECG waveform displays with 250 Hz samples
- [ ] Log console shows telemetry frames being written
- [ ] Screenshot captured showing full UI
- [ ] Screenshot saved to `project-dashboard/screenshots/simulator-baseline-v1.0.png`

---

## Verification Checklist

### 1. Functional
- [ ] Simulator builds without errors
- [ ] Simulator launches successfully
- [ ] UI displays correctly
- [ ] Vitals update in real-time (visible changes every second)
- [ ] Waveform animates smoothly (no stuttering)
- [ ] Shared memory buffer created (verified via lsof)
- [ ] Unix socket listening (verified via ls/nc)

### 2. Code Quality
- [ ] Build warnings addressed (Qt API deprecations)
- [ ] Proper error handling for shared memory operations
- [ ] Doxygen comments for public APIs (if adding new code)
- [ ] No memory leaks (run with leaks tool on macOS if possible)

### 3. Documentation
- [ ] Build instructions updated in `sensor-simulator/README.md` (if changes made)
- [ ] Platform-specific notes added (macOS shm_open usage)
- [ ] Screenshot captured and documented

### 4. Integration
- [ ] Shared memory ring buffer visible via diagnostic tools
- [ ] Unix socket can be connected to (`nc -U` succeeds)
- [ ] Frame structure matches documentation (magic=0x534D5242)

### 5. Tests
- [ ] Manual smoke test passed (launch, observe vitals/waveform, check logs)
- [ ] Shared memory verification (via lsof on macOS)
- [ ] Socket verification (nc connection test)

---

## Performance Targets

- Simulator write latency: < 2 ms per frame
- Data rate: 60 Hz vitals + 250 Hz waveforms
- Memory usage: ~8.4 MB shared memory (ring buffer)
- CPU usage: 2-5% when running

---

## Next Steps

After simulator is built and verified:
1. Proceed to **44c-implement-shared-memory-sensor-datasource.md** (z-monitor reader implementation)
2. Test handshake between simulator and z-monitor
3. Verify end-to-end data flow

---

**Estimated Time:** 1-2 hours (includes troubleshooting, screenshot capture)
