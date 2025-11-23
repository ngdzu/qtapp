Decision: Finalize `openapi/telemetry.yaml` and document proto->JSON mapping, error responses, and client codegen steps in `doc/api/README.md`.

Context: Proto is canonical; JSON mapping required for simulator and server; provide example requests and responses.

Constraints:
- Keep API versioned (v1) and stable; include example error payloads and HTTP status codes.

Expected output:
- `openapi/telemetry.yaml`, `doc/api/README.md` describing codegen commands for server/client.

Run/Verify:
- Use `swagger-cli validate openapi/telemetry.yaml` or similar to validate YAML.
