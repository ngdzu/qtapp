# Z Monitor Development Tasks - NET


## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- Categories: INFRA (Infrastructure), DOM (Domain), APP (Application), UI (User Interface), DB (Database), NET (Networking), SEC (Security), TEST (Testing), PERF (Performance), DOC (Documentation), CONFIG (Configuration), DEPLOY (Deployment), PUBLISH (Publishing), I18N (Internationalization), A11Y (Accessibility), MONITOR (Monitoring), MAINT (Maintenance), REG (Regulatory), TRAIN (Training), DATA (Data Management)
- Number: Zero-padded 3-digit sequential (001, 002, ...)
- Examples: `TASK-INFRA-001`, `TASK-UI-042`, `TASK-DB-015`, `TASK-TEST-001`
- **See [.github/ztodo_task_guidelines.md](../.github/instructions/ztodo_task_guidelines.md) for complete task creation guidelines**

**Note:** Existing tasks without IDs are being migrated. New tasks MUST include IDs from creation.

---

## ⚠️ CRITICAL: API Documentation Required

**ALL CODE MUST INCLUDE DOXYGEN-STYLE COMMENTS FROM THE BEGINNING.**

- **Rule:** Every public class, method, property, and enum must be documented with Doxygen-style comments (`/** */`)
- **Guideline:** See [.cursor/rules/api_documentation.mdc](../.cursor/rules/api_documentation.mdc) for complete documentation requirements
- **Reference:** See [project-dashboard/doc/guidelines/DOC-GUIDE-020_api_documentation.md](doc/guidelines/DOC-GUIDE-020_api_documentation.md) for API documentation strategy
- **Enforcement:** Code reviews will reject code without proper documentation

**Documentation is not optional - it is a required part of every public API.**

## ⚠️ CRITICAL: Verification Required

**ALL ZTODO ITEMS MUST BE VERIFIED BEFORE MARKING COMPLETE.**

- **Rule:** Every task must include verification steps and be verified before completion
- **Guideline:** See [.github/ztodo_verification.md](../.github/instructions/ztodo_verification.md) for complete verification workflow
- **Categories:** Functional, Code Quality, Documentation, Integration, Tests, Performance, QML (if applicable)
- **Enforcement:** Tasks cannot be marked complete without passing all verification steps
- **Build Verification:** ALL tasks that create/modify files MUST run `[./scripts/verify-build.sh](scripts/verify-build.sh) <files...>` to verify affected targets build
- **Code Quality Verification:** ALL tasks MUST run `[./scripts/verify_code_quality.sh](scripts/verify_code_quality.sh)` to check lint and documentation compliance
  - The script automatically detects uncommitted files and performs language-specific lint checks (C++, QML, CMake, Markdown, Shell)
  - Validates Doxygen documentation completeness for all classes, functions, and properties
  - Checks spacing, indentation, trailing whitespace, and formatting consistency
  - Usage: `[./scripts/verify_code_quality.sh](scripts/verify_code_quality.sh)` (auto-detects changes) or `[./scripts/verify_code_quality.sh](scripts/verify_code_quality.sh) <file1> <file2>...` (specific files)
- **Status Tracking:** Update verification status in task using ✅ markers (e.g., "1. Functional: ✅ Verified - details")

**Verification is not optional - it is a required part of every task completion.**

**📋 Before starting ANY task, read [.github/ztodo_verification.md](../.github/instructions/ztodo_verification.md) to understand ALL verification requirements.**




---

## Configuration Tasks

- [ ] TASK-CONFIG-001: Implement Configuration Management
  - What: Implement a robust `ConfigManager` to handle application bootstrap configuration. This component should replace or enhance the existing `ConfigLoader`. It must support loading configuration from multiple sources with the following priority: Environment Variables > Configuration File (JSON/INI) > Default Values. Key configuration items include Database Path, Sensor Source Mode, Logging Level, and Shared Memory settings.
  - Why: **DEPLOYMENT FLEXIBILITY:** Hardcoded paths and settings prevent easy deployment in different environments (e.g., Docker, CI/CD, Production). Environment variable support is critical for containerized deployments.
  - Files:
    - [z-monitor/src/infrastructure/config/ConfigManager.h](z-monitor/src/infrastructure/config/ConfigManager.h) (Create new or refactor ConfigLoader)
    - [z-monitor/src/infrastructure/config/ConfigManager.cpp](z-monitor/src/infrastructure/config/ConfigManager.cpp) (Implementation)
    - [z-monitor/src/application/config/AppConfig.h](z-monitor/src/application/config/AppConfig.h) (Update configuration model if needed)
  - Acceptance:
    - `ConfigManager` loads configuration from a file (e.g., `config.ini` or `config.json`)
    - Environment variables (e.g., `ZMON_DB_PATH`, `ZMON_LOG_LEVEL`) override file settings
    - Default values are used if no configuration is provided
    - Returns a populated `AppConfig` struct for dependency injection
    - Unit tests verify priority logic (Env > File > Default)
  - Verification Steps:
    1. Functional: Verify app starts with defaults, respects config file, and respects env var overrides. **Status:** ⏳ Pending implementation
    2. Code Quality: Verify Doxygen comments, no hardcoded strings in logic (use constants). **Status:** ⏳ Pending implementation
    3. Documentation: Document all supported config keys and env vars in `doc/architecture/DOC-ARCH-012_configuration_management.md`. **Status:** ⏳ Pending implementation
    4. Integration: Verify `DIContainer` uses `ConfigManager` to initialize services. **Status:** ⏳ Pending implementation
    5. Tests: Unit tests cover all loading scenarios and precedence rules. **Status:** ⏳ Pending implementation
  - Prompt: `project-dashboard/prompt/59-implement-configuration-management.md`
  - Documentation: See [doc/architecture/DOC-ARCH-012_configuration_management.md](doc/architecture/DOC-ARCH-012_configuration_management.md)
