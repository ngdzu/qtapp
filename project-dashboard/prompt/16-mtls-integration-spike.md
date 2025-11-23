Decision: Implement a small Qt C++ spike `network_mtls_spike` that loads dev client certs into `QSslConfiguration` and performs an HTTPS POST to the simulator to validate mutual TLS handshake.

Context: Use certs produced by the generate script; keep spike separate from production NetworkManager to avoid merging experimental code.

Constraints:
- Log handshake errors verbosely; run on macOS development host.

Expected output:
- `tools/network_mtls_spike/` with CMakeLists and sample code to perform an HTTPS POST with mTLS.

Run/Verify:
- Build and run spike program against running `central-server-simulator` configured to require client certs.
