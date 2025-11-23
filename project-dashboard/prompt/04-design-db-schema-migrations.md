Decision: Produce initial SQL migration files under `doc/migrations/` including `0001_initial.sql` and `0002_indices.sql`, plus `doc/migrations/README.md` describing the migration numbering scheme.

Context: Schema must support tables: `patients`, `vitals`, `ecg_samples`, `pleth_samples`, `alarms`, `system_events`, `settings`, `users`, plus retention metadata and archival queue.

Constraints:
- Normalize where appropriate but optimize indices for trends queries (time + metric). Include `created_at` timestamps.
- Provide sample queries for trends extraction.

Expected output:
- `doc/migrations/0001_initial.sql`, `0002_indices.sql`, `doc/migrations/README.md`, and updated `doc/10_DATABASE_DESIGN.md` referencing migrations.

Run/Verify:
- Use sqlite3 to run migrations against `:memory:` DB and run sample queries to validate indices.
