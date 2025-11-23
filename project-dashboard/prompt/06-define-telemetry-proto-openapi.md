Decision: Add `proto/telemetry.proto` and `openapi/telemetry.yaml` defining canonical telemetry messages (vitals, device status, alarms, batch container, ack semantics).

Context: Proto is canonical; OpenAPI provides JSON mapping for simulator and server. Keep messages minimal and versioned (`package telemetry.v1`).

Constraints:
- Use proto3; include timestamps and device_id fields. Keep schema stable with clear versioning guidance.

Expected output:
- `proto/telemetry.proto`, `openapi/telemetry.yaml`, and `doc/proto_design.md` describing mapping and codegen steps.

Run/Verify:
- Run `protoc --proto_path=proto --cpp_out=build proto/telemetry.proto` (or doc steps) to validate syntax.
