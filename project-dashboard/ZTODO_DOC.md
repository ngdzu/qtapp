# Z Monitor Development Tasks - DOC

## Task ID Convention

**ALL tasks use format: `TASK-{CATEGORY}-{NUMBER}`**

- **See `.github/ztodo_task_guidelines.md` for complete task creation guidelines**

---

## Documentation Tasks

- [ ] TASK-DOC-001: Generate Doxygen Documentation
  - What: Configure Doxygen and generate HTML documentation for the entire codebase.
  - Why: Provides navigable API documentation for developers.
  - Files:
    - `Doxyfile`
    - `doc/api/` (output)
  - Acceptance: `doxygen` command runs without errors, HTML output is browsable.
  - Verification Steps:
    1. Functional: Docs generated
    2. Code Quality: No Doxygen warnings
    3. Documentation: Doxyfile configured
    4. Integration: CI job created
    5. Tests: N/A
  - Prompt: `project-dashboard/prompt/60-generate-doxygen-documentation.md`

- [ ] TASK-DOC-002: Document System Components
  - What: Create detailed Markdown documentation for each major component (MonitoringService, DatabaseManager, etc.).
  - Why: Knowledge transfer and architectural clarity.
  - Files:
    - `doc/components/*.md`
  - Acceptance: All major components have a dedicated doc file.
  - Verification Steps:
    1. Functional: Docs exist
    2. Code Quality: Markdown linting
    3. Documentation: Comprehensive content
    4. Integration: Linked from README
    5. Tests: N/A
  - Prompt: `project-dashboard/prompt/61-document-system-components.md`

- [ ] TASK-DOC-003: Document Threading Model
  - What: Document the threading architecture (Main Thread, Data Acquisition, Database I/O, Network I/O).
  - Why: Critical for preventing race conditions and deadlocks.
  - Files:
    - `doc/architecture/threading_model.md`
  - Acceptance: Clear diagram and explanation of thread roles and interactions.
  - Verification Steps:
    1. Functional: Doc exists
    2. Code Quality: Markdown linting
    3. Documentation: Accurate reflection of code
    4. Integration: Linked from Architecture doc
    5. Tests: N/A
  - Prompt: `project-dashboard/prompt/62-document-threading-model.md`

- [ ] TASK-DOC-004: Capture screenshots of live UI with real data
  - What: Capture high-quality screenshots (1280x800) of z-monitor UI showing live vitals and waveforms. Take screenshots at multiple points: (1) normal vitals, (2) abnormal vitals (high HR), (3) connection lost, (4) waveform detail view. Save to `project-dashboard/screenshots/` with descriptive names. Include timestamp and data source in filename.
  - Why: Screenshots are critical for documentation, user manual, regulatory submissions, and marketing materials. Visual proof that the UI displays live data correctly. Screenshots will be referenced in design docs and SRS.
  - Files:
    - Create: `project-dashboard/screenshots/z-monitor-live-normal-vitals.png` (1280x800, normal HR 72, SpO2 98%)
    - Create: `project-dashboard/screenshots/z-monitor-live-abnormal-vitals.png` (1280x800, HR 140, SpO2 88%)
    - Create: `project-dashboard/screenshots/z-monitor-waveform-detail.png` (1280x800, ECG waveform closeup)
    - Create: `project-dashboard/screenshots/z-monitor-connection-lost.png` (1280x800, disconnected state)
    - Update: `project-dashboard/doc/legacy/architecture_and_design/03_UI_UX_GUIDE.md` (embed screenshots)
  - Dependencies:
    - Simulator running with configurable vitals (normal/abnormal scenarios)
    - z-monitor UI fully functional
    - Screenshot tool available (macOS: Cmd+Shift+4, Linux: scrot/flameshot)
  - Implementation Details:
    - Take screenshots using macOS native tool: Cmd+Shift+4, select 1280x800 region
    - Or use Qt screenshot API if available (QQuickWindow::grabWindow())
    - Naming convention: `z-monitor-<scenario>-<timestamp>.png`
    - Example: `z-monitor-live-normal-vitals-2025-11-29.png`
  - Acceptance:
    - At least 4 screenshots captured showing different scenarios
    - Screenshots are 1280x800 resolution (native UI size)
    - All vitals visible and readable
    - Waveforms rendered correctly (no blank Canvas)
    - Connection status indicator visible
    - Screenshots embedded in `project-dashboard/doc/legacy/architecture_and_design/03_UI_UX_GUIDE.md`
  - Verification Steps:
    1. Capture: Take 4+ screenshots covering different scenarios **Status:** ⏳ Pending
    2. Quality Check: Verify resolution, clarity, all UI elements visible **Status:** ⏳ Pending
    3. Documentation: Embed in docs with captions, commit to repo **Status:** ⏳ Pending
  - Prompt: `project-dashboard/prompt/44f-capture-live-ui-screenshots.md`

