---
alwaysApply: true
---

# ZTODO Task Guidelines

## Overview

This document provides guidelines for creating well-structured, actionable tasks in `ZTODO.md`. Following these guidelines ensures tasks are clear, complete, and verifiable.

## Critical Rules

1. **Every task MUST have a unique ID** - Use format: `TASK-{CATEGORY}-{NUMBER}` (e.g., TASK-INF-001, TASK-UI-042)
2. **Every task MUST be verifiable** - Include explicit verification steps
3. **Every task MUST reference verification doc** - Link to `.github/ztodo_verification.md`
4. **Every task MUST have acceptance criteria** - Clear definition of "done"
5. **Every task MUST list affected files** - What will be created/modified
6. **Every task MUST explain WHY** - Business/technical rationale
7. **Every task MUST avoid code duplication** - Follow DRY principle (see [DRY.md](./DRY.md))

## Task ID Format

**Format:** `TASK-{CATEGORY}-{NUMBER}`

**Categories:**
- `INFRA` - Infrastructure (build system, CMake, vcpkg, CI/CD, deployment pipelines)
- `DOM` - Domain (aggregates, value objects, domain events, business logic)
- `APP` - Application (services, use cases, application layer)
- `UI` - User Interface (QML, controllers, views, components)
- `DB` - Database (schema, migrations, repositories, ORM)
- `NET` - Networking (telemetry, REST APIs, WebSocket, gRPC)
- `SEC` - Security (authentication, authorization, encryption, certificates)
- `TEST` - Testing (unit tests, integration tests, mocks, simulators, test harnesses)
- `PERF` - Performance (benchmarks, profiling, optimization, load testing)
- `DOC` - Documentation (design docs, API docs, architecture diagrams)
- `CONFIG` - Configuration (runtime settings, feature flags, environment-specific configs)
- `DEPLOY` - Deployment (packaging, installers, Docker images, Kubernetes)
- `PUBLISH` - Publishing (app store submissions, release management, distribution)
- `I18N` - Internationalization (translations, localization, language support)
- `A11Y` - Accessibility (screen readers, keyboard navigation, WCAG compliance)
- `MONITOR` - Monitoring (application metrics, logging, tracing, alerts)
- `MAINT` - Maintenance (refactoring, tech debt, cleanup, deprecation)
- `REG` - Regulatory (HIPAA, FDA 510(k), IEC 62304, audit trails, validation)
- `TRAIN` - Training (user guides, tutorials, onboarding materials)
- `DATA` - Data Management (migration, archival, retention, backup/restore)

**Number:** Zero-padded 3-digit sequential number (001, 002, ..., 999)

**Examples:**
- `TASK-INFRA-001` - First infrastructure task (CMake setup)
- `TASK-UI-042` - 42nd UI task (new QML component)
- `TASK-DB-015` - 15th database task (schema migration)
- `TASK-TEST-001` - First testing task (test harness setup)
- `TASK-DEPLOY-003` - Third deployment task (Docker configuration)
- `TASK-PUBLISH-001` - First publishing task (app store submission)
- `TASK-REG-005` - Fifth regulatory task (FDA documentation)
- `TASK-I18N-002` - Second internationalization task (Spanish translation)

**Guidelines:**
- Assign IDs when creating tasks (sequential within category)
- IDs are permanent - don't reuse after task completion
- Use IDs in commit messages, PR titles, and documentation
- Reference related tasks using IDs (e.g., "Depends on TASK-DB-003")
- Include ID in prompt filename: `{task-id}-{brief-name}.md`

### Category Selection Guide

**When to use each category:**

**Core Development:**
- `INFRA` - Build systems (CMake, vcpkg), CI/CD pipelines, project scaffolding, logging infrastructure
- `DOM` - Business rules, aggregates, entities, value objects, domain events
- `APP` - Use cases, application services, orchestration logic
- `UI` - QML views, controllers, components, user interaction
- `DB` - Schema design, migrations, repositories, query optimization

**External Integration:**
- `NET` - REST APIs, WebSocket, gRPC, telemetry transmission
- `SEC` - Authentication, encryption, certificates, secure communication

