# DeviceSimulator Adapter — WebSocket Adapter Spec

Purpose
-------
This document specifies a small adapter that implements the `IDeviceSimulator` contract by connecting to the out-of-process `sensor-simulator` over WebSocket. The Z Monitor should only depend on the `IDeviceSimulator` interface; the adapter bridges the network transport and the interface.

When to use
-----------
- Development machines without hardware sensors.
- Automated UI or integration tests that require deterministic sensor streams.
- Local demos where the Z Monitor should consume sensor data but not know about the simulator transport.

Responsibilities
----------------
- Maintain a WebSocket client connection to the simulator endpoint (default `ws://localhost:9002`).
- Parse incoming JSON telemetry messages and translate them into `IDeviceSimulator` callbacks or internal method calls (e.g., `PushWaveformChunk`).
- Provide `StartProfile` / `Pause` / `Resume` by sending control messages to the simulator (if the simulator supports control messages) or simulate the behavior locally when not supported.
- Implement reconnection with exponential backoff and surface connection state to the host application.

Message formats (expected from `sensor-simulator`)
--------------------------------------------------
- `vitals`:
  ```json
  {"type":"vitals","timestamp_ms":1700000000000,"hr":72,"spo2":98,"rr":16}
  ```
- `waveform`:
  ```json
  {"type":"waveform","channel":"ecg","sample_rate":250,"start_timestamp_ms":1700000000000,"values":[...numbers...]}
  ```
- `alarm`:
  ```json
  {"type":"alarm","level":"critical","timestamp_ms":1700000000000}
  ```
- `notification`:
  ```json
  {"type":"notification","text":"Manual notification","timestamp_ms":1700000000000}
  ```

Adapter design sketch (C++ / Qt)
--------------------------------
- Class: `WebSocketDeviceSimulatorAdapter : public IDeviceSimulator`
- Members:
  - `QWebSocket m_socket;`
  - `QTimer m_reconnectTimer;`
  - `PacketCallback m_callback; QObject* m_callbackContext = nullptr;`

- Key behaviors:
  - `SetPacketCallback(cb, context)`: store `m_callback` and `m_callbackContext`.
  - `Connect()`: open `m_socket` to `ws://localhost:9002`, attach `textMessageReceived` slot.
  - `onTextMessageReceived(msg)`: parse JSON -> if `vitals` or `alarm` or `waveform`, call `dispatchPacket(msg)` which will `QMetaObject::invokeMethod(m_callbackContext, ...)` to run the callback on the requested thread (or call directly if `m_callbackContext==nullptr`).
  - `StartProfile(path, scale)`: send JSON control message to simulator `{"command":"start_profile","path":"...","time_scale":10.0}` if supported; otherwise return false.

Failure & backpressure
----------------------
- If the socket disconnects, the adapter should call `m_reconnectTimer` with an exponential backoff up to a cap (e.g., 30s).
- If message parsing fails, log with `qWarning()` and drop the message.

Acceptance tests
----------------
- Unit test: mock `QWebSocket` to push known `vitals` and verify `PacketCallback` is invoked with matching JSON.
- Integration test: run `project-dashboard/sensor-simulator` locally and verify adapter connects, receives a `vitals` message within N seconds, and that the Z Monitor receives the same.

Notes
-----
- The adapter keeps the device code agnostic of the transport; switching from WebSocket to a different transport (serial, unix socket) requires only writing a new adapter implementing `IDeviceSimulator`.
- For performance-sensitive path (high sample-rate waveform streaming), the adapter should translate JSON waveform arrays into binary or protobuf frames if the device ingestion pipeline supports it.
