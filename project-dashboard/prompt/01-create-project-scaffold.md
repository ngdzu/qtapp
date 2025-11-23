Decision: Create canonical project scaffolding under `project-dashboard/` with minimal placeholder files so other tasks have a stable workspace.

Context: Repo root `project-dashboard/`. Other docs reference `src/`, `resources/`, `doc/`, `proto/`, `openapi/`, `tests/`, `central-server-simulator/`, `doc/migrations/`.

Constraints:
- Do not implement business logic; create skeletons and `.gitkeep` placeholders for empty directories.
- Provide minimal top-level `CMakeLists.txt` and `README.md` (short, pointing to next steps).

Expected output:
- New folders (if missing): `src/`, `resources/`, `doc/`, `proto/`, `openapi/`, `tests/`, `central-server-simulator/`, `doc/migrations/`.
- Files: top-level `CMakeLists.txt` (minimal comment), `README.md` (1-paragraph), `doc/migrations/README.md`.

Run/Verify:
- `ls project-dashboard/` shows the folders; open created README files.
