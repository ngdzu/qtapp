# Z Monitor Development Tasks

## ⚠️ CRITICAL: API Documentation Required

**ALL CODE MUST INCLUDE DOXYGEN-STYLE COMMENTS FROM THE BEGINNING.**

- **Rule:** Every public class, method, property, and enum must be documented with Doxygen-style comments (`/** */`)
- **Guideline:** See `.cursor/rules/api_documentation.mdc` for complete documentation requirements
- **Reference:** See `doc/26_API_DOCUMENTATION.md` for API documentation strategy
- **Enforcement:** Code reviews will reject code without proper documentation

**Documentation is not optional - it is a required part of every public API.**

## ⚠️ CRITICAL: Verification Required

**ALL ZTODO ITEMS MUST BE VERIFIED BEFORE MARKING COMPLETE.**

- **Rule:** Every task must include verification steps and be verified before completion
- **Guideline:** See `.cursor/rules/ztodo_verification.mdc` for complete verification workflow
- **Categories:** Functional, Code Quality, Documentation, Integration, Tests
- **Enforcement:** Tasks cannot be marked complete without passing all verification steps

**Verification is not optional - it is a required part of every task completion.**

---

## Sequential Tasks (must be done in order)

- [ ] Refactor Settings: Remove Bed ID, Add Device Label and ADT Workflow
  - What: Remove `bedId` setting from SettingsManager and SettingsController. Add `deviceLabel` setting (static device identifier/asset tag). Update PatientManager to support ADT workflow with admission/discharge methods. Update database schema to add `admission_events` table and enhance `patients` table with ADT columns (bed_location, admitted_at, discharged_at, admission_source, device_label).
  - Why: Aligns device configuration with hospital ADT workflows. Separates device identity (Device Label) from patient assignment (Bed Location in Patient object). Enables proper patient lifecycle management.
  - Files: `src/core/SettingsManager.cpp/h`, `src/ui/SettingsController.cpp/h`, `src/core/PatientManager.cpp/h`, `doc/migrations/0003_adt_workflow.sql`, update `doc/10_DATABASE_DESIGN.md`.
  - Changes:
    - Remove `bedId` from settings table and SettingsManager
    - Add `deviceLabel` to settings (static asset tag, e.g., "ICU-MON-04")
    - Add `admission_events` table for audit trail
    - Enhance `patients` table with ADT columns
    - Update PatientManager with `admitPatient()`, `dischargePatient()`, `transferPatient()` methods
    - Update PatientController with admission state properties and methods
  - Acceptance: Settings no longer contains `bedId`, `deviceLabel` is displayed in Settings View, PatientManager supports ADT workflow, admission events are logged to database.
  - Verification Steps:
    1. Functional: Verify `bedId` removed, `deviceLabel` works, ADT methods function correctly, admission events logged
    2. Code Quality: Run linter, check Doxygen comments, verify no warnings
    3. Documentation: Update `doc/10_DATABASE_DESIGN.md`, verify `doc/19_ADT_WORKFLOW.md` is accurate
    4. Integration: Build succeeds, all tests pass, database migration works
    5. Tests: Write unit tests for ADT methods, integration tests for workflow, verify database schema
  - Documentation: See `doc/19_ADT_WORKFLOW.md` for complete ADT workflow specification.
  - Prompt: `project-dashboard/prompt/08a-refactor-settings-adt.md`  (When finished: mark this checklist item done.)

- [ ] Create project scaffolding and repo checklist
  - What: Ensure `project-dashboard/` contains the canonical folders: `src/`, `resources/`, `doc/`, `proto/`, `openapi/`, `tests/`, `central-server-simulator/` and `doc/migrations/`.
  - Why: Provides a stable structure to place interfaces, tests, and docs.
  - Artifacts: `CMakeLists.txt` (top-level), empty `proto/` and `openapi/` dirs, `doc/migrations/README.md`.
  - Prompt: `project-dashboard/prompt/01-create-project-scaffold.md`  (When finished: mark this checklist item done.)

- [ ] Define public C++ service interfaces (headers only)
  - What: Create minimal header-only interface sketches for: `IDatabaseManager`, `INetworkManager`, `IAlarmManager`, `IDeviceSimulator`, `ISettingsManager`, `IPatientLookupService`, `ITelemetryServer`, `IAuthenticationService`, and `IArchiver`.
  - Why: Interfaces allow test-first development (mocks) and make DI decisions easier.
  - Files: `src/core/interfaces/*.h` (one header per interface), `doc/interfaces.md` with rationale and method signatures.
  - Note: `IPatientLookupService` interface is documented in `doc/interfaces/IPatientLookupService.md` and provides patient lookup from external systems (HIS/EHR) by patient ID.
  - Note: `ITelemetryServer` interface is documented in `doc/interfaces/ITelemetryServer.md` and provides server communication abstraction with support for configurable server URLs and mock implementations for testing.
  - Prompt: `project-dashboard/prompt/02-define-public-interfaces.md`  (When finished: mark this checklist item done.)

- [ ] Create unit test harness + mock objects
  - What: Add `tests/CMakeLists.txt`, pick test framework (recommend `GoogleTest`), add `tests/mocks/` with mock classes that implement the interfaces.
  - Why: Unit tests should drive API decisions. Mocks let you write controller tests before production implementation.
  - Files: `tests/CMakeLists.txt`, `tests/mock_DatabaseManager.h`, `tests/mock_NetworkManager.h`, `tests/mock_PatientLookupService.h`, `tests/mock_TelemetryServer.h`, example test `tests/test_alarm_manager.cpp`.
  - Note: `MockPatientLookupService` should return hardcoded patient data for testing and support simulated failures.
  - Note: `MockTelemetryServer` should swallow all data without sending to real server, return immediate success responses, and support simulated failures for testing.
  - Prompt: `project-dashboard/prompt/03-create-unit-test-harness.md`  (When finished: mark this checklist item done.)

- [ ] Design database schema + write migration SQLs
  - What: Finalize DDL for tables: `patients`, `vitals`, `ecg_samples`, `pleth_samples`, `alarms`, `system_events`, `settings`, `users`, `certificates`, `security_audit_log`. Add indices, retention metadata table, and `archival_queue`.
  - Why: Deterministic schema is required before implementing `DatabaseManager` and `TrendsController`.
  - Files: `doc/migrations/0001_initial.sql`, `doc/migrations/0002_add_indices.sql`, `doc/10_DATABASE_DESIGN.md` update, ERD SVG in `doc/`.
  - Note: The `settings` table must support `deviceId`, `bedId`, `measurementUnit`, `serverUrl`, and `useMockServer` configuration options. See `doc/10_DATABASE_DESIGN.md` for the settings table schema.
  - Note: The `patients` table serves as a cache for patient lookups. Add `last_lookup_at` and `lookup_source` columns to track when patient data was retrieved from external systems. See `doc/10_DATABASE_DESIGN.md` for details.
  - Note: The `certificates` table must track certificate lifecycle including expiration, revocation, and validation status. The `security_audit_log` table must store all security-relevant events for audit and compliance. See `doc/10_DATABASE_DESIGN.md` for detailed schemas.
  - Prompt: `project-dashboard/prompt/04-design-db-schema-migrations.md`  (When finished: mark this checklist item done.)

- [ ] Implement DatabaseManager spike (in-memory + SQLCipher plan)
  - What: Implement a minimal, test-only `DatabaseManager` that uses an in-memory SQLite for tests. Document how SQLCipher will be integrated and add CMake options to enable/disable SQLCipher.
  - Why: Validates schema and migrations without full SQLCipher integration yet.
  - Files: `src/core/DatabaseManagerStub.cpp/h`, `tests/db_smoke_test.cpp`, `CMakeLists` options: `-DENABLE_SQLCIPHER=ON/OFF`.
  - Prompt: `project-dashboard/prompt/05-implement-dbmanager-spike.md`  (When finished: mark this checklist item done.)

- [ ] Define telemetry proto and/or OpenAPI spec (canonical schema)
  - What: Create `proto/telemetry.proto` and `openapi/telemetry.yaml`. Include message definitions for vitals, device status, alarms, and batching semantics.
  - Why: Having canonical schema lets the simulator, server, and device agree on payloads. Protobuf + JSON mapping recommended.
  - Files: `proto/telemetry.proto`, `openapi/telemetry.yaml`, `doc/proto_design.md`.
  - Prompt: `project-dashboard/prompt/06-define-telemetry-proto-openapi.md`  (When finished: mark this checklist item done.)

- [ ] Implement basic NetworkManager test double + API contract
  - What: Using the proto/OpenAPI, implement a mock `NetworkManager` (no TLS) that records requests and simulates server responses (200, 500, timeout). Add unit tests for retry and backoff behavior.
  - Why: Allows `SystemController`/`NotificationController` unit tests before adding mTLS plumbing.
  - Files: `tests/mock_NetworkManager.h`, `tests/network_retry_test.cpp`.
  - Note: `NetworkManager` should use `ITelemetryServer` interface. Implement `MockTelemetryServer` that swallows data for testing.
  - Prompt: `project-dashboard/prompt/07-implement-mock-networkmanager.md`  (When finished: mark this checklist item done.)

- [ ] Implement controller skeletons and QML binding stubs
  - What: Create `DashboardController`, `AlarmController`, `SystemController`, `PatientController`, `SettingsController`, `TrendsController`, `NotificationController` as QObject-derived classes exposing Q_PROPERTY and basic signals. Do not implement heavy logic yet.
  - Why: QML UI can be wired to properties and tested for binding behavior early.
  - Files: `src/ui/*.cpp/h` and `resources/qml/Main.qml` with placeholder components.
  - Note: `SettingsController` must expose `deviceId`, `deviceLabel`, `measurementUnit`, `serverUrl`, and `useMockServer` as Q_PROPERTY. `bedId` has been removed - bed location is now part of Patient object managed through ADT workflow.
  - Note: `PatientController` must expose `admitPatient()`, `dischargePatient()`, `openAdmissionModal()`, `scanBarcode()` as Q_INVOKABLE methods and `admissionState`, `isAdmitted`, `bedLocation`, `admittedAt` as Q_PROPERTY for ADT workflow. See `doc/19_ADT_WORKFLOW.md` for complete ADT workflow specification.
  - Prompt: `project-dashboard/prompt/08-controller-skeletons-qml-stubs.md`  (When finished: mark this checklist item done.)

## Testing & Quality Foundations

