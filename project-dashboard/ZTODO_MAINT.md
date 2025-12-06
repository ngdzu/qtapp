# Z Monitor Development Tasks - MAINT

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

---

## Maintenance Tasks

- [ ] TASK-MAINT-001: Standardize Error Handling
  - What: Adopt `Result<T, Error>` pattern across the codebase instead of exceptions or boolean returns.
  - Why: Provides explicit, type-safe error handling that forces developers to check for errors.
  - Files:
    - `z-monitor/src/common/Result.h` (create/update)
    - Refactor existing services to use Result
  - Acceptance: Core services return Result, errors propagated correctly.
  - Verification Steps:
    1. Functional: Error propagation works
    2. Code Quality: No ignored errors
    3. Documentation: Error handling pattern documented
    4. Integration: Refactored code compiles
    5. Tests: Unit tests for Result class
  - Prompt: `project-dashboard/prompt/56-standardize-error-handling.md`

- [ ] TASK-MAINT-002: Refactor Code Organization
  - What: Enforce strict DDD folder structure (Domain, Application, Infrastructure, UI). Move any misplaced files.
  - Why: Maintains architectural integrity and prevents circular dependencies.
  - Files:
    - Move files as needed
    - Update `CMakeLists.txt`
  - Acceptance: All files in correct layers, no dependency violations.
  - Verification Steps:
    1. Functional: App builds and runs
    2. Code Quality: Dependency check
    3. Documentation: Structure documented
    4. Integration: CMake updated
    5. Tests: All tests pass
  - Prompt: `project-dashboard/prompt/58-refactor-code-organization.md`
