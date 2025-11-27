# Protocol Buffers (protobuf) Guidance for Z Monitor

**Document ID:** DESIGN-014  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document describes when and how to use Protocol Buffers in the Z Monitor project. It covers choices (protoc, protobuf-lite, nanopb), schema and versioning rules, in-process vs IPC vs network usage, CMake integration, gRPC recommendations, storage/archival patterns, testing and CI guidance, and an incremental migration plan.

---

## Quick recommendation (TL;DR)
- Use Protocol Buffers (proto3) as the canonical on-the-wire and archival message format for device ↔ server telemetry, acks, and archival blobs.
- For desktop/edge builds use the standard `protoc` generated C++ classes (full runtime) or `protobuf-lite` when you need smaller runtime.
- For memory- or flash-constrained embedded targets, evaluate `nanopb` (C) or `protobuf-lite` and prefer batched messages to reduce per-message overhead.
- Use gRPC where you need typed RPCs and streaming over HTTP/2; otherwise, send serialized protobuf payloads over existing HTTPS endpoints (optionally exposing JSON mapping for interoperable HTTP APIs).
- Keep SQLite relational columns (indexed fields) for queries and store protobuf-serialized BLOBs for compact persisted messages/archives.

## Why use protobuf here
- Compact binary format reduces payload size and parsing cost compared to JSON for large telemetry batches.
- Strong schema with explicit tags supports forward/backward compatibility when rules are followed.
- Generated code reduces serialization/deserialization bugs and enforces a shared contract between device and server.
- Excellent multi-language tooling (C++, Python, Go, Java, JS) which is helpful for backend, integration tests, and tools.

## When not to use protobuf
- Per-sample inner-loop streaming at extremely high rates where bespoke binary framing or shared-memory ring buffers give lower latency/overhead.
- When human-readability is required; for debugging provide JSON mapping or human-readable dumps.

## Protobuf variants and when to choose
- `protoc` + C++ (full runtime)
  - Use for desktop/edge builds with few resource limits.
  - Pros: full-featured (reflection, utilities), easy integration with gRPC.
  - Cons: larger runtime binary size.

- `protobuf-lite` (C++ lite runtime)
  - Use when you need smaller binary/runtime footprint but still use generated C++ code.
  - Pros: smaller, fewer features but supports core serialization.
  - Cons: lacks reflection and some helper utilities.

- `nanopb` (C; code-generator for small targets)
  - Use for microcontrollers or very constrained embedded devices.
  - Pros: very small code size, configurable codegen for only the features used.
  - Cons: C API, fewer conveniences, manual memory management in some cases.

- `FlatBuffers` / `Cap'n Proto` (alternatives)
  - Considered when zero-copy random access to large messages is needed; not recommended as first choice due to less widespread usage in backend ecosystems.

## Schema design & versioning rules (must-follow)
- Use `syntax = "proto3"`.
- Reserve field numbers before use if you want to prevent accidental reuse: `reserved 10, 20;` or `reserved "old_field";`.
- NEVER re-use numeric field tags once assigned and used in production. Prefer adding new fields with new tag numbers.
- Prefer `optional` or presence-tracked fields for values that may or may not be present.
- Avoid `required` (proto2) semantics — proto3 does not support `required`.
- Use `oneof` for mutually exclusive fields to save space and clarify intent.
- Use nested messages for grouping logically-related data.
- For repeated numeric fields consider `packed = true` (default in proto3) to improve encoding efficiency.
- Provide explicit documentation comments in `.proto` files — they should be the source of truth for API docs and code generation.

### Compatibility rules
- Adding fields is safe (backwards compatible) as long as you use new tag numbers.
- Removing fields: mark them `reserved` first, and don't reuse the tags.
- Changing field types may be lossy; if you must change semantics, prefer a new field and keep the old one for a deprecation period.

## Message design patterns for this project
- Telemetry batching: use a `TelemetryBatch` message with repeated submessages (e.g., `Vitals`, `WaveformChunk`). Batching reduces per-message overhead and TLS handshake pressure.
- Waveform data: use `bytes` for pre-compressed chunks or repeated fixed-int arrays for raw samples if not compressing. Large repeated arrays may be heavy; chunk large waveforms into multiple `waveform_chunk` entries.
- Versioning: include an explicit `schema_version` or `format_version` field at top-level messages for quick compatibility checks.

Example `telemetry.proto`
```
syntax = "proto3";
package meddash.telemetry;

message Vitals {
  int64 timestamp_ms = 1;
  float heart_rate = 2;
  float spo2 = 3;
  float systolic = 4;
  float diastolic = 5;
}

message TelemetryBatch {
  string device_id = 1;
  int64 batch_timestamp_ms = 2;
  repeated Vitals vitals = 3;
  repeated bytes waveform_chunks = 4; // compressed or raw chunks
  string schema_version = 5; // optional string version
}
```