**Quality & Verification:**
- `TEST` - Unit tests, integration tests, E2E tests, mocks, simulators, test harnesses
- `PERF` - Benchmarks, profiling, optimization, performance tuning, load testing
- `DOC` - Architecture docs, API documentation, user guides

**Release & Operations:**
- `DEPLOY` - Packaging, Docker images, Kubernetes manifests, installers (`.dmg`, `.exe`, `.msi`)
- `PUBLISH` - App Store submission, Google Play upload, version numbering, release notes
- `MONITOR` - Runtime metrics, logging aggregation, alerting, health checks, observability
- `CONFIG` - Runtime configuration (feature flags, user preferences, environment-specific settings)

**Specialized Concerns:**
- `I18N` - Translation files (`.ts`, `.qm`), locale support, RTL layouts
- `A11Y` - ARIA labels, keyboard navigation, screen reader support, contrast ratios
- `REG` - FDA 510(k), HIPAA compliance, IEC 62304, audit trails, validation
- `DATA` - Data archival, backup/restore, retention policies, migration scripts
- `MAINT` - Refactoring, code cleanup, dependency updates, deprecation
- `TRAIN` - User manuals, training videos, interactive tutorials, quick-start guides

**Example Task IDs by Category:**
```markdown
TASK-INFRA-001: Setup vcpkg Integration for Dependency Management
TASK-TEST-002: Implement Mock Sensor Simulator for Offline Testing
TASK-DEPLOY-001: Create Multi-stage Dockerfile for Production Builds
TASK-PUBLISH-001: Prepare App Store Submission (Screenshots, Metadata, Privacy Policy)
TASK-I18N-001: Implement Translation Infrastructure (Qt Linguist Integration)
TASK-A11Y-001: Add ARIA Labels and Keyboard Navigation to All QML Views
TASK-MONITOR-001: Integrate Prometheus Metrics Exporter for Device Telemetry
TASK-CONFIG-001: Implement Feature Flag System with Runtime Toggle
TASK-REG-001: Generate FDA 510(k) Software Documentation Package
TASK-DATA-001: Implement 90-Day Data Archival with Encrypted Export
TASK-MAINT-001: Refactor Alarm Detection Path (Remove Technical Debt)
TASK-TRAIN-001: Create Interactive Onboarding Tutorial for New Users
```

## Handling Multi-Category Tasks

**Question:** What if a task touches multiple categories (e.g., database + infrastructure + UI)?

**Answer:** Break it down! Tasks should follow the **Single Responsibility Principle**.

### Why Break Down Multi-Category Tasks?

1. **Clearer Ownership** - Each task has one clear domain/owner
2. **Better for AI Execution** - Smaller, focused tasks are easier to execute correctly
3. **Explicit Dependencies** - Breaking down reveals true dependencies between tasks
4. **Parallel Work** - Different team members or AI sessions can work simultaneously
5. **Better Progress Tracking** - Track completion by category/domain
6. **Easier Verification** - Focused tasks have clearer acceptance criteria

### Strategy 1: Choose Primary Category + List Dependencies

Identify the **main deliverable** and use that category. List other tasks as dependencies:

**Example:**
```markdown
- [ ] TASK-UI-011: Implement Patient Admission Workflow UI
  - What: Create AdmissionModal.qml, wire PatientController, display patient data from service
  - Why: Provides user interface for admitting patients to the monitoring system
  - Files: 
    - `src/interface/qml/views/AdmissionModal.qml` (new)
    - `src/interface/controllers/PatientController.cpp/h` (modified)
  - Dependencies: 
    - TASK-DB-005: patients table schema must exist
    - TASK-INFRA-027: PatientRepository must be implemented
    - TASK-APP-002: AdmissionService must be implemented
  - Acceptance: UI displays patient data, admission button triggers service call, success/error states shown
```

**When to use:** Task primarily delivers one artifact (UI, service, schema) but needs other components to exist first.

### Strategy 2: Break into Sequential Tasks

Create separate tasks that must be done in order, from infrastructure up to UI:

