Decision: Create a docker-compose test harness (`docker-compose.test.yml`) that brings up a headless Z Monitor and the server simulator for scripted E2E scenarios.

Context: Useful for CI integration tests; compose should be lightweight and time-bounded.

Constraints:
- Use minimal images; in CI avoid running full GUI — run headless modes or test harness binaries.

Expected output:
- `docker-compose.test.yml`, `tests/e2e/run_scenarios.sh` that runs scenarios and asserts results.

Run/Verify:
- `docker-compose -f docker-compose.test.yml up --build` and run the scenario script to validate end-to-end flows.