## Storage & archival patterns
- Keep key searchable columns (device_id, patient_id, timestamp ranges, alarm flags) as explicit columns in SQLite to allow efficient queries.
- Store the full serialized proto as a BLOB in an `archive_blob` column for complete fidelity and restore.
- Add indexes on frequently queried columns (timestamp, patient_id, device_id).
- For long-term archival, create a compact archive file format (e.g., gzipped tar of many protobuf blobs) or use per-day protobuf files containing batched messages.

## Network API choices (device ↔ server)
- gRPC (recommended if server supports it): typed RPCs, streaming, HTTP/2, built-in retries and flow control. Use for large telemetry uploads and control plane RPCs.
- REST + protobuf payloads: if server expects HTTP/JSON, either:
  - Use JSON mapping (MessageToJsonString / Parse) for interoperability, or
  - Send Content-Type `application/x-protobuf` with raw proto bytes and provide server-side handlers.
- Gateway approach: expose gRPC on the backend and a REST gateway for legacy clients.

## CMake integration and build tips
- Use `find_package(Protobuf REQUIRED)` and `protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})` for canonical integration.
- Prefer out-of-source generation and add generated files to a `proto_defs` library target. Example:

```cmake
find_package(Protobuf REQUIRED)
set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/telemetry.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
add_library(proto_defs ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(proto_defs PRIVATE ${Protobuf_LIBRARIES})
```

- For `nanopb`, add a custom generation step in CMake calling `protoc --nanopb_out=...` or include nanopb CMake helper scripts.
- CI: cache `protoc` tool and pre-generate proto artifacts to reduce build flakiness.

## Qt integration notes
- Generated protobuf C++ classes are plain C++ types; do **not** make them `QObject` subclasses. Instead wrap them in `QObject`-based adapters if the UI or QML needs properties/signals.
- Keep translation between `QObject` models and protobuf models at controller boundaries (e.g., `TelemetryController` serializes protobufs and hands them to `NetworkManager`).
- Avoid passing `QObject*` deep into generated proto layers — keep generated protos as pure data models.

## Debugging and human-readability
- Provide debug utilities to convert proto messages to JSON for logging and diagnostics:
  - `google::protobuf::util::MessageToJsonString()` (desktop)
  - For `nanopb`, add custom text dumps.
- When receiving a binary proto in logs, include a short human-friendly summary (device_id, timestamp, count) to simplify triage.

## Testing and CI
- Include unit tests that serialize/deserialize messages and run compatibility tests for older schema versions.
- Add a schema compatibility test that compiles generated code from older `.proto` files and runs round-trip tests (optional: keep a `proto-compat` branch or artifact set).
- Add proto linting (e.g., `buf` from Buf build tools) and a pre-commit hook for `.proto` style.
- In CI, ensure generated C++ files are consistent with committed generated outputs (or generate in CI and verify no changes).

## Embedded & resource-constrained targets
- Use `nanopb` or `protobuf-lite`.
- Use batch messages to reduce header overhead.
- Prefer fixed-size or pre-allocated buffers at the wire layer; avoid dynamic per-sample allocation.
- Measure binary size and runtime memory use for a realistic feature subset; iterate on which proto features to include.

## Migration plan and API docs
1. Create `proto/` directory with canonical `.proto` files and a README describing the canonical formats.
2. Update API docs to reference `.proto` files as the source of truth. If an OpenAPI/Swagger document exists, mark protobuf as canonical and provide JSON schema derived from `.proto` where necessary.
3. Implement server and device handlers for the new proto endpoints; provide both proto and JSON endpoints during transition if needed.
4. Add compatibility tests and a deprecation schedule for old JSON-only endpoints.

## Security considerations
- Protobuf itself does not encrypt or sign messages. Always protect telemetry in transit with TLS (mTLS recommended for device authentication).
- Validate message sizes and field counts to avoid DoS via large repeated fields.
- When storing serialized blobs, treat them as sensitive data if they contain PHI; encrypt at rest according to the project's key management policy.

## Example workflows & commands
- Generate C++ code (desktop):

```bash
protoc --cpp_out=gen proto/telemetry.proto
# or via CMake's protobuf_generate_cpp helper
```

- Generate nanopb sources:

```bash
protoc --plugin=protoc-gen-nanopb --nanopb_out=gen proto/telemetry.proto
```

- Convert proto to JSON (debug):

```cpp
#include <google/protobuf/util/json_util.h>
std::string json;
google::protobuf::util::MessageToJsonString(proto_msg, &json);
```

## Checklist for adopting protobuf
- [ ] Add canonical `.proto` files to `proto/` and commit them.
- [ ] Add CMake generation rules and a `proto_defs` target.
- [ ] Provide a `telemetry.proto` example and integrate into a small producer/consumer test.
- [ ] Decide on `protobuf` vs `protobuf-lite` vs `nanopb` per target and document choices.
- [ ] Add schema linting (e.g., `buf`) and CI checks.
- [ ] Update API docs to reference proto schemas and add JSON mapping notes.

---

If you'd like, I can implement option (A) from earlier: create `proto/telemetry.proto`, add CMake rules to generate code, and a tiny example producer/consumer in C++ that serializes a `TelemetryBatch` and writes it to a file. This will show integration end-to-end and help measure binary size for `protoc` and `nanopb` targets.
