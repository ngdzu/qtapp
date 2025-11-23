Decision: Implement `MockNetworkManager` test double that records requests and simulates responses (200/500/timeout) to test retry/backoff logic.

Context: Keep TLS out of scope for the mock; use telemetry proto/OpenAPI shapes for payloads.

Constraints:
- Provide hooks to configure simulated latency, failure rates, and response codes.
- Keep API matching the later real `INetworkManager` methods.

Expected output:
- `tests/mocks/mock_NetworkManager.h` and tests like `tests/network_retry_test.cpp`.

Run/Verify:
- Run the unit test demonstrating retries and backoff behavior using the mock.
