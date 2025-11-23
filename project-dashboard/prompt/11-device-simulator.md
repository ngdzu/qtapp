Decision: Implement a test-only `DeviceSimulator` providing deterministic and random modes, emitting vitals and waveform samples and supporting event injection.

Context: Useful for UI demos and unit tests; output should be timestamped samples and vitals records.

Constraints:
- Keep the API simple (Start/Stop/LoadProfile/InjectEvent) and provide deterministic profiles for tests.

Expected output:
- `src/core/DeviceSimulator.h/.cpp` (stubbed), sample profile files under `resources/sim_profiles/`, and tests exercising deterministic playback.

Run/Verify:
- Unit test subscribes to simulator signals and asserts expected sequence for a deterministic profile.