- [ ] TASK-DOC-005: Add mermaid render script and CI check
  - What: Add `scripts/render-mermaid.sh` and a CI job that runs it and fails on parse errors. Document usage in `.github/copilot-instructions.md`.
  - Why: Prevents malformed diagrams from being committed (we had parser issues earlier).
  - Prompt: `project-dashboard/prompt/20-render-mermaid-script-ci.md`

- [ ] TASK-DOC-007: Generate Complete API Documentation with Doxygen
  - What: Configure Doxygen to generate comprehensive API documentation for all public classes, methods, and interfaces. Create custom Doxygen theme matching project branding. Generate documentation in HTML and PDF formats. Set up automated documentation generation in CI pipeline. Ensure all public APIs have Doxygen comments (grep verification).
  - Why: API documentation is critical for maintainability and onboarding. Automated generation ensures documentation stays synchronized with code. CI integration prevents missing documentation.
  - Files:
    - Doxyfile (Doxygen configuration)
    - doc/doxygen/custom_theme/ (custom theme files)
    - .github/workflows/documentation.yml (CI workflow for doc generation)
    - scripts/generate_api_docs.sh (documentation generation script)
    - scripts/verify_api_docs.py (verification script to check all public APIs have Doxygen comments)
  - Acceptance: Doxygen configured, custom theme applied, HTML/PDF docs generated, CI workflow generates docs on every commit, all public APIs have Doxygen comments (grep verification passes), documentation published to GitHub Pages or artifact storage.
  - Verification Steps:
    1. Functional: Doxygen generates docs, HTML output works, PDF output works, CI workflow succeeds
    2. Code Quality: All public APIs have Doxygen comments (grep verification), no Doxygen warnings
    3. Documentation: Doxygen configuration documented, theme customization documented
    4. Integration: CI workflow publishes docs, docs accessible via URL
    5. Tests: Verification script checks all public APIs documented, grep confirms no undocumented APIs
  - Dependencies: Doxygen tool, CI infrastructure
  - Documentation: See .cursor/rules/api_documentation.mdc for Doxygen requirements. See project-dashboard/doc/guidelines/DOC-GUIDE-020_api_documentation.md for API documentation strategy.
  - Prompt: project-dashboard/prompt/TASK-DOC-007-doxygen-generation.md

- [ ] TASK-DOC-008: Create Architecture Decision Records (ADRs)
  - What: Create Architecture Decision Records in doc/adr/ documenting key architectural decisions: DDD layering, thread model, database choice (SQLite), ORM choice (QxOrm hybrid), logging strategy (async with queue), sensor integration (shared memory), authentication (hospital server), telemetry (batch upload). Use standard ADR template (Context, Decision, Consequences, Alternatives Considered).
  - Why: ADRs provide historical context for architectural decisions. Helps new developers understand "why" not just "what". Documents tradeoffs and alternatives considered. Critical for long-term maintainability.
  - Files:
    - doc/adr/0001-domain-driven-design.md
    - doc/adr/0002-thread-model.md
    - doc/adr/0003-database-sqlite.md
    - doc/adr/0004-orm-qxorm-hybrid.md
    - doc/adr/0005-async-logging.md
    - doc/adr/0006-shared-memory-sensors.md
    - doc/adr/0007-hospital-authentication.md
    - doc/adr/0008-batch-telemetry.md
    - doc/adr/README.md (ADR index)
  - Acceptance: ADRs created for all major architectural decisions, ADRs follow standard template, ADR index created, decisions documented with context/consequences/alternatives.
  - Verification Steps:
    1. Functional: ADRs complete, template followed, index created
    2. Code Quality: ADRs well-written, clear, concise
    3. Documentation: ADR index links to all ADRs, ADRs cross-reference related docs
    4. Integration: ADRs integrated into main documentation structure
    5. Tests: Documentation review, ADRs accurate and complete
  - Documentation: Use standard ADR template. See existing architecture docs for decision context.
  - Prompt: project-dashboard/prompt/TASK-DOC-008-architecture-decision-records.md

- [ ] TASK-DOC-009: Create Onboarding Guide for New Developers
  - What: Create comprehensive onboarding guide in doc/ONBOARDING.md covering: development environment setup, project structure overview, architecture principles (DDD, thread model), coding standards, testing workflow, common tasks (adding a new feature, fixing a bug), troubleshooting common issues. Include links to all relevant documentation.
  - Why: Reduces onboarding time for new developers. Ensures consistent understanding of architecture and standards. Provides clear path from setup to first contribution.
  - Files:
    - doc/ONBOARDING.md
    - doc/CONTRIBUTING.md (contribution guidelines)
    - doc/TROUBLESHOOTING.md (common issues and solutions)