**Example:**
```markdown
## Patient Admission Feature (Sequential Tasks)

- [ ] TASK-DB-005: Create patients table schema and migration
  - What: Define patients table with columns: id, mrn, name, dob, allergies, admitted_at, discharged_at
  - Files: `schema/migrations/0005_create_patients.sql`
  - Acceptance: Migration runs successfully, table created with indexes

- [ ] TASK-INFRA-027: Implement PatientRepository (depends on TASK-DB-005)
  - What: Implement IPatientRepository interface with CRUD operations
  - Files: `src/infrastructure/persistence/PatientRepository.cpp/h`
  - Acceptance: Can create, read, update, delete patient records

- [ ] TASK-APP-002: Implement AdmissionService (depends on TASK-INFRA-027)
  - What: Implement admission/discharge/transfer business logic using PatientRepository
  - Files: `src/application/services/AdmissionService.cpp/h`
  - Acceptance: Admission workflow validates data, persists patient, logs action

- [ ] TASK-UI-011: Create AdmissionModal.qml (depends on TASK-APP-002)
  - What: Create QML UI for patient admission form
  - Files: `src/interface/qml/views/AdmissionModal.qml`
  - Acceptance: Form displays, validation works, calls AdmissionService on submit
```

**When to use:** Feature requires layered implementation (database → repository → service → UI) where each layer depends on the previous one.

### Strategy 3: Parent Task with Categorized Subtasks

For truly integrated features, create a parent task with subtasks that have their own IDs:

**Example:**
```markdown
- [ ] TASK-APP-002: Implement Complete Patient Admission Feature
  - What: End-to-end patient admission capability including database, services, and UI
  - Why: Core requirement for ADT workflow - admits patients to monitoring system
  - Subtasks (must complete in order):
    - [ ] TASK-DB-005: Create patients table schema and migration
    - [ ] TASK-INFRA-027: Implement PatientRepository with CRUD operations
    - [ ] TASK-APP-002a: Implement AdmissionService business logic
    - [ ] TASK-UI-011: Create AdmissionModal.qml and wire to controller
    - [ ] TASK-TEST-015: Add integration tests for complete admission workflow
  - Acceptance: User can admit patient via UI, data persists to database, action logged
  - Verification: All subtasks verified individually + end-to-end integration test passes
  - Note: Parent task cannot be marked complete until ALL subtasks are complete and verified
```

**When to use:** Large feature that must be delivered as a cohesive unit, but internal implementation spans multiple categories.

### Strategy 4: Use "Integration" Task as Final Step

Break down by category, then add final integration task:

**Example:**
```markdown
## Patient Admission Feature

- [ ] TASK-DB-005: Create patients table schema
- [ ] TASK-INFRA-027: Implement PatientRepository  
- [ ] TASK-APP-002: Implement AdmissionService
- [ ] TASK-UI-011: Create AdmissionModal.qml
- [ ] TASK-TEST-015: Integration test for complete admission workflow (depends on all above)
  - What: E2E test that verifies UI → Service → Repository → Database flow
  - Why: Validates all components work together correctly
  - Acceptance: Test covers happy path, error cases, and edge cases
```

**When to use:** Tasks can be developed independently but need final integration verification.

### Decision Tree: How to Handle Multi-Category Tasks

```
Is the task primarily about ONE deliverable (UI, service, schema)?
├─ YES → Use Strategy 1: Primary category + Dependencies
└─ NO → Does it span multiple layers (DB → Service → UI)?
    ├─ YES → Are layers independent?
    │   ├─ YES → Use Strategy 2: Sequential Tasks
    │   └─ NO → Use Strategy 3: Parent + Subtasks
    └─ NO → Use Strategy 4: Separate tasks + Integration test
```

### Real Example from ZTODO.md

**Good decomposition already in place:**