- [ ] Implement unified testing workflow
  - What: Create GoogleTest + Qt Test scaffolding under `tests/unit/`, integration suites under `tests/integration/`, E2E suites under `tests/e2e/`, and benchmark suites under `tests/benchmarks/`. Apply `ctest` labels (`unit`, `integration`, `benchmark`) and follow the process documented in `doc/18_TESTING_WORKFLOW.md`.
  - Why: Testing groundwork must be established early to support iterative development.

- [ ] Add coverage pipeline
  - What: Enable coverage builds with `-DENABLE_COVERAGE=ON`, integrate `lcov`/`genhtml`, and enforce minimum 80% line coverage on `src/core/`. Publish reports from `build_coverage/coverage/index.html`.
  - Why: Maintains confidence in critical code paths.

- [ ] Integrate benchmarking harness
  - What: Add Google Benchmark targets for AlarmManager, SignalProcessor, telemetry serialization, and certificate validation routines. Store results (CSV/JSON) for regression tracking.
  - Why: Detects performance regressions early.

- [ ] Automate lint/static analysis
  - What: Extend `scripts/run_tests.sh lint` to invoke clang-format, clang-tidy, and cppcheck; gate CI on lint success.
  - Why: Keeps codebase consistent and surfaces issues before compilation.

- [ ] Add CI workflows for build + tests
  - What: Add GitHub Actions (or preferred CI) jobs: `build`, `unit-tests`, `render-diagrams`, `integration-tests` that run the server simulator.
  - Why: Keeps repo healthy and verifies that docs/diagrams render correctly in CI.
  - Prompt: `project-dashboard/prompt/19-ci-workflows-build-tests.md`  (When finished: mark this checklist item done.)

- [ ] Enforce testing workflow in CI
  - What: Update CI workflows to call `./scripts/run_tests.sh all`, publish coverage + benchmark artifacts, and fail builds when thresholds/regressions occur.
  - Why: Ensures automation matches the documented workflow.

- [ ] Add mermaid render script and CI check
  - What: Add `scripts/render-mermaid.sh` and a CI job that runs it and fails on parse errors. Document usage in `.github/copilot-instructions.md`.
  - Why: Prevents malformed diagrams from being committed (we had parser issues earlier).
  - Prompt: `project-dashboard/prompt/20-render-mermaid-script-ci.md`  (When finished: mark this checklist item done.)

- [ ] Add E2E containerized test harness
  - What: Compose the Z Monitor (headless) and the server simulator in docker-compose test environment and run basic E2E scenarios.
  - Why: Validates connectivity, DB writes, and archival behavior in a reproducible environment.
  - Prompt: `project-dashboard/prompt/21-e2e-containerized-harness.md`  (When finished: mark this checklist item done.)

- [ ] Implement Archiver interface and archiving tests
  - What: Create `IArchiver` interface and tests that show moving rows older than 7 days to an `archived_vitals` table or external archive file. Add unit tests for retention policy enforcement.
  - Why: Archival is required by requirements; must be testable and configurable.
  - Prompt: `project-dashboard/prompt/18-implement-archiver-interface.md`  (When finished: mark this checklist item done.)


## Parallel Tasks (can be done concurrently)

- [ ] QML UI skeleton and components
  - What: Implement `Main.qml`, `Sidebar.qml`, `TopBar.qml`, `StatCard.qml`, `PatientBanner.qml`, and placeholder `views/` (DashboardView, DiagnosticsView, TrendsView, SettingsView, LoginView, AdmissionModal).
  - Why: Visual scaffolding enables early UX validation and manual QA.
  - Acceptance: QML app boots and displays placeholders at `1280x800`.
  - Note: `SettingsView.qml` must include Device Configuration section with Device Label (read-only display), Device ID input, and Measurement Unit dropdown (metric/imperial). Bed ID has been removed - bed location is now part of Patient object. See `doc/03_UI_UX_GUIDE.md` section 4.4 for specifications.
  - Note: `AdmissionModal.qml` must provide admission method selection (Manual Entry, Barcode Scan, Central Station), patient lookup, patient preview with bed location override, and admission confirmation. See `doc/03_UI_UX_GUIDE.md` section 4.5 and `doc/19_ADT_WORKFLOW.md` for specifications.
  - Note: `PatientBanner.qml` must display patient name prominently when admitted, or "DISCHARGED / STANDBY" when no patient is admitted. Should be tappable to open Admission Modal when no patient is assigned. See `doc/03_UI_UX_GUIDE.md` section 5.1 for specifications.
  - Prompt: `project-dashboard/prompt/09-qml-ui-skeleton.md`  (When finished: mark this checklist item done.)

- [ ] Alarm UI & animation prototypes (QML)
  - What: Prototype critical alarm full-screen flash, per-card highlight, audio stubs, and Alarm History panel in QML.
  - Why: Visual design for alarms should be validated separately from backend logic.
  - Prompt: `project-dashboard/prompt/10-alarm-ui-prototypes.md`  (When finished: mark this checklist item done.)

- [ ] DeviceSimulator and synthetic signal generation
  - What: Implement a test-only `DeviceSimulator` capable of generating vitals, ECG waveform samples, pleth waveform, and simulated events (arrhythmia, motion artifact) for UI demos.
  - Why: Provides deterministic input for UI, controller, and integration tests.
  - Prompt: `project-dashboard/prompt/11-device-simulator.md`  (When finished: mark this checklist item done.)

- [ ] Implement `WebSocketDeviceSimulatorAdapter` (IDeviceSimulator)
  - What: Implement an adapter `WebSocketDeviceSimulatorAdapter` that implements `IDeviceSimulator` and connects to `sensor-simulator` (`ws://localhost:9002`). Adapter should translate incoming JSON messages into `PacketCallback` invocations and provide optional control commands to the simulator.
  - Why: The device should depend only on `IDeviceSimulator`; this adapter lets the device run without hardware by connecting to the simulator.
  - Acceptance: `WebSocketDeviceSimulatorAdapter` builds and passes a unit test where a mocked websocket delivers a `vitals` JSON and the adapter invokes the registered `PacketCallback` with equivalent JSON.
  - Prompt: `project-dashboard/prompt/11-device-simulator.md`  (When finished: mark this checklist item done.)

- [ ] Add factory and configuration for selecting simulator implementation
  - What: Add `DeviceSimulatorFactory::Create()` which returns either an in-process `MockDeviceSimulator` for unit tests or `WebSocketDeviceSimulatorAdapter` for local dev based on a config option / CMake define or runtime flag.
  - Why: Keeps Z Monitor code decoupled from transport; simplifies CI and developer workflows.
  - Acceptance: Device main can call `auto sim = DeviceSimulatorFactory::Create(config)` and receive a valid `IDeviceSimulator*` for use.
  - Prompt: `project-dashboard/prompt/11-device-simulator.md`  (When finished: mark this checklist item done.)

 - [x] Sensor-simulator (WebSocket sensor stream + QML UI)
  - What: Provide a Qt Quick (QML + C++) sensor simulator in `project-dashboard/sensor-simulator/` that streams sensor data to the device over a WebSocket (default `ws://localhost:9002`) and exposes a `Simulator` `QObject` to QML. UI must have buttons to trigger `critical`, `warning`, and `notification` events and a `Play Demo` timeline. Provide a `Dockerfile` for quick containerized runs and instructions to run locally (X11 / XQuartz notes).
  - Why: This component supplies simulated sensor data to the Z Monitor during development and testing without requiring hardware.
  - Acceptance: `project-dashboard/sensor-simulator` builds; connecting a client to `ws://localhost:9002` receives periodic `vitals` + `waveform` JSON messages; clicking UI buttons emits signals and sends `alarm`/`notification` messages to connected clients.
  - Status: ✅ **COMPLETED** - Added real-time ECG waveform visualization with PQRST complex generation. Enhanced log console with filters, pause functionality, and improved styling. All features implemented and tested.
  - Prompt: `project-dashboard/prompt/11-device-simulator.md`  (When finished: mark this checklist item done.)

 - [ ] Integrate Sensor-simulator into top-level build
  - What: Ensure the top-level `CMakeLists.txt` adds `add_subdirectory(project-dashboard/sensor-simulator)` so the simulator builds with the repository and can be built in CI.
  - Why: Makes it easy to build the simulator in automation and to run smoke tests.
  - Acceptance: `cmake ..` at repo root configures the simulator target.
  - Prompt: `project-dashboard/prompt/21-e2e-containerized-harness.md`  (When finished: mark this checklist item done.)

 - [ ] Containerized acceptance tests for simulator
  - What: Add `docker-compose.simulator.yml` that runs `central-server-simulator` and the `sensor-simulator` (headless or with virtual display) to exercise basic scenarios.
  - Why: Enables repeatable E2E smoke tests in CI.
  - Acceptance: `docker-compose -f docker-compose.simulator.yml up --build` brings up server + simulator and runs a smoke script.
  - Prompt: `project-dashboard/prompt/21-e2e-containerized-harness.md`  (When finished: mark this checklist item done.)

- [ ] Central server simulator (mTLS later)
  - What: Create `central-server-simulator/` with a simple REST endpoint `POST /api/telemetry` that can accept JSON and returns ack. Implement toggles to simulate network failures and delays.
  - Why: Enables local end-to-end testing of networking flows.
  - Note: Add optional `GET /api/patients/{patientId}` endpoint for patient lookup to support `IPatientLookupService` integration. This endpoint should return patient demographics in JSON format.
  - Note: Server URL should be configurable through `SettingsManager` (default: "https://localhost:8443"). The `NetworkManager` should use `ITelemetryServer` interface, allowing for `MockTelemetryServer` implementation that swallows data for testing without requiring server infrastructure.
  - Note: Server must implement mTLS, validate client certificates, verify digital signatures on payloads, check timestamps for replay prevention, and enforce rate limiting. See `doc/06_SECURITY.md` section 6.7 for server-side security requirements.
  - Prompt: `project-dashboard/prompt/12-central-server-simulator.md`  (When finished: mark this checklist item done.)

- [ ] Implement mockable logging and LogService binding to QML
  - What: Create `LogService` that queues messages and exposes them to QML as a model for the Diagnostics view. Support different levels (Info/Warn/Error).
  - Why: Diagnostics and logs are required for debugging and QA.
  - Prompt: `project-dashboard/prompt/13-logservice-qml-model.md`  (When finished: mark this checklist item done.)

