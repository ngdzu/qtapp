# Z Monitor Development Tasks

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- Categories: INFRA (Infrastructure), DOM (Domain), APP (Application), UI (User Interface), DB (Database), NET (Networking), SEC (Security), TEST (Testing), PERF (Performance), DOC (Documentation), CONFIG (Configuration), DEPLOY (Deployment), PUBLISH (Publishing), I18N (Internationalization), A11Y (Accessibility), MONITOR (Monitoring), MAINT (Maintenance), REG (Regulatory), TRAIN (Training), DATA (Data Management)
- Number: Zero-padded 3-digit sequential (001, 002, ...)
- Examples: `TASK-INFRA-001`, `TASK-UI-042`, `TASK-DB-015`, `TASK-TEST-001`
- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

**Note:** Existing tasks without IDs are being migrated. New tasks MUST include IDs from creation.

---

## ⚠️ CRITICAL: API Documentation Required

**ALL CODE MUST INCLUDE DOXYGEN-STYLE COMMENTS FROM THE BEGINNING.**

- **Rule:** Every public class, method, property, and enum must be documented with Doxygen-style comments (`/** */`)
- **Guideline:** See `.cursor/rules/api_documentation.mdc` for complete documentation requirements
- **Reference:** See `project-dashboard/doc/guidelines/DOC-GUIDE-020_api_documentation.md` for API documentation strategy
- **Enforcement:** Code reviews will reject code without proper documentation

**Documentation is not optional - it is a required part of every public API.**

## ⚠️ CRITICAL: Verification Required

**ALL ZTODO ITEMS MUST BE VERIFIED BEFORE MARKING COMPLETE.**

- **Rule:** Every task must include verification steps and be verified before completion
- **Guideline:** See `.github/ztodo_verification.md` for complete verification workflow
- **Categories:** Functional, Code Quality, Documentation, Integration, Tests, Performance, QML (if applicable)
- **Enforcement:** Tasks cannot be marked complete without passing all verification steps
- **Build Verification:** ALL tasks that create/modify files MUST run `./scripts/verify-build.sh <files...>` to verify affected targets build
- **Code Quality Verification:** ALL tasks MUST run `./scripts/verify_code_quality.sh` to check lint and documentation compliance
  - The script automatically detects uncommitted files and performs language-specific lint checks (C++, QML, CMake, Markdown, Shell)
  - Validates Doxygen documentation completeness for all classes, functions, and properties
  - Checks spacing, indentation, trailing whitespace, and formatting consistency
  - Usage: `./scripts/verify_code_quality.sh` (auto-detects changes) or `./scripts/verify_code_quality.sh <file1> <file2>...` (specific files)
- **Status Tracking:** Update verification status in task using ✅ markers (e.g., "1. Functional: ✅ Verified - details")

**Verification is not optional - it is a required part of every task completion.**

**📋 Before starting ANY task, read `.github/ztodo_verification.md` to understand ALL verification requirements.**

---

## Task Categories

The Z Monitor development tasks have been split into categorized files for better manageability.

### Core Implementation
- [Infrastructure Tasks](ZTODO_INFRA.md) - Database, Hardware, Network, Logging
- [Domain Tasks](ZTODO_DOM.md) - Patient, Vitals, Alarms, Telemetry Models
- [Application Tasks](ZTODO_APP.md) - Managers, Services, Use Cases
- [UI Tasks](ZTODO_UI.md) - QML, Controllers, Views, Components
- [Database Tasks](ZTODO_DB.md) - Schema, Migrations, Repositories
- [Networking Tasks](ZTODO_NET.md) - Telemetry, REST APIs, WebSocket

### Quality & Verification
- [Testing Tasks](ZTODO_TEST.md) - Unit Tests, Integration Tests, Simulators
- [Performance Tasks](ZTODO_PERF.md) - Benchmarks, Profiling, Optimization
- [Security Tasks](ZTODO_SEC.md) - Encryption, Auth, Provisioning
- [Maintenance Tasks](ZTODO_MAINT.md) - Refactoring, Tech Debt, Cleanup

### Architecture & Design
- [Documentation Tasks](ZTODO_DOC.md) - Doxygen, Diagrams, Compliance
- [Configuration Tasks](ZTODO_CONFIG.md) - Runtime Settings, Feature Flags
- [Data Management Tasks](ZTODO_DATA.md) - Migration, Archival, Retention

---

## Workflow

1. **Pick a task** from one of the category files above.
2. **Follow the instructions** in the task description.
3. **Verify** the implementation using the verification steps.
4. **Mark complete** in the specific file.