```markdown
# ADT Workflow (well-decomposed)
- [ ] TASK-INFRA-022: Implement LocalPatientService (Development IPatientLookupService)
- [ ] TASK-INFRA-023: Add Development Mode Macro & CMake Option  
- [ ] TASK-APP-001: Implement AdmissionService with IPatientLookupService integration
- [ ] TASK-UI-006: Create ADT UI Components (AdmissionModal.qml & PatientBanner.qml)
- [ ] TASK-UI-007: Implement Barcode Scanning Abstraction (IAdmissionMethod + MockBarcodeScanner)
- [ ] TASK-UI-008: Wire PatientController to AdmissionService & ADT UI
- [ ] TASK-TEST-013: Implement ADT Workflow Tests (Unit + Integration + QML)
- [ ] TASK-DB-002: Sample Patient Seed & Migration
- [ ] TASK-DOC-006: Update ADT Documentation & Architecture References
```

**Why this works well:**
- Each task has a single, clear category
- Dependencies are implicit from ordering (infrastructure → application → UI → tests)
- Each task can be worked on independently once dependencies are met
- Easy to track progress: "All INFRA tasks for ADT done, now working on APP layer"

### Anti-Pattern: Avoid Single Monolithic Tasks

**❌ Bad Example:**
```markdown
- [ ] TASK-???-001: Implement Patient Admission Feature
  - What: Create database schema, repository, service, UI, tests, and documentation
  - Files: (50+ files across all layers)
  - Acceptance: Everything works
```

**Problems:**
- Unclear which category to use
- Too large for single work session
- Hard to verify incrementally
- Blocks parallel work
- Difficult to estimate effort
- Unclear what "done" means

**✅ Good Example:** Break into 9 focused tasks (shown above in Real Example)

### Key Principle

**"If you're unsure which category a task belongs to, it's probably too big."**

Break it down until each task has an obvious primary category.

## Task Structure Template

```markdown
- [ ] TASK-{CATEGORY}-{NUMBER}: Task Title (Brief, Action-Oriented, 3-7 words)
  - What: Detailed description of what to implement/modify/create
  - Why: Business/technical rationale - why is this needed?
  - Files: Explicit list of files to create/modify
  - Acceptance: Clear, testable criteria for completion
  - Dependencies: (Optional) Prerequisites or blockers (reference task IDs)
  - Subtasks: (Optional) Break down complex tasks
    - [ ] Subtask 1
    - [ ] Subtask 2
  - Verification Steps:
    1. Functional: [Specific functional checks]
    2. Code Quality: [Linter, build, documentation checks]
    3. Documentation: [Doc updates required]
    4. Integration: [Build and test requirements]
    5. Tests: [Test requirements and coverage]
    6. Performance: (If applicable) [Performance requirements]
    7. QML: (If applicable) [QML-specific checks]
  - Prompt: `project-dashboard/prompt/{task-id}-{brief-name}.md`
```

## Section Breakdown

### Task Title

**Format:** `TASK-{CATEGORY}-{NUMBER}: {Action Verb} {Component} ({Context})`

**Good Examples:**
- ✅ "TASK-INF-015: Implement LocalPatientService (Development IPatientLookupService)"
- ✅ "TASK-INF-016: Add Development Mode Macro & CMake Option"
- ✅ "TASK-UI-023: Create ADT UI Components (AdmissionModal.qml & PatientBanner.qml)"
- ✅ "TASK-DB-008: Fix Database Migration Transaction Handling"

**Bad Examples:**
- ❌ "Work on patient stuff" (no ID, too vague)
- ❌ "TASK-XYZ-001: Fix bugs" (invalid category, not specific)
- ❌ "Update code to make it better" (no ID, no concrete action)

**Guidelines:**
- Start with action verb (Implement, Create, Add, Fix, Refactor, Update)
- Include key component/artifact name
- Use parentheses for context/clarification
- Keep to 3-7 words when possible
- Be specific and concrete

### What Section

**Purpose:** Detailed technical description of the work to be done.

**Structure:**
1. **Primary objective** - What is being built/modified
2. **Technical details** - Classes, interfaces, methods to implement
3. **Key requirements** - Critical features or constraints
4. **Integration points** - How it connects to existing code

**Example:**
```markdown
- What: Implement `LocalPatientService` class in `src/infrastructure/patient/LocalPatientService.cpp/h` 
  that implements `IPatientLookupService` interface. Provides seeded patient roster (≥10 patients), 
  supports MRN and fuzzy name search, includes latency simulation (0/25/100ms configurable), and 
  failure mode testing. Returns `Result<PatientInfo, Error>` for all operations.
```

