Decision: Add `scripts/generate-selfsigned-certs.sh` that creates a dev CA, server cert, and client cert for local testing; document PKCS#12 conversion and security notes in `central-server-simulator/certs/README.md`.

Context: Must be run only in dev; private keys must not be committed. Use `openssl` commands with clear subject values.

Constraints:
- Script must mark outputs as dev-only and produce readable filenames for integration with NetworkManager spike.

Expected output:
- `scripts/generate-selfsigned-certs.sh` and `central-server-simulator/certs/README.md` with instructions.

Run/Verify:
- Run script locally and verify it produces `ca.pem`, `server.crt`, `server.key`, `client.crt`, `client.key` and optional `.p12` files.
