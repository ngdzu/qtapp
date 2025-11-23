
# Sensor Simulator

Sensor Simulator is a small Qt-based application that simulates device sensors and streams JSON telemetry over WebSocket.

## Description

The simulator exposes a WebSocket server and a simple QML UI (title: "Sensor Simulator") you can use to generate telemetry, alarms and notifications for development and testing.

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

## Notes

- The QML UI provides buttons to trigger `Critical`, `Warning`, and `Notification` events, and a `Play Demo` sequence.
- Logs may show a `qrc:/qml/Main.qml: No such file or directory` warning; the app currently falls back to the filesystem QML.