**Guidelines:**
- Start with the main artifact being created/modified
- Specify exact file paths
- List interfaces being implemented
- Include quantifiable requirements (e.g., "≥10 patients", "< 50ms latency")
- Mention return types and error handling approach
- Reference design patterns used (e.g., "Strategy pattern", "Repository pattern")

### Why Section

**Purpose:** Business and technical rationale - why this work matters.

**Structure:**
1. **Business value** - User/stakeholder benefit
2. **Technical benefit** - Architecture/quality improvement
3. **Alignment** - How it fits into larger goals

**Example:**
```markdown
- Why: Enables offline development and admission workflow without HIS/EHR dependency; supports UI 
  prototyping and automated testing with realistic patient data; follows DDD principles by separating 
  domain interfaces from infrastructure implementations.
```

**Guidelines:**
- Answer "why now?" and "why this approach?"
- Connect to user needs or system requirements
- Reference architecture principles (DDD, SOLID, etc.)
- Mention testing/development benefits
- Link to compliance or regulatory needs if applicable

### Files Section

**Purpose:** Explicit list of ALL files that will be created, modified, or deleted.

**Structure:**
1. **New files** - List with full paths from project root
2. **Modified files** - Existing files that will change
3. **Deleted files** - (If applicable) Files being removed
4. **Documentation** - Docs that need updates

**Example:**
```markdown
- Files: 
  - `src/infrastructure/patient/LocalPatientService.h` (new)
  - `src/infrastructure/patient/LocalPatientService.cpp` (new)
  - `schema/sample/patients_seed.sql` (new)
  - `src/infrastructure/CMakeLists.txt` (modified - add new sources)
  - `doc/z-monitor/architecture_and_design/19_ADT_WORKFLOW.md` (modified - add Development Lookup section)
  - `tests/unit/infrastructure/LocalPatientServiceTest.cpp` (new)
```

**Guidelines:**
- Use relative paths from project root
- Mark each file as (new), (modified), or (deleted)
- Add brief note for modified files explaining what changes
- Include test files
- Include documentation updates
- Include CMakeLists.txt if adding new sources
- Include resource files (.qml, .qrc) if applicable

### Acceptance Section

**Purpose:** Clear, testable criteria that define when the task is complete.

**Structure:**
- List of concrete, verifiable statements
- Each criterion should be testable (manual or automated)
- Include both functional and non-functional requirements

**Example:**
```markdown
- Acceptance: 
  - Provides ≥10 realistic sample patients with varied allergies and bed locations
  - MRN lookup completes successfully in < 50ms (cache hit)
  - Fuzzy name search (case-insensitive substring) returns correct matches
  - Latency simulation toggle works (0ms / 25ms / 100ms via constructor param)
  - All public methods return `Result<T, Error>` with meaningful error messages
  - All public APIs have complete Doxygen documentation
  - Unit tests achieve ≥80% code coverage
  - Integrates with AdmissionService via dependency injection
```

**Guidelines:**
- Use quantifiable criteria where possible (≥80%, < 50ms, etc.)
- Include performance requirements
- Specify error handling expectations
- Mention documentation requirements
- Include integration points
- Reference test coverage targets
- Use ✅/❌ testable assertions

### Dependencies Section (Optional)

**Purpose:** List prerequisites and blockers using task IDs for traceability.

**Example:**
```markdown
- Dependencies: 
  - TASK-DOM-005: `IPatientLookupService` interface must be defined
  - TASK-DB-012: Database schema must include `patients` table
  - TASK-INF-016: CMake dev mode option (`Z_MONITOR_DEV_MODE`) must be implemented
  - Blocks: TASK-INF-017 (Inject LocalPatientService in main.cpp)
  - External: Qt 6.5+, CMake 3.20+
```

**Guidelines:**
- Use task IDs to reference internal dependencies
- Clearly mark "Blocks" vs "Depends on"
- List external dependencies (libraries, tools)
- Note if task can proceed in parallel
- Include version requirements for external deps

