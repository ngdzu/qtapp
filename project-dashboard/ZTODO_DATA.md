# Z Monitor Development Tasks - DATA

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

---

## Data Management Tasks

- [ ] TASK-DATA-001: Implement Archiver interface and archiving tests
  - What: Implement `IArchiver` interface and tests that show moving rows older than 7 days to an external archive file. Add unit tests for retention policy enforcement.
  - Why: Archival is required by requirements; must be testable and configurable.
  - Prompt: `project-dashboard/prompt/18-implement-archiver-interface.md`
  - Subtasks:
    - [ ] Create documentation `doc/interfaces/DOC-API-031_iarchiver.md` defining the interface and archival format.
    - [ ] Implement `SQLiteArchiver` in `src/infrastructure/persistence/SQLiteArchiver.h/cpp` implementing `IArchiver`.
      - Should support archiving to a file (e.g., JSON or compressed SQLite).
      - Should delete archived records from the source table.
      - Should be transactional.
    - [ ] Create unit tests in `tests/unit/infrastructure/persistence/ArchiverTest.cpp`.
      - Verify data older than retention period is archived.
      - Verify data newer than retention period is kept.
      - Verify source data is deleted after successful archive.
    - [ ] Create `tools/archive_validator` tool to verify archive file integrity.
  - Verification:
    - [ ] **Functional:** Verify `archive()` moves data correctly and deletes it from source.
    - [ ] **Code Quality:** Run `clang-format` and `clang-tidy`. Ensure Doxygen comments.
    - [ ] **Documentation:** Verify `DOC-API-031_iarchiver.md` matches implementation.
    - [ ] **Integration:** Verify it works with `SQLiteVitalsRepository` (or mock).
    - [ ] **Tests:** Verify `ArchiverTest` passes.