- [ ] TASK-DOC-004: Review Simulator Documentation
  - What: Review and update doc/35_SENSOR_SIMULATOR.md to ensure it accurately reflects the current design and requirements for the sensor simulator. Add details about configuration options (frequency, patterns), data format, and integration points.
  - Why: **DOCUMENTATION:** The sensor simulator is a critical component for development and testing. Accurate documentation ensures that the implementation meets requirements and that developers understand how to use and configure it.
  - Files:
    - doc/35_SENSOR_SIMULATOR.md (update content)
  - Acceptance:
    - Documentation covers architecture, configuration, and usage
    - Sequence diagrams updated (if needed)
    - Configuration examples provided
  - Verification Steps:
    1. Functional: Documentation is readable and accurate. **Status:** ⏳ Pending review
    2. Code Quality: Markdown formatting correct, links valid. **Status:** ⏳ Pending review
    3. Documentation: Updated file committed. **Status:** ⏳ Pending review
    4. Integration: N/A
    5. Tests: N/A
  - Dependencies: None
  - Documentation: See doc/35_SENSOR_SIMULATOR.md

- [ ] TASK-DOC-005: Create Simulator Prompt
  - What: Create a prompt file project-dashboard/prompt/simulator-implementation.md that describes the task of implementing the sensor simulator. This prompt will be used to guide the AI assistant in generating the simulator code. Include requirements for: SensorSimulator class, ISensor interface, SimulatedSensor implementation, configuration loading, and data generation patterns (sine wave, random, constant).
  - Why: **AI ASSISTANCE:** A well-structured prompt ensures that the AI generates high-quality, compliant code that meets all requirements. It serves as a specification for the implementation task.
  - Files:
    - project-dashboard/prompt/simulator-implementation.md (create file)
  - Acceptance:
    - Prompt covers all architectural components
    - Requirements for configuration and data generation specified
    - Coding standards (Doxygen, error handling) mentioned
  - Verification Steps:
    1. Functional: Prompt clearly describes the task. **Status:** ⏳ Pending creation
    2. Code Quality: Markdown formatting correct. **Status:** ⏳ Pending creation
    3. Documentation: Prompt file committed. **Status:** ⏳ Pending creation
    4. Integration: N/A
    5. Tests: N/A
  - Dependencies: doc/35_SENSOR_SIMULATOR.md
  - Documentation: See doc/35_SENSOR_SIMULATOR.md

- [ ] TASK-REG-006: Generate FDA 510(k) Software Documentation Package
  - What: Generate comprehensive FDA 510(k) software documentation package including: Software Development Plan, Software Requirements Specification (SRS), Software Design Specification (SDS), Software Verification and Validation Plan (SVVP), Hazard Analysis, Traceability Matrix (requirements → design → tests), Risk Management File (ISO 14971). Automated generation where possible (traceability matrix from code/tests).
  - Why: FDA 510(k) submission requires comprehensive software documentation. Automated generation ensures documentation stays synchronized with code. Traceability matrix proves requirements are implemented and tested.
  - Files:
    - doc/regulatory/fda-510k/software-development-plan.md
    - doc/regulatory/fda-510k/software-requirements-specification.md
    - doc/regulatory/fda-510k/software-design-specification.md
    - doc/regulatory/fda-510k/software-verification-validation-plan.md
    - doc/regulatory/fda-510k/hazard-analysis.md
    - doc/regulatory/fda-510k/risk-management-file.md
    - scripts/generate_traceability_matrix.py (automated traceability matrix generation)
    - doc/regulatory/fda-510k/traceability-matrix.md (generated)
  - Acceptance: All FDA 510(k) documents created, traceability matrix generated from code/tests, documents follow FDA guidance, hazard analysis complete, risk management file complete.
  - Verification Steps:
    1. Functional: Traceability matrix generation works, all requirements traced to design/tests
    2. Code Quality: Documentation follows FDA guidance, consistent formatting
    3. Documentation: All FDA 510(k) documents complete, cross-references work
    4. Integration: Documentation integrates with existing architecture docs
    5. Tests: Traceability matrix validation, requirement coverage verification
  - Dependencies: Existing architecture and design documentation
  - Documentation: See FDA 510(k) guidance documents. See project-dashboard/doc/architecture/DOC-ARCH-001_system_architecture.md for system architecture.
  - Prompt: project-dashboard/prompt/TASK-REG-006-fda-510k-documentation.md