### Subtasks Section (Optional)

**Purpose:** Break down complex tasks into manageable pieces.

**When to use:**
- Task spans multiple files/components
- Task has distinct phases (design → implement → test)
- Task involves multiple team members
- Estimated effort > 4 hours

**Example:**
```markdown
- Subtasks:
  - [ ] Define `LocalPatientService` class interface
  - [ ] Implement patient roster loading from seed SQL
  - [ ] Implement MRN lookup with caching
  - [ ] Implement fuzzy name search algorithm
  - [ ] Add latency simulation mechanism
  - [ ] Add Doxygen documentation for all public APIs
  - [ ] Implement unit tests (≥80% coverage)
  - [ ] Update CMakeLists.txt to include new sources
```

**Guidelines:**
- Each subtask is a distinct, completable unit
- Order subtasks logically (dependencies first)
- Use checkboxes `- [ ]` for tracking
- Keep subtasks focused (1-2 hours max)
- Mark completed subtasks with `- [x]`

### Verification Steps Section (REQUIRED)

**Purpose:** Explicit checklist following `.github/ztodo_verification.md` guidelines.

**Must include these categories:**
1. **Functional** - Does it work as specified?
2. **Code Quality** - Does it meet standards?
3. **Documentation** - Are docs updated?
4. **Integration** - Does it integrate correctly?
5. **Tests** - Do tests pass?
6. **Performance** - (If applicable) Does it meet performance targets?
7. **QML** - (If applicable) Does QML code meet standards?

**Example:**
```markdown
- Verification Steps:
  1. Functional: MRN lookup succeeds; fuzzy search returns correct results; latency simulation validated; error cases handled gracefully
  2. Code Quality: clang-format/tidy clean; no compiler warnings; Doxygen comments complete; `./scripts/verify-build.sh` passes for affected files
  3. Documentation: `19_ADT_WORKFLOW.md` updated with Development Lookup section; cross-references validated; examples tested
  4. Integration: Builds with z_monitor_infrastructure target; works with AdmissionService via DI; no breaking changes
  5. Tests: Unit tests pass; ≥80% coverage; MRN hit/miss cases tested; fuzzy search tested; failure simulation tested
  6. Performance: Lookup completes in < 50ms; no memory leaks (AddressSanitizer)
```

**Guidelines:**
- Each category must have specific checks (not just "verify X")
- Reference specific tools (`clang-format`, `verify-build.sh`, etc.)
- Include file paths for documentation updates
- Specify coverage targets
- Mention specific test scenarios
- Update status as you complete each step: `1. Functional: ✅ Verified - all requirements met`

**📋 See `.github/ztodo_verification.md` for complete verification requirements and checklist templates.**

### Prompt Section (REQUIRED)

**Purpose:** Link to detailed implementation prompt file.

**Format:**
```markdown
- Prompt: `project-dashboard/prompt/{task-id}-{brief-name}.md`
```

**Examples:**
- `project-dashboard/prompt/TASK-INFRA-015-local-patient-service.md`
- `project-dashboard/prompt/TASK-INFRA-016-dev-mode-macro.md`
- `project-dashboard/prompt/TASK-TEST-002-mock-sensor-simulator.md`
- `project-dashboard/prompt/TASK-TEST-001-verify-build-script.md`

**Guidelines:**
- Use consistent naming: `{task-id}-{brief-kebab-case-name}.md`
- Place all prompts in `project-dashboard/prompt/`
- Create prompt file BEFORE starting implementation
- Prompt should be comprehensive enough to execute task independently
- Include task ID in first line of prompt file

## Task Categorization

### Sequential Tasks
Tasks that **must** be done in order due to hard dependencies.

```markdown
## Sequential Tasks (must be done in order)

- [x] Task A (blocks Task B)
- [ ] Task B (depends on Task A)
```

### Parallel Tasks
Tasks that **can** be done concurrently without conflicts.

```markdown
## Parallel Tasks (can be done concurrently)

- [ ] Task X (independent)
- [ ] Task Y (independent)
```

### By Priority
Use section headers to organize by priority:

