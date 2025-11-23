Decision: Add `IArchiver` interface spec and tests that create archive packages (compressed sqlite or proto bundle) and simulate upload/purge cycles.

Context: Archiver moves data older than retention to archives; should support dry-run and safe purge.

Constraints:
- Ensure idempotency and resume-on-failure semantics; archive format must be versioned.

Expected output:
- `doc/interfaces/IArchiver.md`, `tests/archiver_tests.cpp`, and example archive creation code in `tools/` for validation.

Run/Verify:
- Run tests that create, verify, and purge archives in a temp directory.
