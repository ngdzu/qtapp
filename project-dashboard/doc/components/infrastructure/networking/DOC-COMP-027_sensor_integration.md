---
doc_id: DOC-COMP-027
title: Sensor Integration Architecture
version: 1.0
category: Components
subcategory: Infrastructure
status: Draft
owner: Infrastructure Team
reviewers: [Architecture, Real-Time]
last_reviewed: 2025-12-01
next_review: 2026-03-01
related_docs:
  - DOC-ARCH-001_software_architecture.md
  - DOC-ARCH-011_thread_model.md
  - DOC-COMP-026_data_caching_strategy.md
  - DOC-COMP-028_waveform_display.md
tags: [sensors, shared-memory, ipc, memfd, ring-buffer]
source:
  path: project-dashboard/doc/z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md
  original_id: DESIGN-037
  last_updated: 2025-11-27
---

# Purpose
Defines sensor data integration using shared-memory ring buffer (memfd) for < 16ms transport latency. Covers ISensorDataSource abstraction, SharedMemorySensorDataSource implementation, and Unix domain socket handshake for file descriptor exchange.

# Architecture

## Transport Strategy
- **Control Socket (Unix Domain):** One-time handshake to exchange memfd file descriptor via SCM_RIGHTS; NOT used for data transfer
- **Data Buffer (Shared Memory):** All sensor data (60 Hz vitals, 250 Hz waveforms) transferred via lock-free ring buffer; zero-copy, < 16ms latency
- **Why Socket + Shared Memory?** File descriptors cannot be passed through shared memory; socket exchanges FD once, then all data flows through direct memory access

## Ring Buffer Layout
```
SharedMemoryHeader:
  - magic: 0x534D5252
  - version: 1
  - slotCount: 2048
  - atomic<writeIndex>
  - atomic<heartbeatNs>
  - frameSizeBytes

SensorFrame:
  - timestampNs
  - sampleCount
  - channelMask
  - VitalPayload
  - WaveformPayload[MAX_CHANNELS]
  - crc32
```

## Data Flow

### Connection Setup (One-Time, Socket)
```
Sensor Simulator creates memfd
  → Unix domain socket handshake
  → SCM_RIGHTS passes file descriptor
  → SharedMemorySensorDataSource receives memfd
  → mmap shared memory
  → Socket disconnected (no longer used)
```

### Data Transfer (Continuous, Shared Memory)
```
Sensor Simulator
  → memfd write @ 60 Hz / 250 Hz (DIRECT MEMORY)
  → SharedMemoryRingBuffer::writeFrame()
  → atomic writeIndex (NO SOCKET I/O)
  → SharedMemorySensorDataSource::pollFrames()
  → Binary decode (NO SOCKET I/O)
  → emit vitalSignsReceived(VitalRecord)
  → MonitoringService (< 16ms transport)
    ├─ VitalsCache::append()
    ├─ AlarmManager::processVitalSigns()
    ├─ UI update
    └─ TelemetryBatch::addVital()
```

# ISensorDataSource Interface
```cpp
class ISensorDataSource : public QObject {
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isActive() const = 0;
    virtual DataSourceInfo getInfo() const = 0;

signals:
    void vitalSignsReceived(const VitalRecord&);
    void waveformSampleReceived(const WaveformSample&);
    void connectionStatusChanged(bool connected, const QString& sensorType);
    void sensorError(const SensorError&);
};
```

# Implementations
- **SharedMemorySensorDataSource:** Connects to simulator via memfd ring buffer; < 16ms latency
- **SimulatorDataSource:** Internal Qt Timer-based fallback
- **MockSensorDataSource:** Deterministic testing with predefined data
- **HardwareSensorAdapter:** Future real hardware sensors (serial/USB)
- **ReplayDataSource:** Replay recorded data for debugging

# Platform-Specific Notes

## macOS (memfd Compatibility)
- Linux: `memfd_create()` (anonymous, auto-cleanup)
- macOS: `shm_open()` with name `/zmonitor-sim-ring`, requires `shm_unlink()` for cleanup
- File descriptor passing via `SCM_RIGHTS` identical on both platforms
- After `mmap()`, all operations identical

# Dependency Injection
```cpp
if (Settings::instance()->useSharedMemorySimulator()) {
    registerSingleton<ISensorDataSource>([]() {
        return new SharedMemorySensorDataSource(
            Settings::instance()->sharedMemorySocketPath(),
            Settings::instance()->sharedMemoryRingName());
    });
}
```

# Verification
- Functional: Verify < 16ms latency from simulator write to MonitoringService; validate CRC, detect stalled writer
- Code Quality: Platform-specific code isolated via preprocessor; no hardcoded paths
- Documentation: Diagram showing socket handshake + shared memory data flow
- Integration: End-to-end test with sensor-simulator; measure frame drops under load
- Tests: Unit tests for ring buffer, frame decode, heartbeat timeout; integration tests with mock memfd

# Document Metadata
| Field          | Value        |
| -------------- | ------------ |
| Original Doc   | DESIGN-037   |
| Migration Date | 2025-12-01   |
| New Doc ID     | DOC-COMP-027 |

# Revision History
- 1.0 (2025-12-01): Migrated from 37_SENSOR_INTEGRATION.md; consolidated shared-memory architecture.
