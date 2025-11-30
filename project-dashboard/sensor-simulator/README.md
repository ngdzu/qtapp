
# Sensor Simulator

Sensor Simulator is a Qt-based application that simulates sensors for the Z Monitor and publishes binary telemetry frames into a shared-memory ring buffer (memfd + Unix-domain control socket).

## Description

The simulator exposes a modern QML UI (title: "Telemetry Simulator") and a low-latency shared-memory writer that provides:

- **Real-time Vitals Display**: Heart Rate, SpO2, and Respiration Rate cards with live updates
- **ECG Waveform Visualization**: Real-time ECG Lead II waveform with PQRST complex generation
- **Interactive Controls**: Manual trigger buttons for Critical, Warning, and Notification events
- **Demo Sequence**: Automated demo that plays through alarm scenarios
- **Log Console**: Filterable telemetry stream with pause/resume functionality
- **Shared-Memory Publisher**: Streams 60 Hz vitals and 250 Hz ECG waveform batches via `memfd://zmonitor-sim-ring`

## Build & Run (Local)

### Quick Build (macOS)

Use the provided build script:

```bash
cd project-dashboard/sensor-simulator
./scripts/build_local.sh
```

To clean and rebuild:

```bash
./scripts/build_local.sh clean
```

### Manual Build (macOS)

```bash
cd project-dashboard/sensor-simulator
mkdir -p build && cd build
CMAKE_PREFIX_PATH=/Users/dustinwind/Qt/6.9.2/macos cmake ..
make -j8
./sensor_simulator
```

### Manual Build (Linux)

```bash
cd project-dashboard/sensor-simulator
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./sensor_simulator
```

### Platform Notes

**macOS:**
- Uses POSIX `shm_open()` as memfd polyfill (Linux `memfd_create()` not available)
- Requires Qt 6.9.2 installed at `/Users/dustinwind/Qt/6.9.2/macos` (adjust `CMAKE_PREFIX_PATH` if different)
- Shared memory objects created via `shm_open()` instead of memfd
- Unix domain socket created at `/tmp/z-monitor-sensor.sock`
- WebSocket support is optional (requires Qt6::WebSockets module)

**Linux:**
- Uses native `memfd_create()` for shared memory
- Shared memory visible at `/dev/shm/zmonitor-sim-ring`

## Run with Docker Compose

```bash
cd project-dashboard
docker compose -f docker-compose.simulator.yml up --build
```

## Shared-Memory Transport

- **Control Socket:** `unix://run/zmonitor-sim.sock`
- **Ring Buffer Name:** `zmonitor-sim-ring` (visible as `/dev/shm/zmonitor-sim-ring` for diagnostics)
- **Latency Target:** < 16 ms end-to-end (simulator write → Z-Monitor UI)

## Features

### Real-time ECG Waveform
- Generates realistic PQRST complex waveforms at 250 Hz sample rate
- Displays last 300 samples in a scrolling chart
- Matches the React implementation's waveform generation algorithm

### Vitals Simulation
- Heart Rate: Random walk between 50-160 BPM
- SpO2: Random walk between 85-100%
- Respiration Rate: Random walk between 8-30 RPM

### Log Console
- Filter by level: ALL, CRITICAL, WARNING, INFO, INTERNAL
- Pause/Resume stream updates
- Clear logs functionality
- Color-coded log entries matching event types
- Displays up to 200 most recent log entries

### Ring Buffer Layout

The ring buffer uses the following structure (matches Z-Monitor's `SharedMemoryRingBuffer` reader):

```
RingBufferHeader {
  magic = 0x534D5242 ('SMRB')
  version = 1
  frameSize = 4096 bytes
  frameCount = 2048
  std::atomic<uint64_t> writeIndex
  std::atomic<uint64_t> heartbeatTimestamp (ms since epoch)
  uint32_t crc32
}

SensorFrame {
  uint8_t type (Vitals=0x01, Waveform=0x02, Heartbeat=0x03)
  uint64_t timestamp (ms since epoch)
  uint32_t sequenceNumber
  uint32_t dataSize
  uint32_t crc32
  // JSON data payload follows (variable length)
}
```

**Frame Types:**
- **Vitals Frame:** JSON payload with `{"hr":72,"spo2":98,"rr":16}`
- **Waveform Frame:** JSON payload with `{"channel":"ecg","sample_rate":250,"start_timestamp_ms":...,"values":[...]}`

**Writing Process:**
1. Writer gets next write index (atomic read)
2. Serializes data as JSON
3. Writes frame to `slots[writeIndex % frameCount]`
4. Calculates CRC32 and stores in frame
5. Atomically increments `writeIndex` (release semantics)
6. Updates `heartbeatTimestamp` in header

**Reading Process (Z-Monitor):**
1. Reader tracks local `readIndex`
2. Polls `writeIndex` in header (atomic read, acquire semantics)
3. If `writeIndex != readIndex`, reads frame
4. Validates CRC32
5. Parses JSON and emits Qt signals
6. Advances `readIndex`

The control socket (`/tmp/z-monitor-sensor.sock`) is used **ONLY** for the initial handshake to pass the `memfd` file descriptor via `SCM_RIGHTS`. After the handshake, the socket is disconnected and all data flows through shared memory only.

## Notes

- Waveform data is generated using a synthetic ECG algorithm that produces realistic PQRST complexes
- All UI components use the same color scheme and styling as the reference React app


