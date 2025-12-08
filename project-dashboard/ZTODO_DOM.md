# Z Monitor Development Tasks - DOM

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

---

## Domain Tasks

- [x] TASK-DOM-001: Implement PatientAggregate.cpp
  - What: Implement the `PatientAggregate` class methods in `z-monitor/src/domain/aggregates/PatientAggregate.cpp`. The header file `PatientAggregate.h` already exists. This implementation should cover:
    - Constructor logic (initializing with ID, MRN, Name).
    - Business logic for admitting, discharging, and transferring patients.
    - Validation logic (e.g., cannot admit an already admitted patient).
    - Domain event generation (e.g., `PatientAdmitted`, `PatientDischarged`).
    - Getters and setters for patient properties (Age, Gender, Allergies, etc.).
    - **Business Rules to Implement:**
      - **BR-PAT-001 (Unique Identity):** Patient must have unique ID and non-empty MRN.
      - **BR-PAT-002 (Admission):** Cannot admit a patient who is already admitted.
      - **BR-PAT-003 (Discharge):** Cannot discharge a patient who is not currently admitted.
      - **BR-PAT-004 (Transfer):** Cannot transfer a patient who is not currently admitted.
      - **BR-PAT-005 (Timestamps):** Discharge time must be > Admission time.
      - **BR-PAT-006 (Validation):** Name/MRN cannot be empty; DOB cannot be in future.
      - **BR-PAT-007 (Events):** State changes must emit corresponding domain events.
  - Why: **DOMAIN LOGIC:** The `PatientAggregate` is the core entity representing a patient in the system. Its implementation encapsulates all business rules related to patient management, ensuring data consistency and valid state transitions.
  - Files:
    - `z-monitor/src/domain/aggregates/PatientAggregate.cpp` (new)
    - `z-monitor/src/domain/aggregates/PatientAggregate.h` (existing - verify completeness)
    - `z-monitor/tests/unit/domain/aggregates/PatientAggregateTest.cpp` (new)
    - `z-monitor/src/domain/CMakeLists.txt` (modified - add .cpp file)
  - Acceptance:
    - `PatientAggregate` compiles and links.
    - All methods defined in the header are implemented.
    - Business rules are enforced (e.g., state transitions).
    - Unit tests verify all public methods and edge cases.
  - Verification Steps:
    1. Functional: Can create patient, admit, discharge, and update details. **Status:** ✅ Verified - Implemented and verified via unit tests.
    2. Code Quality: Follows DDD principles, Doxygen comments, no magic strings. **Status:** ✅ Verified - Code follows DDD, uses Result<T>, and has Doxygen comments. Fixed placement new issue.
    3. Documentation: Class behavior documented. **Status:** ✅ Verified - Header and implementation documented.
    4. Integration: Compiles and links with domain library. **Status:** ✅ Verified - Built successfully.
    5. Tests: Unit tests cover lifecycle and validation logic. **Status:** ✅ Verified - 14/14 tests passed in PatientAggregateTest.
  - Dependencies: 
    - `PatientAggregate.h` (existing)
    - Domain Event definitions (if emitting events)
  - Prompt: `project-dashboard/prompt/TASK-DOM-001-patient-aggregate-impl.md`
