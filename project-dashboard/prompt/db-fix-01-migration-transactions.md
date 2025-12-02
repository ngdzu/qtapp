# Database Migration Transaction Handling

## Overview
This document describes the approach for handling database migration transactions in Z Monitor.

## Approach
- **All migration files are executed inside a programmatic transaction using Qt's QSqlDatabase API.**
- **Migration SQL files MUST NOT contain explicit `BEGIN`, `COMMIT`, or `ROLLBACK` statements.**
- Any transaction commands found in migration files are ignored by the migration runner.
- This ensures atomic migration execution and prevents partial schema updates.

## Rationale
- Programmatic transaction handling guarantees that each migration is either fully applied or fully rolled back.
- Avoids issues with nested or conflicting transaction commands in SQL files.
- Simplifies migration file authoring: developers do not need to manage transactions manually.

## Implementation Details
- See `DatabaseManager::executeMigrations()` in `src/infrastructure/persistence/DatabaseManager.cpp/h`.
- The migration runner splits SQL files into statements and executes each inside a transaction.
- If any statement fails, the transaction is rolled back and migration is aborted.

## Migration File Requirements
- Do not include explicit transaction commands (`BEGIN`, `COMMIT`, `ROLLBACK`) in migration SQL files.
- Use standard DDL and DML statements only.

## Verification
- Migration tests must pass with programmatic transaction handling.
- Build and CI must succeed after migration changes.

---

_Last updated: 2025-12-02_
