Decision: Create a lightweight central server simulator (`central-server-simulator/app.py`) exposing `POST /api/telemetry` and toggles to simulate 200/500/delay; include dev certs README.

Context: Simulator will be used in local E2E tests; mTLS support added later.

Constraints:
- Keep dependency footprint small (Flask). Provide toggles via query params or a simple UI.

Expected output:
- `central-server-simulator/app.py`, `central-server-simulator/requirements.txt`, `central-server-simulator/README.md`, and `central-server-simulator/certs/README.md` (dev-only certs guidance).

Run/Verify:
- Start simulator and POST sample telemetry JSON to `/api/telemetry` to receive ack; toggle failure modes to test retries.
