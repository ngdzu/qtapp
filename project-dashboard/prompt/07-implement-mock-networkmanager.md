Decision: Implement `MockNetworkManager` test double that records requests and simulates responses (200/500/timeout) to test retry/backoff logic.

Context: Keep TLS out of scope for the mock; use telemetry proto/OpenAPI shapes for payloads.

Constraints:
- Provide hooks to configure simulated latency, failure rates, and response codes.
- Keep API matching the later real `NetworkManager` methods (production implementation to be added later).
- Use "Mock" prefix in class name per naming conventions (see `.cursor/rules/naming_conventions.mdc`).

Expected output:
- `z-monitor/src/infrastructure/network/MockNetworkManager.h/cpp` and tests like `tests/unit/infrastructure/network/network_retry_test.cpp`.

Run/Verify:
- Run the unit test demonstrating retries and backoff behavior using the mock.