- [ ] Implement PatientManager with IPatientLookupService integration
  - What: Implement `PatientManager` to integrate with `IPatientLookupService` for patient lookups. Add `loadPatientById()` method that first checks local database, then uses lookup service if not found. Cache lookup results in local `patients` table.
  - Why: Enables quick patient assignment by entering patient ID, with automatic lookup from external systems (HIS/EHR).
  - Files: `src/core/PatientManager.cpp/h`, implement integration with `IPatientLookupService`, update `DatabaseManager` to support patient caching.
  - Acceptance: `PatientManager::loadPatientById(id)` successfully looks up patient from external system and caches result locally. Unit tests with `MockPatientLookupService` verify lookup flow.
  - Prompt: `project-dashboard/prompt/13b-patient-lookup-integration.md`  (When finished: mark this checklist item done.)


## Security & Certificates (ordered but distinct)

- [x] Define security architecture and provisioning plan
  - What: Finalize how device certificates will be provisioned, where certs are stored in `resources/certs/`, and the CA trust model. Document in `doc/06_SECURITY.md`.
  - Why: Security design must be agreed before writing any cert-generation scripts.
  - Note: Comprehensive certificate provisioning guide with step-by-step instructions and workflow diagrams is available in `doc/15_CERTIFICATE_PROVISIONING.md` and `doc/15_CERTIFICATE_PROVISIONING.mmd`. This includes CA setup, device certificate generation, installation, validation, renewal, and revocation processes.
  - Note: Security documentation has been enhanced with detailed authentication, session management, secure boot, tamper detection, and incident response procedures. See `doc/16_DOCUMENTATION_IMPROVEMENTS.md` for complete list of improvements.
  - Prompt: `project-dashboard/prompt/14-security-architecture-provisioning.md`  (When finished: mark this checklist item done.)

- [ ] Add scripts for CA + cert generation (after infra agreed)
  - What: Create `scripts/generate-selfsigned-certs.sh` that generates CA, server, and client certs for local testing. Include instructions for converting to PKCS12 if needed.
  - Why: Provides reproducible certs for simulator and device tests. NOTE: create *after* the previous task is approved.
  - Files: `scripts/generate-selfsigned-certs.sh`, `central-server-simulator/certs/README.md`.
  - Note: Script should follow the step-by-step process documented in `doc/15_CERTIFICATE_PROVISIONING.md`. Include options for: CA creation, device certificate generation with device ID in SAN, certificate bundle creation, and PKCS12 export. Reference workflow diagrams in `doc/15_CERTIFICATE_PROVISIONING.mmd` for process flow.
  - Prompt: `project-dashboard/prompt/15-generate-selfsigned-certs-script.md`  (When finished: mark this checklist item done.)

- [ ] Implement device provisioning and pairing system
  - What: Implement QR code-based device provisioning workflow to replace manual network configuration. Includes QR code generation, pairing code management, secure configuration push, and connection testing.
  - Why: Industry-standard approach for embedded medical devices eliminates manual certificate installation and reduces configuration errors. Provides secure, auditable device provisioning.
  - Files: `src/core/ProvisioningService.cpp/h`, `src/controllers/ProvisioningController.cpp/h`, update `SettingsView.qml` with provisioning UI, update `NetworkManager` for provisioned configuration.
  - Features:
    - QR code generation with device ID, IP, pairing code, and time-limited token
    - Pairing code generation (format: XXX-XXX-XXX, expires after 10 minutes)
    - Secure configuration payload (encrypted with device public key, signed by Central Station)
    - Configuration validation and application
    - Connection testing after provisioning
    - Re-provisioning support (change server configuration)
    - Provisioning state machine (NotProvisioned, ReadyToPair, Pairing, Configuring, Provisioned, Error)
    - Audit logging of all provisioning events
    - Development mode: "Simulate Configuration" button for testing
  - UI Components:
    - Provisioning status indicator
    - QR code display (regenerated every 30 seconds)
    - Pairing code display with copy button
    - Expiration timer countdown
    - Status messages for each state
    - Action buttons (Enter Provisioning Mode, Regenerate QR Code, Cancel, Re-provision)
    - Connected status view (read-only, shows server URL, certificate status, connection stats)
  - Security:
    - Explicit provisioning mode activation (requires Technician role)
    - Time-limited pairing codes (10 minutes)
    - One-time use pairing codes
    - Encrypted and signed configuration payloads
    - Configuration signature validation
    - All events logged to `security_audit_log`
  - Acceptance: Device can be provisioned via QR code scan, configuration is securely pushed and applied, device connects to server after provisioning. Re-provisioning works correctly. All provisioning events are logged.
  - Tests: QR code generation/validation, pairing code expiration, configuration encryption/decryption, signature validation, state machine transitions, error handling, audit logging.
  - Documentation: See `doc/17_DEVICE_PROVISIONING.md` for complete provisioning workflow specification.
  - Prompt: `project-dashboard/prompt/17-device-provisioning.md`  (When finished: mark this checklist item done.)

