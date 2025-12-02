# Test Fixtures for Z Monitor

This folder contains shared GoogleTest fixtures used by integration tests.

- `DatabaseTestFixture`: Opens a shared in-memory SQLite database (`file::memory:?cache=shared`), creates a minimal or generated schema, and provides helpers:
  - `db()` returns the write connection
  - `databaseManager()` returns the `DatabaseManager*`
  - Ensures clean setup/teardown per test

- `RepositoryTestFixture`: Extends `DatabaseTestFixture` and is intended for repository tests. Add seeding helpers here (e.g., creating an admitted patient).

Schema application strategy:
- For focused tests, we apply only `schema/generated/ddl/create_tables.sql` to avoid unrelated failures, and we guarantee the `patients` table exists.
- When broader coverage is needed, extend the fixture to apply additional tables/indices incrementally per test.

Notes:
- Foreign keys are toggled safely within the fixture.
- Read/write connections share the same in-memory database via the SQLite URI.