```markdown
## Infrastructure Foundation (Early Priority)
## Core Features (High Priority)
## Testing & Quality Foundations
## Documentation & Compliance
```

## Special Task Types

### Build/Infrastructure Tasks

**Additional Requirements:**
- Always include build verification: `./scripts/verify-build.sh <files...>`
- List CMakeLists.txt changes explicitly
- Mention target names being modified
- Include build matrix testing (dev/release modes)

**Example:**
```markdown
- Files:
  - `src/infrastructure/CMakeLists.txt` (modified - add LocalPatientService to z_monitor_infrastructure target)
  - `CMakeLists.txt` (modified - add Z_MONITOR_DEV_MODE option)
- Verification Steps:
  2. Code Quality: `./scripts/verify-build.sh src/infrastructure/patient/LocalPatientService.{h,cpp}` passes
```

### QML/UI Tasks

**Additional Requirements:**
- Include qmllint verification
- Specify accessibility requirements
- Mention responsive layout testing
- Include screenshot/visual verification

**Example:**
```markdown
- Verification Steps:
  7. QML: No qmllint errors; no binding loops; accessibility labels present; tested on 1280x800 and 1920x1080
```

### Performance-Critical Tasks

**Additional Requirements:**
- Specify exact performance targets
- Include benchmark verification
- Mention profiling requirements
- Include memory leak checks

**Example:**
```markdown
- Acceptance:
  - Alarm detection latency < 50ms (p99)
  - Memory usage ≤ 400 MB for 3-day cache
- Verification Steps:
  6. Performance: Benchmark shows < 50ms latency; no regressions vs baseline; no memory leaks (AddressSanitizer)
```

### Database/Schema Tasks

**Additional Requirements:**
- Include migration SQL files
- Specify schema version changes
- Include rollback plan
- Mention data migration if applicable

**Example:**
```markdown
- Files:
  - `schema/migrations/0004_add_admission_events.sql` (new)
  - `schema/database.yaml` (modified - add admission_events table)
  - `schema/rollback/0004_rollback.sql` (new)
- Verification Steps:
  1. Functional: Migration runs successfully; rollback restores previous state; data integrity maintained
```

### Documentation Tasks

**Additional Requirements:**
- Include diagram updates (Mermaid)
- Specify cross-reference validation
- Include example testing
- Mention API doc generation

**Example:**
```markdown
- Files:
  - `doc/z-monitor/architecture_and_design/19_ADT_WORKFLOW.md` (modified)
  - `doc/diagrams/adt_flow.mmd` (new)
- Verification Steps:
  3. Documentation: Cross-references validated (no broken links); Mermaid diagram renders; examples tested
```

## Common Pitfalls to Avoid

### ❌ Vague Task Descriptions
```markdown
- [ ] Fix patient management
  - What: Update patient code
  - Why: It needs to be better
```

### ✅ Specific Task Descriptions
```markdown
- [ ] Implement AdmissionService with IPatientLookupService integration
  - What: Create AdmissionService class implementing patient admission/discharge/transfer workflows 
    with lookup service integration and caching via IPatientRepository
  - Why: Centralizes ADT business logic; enables offline development; supports audit trail
```

### ❌ Missing Acceptance Criteria
```markdown
- Acceptance: Should work correctly
```

### ✅ Clear Acceptance Criteria
```markdown
- Acceptance: 
  - Admission persists patient data to database
  - Discharge updates patient.discharged_at timestamp
  - All operations log to action_log table
  - Returns Result<void, Error> for all operations
  - ≥85% test coverage
```

### ❌ Incomplete File List
```markdown
- Files: PatientService.cpp
```

### ✅ Complete File List
```markdown
- Files:
  - `src/application/services/AdmissionService.h` (new)
  - `src/application/services/AdmissionService.cpp` (new)
  - `src/application/CMakeLists.txt` (modified - add AdmissionService)
  - `tests/unit/application/AdmissionServiceTest.cpp` (new)
  - `doc/z-monitor/architecture_and_design/19_ADT_WORKFLOW.md` (modified - add workflow diagram)
```

