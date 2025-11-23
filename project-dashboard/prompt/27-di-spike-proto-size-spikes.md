Decision: Run two optional spikes: (1) DI container evaluation (Boost.DI vs manual), (2) nanopb/protobuf-lite size spike for embedded telemetry.

Context: Collect metrics and pros/cons to include in `doc/13_DEPENDENCY_INJECTION.md` and `doc/14_PROTOCOL_BUFFERS.md`.

Constraints:
- Keep spikes minimal and self-contained; measure binary size and build complexity.

Expected output:
- Short reports in `doc/` with conclusions and recommendations.

Run/Verify:
- Build small sample projects and record binary sizes and code complexity notes.
