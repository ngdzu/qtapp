---
doc_id: DOC-API-011
title: IDeviceSimulator Interface Specification
version: 1.0
status: Draft
category: API
subcategory: Infrastructure
---

# IDeviceSimulator — Interface Specification

Purpose
-------
This document defines the C++ interface expected by the Z Monitor for a local in-process or out-of-process simulator adapter. The interface is intended to be small, testable, and usable by both production code and unit tests to inject telemetry events and waveform chunks.

Design Goals
------------
- Minimal dependencies on Qt — interface uses `std::chrono` and plain POD types for interoperability.
- Thread-safe: callers may invoke methods from background threads; callbacks are invoked on a configurable `QObject` context via Qt signal/slot where necessary.
- Deterministic: `InjectProfile` takes a profile that describes all timestamps as epoch-ms.

Interface (pseudo-header)
-------------------------
```cpp
class IDeviceSimulator {
public:
    virtual ~IDeviceSimulator() = default;

    // Start playback of a profile file. Returns true on accepted.
    virtual bool StartProfile(const std::string& profilePath, double timeScale = 1.0) = 0;

    // Pause/resume/stop control
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Stop() = 0;

    // Inject a single event immediately (vitals or alarm)
    virtual void InjectEvent(const std::string& eventJson) = 0;

    // Push a waveform chunk. Caller provides start timestamp (epoch ms), sample rate, channel and values.
    virtual void PushWaveformChunk(int64_t startTimestampMs, double sampleRateHz, int channel, const std::vector<int16_t>& values) = 0;

    // Callback registration: onPacket is invoked on the provided QObject's thread via queued connection.
    using PacketCallback = std::function<void(const std::string& packetJson)>;
    virtual void SetPacketCallback(PacketCallback cb, QObject* targetContext = nullptr) = 0;
};
```

Threading Model
---------------
- Implementations MUST be callable from any thread.
- Callbacks registered via `SetPacketCallback` MUST be invoked on the `targetContext` QObject's thread using `QMetaObject::invokeMethod` with `Qt::QueuedConnection` if `targetContext` is non-null.

Ownership and Lifetime
----------------------
- The host application owns the `IDeviceSimulator` instance and must call `Stop()` before destruction.

Example Usage
-------------
1. Create simulator instance (factory function e.g. `CreateLocalSimulator()`).
2. Register callback to receive telemetry packets and pass them into device ingestion code.
3. Call `StartProfile("profiles/example.json", 10.0)` for accelerated CI playback.

Testing Notes
-------------
- Provide a `MockDeviceSimulator` for unit tests that implements the interface deterministically.

Adapter Mapping (WebSocket sensor-simulator)
-------------------------------------------
Implementations may be in-process (mock) or out-of-process adapters. One planned out-of-process adapter is a WebSocket-based adapter that connects to the `sensor-simulator` (`ws://localhost:9002`) and translates incoming sensor JSON messages into the `IDeviceSimulator` callback model.

Guidelines for a WebSocket adapter implementation:
- Connection: maintain a `QWebSocket` client that connects to the simulator and reconnects with backoff on disconnects.
- Message parsing: parse incoming JSON messages and invoke the `PacketCallback` with the canonical JSON blob or translated protobuf bytes.
- Threading: parse messages on the Qt event loop thread and use `QMetaObject::invokeMethod` to dispatch into the `targetContext` if provided by `SetPacketCallback`.
- Control: the adapter may implement `StartProfile`, `Pause`, `Resume`, and `Stop` by sending control messages to the simulator's control channel (if supported), otherwise these methods can be no-ops for passive adapters.

Example mapping (JSON message types -> adapter action):
- `{"type":"vitals", ...}` -> call `PacketCallback` with the vitals JSON (or construct a `VitalsSample` and dispatch).
- `{"type":"waveform", ...}` -> translate to `PushWaveformChunk(...)` internally or forward raw chunk via `PacketCallback`.
- `{"type":"alarm", "level":"critical"}` -> forward as an alarm packet to the device ingestion path via `PacketCallback`.

Log-level mapping and semantics
--------------------------------
- The simulator emits structured log events with levels: `Critical`, `Warning`, `Info`, and `Debug`.
- Device adapters should forward `alarm` messages with high-severity levels as `Critical` or `Warning` depending on urgency. For example, a medication that will be out in one hour or 15 minutes should be surfaced as a `Warning` (it is an actionable alert that requires operator attention) rather than a low-priority `Info` message. The `Messages` category is intentionally omitted to keep log semantics clear — use `Info` for non-urgent messages and `Warning` for time-sensitive, actionable conditions.
- When translating simulator events into the device ingestion path, include the `level` field on the outbound packet JSON so downstream consumers can filter or escalate appropriately.

Security note: if the simulator is run across a network boundary or in CI, secure the WebSocket (wss) or use a local VPN / tunnel. The adapter should validate any protocol-level authentication tokens when present.
