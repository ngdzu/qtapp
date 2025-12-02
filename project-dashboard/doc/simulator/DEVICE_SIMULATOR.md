---
doc_id: DOC-COMP-050
title: Device Simulator - Overview and Design
version: 1.0
status: Draft
category: Component
subcategory: Infrastructure/Simulator
---

# Device Simulator — Overview and Design

Purpose
-------
This document describes the device-side and server-side simulator architecture used for development, UI demos, and end-to-end testing. The simulator is intended to produce realistic, timestamped vitals and waveform data that the Z Monitor can consume in realtime or in accelerated playback for CI.

Goals
-----
- Provide deterministic playback for reproducible E2E tests.
- Provide scripted event injection (arrhythmia, motion artifact, alarms) for UI and alarm testing.
- Support multiple transports (HTTP POST for batch, WebSocket/SSE for realtime) and both JSON and protobuf encodings.
- Support configurable sample rates and waveform chunking to emulate real devices.
- Be runnable locally (CLI), in docker-compose for CI, and as a lightweight service for integration testing.

Key Concepts
------------
- Run modes
  - deterministic: replay a profile file with absolute ms offsets (preferred for tests)
  - scripted: run a timeline of events and waveform sequences
  - random/noise: generate stochastic signals for stress testing
  - live-file: stream a recorded raw waveform file (wav/raw) in realtime

- Message formats
  - Canonical (recommended): `proto/telemetry.proto` (proto3). Use `TelemetryPacket` and `BatchContainer` types.
  - JSON: for quick debugging and simulator REST endpoints; map fields to the OpenAPI `openapi/telemetry.yaml`.

- Transports
  - REST batch: `POST /api/telemetry` — accepts JSON or application/x-protobuf batch payloads.
  - Realtime (optional): WebSocket or Server-Sent Events (SSE) for low-latency streaming of waveform chunks.
  - Control API: `POST /api/sim/control` for pause/play/inject commands, useful in CI orchestration.

Data Model & Timing
-------------------
- Vitals: occasional scalar samples (heart rate, spo2, respiratory rate) with one timestamp per sample. Default sample rate: 1 Hz (configurable).
- Waveforms: ECG, pleth, and other waveforms provided as chunked arrays. Each waveform chunk MUST include metadata: `channel`, `start_timestamp` (epoch ms), `sample_rate` (Hz), `seqno`, and `values` (signed integers or floats). Typical ECG sample rates: 250–500 Hz; pleth: 100–200 Hz.
- Timestamps MUST be provided in epoch-milliseconds (UTC) or RFC3339 with timezone; profile files should use epoch-ms for determinism.

Profile Format (recommended)
----------------------------
- Profiles are JSON files under `central-server-simulator/profiles/` with sections:
  - `metadata`: name, description, authors, playback_speed
  - `timeline`: array of items with `{ offset_ms, type, payloadRef }` where `offset_ms` is ms from profile start
  - `waveforms`: references to waveform binary/CSV files with associated sample_rate and channel

Example timeline items
  - `{ offset_ms: 0, type: "vitals", payloadRef: "vitals_start.json" }`
  - `{ offset_ms: 5000, type: "event", payloadRef: "arrhythmia_high.json" }`

Determinism & CI
-----------------
- Playback must support a `--time-scale` option (e.g. 1x, 10x) so CI can accelerate scenarios.
- Deterministic profiles should avoid system-clock dependency — simulator exposes a simulated clock used to compute timestamps.

Backpressure & Robustness
-------------------------
- When streaming realtime waveform chunks, the device may signal `PAUSE` or `SLOW` to the simulator via control API.
- Simulator should offer a configurable send-buffer and drop-policy (drop-oldest / drop-newest) to emulate constrained devices.

Control & Admin API
-------------------
- `POST /api/sim/control` — JSON body `{ command: "play" | "pause" | "stop" | "inject", args: {...} }`.
- `GET /api/sim/status` — health and current playback position.

Security
--------
- The simulator supports optional mTLS. Dev certs live under `central-server-simulator/certs/` (README explains usage). Do NOT commit private keys.
- For quick runs, simulator accepts unsecured HTTP/JSON; for CI and mTLS integration tests use protobuf over TLS with client certs.

Observability
-------------
- Expose logs (stdout) and a `/metrics` endpoint (prometheus) for sample-rates, dropped-chunks, and latency statistics.

Acceptance Criteria
-------------------
- `doc/simulator/DEVICE_SIMULATOR.md` exists (this file).
- `central-server-simulator/README.md` describes how to run the simulator and use profiles.
- A deterministic profile and small waveform examples exist under `central-server-simulator/profiles/`.
- The simulator supports REST batch ingest and control API as specified.

Next work (implementation)
--------------------------
- Create `central-server-simulator/app.py` with minimal Flask (or FastAPI) implementation for REST endpoints and control API.
- Provide example profile and small waveform chunk stored in `central-server-simulator/profiles/`.
- Add docker-compose service for simulator for CI and local runs.

UI Simulator (Qt Quick)
-----------------------
We provide a lightweight Qt Quick-based UI simulator in `project-dashboard/data-simulator/` that exposes a `Simulator` QObject to QML. Key features:

- Buttons to simulate clinical conditions and user actions:
  - `Trigger Critical` — emits an alarm with level `critical` (causes UI alarm flows to exercise audible/visual alarm state).
  - `Trigger Warning` — emits an alarm with level `warning`.
  - `Notify` — send an informational notification message.
  - `Play Demo` — runs a short timeline that triggers a critical alarm, a notification, then a warning to demonstrate state transitions.

- The QML UI logs events in an on-screen list and updates a `Last event` field so manual QA can verify behavior quickly.

- The backend `Simulator` is a small `QObject` (C++) that exposes `Q_INVOKABLE` methods and Qt signals used by QML. Implementers can extend `Simulator` to POST telemetry to the central server or to implement profile-based playback.

Running the UI locally
----------------------
1. Build locally (macOS, with Qt6 and CMake installed):

```bash
cd project-dashboard/data-simulator
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./qt_device_simulator
```

2. Run in Docker (requires the `qtapp-qt-dev-env:latest` and `qtapp-qt-runtime:latest` images described in the repo guidelines). For macOS with XQuartz, set DISPLAY to `host.docker.internal:0` and allow connections in XQuartz; for Linux mount `/tmp/.X11-unix`.

macOS example (XQuartz):

```bash
docker build -t qt-data-simulator -f project-dashboard/data-simulator/Dockerfile project-dashboard/data-simulator
docker run --rm -e DISPLAY=host.docker.internal:0 qt-data-simulator
```

Linux example (local X11 socket):

```bash
docker build -t qt-data-simulator -f project-dashboard/data-simulator/Dockerfile project-dashboard/data-simulator
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-data-simulator
```

Notes: containerized GUI requires an X server on the host (XQuartz on macOS) and proper security settings; these commands are convenient for local demos but CI runs should use headless testing strategies (virtual framebuffers or image-based screenshot comparisons).

Acceptance criteria (UI)
-----------------------
- `project-dashboard/data-simulator` builds to an executable `qt_device_simulator`.
- Clicking `Trigger Critical` emits an `alarmTriggered("critical")` signal and updates the UI log.
- `Play Demo` runs a small timeline demonstrating state transitions.

