Decision: Update `doc/10_DATABASE_DESIGN.md` to include final DDL, ERD SVG, index rationale, and sample trend queries.

Context: Use migration SQL files and earlier `10_DATABASE_DESIGN_EXT.md` as source material.

Constraints:
- Keep ERD small and readable; include notes on retention and archival tables.

Expected output:
- Updated `doc/10_DATABASE_DESIGN.md` and generated `doc/10_DATABASE_DESIGN.svg` ERD file.

Run/Verify:
- Render ERD and run sample SQL queries against a temp DB to verify correctness.
