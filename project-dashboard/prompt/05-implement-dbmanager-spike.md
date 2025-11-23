Decision: Create a `DatabaseManagerStub` that uses in-memory SQLite to validate schema and migrations; add a CMake option `ENABLE_SQLCIPHER` documented for later.

Context: Use the migration SQLs from `doc/migrations/` to initialize `:memory:` DB and run smoke tests.

Constraints:
- Keep the stub small; no SQLCipher linking yet. Provide clear TODOs and documentation for enabling SQLCipher.

Expected output:
- `src/core/DatabaseManagerStub.cpp/.h`, `tests/db_smoke_test.cpp`, `CMakeLists` option `ENABLE_SQLCIPHER` documented.

Run/Verify:
- Build tests and run the smoke test to ensure migrations apply and basic inserts/queries succeed.