### ❌ Generic Verification Steps
```markdown
- Verification Steps:
  1. Functional: Test it
  2. Code Quality: Make sure it's good
  3. Tests: Add tests
```

### ✅ Specific Verification Steps
```markdown
- Verification Steps:
  1. Functional: Admission workflow completes; discharge updates database; transfer creates log entries; error cases return proper Result
  2. Code Quality: No warnings (-Wall -Wextra); Doxygen complete; `./scripts/verify-build.sh src/application/services/AdmissionService.{h,cpp}` passes
  3. Documentation: `19_ADT_WORKFLOW.md` section 5 updated; workflow diagram added; cross-refs validated
  4. Integration: Builds with z_monitor_application; PatientController integration works; no breaking changes
  5. Tests: Unit tests ≥85% coverage; admit/discharge/transfer scenarios pass; error paths tested
```

## Verification Status Tracking

As you complete each verification step, update the task:

**Before:**
```markdown
- Verification Steps:
  1. Functional: Test admission workflow
  2. Code Quality: Run linters
```

**After (in progress):**
```markdown
- Verification Steps:
  1. Functional: ✅ Verified - Admission/discharge/transfer all work; error cases handled
  2. Code Quality: ⏳ In Progress - Linters pass, working on Doxygen comments
  3. Documentation: ⏳ Not Started
```

**After (complete):**
```markdown
- Verification Steps:
  1. Functional: ✅ Verified - Admission/discharge/transfer all work; error cases handled
  2. Code Quality: ✅ Verified - No warnings; Doxygen complete; build verification passed
  3. Documentation: ✅ Verified - ADT workflow doc updated; diagram added and renders
  4. Integration: ✅ Verified - All tests pass; no breaking changes
  5. Tests: ✅ Verified - 87% coverage; all scenarios tested
```

## Task Naming Conventions

### Pattern: `{Verb} {Component} ({Context})`

**Verbs:**
- Implement - New code/feature
- Create - New artifact/file
- Add - Extend existing
- Fix - Bug fix
- Refactor - Code improvement
- Update - Modify existing
- Remove - Delete obsolete code

**Component:**
- Specific class/service/module name
- File name for single-file changes

**Context (optional):**
- Parent system
- Technology/framework
- Implementation approach

**Examples:**
- Implement LocalPatientService (Development IPatientLookupService)
- Create ADT UI Components (AdmissionModal.qml & PatientBanner.qml)
- Add Development Mode Macro & CMake Option
- Fix Database Migration Transaction Handling
- Refactor Settings: Remove Bed ID, Add Device Label
- Update QML UI to display live sensor data with waveform rendering

## Integration with AI Assistant

When creating tasks for AI execution:

1. **Be explicit** - AI needs clear, unambiguous instructions
2. **Include examples** - Show expected output/behavior
3. **Reference docs** - Link to architecture/design documents
4. **Specify tools** - Mention exact commands/scripts to run
5. **Define success** - Clear acceptance criteria AI can verify

## Checklist for Task Creation

Before adding a task to ZTODO.md:

- [ ] Task has unique ID in format TASK-{CATEGORY}-{NUMBER}
- [ ] Title is action-oriented and specific (3-7 words)
- [ ] "What" section provides technical details
- [ ] "Why" section explains business/technical rationale
- [ ] "Files" section lists ALL affected files with paths
- [ ] "Acceptance" section has testable criteria
- [ ] "Verification Steps" covers all 5+ categories
- [ ] "Prompt" file path is specified with task ID
- [ ] Subtasks are included if task is complex (>4 hours)
- [ ] Dependencies reference task IDs if applicable
- [ ] Task references `.github/ztodo_verification.md`
- [ ] Performance requirements specified if applicable
- [ ] QML verification included if UI task
- [ ] Build verification included if code changes

## Related Documents

- `.github/ztodo_verification.md` - Complete verification workflow and requirements
- `project-dashboard/ZTODO.md` - Active task list
- `project-dashboard/prompt/` - Task-specific implementation prompts
- `.github/copilot-instructions.md` - Coding standards and workflow

---

**Remember: A well-written task is half the implementation. Invest time in clear, complete task descriptions to ensure successful execution.**
