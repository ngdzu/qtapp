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

## Networking Tasks

*(No tasks currently assigned to this category)*

- [ ] TASK-NET-001: Define Telemetry Protocol Buffers
  - What: Create .proto files defining the telemetry data structure for sensor readings. Define messages for SensorData, SystemStatus, and Alert. Include fields for timestamp, sensor ID, value, unit, and status code. Configure CMake to generate C++ classes from .proto files using protobuf_generate_cpp.
  - Why: **DATA SERIALIZATION:** Efficient and structured data transmission requires a defined protocol. Protocol Buffers provide a compact binary format, schema evolution support, and generated C++ classes, ensuring type safety and performance for high-frequency sensor data.
  - Files:
    - [z-monitor/src/infrastructure/network/proto/telemetry.proto](z-monitor/proto/telemetry.proto) (create proto definition)
    - [z-monitor/src/infrastructure/network/CMakeLists.txt](z-monitor/src/infrastructure/CMakeLists.txt) (configure protobuf generation)
    - [z-monitor/src/domain/entities/SensorData.h](z-monitor/src/domain) (update to use/map to proto classes if needed)
  - Acceptance:
    - telemetry.proto defines all required messages and fields
    - CMake generates C++ classes (telemetry.pb.h, telemetry.pb.cc) during build
    - Generated classes compile without errors
    - Unit tests verify serialization and deserialization of telemetry messages
  - Verification Steps:
    1. Functional: Proto files compile, C++ classes generated, serialization/deserialization works correctly. **Status:** ⏳ Pending implementation
    2. Code Quality: Proto definitions follow style guide, generated code compiles cleanly, CMake configuration correct. **Status:** ⏳ Pending implementation
    3. Documentation: Proto file documented with comments, generated API usage documented. **Status:** ⏳ Pending implementation
    4. Integration: Generated classes integrated into build system, accessible from other modules. **Status:** ⏳ Pending implementation
    5. Tests: Unit tests for serialization/deserialization of all message types. **Status:** ⏳ Pending implementation
  - Dependencies: Requires Protocol Buffers compiler (protoc) and library
  - Documentation: See Protocol Buffers documentation, [doc/02_ARCHITECTURE.md](doc/architecture/DOC-ARCH-005_data_flow_and_caching.md) (Data Flow)

- [ ] TASK-NET-002: Implement Mock Network Manager
  - What: Create a MockNetworkManager class implementing the INetworkManager interface. This mock should simulate network connectivity states (connected, disconnected, connecting) and data transmission (send success/failure). It will be used for testing the application's behavior under various network conditions without requiring a real network connection.
  - Why: **TESTABILITY:** Testing network-dependent features (e.g., data sync, alert transmission) requires control over network state. A mock manager allows deterministic testing of connection handling, retry logic, and error reporting, ensuring robustness in unstable network environments.
  - Files:
    - [z-monitor/tests/mocks/MockNetworkManager.h](z-monitor/src/infrastructure/network/MockNetworkManager.h) (create mock class)
    - [z-monitor/tests/mocks/MockNetworkManager.cpp](z-monitor/src/infrastructure/network/MockNetworkManager.cpp) (implement mock logic)
    - [z-monitor/src/domain/interfaces/INetworkManager.h](z-monitor/src/domain/interfaces) (ensure interface is testable)
    - [z-monitor/tests/unit/infrastructure/network/NetworkManagerTest.cpp](z-monitor/tests/unit/infrastructure/network) (test the mock itself)
  - Acceptance:
    - MockNetworkManager implements all INetworkManager methods
    - Mock allows setting connection state (Connected, Disconnected)
    - Mock allows simulating send success and failure
    - Unit tests verify mock behavior matches expected interface contract
  - Verification Steps:
    1. Functional: Mock behaves as expected (simulates states, returns configured results). **Status:** ⏳ Pending implementation
    2. Code Quality: Mock follows naming conventions, Doxygen comments, clean implementation. **Status:** ⏳ Pending implementation
    3. Documentation: Mock usage documented in test guide. **Status:** ⏳ Pending implementation
    4. Integration: Mock usable in other unit tests (e.g., DataSyncTest). **Status:** ⏳ Pending implementation
    5. Tests: Unit tests for MockNetworkManager verify state transitions and return values. **Status:** ⏳ Pending implementation
  - Dependencies: INetworkManager interface definition
  - Documentation: See [doc/testing/TEST-GUIDE-001_mocking_strategy.md](doc/foundation/08_testing_strategies/02_test_doubles.md)