- [ ] Create automated certificate provisioning script
  - What: Create comprehensive automation script `scripts/provision-device-certificate.sh` that automates the complete certificate provisioning workflow for devices. Script should handle CA setup (if needed), device certificate generation, validation, and optionally installation/transfer to device.
  - Why: Automates the manual certificate provisioning process documented in `doc/15_CERTIFICATE_PROVISIONING.md`, reducing human error and ensuring consistent certificate generation across all devices.
  - Files: `scripts/provision-device-certificate.sh`, `scripts/cert-utils.sh` (helper functions), `scripts/cert-config.conf` (configuration template).
  - Features:
    - Interactive and non-interactive modes (for CI/CD)
    - CA setup automation (create CA if it doesn't exist)
    - Device certificate generation with device ID binding in SAN
    - Certificate validation (expiration, chain verification, device ID extraction)
    - Certificate bundle creation (cert + CA cert)
    - Optional PKCS12 export
    - Certificate installation to device (via SCP, USB path, or provisioning API)
    - Database registration (if device database accessible)
    - Support for batch provisioning (multiple devices)
    - Certificate renewal automation (generate new cert, parallel installation)
    - Certificate revocation support (update CRL)
  - Configuration:
    - Device ID, serial number, organization details
    - Certificate validity period (default: 365 days)
    - Key size (CA: 4096-bit, Device: 2048-bit)
    - Output directory and file naming conventions
    - Device transfer method (SCP, USB, API)
  - Acceptance: Script successfully provisions certificates following the workflow in `doc/15_CERTIFICATE_PROVISIONING.md`. Generated certificates pass validation, device ID is correctly embedded in SAN, and certificates can be installed on devices. Script includes error handling, logging, and dry-run mode.
  - Tests: Unit tests for certificate generation, validation, device ID extraction. Integration tests for complete provisioning workflow. Verify certificates work with mTLS connections.
  - Prompt: `project-dashboard/prompt/15b-automated-certificate-provisioning.md`  (When finished: mark this checklist item done.)

- [ ] mTLS integration spike for NetworkManager
  - What: Implement a small C++ example that configures `QSslConfiguration` with the generated client cert and validates handshake against the simulator using mutual auth.
  - Why: Confirms approach works on target platforms before full NetworkManager implementation.
  - Note: Must include certificate validation (expiration, revocation, device ID match), TLS 1.2+ enforcement, strong cipher suites, and basic security audit logging. See `doc/06_SECURITY.md` section 6 for comprehensive security requirements.
  - Note: Follow certificate provisioning steps in `doc/15_CERTIFICATE_PROVISIONING.md` to generate test certificates. Use workflow diagrams in `doc/15_CERTIFICATE_PROVISIONING.mmd` as reference for certificate lifecycle.
  - Prompt: `project-dashboard/prompt/16-mtls-integration-spike.md`  (When finished: mark this checklist item done.)

- [ ] Implement comprehensive security for data transmission
  - What: Implement full security architecture for telemetry and sensor data transmission including: certificate management and validation, digital signatures on payloads, timestamp/nonce for replay prevention, rate limiting, circuit breaker pattern, and security audit logging.
  - Why: Ensures secure, authenticated, and auditable transmission of sensitive patient data to central server.
  - Files: `src/core/NetworkManager.cpp/h`, `src/core/CertificateManager.cpp/h`, `src/core/SecurityAuditLogger.cpp/h`, update `DatabaseManager` for security audit log storage.
  - Note: CRL checking is **mandatory for production** (not optional). Clock skew tolerance is ±1 minute for production, ±5 minutes for development. See `doc/06_SECURITY.md` section 6 for detailed requirements.
  - **CRITICAL - Patient Data Association:**
    - All telemetry data MUST include patient MRN (Medical Record Number) for proper patient association
    - `NetworkManager` must automatically retrieve current patient MRN from `PatientManager` when sending telemetry
    - Validate that patient is admitted (`patientMrn` is not empty) before sending patient data
    - If no patient is admitted, do not send patient telemetry data (device in STANDBY state)
    - Each data record (vitals, alarms, etc.) must include `patientMrn` for proper association
    - Server must validate `patientMrn` presence and validity before processing patient data
  - Security Features:
    - Certificate lifecycle management (validation, expiration checking, revocation)
    - Digital signatures (ECDSA or RSA) on all telemetry payloads
    - Replay attack prevention (timestamp validation, nonce)
    - Rate limiting (60 requests/minute, configurable)
    - Circuit breaker for repeated failures
    - Security audit logging to `security_audit_log` table
  - Acceptance: All telemetry data is signed and validated, certificates are checked on startup and periodically, security events are logged, rate limiting prevents abuse, connection failures trigger circuit breaker, and **all patient data includes patient MRN for proper association**.
  - Tests: Certificate validation tests, signature verification tests, replay attack prevention tests, rate limiting tests, audit log verification tests, **patient MRN association tests** (verify MRN is included in all telemetry payloads).
  - Prompt: `project-dashboard/prompt/16b-comprehensive-security-implementation.md`  (When finished: mark this checklist item done.)


## Database Encryption & Archival (dependent)

- [ ] SQLCipher integration plan and build spike
  - What: Research how to add SQLCipher to the CMake build for macOS/Linux and add a spike that compiles and links SQLCipher for local runs.
  - Why: Encryption-at-rest is mandatory for patient data.
  - Prompt: `project-dashboard/prompt/17-sqlcipher-integration-plan.md`  (When finished: mark this checklist item done.)

- [ ] Implement Archiver interface and archiving tests
  - What: Create `IArchiver` interface and tests that show moving rows older than 7 days to an `archived_vitals` table or external archive file. Add unit tests for retention policy enforcement.
  - Why: Archival is required by requirements; must be testable and configurable.
  - Prompt: `project-dashboard/prompt/18-implement-archiver-interface.md`  (When finished: mark this checklist item done.)



## Software Engineering Best Practices & Design Decisions

- [ ] Review and implement Error Handling Strategy
  - What: Implement comprehensive error handling following `doc/20_ERROR_HANDLING_STRATEGY.md`, including Result<T,E> pattern, error codes, recovery strategies, and error propagation.
  - Why: Ensures consistent, type-safe error handling across the application with proper recovery and user feedback.
  - Files: Create `src/utils/Result.h`, update all service classes to use Result pattern, implement error codes enum, add error recovery logic.
  - Acceptance: All operations use Result pattern or signals for error propagation, error codes are standardized, recovery strategies are implemented, errors are logged appropriately.
  - Tests: Error handling tests, recovery tests, error propagation tests.
  - Prompt: `project-dashboard/prompt/20-error-handling-implementation.md`  (When finished: mark this checklist item done.)

- [ ] Review and implement Logging Strategy
  - What: Implement structured logging following `doc/21_LOGGING_STRATEGY.md`, including log levels, structured context, log rotation, and performance optimization.
  - Why: Provides comprehensive, searchable logging with appropriate performance characteristics for real-time systems.
  - Files: Update `src/core/LogService.cpp/h`, implement log rotation, add structured context support, implement async logging.
  - Acceptance: Logging uses structured format, log rotation works, async logging doesn't block threads, sensitive data is not logged, logs are searchable and filterable.
  - Tests: Logging tests, rotation tests, performance tests, security tests (verify no sensitive data).
  - Prompt: `project-dashboard/prompt/21-logging-strategy-implementation.md`  (When finished: mark this checklist item done.)

- [ ] Review and implement Code Organization
  - What: Organize code following `doc/22_CODE_ORGANIZATION.md`, including directory structure, namespace conventions, module boundaries, and dependency rules.
  - Why: Ensures maintainable, scalable codebase with clear module boundaries and dependencies.
  - Files: Reorganize source files if needed, add namespaces, update includes, verify module boundaries.
  - Acceptance: Code follows directory structure, namespaces are used correctly, no circular dependencies, includes are organized, module boundaries are respected.
  - Tests: Build system tests, dependency analysis tests.
  - Prompt: `project-dashboard/prompt/22-code-organization-review.md`  (When finished: mark this checklist item done.)

- [ ] Review and implement Memory & Resource Management
  - What: Implement memory management following `doc/23_MEMORY_RESOURCE_MANAGEMENT.md`, including smart pointers, RAII, pre-allocation, and resource cleanup.
  - Why: Prevents memory leaks, ensures predictable performance, and manages resources correctly.
  - Files: Update all classes to use smart pointers, implement RAII for resources, pre-allocate buffers for real-time operations, add resource cleanup.
  - Acceptance: No memory leaks detected, smart pointers used for dynamic memory, resources cleaned up properly, pre-allocation implemented for hot paths.
  - Tests: Memory leak tests, resource cleanup tests, performance tests.
  - Prompt: `project-dashboard/prompt/23-memory-management-review.md`  (When finished: mark this checklist item done.)

- [ ] Review and implement Configuration Management
  - What: Implement configuration management following `doc/24_CONFIGURATION_MANAGEMENT.md`, including validation, defaults, migration, and audit logging.
  - Why: Ensures type-safe, validated configuration with proper defaults and migration support.
  - Files: Update `src/core/SettingsManager.cpp/h`, implement validation, add configuration migration, implement audit logging.
  - Acceptance: All configuration is validated, defaults are loaded, migration works, changes are audited, type-safe accessors work.
  - Tests: Configuration validation tests, migration tests, audit logging tests.
  - Prompt: `project-dashboard/prompt/24-configuration-management-implementation.md`  (When finished: mark this checklist item done.)

- [ ] Review and implement API Versioning
  - What: Implement API versioning following `doc/25_API_VERSIONING.md`, including version negotiation, backward compatibility, and migration support.
  - Why: Enables API evolution while maintaining compatibility with existing clients.
  - Files: Update `src/core/NetworkManager.cpp/h`, implement version negotiation, add API version detection, implement migration logic.
  - Acceptance: API versioning works, backward compatibility maintained, version negotiation successful, migration guides provided.
  - Tests: Version compatibility tests, negotiation tests, migration tests.
  - Prompt: `project-dashboard/prompt/25-api-versioning-implementation.md`  (When finished: mark this checklist item done.)

## Documentation, Compliance & Diagrams

- [ ] Set up API documentation generation with Doxygen
  - What: Configure Doxygen to generate API documentation from source code comments. Create Doxyfile configuration, establish comment style guidelines, integrate with CMake build system, and set up documentation generation workflow.
  - Why: Ensures API documentation stays synchronized with codebase and provides comprehensive reference for developers. Auto-generated documentation reduces maintenance burden and ensures consistency.
  - Files: Create `project-dashboard/Doxyfile`, update `CMakeLists.txt` with Doxygen target, create `doc/26_API_DOCUMENTATION.md` with guidelines, add documentation comments to all public APIs.
  - Configuration:
    - Doxygen version: 1.9+ (supports modern C++ and Qt)
    - Output format: HTML (primary), PDF (optional)
    - Enable Qt-specific features (Q_OBJECT, Q_PROPERTY, signals/slots)
    - Enable class diagrams and dependency graphs
    - Configure module groups (CoreServices, Controllers, Interfaces, Models, Utils)
  - Comment Style:
    - Use Doxygen-style comments (`/** */`)
    - Document all public classes, methods, parameters, return values
    - Include examples for complex APIs
    - Use cross-references (@see, @sa)
    - Organize into modules using @defgroup
  - Build Integration:
    - Add `docs` target to CMake
    - Generate documentation on `cmake --build build --target docs`
    - Optionally publish to documentation server in CI/CD
  - CI/CD Integration:
    - ✅ Pre-commit hook: Optional lightweight check (warning only, doesn't block commits) - see `.pre-commit-config.yaml`
    - ✅ GitHub Actions workflow: Automatic generation nightly and on code changes - see `.github/workflows/doxygen-docs.yml`
    - Workflow runs at 2 AM UTC daily, on push to main/master, and on PRs (can be disabled if too slow)
    - Documentation artifacts uploaded for review
    - Fails if too many undocumented items (>10 threshold)
  - Acceptance: Doxygen generates complete API documentation, all public APIs are documented, documentation is accessible via HTML, diagrams are generated, documentation stays synchronized with code, CI/CD workflow runs successfully.
  - Verification Steps:
    1. Functional: Doxygen generates docs successfully, all public APIs appear in docs, HTML is accessible, diagrams render correctly
    2. Code Quality: Doxyfile is properly configured, no Doxygen warnings for critical items, documentation follows style
    3. Documentation: `doc/26_API_DOCUMENTATION.md` is complete, `scripts/README_DOXYGEN.md` explains workflows, README updated
    4. Integration: CMake `docs` target works, GitHub Actions workflow runs successfully, pre-commit hook works (optional)
    5. Tests: Documentation coverage check passes, all links work, examples compile and run
  - Tests: Documentation coverage check (fail CI if public APIs undocumented), verify all links work, check examples compile.
  - Documentation: See `doc/26_API_DOCUMENTATION.md` for complete API documentation strategy. See `scripts/README_DOXYGEN.md` for workflow details.
  - Prompt: `project-dashboard/prompt/26-api-documentation-setup.md`  (When finished: mark this checklist item done.)

- [ ] Document all public APIs with Doxygen comments
  - What: Add comprehensive Doxygen comments to all public classes, methods, properties, and enums. Ensure 100% coverage of public APIs. Include examples, cross-references, and usage notes.
  - Why: Complete API documentation enables developers to understand and use the API effectively. Examples and cross-references improve usability.
  - Files: Update all header files in `src/core/`, `src/controllers/`, `src/interfaces/`, `src/models/` with Doxygen comments.
  - Coverage Requirements:
    - All public classes: Class description, purpose, usage notes
    - All public methods: Method description, parameters, return values, exceptions, examples
    - All Q_PROPERTY: Property description, access notes
    - All enums: Enum description, value descriptions
    - All namespaces: Namespace purpose and organization
  - Examples: Include code examples for complex APIs (NetworkManager, DatabaseManager, etc.)
  - Cross-References: Link related classes, methods, and design documents
  - Acceptance: 100% coverage of public APIs, all examples compile and work, all cross-references valid, documentation is clear and helpful.
  - Tests: Run Doxygen and verify no warnings for undocumented public APIs, verify examples compile, check cross-references.
  - Prompt: `project-dashboard/prompt/27-api-documentation-comments.md`  (When finished: mark this checklist item done.)

- [ ] Update `doc/10_DATABASE_DESIGN.md` and add ERD
  - What: Consolidate the extended DDL into `doc/10_DATABASE_DESIGN.md`, include ERD and index rationale, and retention/archival notes.
  - Prompt: `project-dashboard/prompt/22-update-db-design-erd.md`  (When finished: mark this checklist item done.)

- [ ] Produce API docs: OpenAPI + proto docs
  - What: Finalize `openapi/telemetry.yaml` and ensure codegen steps are documented in `doc/`. Add `doc/api/README.md` describing mapping between proto and JSON.
  - Prompt: `project-dashboard/prompt/23-produce-api-docs.md`  (When finished: mark this checklist item done.)

- [ ] Create SRS and V&V outlines
  - What: Add `doc/SRS.md` (feature list, acceptance criteria) and `doc/VVPlan.md` for verification and validation testing; include list of safety-critical tests.
  - Prompt: `project-dashboard/prompt/24-srs-vvplan.md`  (When finished: mark this checklist item done.)

- [ ] Create threat model summary and FMEA sketch
  - What: Draft `doc/threat_model.md` and `doc/FMEA.md` focusing on data confidentiality (at-rest/in-transit), certificate compromise, tampering, and mitigations.
  - Prompt: `project-dashboard/prompt/25-threatmodel-fmea.md`  (When finished: mark this checklist item done.)


## UX & Clinical Validation

- [ ] Perform UI walkthrough and polish
  - What: Iterate on the QML layout for `1280x800`, validate readability of stat cards, colors, and alarm indicators with clinical stakeholders.
  - Prompt: `project-dashboard/prompt/26-ui-walkthrough-i18n.md`  (When finished: mark this checklist item done.)

- [ ] Add translation skeletons and i18n check
  - What: Ensure all strings use `qsTr()` and add `i18n/en_US.ts`, `i18n/es_ES.ts` placeholders. Add a script to extract strings and compile `.qm` files.


## Optional Spikes and Performance

- [ ] DI container spike (optional)
  - What: Evaluate `Boost.DI` and simple manual DI patterns; create `doc/13_DEPENDENCY_INJECTION.md` with recommendation. Implement a tiny `AppContainer` prototype if desired.
  - Prompt: `project-dashboard/prompt/27-di-spike-proto-size-spikes.md`  (When finished: mark this checklist item done.)

- [ ] Proto size & nanopb spike for embedded targets
  - What: Generate nanopb or protobuf-lite builds to measure code size and runtime cost on target. Document tradeoffs in `doc/14_PROTOCOL_BUFFERS.md`.


## Release & Packaging

- [ ] Final multi-stage Dockerfile and runtime optimization
  - What: Create builder and runtime stages using `qtapp-qt-dev-env:latest` and `qtapp-qt-runtime-nano:latest`. Ensure final image copies only runtime artifacts.

- [ ] Packaging and install target
  - What: Confirm `CMakeLists.txt` installs executable to `/opt/lesson##/` (or `/opt/project-dashboard/`) and create `release/README.md` with run instructions for macOS and Linux.


---

Notes and conventions

- Checklist items use unchecked boxes without numeric prefixes so items can be inserted freely.
- Where possible, produce small, incremental PRs with focused scope (e.g., "Add Database interface and tests", "Add QML skeleton and controllers").
- Prefer test-first approach where feasible: write interface + mocks + tests, then implement production code.
- Keep docs and artifacts next to code: `doc/`, `proto/`, `openapi/`, `scripts/`.
- When creating scripts that produce certs or keys, never check private keys into the repo; keep example certs in `central-server-simulator/certs/` for local dev only (with README explaining they are dev-only).

If you'd like, I can now:
- mark this item in the project's todo list as completed (I will after writing this file),
- or open a PR that adds `ZTODO.md` and the `scripts/render-mermaid.sh` placeholder.


## Service Interface Specifications (write detailed docs)

These documents should live under `doc/interfaces/` and include an interface overview, responsibilities, threading model, lifecycle/ownership rules, public method signatures (C++ style), error semantics, example code paths and sequence diagrams (where helpful), and a list of unit/integration tests the implementation must satisfy.

- Prompt: `project-dashboard/prompt/02-define-public-interfaces.md`  (When finished: mark this section's interface docs done.)

- [ ] `doc/interfaces/IDatabaseManager.md`
  - Purpose: Persistent storage and schema migrations for vitals, events, alarms, settings and users. Must support encrypted DB (SQLCipher) and an in-memory mode for tests.
  - Responsibilities:
    - Open/close database with optional encryption key.
    - Run schema migrations and report migration status.
    - Provide single-writer API for inserts (thread-safe enqueue to DB writer), and synchronous read/query APIs optimized for Trends queries.
    - Archive old data and expose archiver hooks.
  - Threading & ownership:
    - `IDatabaseManager` is owned by `AppContainer` (or `main()`); it spawns a dedicated DB-writer thread.
    - Read queries may be served on calling thread but with connection pooling or snapshot reads to avoid writer locks.
  - Key API (suggested signatures):
    - `virtual bool Open(const std::string &path, const std::optional<std::string>& encryptionKey) = 0;`
    - `virtual void Close() = 0;`
    - `virtual Result BeginTransaction() = 0;` / `Commit()` / `Rollback()`
    - `virtual Result EnqueueInsertVitals(const VitalsRecord &r) = 0;` (non-blocking)
    - `virtual std::vector<VitalsPoint> QueryVitals(const QueryRange &r) = 0;` (sync or async variant)
    - `virtual Result ExecuteMigration(const std::string &sql) = 0;`
    - `virtual Result ArchiveOlderThan(std::chrono::system_clock::time_point cutoff) = 0;`
  - Error handling: return `Result` with error codes (enum) rather than throwing exceptions; include `DB_LOCK`, `NOT_FOUND`, `MIGRATION_REQUIRED`, `ENCRYPTION_ERROR`.
  - Example code path: UI requests 1-hour trend -> `TrendsController::RequestTrend(metric, range)` -> calls `IDatabaseManager::QueryVitals(range)` -> data returned to QML (via `TrendsController` signal).
  - Tests to write: open/close, enqueue insert under concurrent producers, migration application order, archive operation correctness, encrypted DB open/close (smoke)

- [ ] `doc/interfaces/INetworkManager.md`
  - Purpose: Reliable, authenticated transport to central server; manages connection state and telemetry batching.
  - Responsibilities:
    - Configure TLS/mTLS credentials from `resources/certs/`.
    - Batch telemetry messages and send using backoff & retry; surface ack/failed delivery metrics.
    - Provide health and connection state to `SystemController`.
    - Integrate with `ITelemetryServer` interface for server communication.
  - Threading & ownership:
    - Runs worker thread(s) for network I/O or uses Qt event loop + QNetworkAccessManager on the main thread depending on platform; prefer dedicated network thread for blocking crypto ops.
  - Key API:
    - `virtual void ConfigureSsl(const SslConfig &cfg) = 0;`
    - `virtual ConnectionState Connect() = 0;`
    - `virtual void Disconnect() = 0;`
    - `virtual Result SendTelemetryBatch(const std::vector<TelemetryPacket>& batch, Callback ack) = 0;`
    - `virtual void SetRetryPolicy(const RetryPolicy &p) = 0;`
    - `virtual void SetServerUrl(const QString& url) = 0;`
    - `virtual QString GetServerUrl() const = 0;`
  - Security features: Certificate validation (expiration, revocation, device ID match), digital signatures on payloads, replay prevention (timestamp/nonce), rate limiting, circuit breaker, security audit logging.
  - Error semantics: surface transient vs permanent errors (e.g., `CERT_INVALID`, `CERT_EXPIRED`, `CERT_REVOKED`, `TLS_HANDSHAKE_FAILED`, `NETWORK_UNREACHABLE`, `RATE_LIMIT_EXCEEDED`).
  - Example code path: `DeviceSimulator` emits vitals -> `DashboardController` enqueues to `IDatabaseManager` and asks `INetworkManager` to send batched telemetry via `ITelemetryServer`; `NetworkManager` validates certificate, signs payload, sends with mTLS; on failure, `INetworkManager` persists unsent batches to disk via `IDatabaseManager` archival queue and logs security event to `security_audit_log`.
  - Tests to write: configurable failures, backoff timing behavior, SSL config validation, acknowledgment handling, server URL configuration, mock server integration, certificate validation, signature verification, replay prevention, rate limiting, audit logging.
  - Note: Interface documentation exists at `doc/interfaces/ITelemetryServer.md`. See `doc/06_SECURITY.md` section 6 for comprehensive security architecture.

- [ ] `doc/interfaces/ITelemetryServer.md`
  - Purpose: Interface for sending telemetry data and sensor data to a central monitoring server. Abstracts server communication to support multiple implementations (production, mock, file-based).
  - Responsibilities:
    - Send telemetry data batches to the server
    - Send sensor data to the server
    - Handle server responses (acknowledgments, error codes)
    - Manage connection state and health
    - Support configurable server endpoints (URL, port, protocol)
  - Key API:
    - `virtual void SetServerUrl(const QString& url) = 0;`
    - `virtual QString GetServerUrl() const = 0;`
    - `virtual bool Connect() = 0;`
    - `virtual void SendTelemetryAsync(const TelemetryData& data, std::function<void(const ServerResponse&)> callback) = 0;`
    - `virtual void SendSensorDataAsync(const SensorData& data, std::function<void(const ServerResponse&)> callback) = 0;`
  - Implementation variants: `NetworkTelemetryServer` (production), `MockTelemetryServer` (testing - swallows data), `FileTelemetryServer` (offline testing)
  - Tests to write: connection management, telemetry/sensor data transmission, error handling, mock server behavior, server URL configuration.
  - Note: Interface documentation exists at `doc/interfaces/ITelemetryServer.md`.

- [ ] `doc/interfaces/IAlarmManager.md`
  - Purpose: Centralized alarm evaluation, escalation, history and acknowledgment logic.
  - Responsibilities:
    - Accept alarm events (from analyzers or thresholds), deduplicate, prioritize, and emit signals for UI and logging.
    - Maintain alarm state (active, acknowledged, silenced), history and persistence via `IDatabaseManager`.
  - Threading & ownership:
    - Runs on `AlarmThread` or be co-located with `DeviceSimulator` on single-core devices; must be thread-safe for event submission.
  - Key API:
    - `virtual AlarmId RaiseAlarm(const AlarmDescriptor &desc) = 0;`
    - `virtual Result Acknowledge(AlarmId id, UserId by) = 0;`
    - `virtual Result Silence(AlarmPriority level, Duration d) = 0;`
    - `virtual std::vector<ActiveAlarm> GetActiveAlarms() = 0;`
    - `virtual std::vector<AlarmHistoryEntry> GetHistory(TimeRange r) = 0;`
  - Example code path: ECG analyzer detects arrhythmia -> `IAlarmManager::RaiseAlarm()` -> emits `OnAlarmRaised` -> `AlarmController` highlights card and triggers audible tone.
  - Tests to write: priority ordering, silence behavior, acknowledgement persistence, history correctness.

- [ ] `doc/interfaces/IDeviceSimulator.md`
  - Purpose: Deterministic or pseudo-random signal generator used for UI demos and tests.
  - Responsibilities:
    - Generate vitals streams (ECG, pleth), and inject events (arrhythmia, motion artifact) on schedule or via API.
    - Allow deterministic playback of recorded trace files for regression tests.
  - Key API:
    - `virtual void Start() = 0;`
    - `virtual void Stop() = 0;`
    - `virtual void LoadProfile(const SimulatorProfile&) = 0;`
    - `virtual void InjectEvent(const SimulatorEvent&) = 0;`
    - Signals: `OnVitalsSample(VitalsSample)`, `OnWaveformSample(WaveformSample)`.
  - Example code path: tests subscribe to `OnVitalsSample`, verify values drive `AlarmManager` logic.
  - Tests to write: deterministic playback matches expected triggers, event injection causes expected alarms.

- [ ] `doc/interfaces/IPatientLookupService.md`
  - Purpose: Interface for looking up patient information from external systems (HIS/EHR) by patient ID.
  - Responsibilities:
    - Query external patient information systems by patient ID (or MRN)
    - Return structured patient data including demographics, allergies, and safety information
    - Handle lookup failures gracefully (network errors, patient not found, etc.)
    - Support both synchronous and asynchronous lookup patterns
  - Key API:
    - `virtual std::optional<PatientInfo> LookupPatient(const QString& patientId) = 0;` (synchronous)
    - `virtual void LookupPatientAsync(const QString& patientId, std::function<void(const std::optional<PatientInfo>&)> callback) = 0;` (asynchronous)
    - `virtual bool IsAvailable() const = 0;`
    - `virtual QString GetLastError() const = 0;`
  - Implementation variants: `MockPatientLookupService` (testing), `NetworkPatientLookupService` (production), `DatabasePatientLookupService` (fallback)
  - Tests to write: synchronous/asynchronous lookups, error handling, concurrent lookups, integration with PatientManager.
  - Note: Interface documentation exists at `doc/interfaces/IPatientLookupService.md`.

- [ ] `doc/interfaces/ISettingsManager.md`
  - Purpose: Persistent configuration store for device settings and thresholds.
  - Responsibilities:
    - Read/write typed settings, validation, defaulting, and notification of changes.
    - Persist settings via `IDatabaseManager` or settings file.
    - Manage device configuration: Device ID, Bed ID, and Measurement Unit (metric/imperial).
  - Key API:
    - `virtual std::optional<SettingValue> Get(const std::string &key) = 0;`
    - `virtual Result Set(const std::string &key, const SettingValue &v) = 0;`
    - `virtual void Subscribe(SettingsObserver*) = 0;`
  - Required Settings:
    - `deviceId`: Unique device identifier (QString)
    - `bedId`: Bed/room location identifier (QString)
    - `measurementUnit`: Measurement system preference ("metric" or "imperial")
  - Tests to write: validation rules, persistence, migration of settings schema, device configuration persistence.

- [ ] `doc/interfaces/IAuthenticationService.md`
  - Purpose: PIN-based local authentication and role enforcement for UI actions.
  - Responsibilities:
    - Authenticate users (PIN), maintain current session, enforce simple lockout policies, provide role checks.
    - Audit login attempts into `system_events`/`alarms` as required.
  - Threading & ownership:
    - Lightweight; called from UI thread. For cryptographic or slow ops, run on a worker thread.
  - Key API:
    - `virtual AuthResult AuthenticatePin(const UserId &user, const std::string &pin) = 0;`
    - `virtual void Logout() = 0;`
    - `virtual bool HasRole(const UserId &user, Role r) const = 0;`
    - `virtual CurrentUserInfo GetCurrentUser() const = 0;`
    - `virtual Result ChangePin(const UserId &, const std::string &oldPin, const std::string &newPin) = 0;`
    - `virtual void RefreshSession() = 0;` (refresh session timeout on activity)
    - `virtual bool CheckSessionTimeout() = 0;` (check if session expired)
    - `virtual bool IsAccountLocked(const UserId &user) = 0;` (check lockout status)
    - `virtual int GetRemainingLockoutTime(const UserId &user) = 0;` (seconds until unlock)
    - `virtual Result UnlockAccount(const UserId &user) = 0;` (requires admin role)
  - Example code path: user enters PIN in `LoginView.qml` -> `AuthenticationService::AuthenticatePin` -> on success emit `OnAuthStateChanged` -> `SystemController` transitions to logged-in state and updates UI.
  - Security notes: 
    - PINs must be stored hashed (SHA-256) + salted (per-user salt)
    - Brute force protection: 5 failed attempts → 15-minute lockout, exponential backoff
    - Session timeout: 30 minutes of inactivity (configurable)
    - All authentication events logged to `security_audit_log`
    - Consider hardware-backed key storage on target platforms
  - Tests to write: correct/incorrect PIN, lockout behavior, role checks.

- [ ] `doc/interfaces/IArchiver.md`
  - Purpose: Responsible for moving expired data out of the primary DB into compressed archive stores and/or remote upload staging.
  - Responsibilities:
    - Batch archival jobs, create archive packages (proto or compressed sqlite), optionally upload to server and remove local rows.
    - Support dry-run mode for verification.
  - Key API:
    - `virtual Result CreateArchive(TimeRange r, ArchiveDescriptor &out) = 0;`
    - `virtual Result UploadArchive(const ArchiveDescriptor &a) = 0;`
    - `virtual Result PurgeArchived(TimeRange r) = 0;`
  - Tests to write: archive creation correctness, safe purge, resume/retry behavior on failures.

- [ ] `doc/interfaces/ILogService.md`
  - Purpose: Centralized logging with a QAbstractListModel to expose log records to QML Diagnostics view.
  - Responsibilities:
    - Append logs with levels and timestamps; allow querying and tailing.
  - Key API:
    - `virtual void Append(LogLevel level, const std::string &msg) = 0;`
    - `virtual QAbstractListModel* AsQmlModel() = 0;`
  - Tests: ensure log ordering, level filtering, model bindings to QML.

- [ ] `doc/interfaces/Controllers.md`
  - Purpose: Document each QML-facing controller and the properties/methods they expose.
  - For each controller include:
    - `DashboardController` — properties: `heartRate`, `spo2`, `ecgWaveformModel`; methods: `StartMonitoring()`, `StopMonitoring()`, `RequestTrend(range)`.
    - `AlarmController` — properties: `activeAlarmsModel`, `historyModel`; methods: `AcknowledgeAlarm(id)`, `Silence(duration)`.
    - `SystemController` — properties: `connectionState`, `appVersion`, `clock`; methods: `Reboot()`, `Shutdown()`.
    - `PatientController` — properties: `patientId`, `patientName`, `patientAge`, `allergies`, `isLookingUp`, `lookupError`; methods: `lookupPatientById(id)`, `clearPatient()`; include examples of QML bindings for Patient Assignment View.
    - `SettingsController` — properties: `deviceId`, `bedId`, `measurementUnit`, `allSettings`; methods: `updateSetting(key, value)`, `resetToDefaults()`; include examples of QML bindings for Device Configuration section.
    - `TrendsController`, `NotificationController` — list properties and key methods; include examples of QML bindings and signal usage.
  - Tests: QML binding smoke tests, property change notifications, method-call round trips.

Action notes:
- File paths: create `doc/interfaces/*.md` for each interface and `doc/interfaces/Controllers.md`.
- Deliverables per interface doc: responsibilities, signatures, example code paths, tests list, and a short sequence diagram (Mermaid) where helpful.
- Prioritization: start with `IDatabaseManager`, `INetworkManager`, `IAlarmManager`, `IAuthenticationService`, then controllers.


## Sequential Tasks (must be done in order)

- [ ] 1. Create project scaffolding and repo checklist
  - What: Ensure `project-dashboard/` contains the canonical folders: `src/`, `resources/`, `doc/`, `proto/`, `openapi/`, `tests/`, `central-server-simulator/` and `doc/migrations/`.
  - Why: Provides a stable structure to place interfaces, tests, and docs.
  - Artifacts: `CMakeLists.txt` (top-level), empty `proto/` and `openapi/` dirs, `doc/migrations/README.md`.

- [ ] 1. Define public C++ service interfaces (headers only)
  - What: Create minimal header-only interface sketches for: `IDatabaseManager`, `INetworkManager`, `IAlarmManager`, `IDeviceSimulator`, `ISettingsManager`, `IAuthenticationService`, and `IArchiver`.
  - Why: Interfaces allow test-first development (mocks) and make DI decisions easier.
  - Files: `src/core/interfaces/*.h` (one header per interface), `doc/interfaces.md` with rationale and method signatures.

- [ ] 1. Create unit test harness + mock objects
  - What: Add `tests/CMakeLists.txt`, pick test framework (recommend `GoogleTest`), add `tests/mocks/` with mock classes that implement the interfaces.
  - Why: Unit tests should drive API decisions. Mocks let you write controller tests before production implementation.
  - Files: `tests/CMakeLists.txt`, `tests/mock_DatabaseManager.h`, `tests/mock_NetworkManager.h`, example test `tests/test_alarm_manager.cpp`.

- [ ] 1. Design database schema + write migration SQLs
  - What: Finalize DDL for tables: `patients`, `vitals`, `ecg_samples`, `pleth_samples`, `alarms`, `system_events`, `settings`, `users`. Add indices, retention metadata table, and `archival_queue`.
  - Why: Deterministic schema is required before implementing `DatabaseManager` and `TrendsController`.
  - Files: `doc/migrations/0001_initial.sql`, `doc/migrations/0002_add_indices.sql`, `doc/10_DATABASE_DESIGN.md` update, ERD SVG in `doc/`.

- [ ] 1. Implement DatabaseManager spike (in-memory + SQLCipher plan)
  - What: Implement a minimal, test-only `DatabaseManager` that uses an in-memory SQLite for tests. Document how SQLCipher will be integrated and add CMake options to enable/disable SQLCipher.
  - Why: Validates schema and migrations without full SQLCipher integration yet.
  - Files: `src/core/DatabaseManagerStub.cpp/h`, `tests/db_smoke_test.cpp`, `CMakeLists` options: `-DENABLE_SQLCIPHER=ON/OFF`.

- [ ] 1. Define telemetry proto and/or OpenAPI spec (canonical schema)
  - What: Create `proto/telemetry.proto` and `openapi/telemetry.yaml`. Include message definitions for vitals, device status, alarms, and batching semantics.
  - Why: Having canonical schema lets the simulator, server, and device agree on payloads. Protobuf + JSON mapping recommended.
  - Files: `proto/telemetry.proto`, `openapi/telemetry.yaml`, `doc/proto_design.md`.

- [ ] 1. Implement basic NetworkManager test double + API contract
  - What: Using the proto/OpenAPI, implement a mock `NetworkManager` (no TLS) that records requests and simulates server responses (200, 500, timeout). Add unit tests for retry and backoff behavior.
  - Why: Allows `SystemController`/`NotificationController` unit tests before adding mTLS plumbing.
  - Files: `tests/mock_NetworkManager.h`, `tests/network_retry_test.cpp`.

- [ ] 1. Implement controller skeletons and QML binding stubs
  - What: Create `DashboardController`, `AlarmController`, `SystemController`, `PatientController`, `SettingsController`, `TrendsController`, `NotificationController` as QObject-derived classes exposing Q_PROPERTY and basic signals. Do not implement heavy logic yet.
  - Why: QML UI can be wired to properties and tested for binding behavior early.
  - Files: `src/ui/*.cpp/h` and `resources/qml/Main.qml` with placeholder components.
  - Note: `SettingsController` must expose `deviceId`, `bedId`, and `measurementUnit` as Q_PROPERTY for Device Configuration section in Settings View.


## Parallel Tasks (can be done concurrently)

- [ ] 1. QML UI skeleton and components
  - What: Implement `Main.qml`, `Sidebar.qml`, `TopBar.qml`, `StatCard.qml`, `PatientBanner.qml`, and placeholder `views/` (DashboardView, DiagnosticsView, TrendsView, SettingsView, LoginView).
  - Why: Visual scaffolding enables early UX validation and manual QA.
  - Acceptance: QML app boots and displays placeholders at `1280x800`.
  - Note: `SettingsView.qml` must include Device Configuration section with inputs for Device ID, Bed ID, and Measurement Unit dropdown (metric/imperial). See `doc/03_UI_UX_GUIDE.md` section 4.4 for specifications.

- [ ] 1. Alarm UI & animation prototypes (QML)
  - What: Prototype critical alarm full-screen flash, per-card highlight, audio stubs, and Alarm History panel in QML.
  - Why: Visual design for alarms should be validated separately from backend logic.

- [ ] 1. DeviceSimulator and synthetic signal generation
  - What: Implement a test-only `DeviceSimulator` capable of generating vitals, ECG waveform samples, pleth waveform, and simulated events (arrhythmia, motion artifact) for UI demos.
  - Why: Provides deterministic input for UI, controller, and integration tests.

- [ ] 1. Central server simulator (mTLS later)
  - What: Create `central-server-simulator/` with a simple REST endpoint `POST /api/telemetry` that can accept JSON and returns ack. Implement toggles to simulate network failures and delays.
  - Why: Enables local end-to-end testing of networking flows.

- [ ] 1. Implement mockable logging and LogService binding to QML
  - What: Create `LogService` that queues messages and exposes them to QML as a model for the Diagnostics view. Support different levels (Info/Warn/Error).
  - Why: Diagnostics and logs are required for debugging and QA.


## Security & Certificates (ordered but distinct)

- [ ] 1. Define security architecture and provisioning plan
  - What: Finalize how device certificates will be provisioned, where certs are stored in `resources/certs/`, and the CA trust model. Document in `doc/06_SECURITY.md`.
  - Why: Security design must be agreed before writing any cert-generation scripts.

- [ ] 1. Add scripts for CA + cert generation (after infra agreed)
  - What: Create `scripts/generate-selfsigned-certs.sh` that generates CA, server, and client certs for local testing. Include instructions for converting to PKCS12 if needed.
  - Why: Provides reproducible certs for simulator and device tests. NOTE: create *after* the previous task is approved.
  - Files: `scripts/generate-selfsigned-certs.sh`, `central-server-simulator/certs/README.md`.

- [ ] 1. mTLS integration spike for NetworkManager
  - What: Implement a small C++ example that configures `QSslConfiguration` with the generated client cert and validates handshake against the simulator using mutual auth.
  - Why: Confirms approach works on target platforms before full NetworkManager implementation.


## Database Encryption & Archival (dependent)

- [ ] 1. SQLCipher integration plan and build spike
  - What: Research how to add SQLCipher to the CMake build for macOS/Linux and add a spike that compiles and links SQLCipher for local runs.
  - Why: Encryption-at-rest is mandatory for patient data.

- [ ] 1. Implement Archiver interface and archiving tests
  - What: Create `IArchiver` interface and tests that show moving rows older than 7 days to an `archived_vitals` table or external archive file. Add unit tests for retention policy enforcement.
  - Why: Archival is required by requirements; must be testable and configurable.


## Testing, CI & Automation (parallelizable)

- [ ] 1. Add CI workflows for build + tests
  - What: Add GitHub Actions (or preferred CI) jobs: `build`, `unit-tests`, `render-diagrams`, `integration-tests` that run the server simulator.
  - Why: Keeps repo healthy and verifies that docs/diagrams render correctly in CI.

- [ ] 1. Add mermaid render script and CI check
  - What: Add `scripts/render-mermaid.sh` and a CI job that runs it and fails on parse errors. Document usage in `.github/copilot-instructions.md`.
  - Why: Prevents malformed diagrams from being committed (we had parser issues earlier).

- [ ] 1. Add E2E containerized test harness
  - What: Compose the Z Monitor (headless) and the server simulator in docker-compose test environment and run basic E2E scenarios.
  - Why: Validates connectivity, DB writes, and archival behavior in a reproducible environment.


## Documentation, Compliance & Diagrams

- [ ] 1. Update `doc/10_DATABASE_DESIGN.md` and add ERD
  - What: Consolidate the extended DDL into `doc/10_DATABASE_DESIGN.md`, include ERD and index rationale, and retention/archival notes.

- [ ] 1. Produce API docs: OpenAPI + proto docs
  - What: Finalize `openapi/telemetry.yaml` and ensure codegen steps are documented in `doc/`. Add `doc/api/README.md` describing mapping between proto and JSON.

- [ ] 1. Create SRS and V&V outlines
  - What: Add `doc/SRS.md` (feature list, acceptance criteria) and `doc/VVPlan.md` for verification and validation testing; include list of safety-critical tests.

- [ ] 1. Create threat model summary and FMEA sketch
  - What: Draft `doc/threat_model.md` and `doc/FMEA.md` focusing on data confidentiality (at-rest/in-transit), certificate compromise, tampering, and mitigations.


## UX & Clinical Validation

- [ ] 1. Perform UI walkthrough and polish
  - What: Iterate on the QML layout for `1280x800`, validate readability of stat cards, colors, and alarm indicators with clinical stakeholders.

- [ ] 1. Add translation skeletons and i18n check
  - What: Ensure all strings use `qsTr()` and add `i18n/en_US.ts`, `i18n/es_ES.ts` placeholders. Add a script to extract strings and compile `.qm` files.


## Optional Spikes and Performance

- [ ] 1. DI container spike (optional)
  - What: Evaluate `Boost.DI` and simple manual DI patterns; create `doc/13_DEPENDENCY_INJECTION.md` with recommendation. Implement a tiny `AppContainer` prototype if desired.

- [ ] 1. Proto size & nanopb spike for embedded targets
  - What: Generate nanopb or protobuf-lite builds to measure code size and runtime cost on target. Document tradeoffs in `doc/14_PROTOCOL_BUFFERS.md`.


## Release & Packaging

- [ ] 1. Final multi-stage Dockerfile and runtime optimization
  - What: Create builder and runtime stages using `qtapp-qt-dev-env:latest` and `qtapp-qt-runtime-nano:latest`. Ensure final image copies only runtime artifacts.

- [ ] 1. Packaging and install target
  - What: Confirm `CMakeLists.txt` installs executable to `/opt/lesson##/` (or `/opt/project-dashboard/`) and create `release/README.md` with run instructions for macOS and Linux.


---

Notes and conventions

- Use `1.` for every checklist item so items can be freely inserted without renumbering.
- Where possible, produce small, incremental PRs with focused scope (e.g., "Add Database interface and tests", "Add QML skeleton and controllers").
- Prefer test-first approach where feasible: write interface + mocks + tests, then implement production code.
- Keep docs and artifacts next to code: `doc/`, `proto/`, `openapi/`, `scripts/`.
- When creating scripts that produce certs or keys, never check private keys into the repo; keep example certs in `central-server-simulator/certs/` for local dev only (with README explaining they are dev-only).

If you'd like, I can now:
- mark this item in the project's todo list as completed (I will after writing this file),
- or open a PR that adds `ZTODO.md` and the `scripts/render-mermaid.sh` placeholder.


## Service Interface Specifications (write detailed docs)

These documents should live under `doc/interfaces/` and include an interface overview, responsibilities, threading model, lifecycle/ownership rules, public method signatures (C++ style), error semantics, example code paths and sequence diagrams (where helpful), and a list of unit/integration tests the implementation must satisfy.

- [ ] 1. `doc/interfaces/IDatabaseManager.md`
  - Purpose: Persistent storage and schema migrations for vitals, events, alarms, settings and users. Must support encrypted DB (SQLCipher) and an in-memory mode for tests.
  - Responsibilities:
    - Open/close database with optional encryption key.
    - Run schema migrations and report migration status.
    - Provide single-writer API for inserts (thread-safe enqueue to DB writer), and synchronous read/query APIs optimized for Trends queries.
    - Archive old data and expose archiver hooks.
  - Threading & ownership:
    - `IDatabaseManager` is owned by `AppContainer` (or `main()`); it spawns a dedicated DB-writer thread.
    - Read queries may be served on calling thread but with connection pooling or snapshot reads to avoid writer locks.
  - Key API (suggested signatures):
    - `virtual bool Open(const std::string &path, const std::optional<std::string>& encryptionKey) = 0;`
    - `virtual void Close() = 0;`
    - `virtual Result BeginTransaction() = 0;` / `Commit()` / `Rollback()`
    - `virtual Result EnqueueInsertVitals(const VitalsRecord &r) = 0;` (non-blocking)
    - `virtual std::vector<VitalsPoint> QueryVitals(const QueryRange &r) = 0;` (sync or async variant)
    - `virtual Result ExecuteMigration(const std::string &sql) = 0;`
    - `virtual Result ArchiveOlderThan(std::chrono::system_clock::time_point cutoff) = 0;`
  - Error handling: return `Result` with error codes (enum) rather than throwing exceptions; include `DB_LOCK`, `NOT_FOUND`, `MIGRATION_REQUIRED`, `ENCRYPTION_ERROR`.
  - Example code path: UI requests 1-hour trend -> `TrendsController::RequestTrend(metric, range)` -> calls `IDatabaseManager::QueryVitals(range)` -> data returned to QML (via `TrendsController` signal).
  - Tests to write: open/close, enqueue insert under concurrent producers, migration application order, archive operation correctness, encrypted DB open/close (smoke)

- [ ] 1. `doc/interfaces/INetworkManager.md`
  - Purpose: Reliable, authenticated transport to central server; manages connection state and telemetry batching.
  - Responsibilities:
    - Configure TLS/mTLS credentials from `resources/certs/`.
    - Batch telemetry messages and send using backoff & retry; surface ack/failed delivery metrics.
    - Provide health and connection state to `SystemController`.
    - Integrate with `ITelemetryServer` interface for server communication.
  - Threading & ownership:
    - Runs worker thread(s) for network I/O or uses Qt event loop + QNetworkAccessManager on the main thread depending on platform; prefer dedicated network thread for blocking crypto ops.
  - Key API:
    - `virtual void ConfigureSsl(const SslConfig &cfg) = 0;`
    - `virtual ConnectionState Connect() = 0;`
    - `virtual void Disconnect() = 0;`
    - `virtual Result SendTelemetryBatch(const std::vector<TelemetryPacket>& batch, Callback ack) = 0;`
    - `virtual void SetRetryPolicy(const RetryPolicy &p) = 0;`
    - `virtual void SetServerUrl(const QString& url) = 0;`
    - `virtual QString GetServerUrl() const = 0;`
  - Error semantics: surface transient vs permanent errors (e.g., `CERT_INVALID`, `TLS_HANDSHAKE_FAILED`, `NETWORK_UNREACHABLE`).
  - Example code path: `DeviceSimulator` emits vitals -> `DashboardController` enqueues to `IDatabaseManager` and asks `INetworkManager` to send batched telemetry via `ITelemetryServer`; on failure, `INetworkManager` persists unsent batches to disk via `IDatabaseManager` archival queue.
  - Tests to write: configurable failures, backoff timing behavior, SSL config validation, acknowledgment handling, server URL configuration, mock server integration.

- [ ] 1. `doc/interfaces/ITelemetryServer.md`
  - Purpose: Interface for sending telemetry data and sensor data to a central monitoring server. Abstracts server communication to support multiple implementations (production, mock, file-based).
  - Responsibilities:
    - Send telemetry data batches to the server
    - Send sensor data to the server
    - Handle server responses (acknowledgments, error codes)
    - Manage connection state and health
    - Support configurable server endpoints (URL, port, protocol)
  - Key API:
    - `virtual void SetServerUrl(const QString& url) = 0;`
    - `virtual QString GetServerUrl() const = 0;`
    - `virtual bool Connect() = 0;`
    - `virtual void SendTelemetryAsync(const TelemetryData& data, std::function<void(const ServerResponse&)> callback) = 0;`
    - `virtual void SendSensorDataAsync(const SensorData& data, std::function<void(const ServerResponse&)> callback) = 0;`
  - Implementation variants: `NetworkTelemetryServer` (production), `MockTelemetryServer` (testing - swallows data), `FileTelemetryServer` (offline testing)
  - Tests to write: connection management, telemetry/sensor data transmission, error handling, mock server behavior, server URL configuration.
  - Note: Interface documentation exists at `doc/interfaces/ITelemetryServer.md`.

- [ ] 1. `doc/interfaces/IAlarmManager.md`
  - Purpose: Centralized alarm evaluation, escalation, history and acknowledgment logic.
  - Responsibilities:
    - Accept alarm events (from analyzers or thresholds), deduplicate, prioritize, and emit signals for UI and logging.
    - Maintain alarm state (active, acknowledged, silenced), history and persistence via `IDatabaseManager`.
  - Threading & ownership:
    - Runs on `AlarmThread` or be co-located with `DeviceSimulator` on single-core devices; must be thread-safe for event submission.
  - Key API:
    - `virtual AlarmId RaiseAlarm(const AlarmDescriptor &desc) = 0;`
    - `virtual Result Acknowledge(AlarmId id, UserId by) = 0;`
    - `virtual Result Silence(AlarmPriority level, Duration d) = 0;`
    - `virtual std::vector<ActiveAlarm> GetActiveAlarms() = 0;`
    - `virtual std::vector<AlarmHistoryEntry> GetHistory(TimeRange r) = 0;`
  - Example code path: ECG analyzer detects arrhythmia -> `IAlarmManager::RaiseAlarm()` -> emits `OnAlarmRaised` -> `AlarmController` highlights card and triggers audible tone.
  - Tests to write: priority ordering, silence behavior, acknowledgement persistence, history correctness.

- [ ] 1. `doc/interfaces/IDeviceSimulator.md`
  - Purpose: Deterministic or pseudo-random signal generator used for UI demos and tests.
  - Responsibilities:
    - Generate vitals streams (ECG, pleth), and inject events (arrhythmia, motion artifact) on schedule or via API.
    - Allow deterministic playback of recorded trace files for regression tests.
  - Key API:
    - `virtual void Start() = 0;`
    - `virtual void Stop() = 0;`
    - `virtual void LoadProfile(const SimulatorProfile&) = 0;`
    - `virtual void InjectEvent(const SimulatorEvent&) = 0;`
    - Signals: `OnVitalsSample(VitalsSample)`, `OnWaveformSample(WaveformSample)`.
  - Example code path: tests subscribe to `OnVitalsSample`, verify values drive `AlarmManager` logic.
  - Tests to write: deterministic playback matches expected triggers, event injection causes expected alarms.

- [ ] 1. `doc/interfaces/ISettingsManager.md`
  - Purpose: Persistent configuration store for device settings and thresholds.
  - Responsibilities:
    - Read/write typed settings, validation, defaulting, and notification of changes.
    - Persist settings via `IDatabaseManager` or settings file.
    - Manage device configuration: Device ID, Bed ID, and Measurement Unit (metric/imperial).
  - Key API:
    - `virtual std::optional<SettingValue> Get(const std::string &key) = 0;`
    - `virtual Result Set(const std::string &key, const SettingValue &v) = 0;`
    - `virtual void Subscribe(SettingsObserver*) = 0;`
  - Required Settings:
    - `deviceId`: Unique device identifier (QString)
    - `bedId`: Bed/room location identifier (QString)
    - `measurementUnit`: Measurement system preference ("metric" or "imperial")
  - Tests to write: validation rules, persistence, migration of settings schema, device configuration persistence.

- [ ] 1. `doc/interfaces/IAuthenticationService.md`
  - Purpose: PIN-based local authentication and role enforcement for UI actions.
  - Responsibilities:
    - Authenticate users (PIN), maintain current session, enforce simple lockout policies, provide role checks.
    - Audit login attempts into `system_events`/`alarms` as required.
  - Threading & ownership:
    - Lightweight; called from UI thread. For cryptographic or slow ops, run on a worker thread.
  - Key API:
    - `virtual AuthResult AuthenticatePin(const UserId &user, const std::string &pin) = 0;`
    - `virtual void Logout() = 0;`
    - `virtual bool HasRole(const UserId &user, Role r) const = 0;`
    - `virtual CurrentUserInfo GetCurrentUser() const = 0;`
    - `virtual Result ChangePin(const UserId &, const std::string &oldPin, const std::string &newPin) = 0;`
  - Example code path: user enters PIN in `LoginView.qml` -> `AuthenticationService::AuthenticatePin` -> on success emit `OnAuthStateChanged` -> `SystemController` transitions to logged-in state and updates UI.
  - Security notes: PINs must be stored hashed + salted; consider hardware-backed key storage on target platforms; rate-limit attempts to mitigate brute force.
  - Tests to write: correct/incorrect PIN, lockout behavior, role checks.

- [ ] 1. `doc/interfaces/IArchiver.md`
  - Purpose: Responsible for moving expired data out of the primary DB into compressed archive stores and/or remote upload staging.
  - Responsibilities:
    - Batch archival jobs, create archive packages (proto or compressed sqlite), optionally upload to server and remove local rows.
    - Support dry-run mode for verification.
  - Key API:
    - `virtual Result CreateArchive(TimeRange r, ArchiveDescriptor &out) = 0;`
    - `virtual Result UploadArchive(const ArchiveDescriptor &a) = 0;`
    - `virtual Result PurgeArchived(TimeRange r) = 0;`
  - Tests to write: archive creation correctness, safe purge, resume/retry behavior on failures.

- [ ] 1. `doc/interfaces/ILogService.md`
  - Purpose: Centralized logging with a QAbstractListModel to expose log records to QML Diagnostics view.
  - Responsibilities:
    - Append logs with levels and timestamps; allow querying and tailing.
  - Key API:
    - `virtual void Append(LogLevel level, const std::string &msg) = 0;`
    - `virtual QAbstractListModel* AsQmlModel() = 0;`
  - Tests: ensure log ordering, level filtering, model bindings to QML.

- [ ] 1. `doc/interfaces/Controllers.md`
  - Purpose: Document each QML-facing controller and the properties/methods they expose.
  - For each controller include:
    - `DashboardController` — properties: `heartRate`, `spo2`, `ecgWaveformModel`; methods: `StartMonitoring()`, `StopMonitoring()`, `RequestTrend(range)`.
    - `AlarmController` — properties: `activeAlarmsModel`, `historyModel`; methods: `AcknowledgeAlarm(id)`, `Silence(duration)`.
    - `SystemController` — properties: `connectionState`, `appVersion`, `clock`; methods: `Reboot()`, `Shutdown()`.
    - `PatientController` — properties: `patientId`, `patientName`, `patientAge`, `allergies`, `isLookingUp`, `lookupError`; methods: `lookupPatientById(id)`, `clearPatient()`; include examples of QML bindings for Patient Assignment View.
    - `SettingsController` — properties: `deviceId`, `bedId`, `measurementUnit`, `allSettings`; methods: `updateSetting(key, value)`, `resetToDefaults()`; include examples of QML bindings for Device Configuration section.
    - `TrendsController`, `NotificationController` — list properties and key methods; include examples of QML bindings and signal usage.
  - Tests: QML binding smoke tests, property change notifications, method-call round trips.

Action notes:
- File paths: create `doc/interfaces/*.md` for each interface and `doc/interfaces/Controllers.md`.
- Deliverables per interface doc: responsibilities, signatures, example code paths, tests list, and a short sequence diagram (Mermaid) where helpful.
- Prioritization: start with `IDatabaseManager`, `INetworkManager`, `IAlarmManager`, `IAuthenticationService`, then controllers.


## ZTODO (Low Priority)

- [ ] Investigate `QCoreApplication::quit()` not exiting the app reliably
  - What: In `project-dashboard/sensor-simulator/Simulator.cpp` we observed cases where calling `QCoreApplication::quit()` did not terminate the process inside the container. A temporary fallback (`std::exit(0)`) was added to `Simulator::quitApp()` to guarantee container termination.
  - Why: This is likely due to threads or blocking operations preventing the Qt event loop from exiting, or cleanup tasks stalling. Investigate thread lifecycles, pending timers, and long-running blocking work that might keep the event loop alive. Once resolved, remove the `std::exit(0)` fallback and ensure graceful shutdown and proper resource cleanup.
  - Priority: Low — leave for later investigation after higher-priority tasks are completed.
  - Acceptance: `QCoreApplication::quit()` cleanly returns control from `app.exec()` and the process exits without needing `std::exit(0)`; update `Simulator::quitApp()` to remove forced exit.

- [x] Improve simulator UI: show last 50 messages with larger font and richer fields
  - What: Updated `project-dashboard/sensor-simulator/qml/Main.qml` to present the last 50 messages, showing time, message id, sensor/source, level, and message text with larger font sizes.
  - Why: Improves readability and usability during demos and manual testing.
  - Acceptance: The simulator UI lists up to 50 most-recent messages and is easier to read; README updated to note the change.

