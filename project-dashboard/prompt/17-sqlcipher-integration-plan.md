Decision: Research and document steps to integrate SQLCipher with CMake, and add a smoke-test target that opens an encrypted SQLite DB.

Context: SQLCipher integration may require building from source or using a system package; provide `ENABLE_SQLCIPHER` CMake option.

Constraints:
- Do not add SQLCipher as mandatory; keep build optional with clear docs and fallback to standard SQLite for CI.

Expected output:
- `doc/sqlcipher_integration.md` with CMake integration options and `tests/sqlcipher_smoke_test.cpp` when enabled.

Run/Verify:
- With SQLCipher enabled, run smoke test to open encrypted DB; without it, ensure fallback works.
