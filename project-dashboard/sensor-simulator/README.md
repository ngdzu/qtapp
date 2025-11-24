
# Sensor Simulator

Sensor Simulator is a Qt-based application that simulates medical device sensors and streams JSON telemetry over WebSocket.

## Description

The simulator exposes a WebSocket server and a modern QML UI (title: "Telemetry Simulator") that provides:

- **Real-time Vitals Display**: Heart Rate, SpO2, and Respiration Rate cards with live updates
- **ECG Waveform Visualization**: Real-time ECG Lead II waveform with PQRST complex generation
- **Interactive Controls**: Manual trigger buttons for Critical, Warning, and Notification events
- **Demo Sequence**: Automated demo that plays through alarm scenarios
- **Log Console**: Filterable telemetry stream with pause/resume functionality
- **WebSocket Server**: Streams vitals and waveform data to connected clients at 5Hz (200ms intervals)

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

## WebSocket Endpoint

```text
ws://localhost:9002
```

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

### WebSocket Protocol
The simulator sends JSON messages with the following structure:
```json
{
  "type": "vitals",
  "timestamp_ms": 1234567890,
  "hr": 75,
  "spo2": 98,
  "rr": 16,
  "waveform": {
    "channel": "ecg",
    "sample_rate": 250,
    "start_timestamp_ms": 1234567890,
    "values": [100, 102, 98, ...]
  }
}
```

## Notes

- Waveform data is generated using a synthetic ECG algorithm that produces realistic PQRST complexes
- All UI components use the same color scheme and styling as the reference React app


