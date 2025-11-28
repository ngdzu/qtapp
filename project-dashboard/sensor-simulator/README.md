
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

```bash
cd project-dashboard/sensor-simulator
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./sensor_simulator
```

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

```
SharedMemoryHeader {
  magic = 'SMRR'
  version = 1
  slotCount = 2048
  frameSize = 1024 bytes
  std::atomic<uint32_t> writeIndex
  std::atomic<uint64_t> heartbeatNs
}

SensorFrame {
  uint64_t timestampNs
  VitalPayload vital   // HR, SpO₂, RR, etc.
  WaveformPayload waveform[2]  // ECG + SpO₂, 10 samples per payload
  uint32_t crc32
}
```

Each frame writes into `slots[writeIndex % slotCount]`, then increments `writeIndex`. The control socket pushes the `memfd` descriptor to Z-Monitor whenever the simulator starts.

## Notes

- Waveform data is generated using a synthetic ECG algorithm that produces realistic PQRST complexes
- All UI components use the same color scheme and styling as the reference React app


