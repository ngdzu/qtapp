---
doc_id: DOC-ARCH-004
title: Technology Stack
version: v1.0
category: Architecture
subcategory: Architecture
status: Draft
owner: Architecture Team
reviewers: 
  - Architecture Team
last_reviewed: 2025-12-01
next_review: 2025-12-01
related_docs:
  - DOC-API-XXX  # Interface specification
  - DOC-ARCH-XXX # Related architecture
related_tasks:
  - TASK-XXX-XXX # Implementation task
related_requirements:
  - REQ-XXX-XXX  # Related requirement
tags:
  - {keyword1}
  - {keyword2}
diagram_files:
  - DOC-COMP-XXX_{diagram_name}.mmd
  - DOC-COMP-XXX_{diagram_name}.svg
---

# DOC-ARCH-004: Technology Stack

## 1. Overview

This document details the technology stack used in the Z Monitor project, covering UI frameworks, backend languages, build systems, databases, networking, and documentation tools.

---

## 2. Core Technologies

### 2.1 UI Framework

**Technology:** Qt 6 with QML

**Version:** Qt 6.x (latest LTS recommended)

**Purpose:** Cross-platform UI framework for embedded touch-screen device

**Key Features:**
- **QML:** Declarative UI language for responsive, touch-friendly interfaces
- **Qt Quick:** Hardware-accelerated rendering engine
- **Qt Quick Controls:** Pre-built UI components (buttons, sliders, etc.)
- **Canvas API:** JavaScript-based waveform rendering

**Rationale:**
- Native performance on embedded hardware
- Touch-first design (required for 8-inch touch screen)
- Declarative syntax reduces UI complexity
- Hardware-accelerated graphics for 60 FPS waveforms

**Alternatives Considered:**
- ❌ Qt Widgets: Less suited for touch interfaces, imperative API
- ❌ Web Technologies (Electron): Excessive memory/CPU overhead for embedded
- ❌ Custom OpenGL: Reinventing the wheel, no UI components

---

### 2.2 Backend Language

**Technology:** C++17

**Compiler Support:** GCC 7+, Clang 5+, MSVC 2017+

**Purpose:** Core business logic, data processing, hardware simulation

**Key Features Used:**
- **std::optional:** Null-safe optional values
- **std::variant:** Type-safe unions for error handling
- **Structured Bindings:** Destructuring for cleaner code
- **Lambda Improvements:** Generic lambdas, capture this by value
- **constexpr if:** Compile-time conditionals

**Rationale:**
- Performance critical for real-time alarm detection (<50ms)
- Type safety for medical device compliance
- Standard library improvements over C++11/14
- Compiler availability on target platform

**Why Not C++20/23:**
- Toolchain support on embedded Linux may be limited
- C++17 provides sufficient features for requirements
- Stability over bleeding-edge features for medical device

---

### 2.3 Build System

**Technology:** CMake 3.16+

**Purpose:** Cross-platform build configuration and dependency management

**Key Features:**
- `find_package(Qt6)` for Qt integration
- `protobuf_generate_cpp()` for Protocol Buffers code generation
- Separate targets for app, tests, benchmarks
- Docker integration for reproducible builds

**Project Structure:**
```cmake
CMakeLists.txt (root)
z-monitor/
  CMakeLists.txt (app)
  tests/CMakeLists.txt (tests)
project-dashboard/
  CMakeLists.txt (docs)
```

**Alternatives Considered:**
- ❌ QMake: Qt-specific, less flexible than CMake
- ❌ Meson: Smaller ecosystem, less Qt integration
- ❌ Bazel: Overkill for single-app project

---

### 2.4 Database

**Technology:** SQLite 3 + SQLCipher

**Version:** SQLite 3.30+, SQLCipher 4.x

**Purpose:** Local encrypted data persistence (vitals, alarms, patient records)

**Key Features:**
- **SQLite:** Self-contained, serverless, zero-configuration SQL database
- **SQLCipher:** Transparent AES-256 encryption extension
- **Qt SQL Module:** Qt integration via `QSqlDatabase`

**Schema:**
- `patients` - Patient demographics and admission state
- `vitals` - Time-series vital sign measurements
- `alarms` - Alarm event history
- `telemetry_queue` - Pending telemetry batches (protobuf BLOBs)
- `audit_log` - Security audit trail

**Rationale:**
- Zero-configuration: No database server required
- Encrypted at rest: Meets HIPAA PHI encryption requirements
- Transactional: ACID guarantees for critical data
- Qt integration: Native support via Qt SQL module

**Performance:**
- ~15% overhead for SQLCipher encryption (acceptable)
- Batch writes (every 10 min) avoid I/O saturation

**Alternatives Considered:**
- ❌ PostgreSQL: Overkill for single-device, requires server
- ❌ Application-Level Encryption: Complex, error-prone
- ❌ No Encryption: HIPAA violation

**Related:** [DOC-ARCH-017: Database Design](./DOC-ARCH-017_database_design.md), Legacy `z-monitor/architecture_and_design/34_SQLCIPHER_INTEGRATION.md`

---

### 2.5 Networking

**Technology:** Qt Network Module (QNetworkAccessManager)

**Protocol:** HTTPS with mutual TLS (mTLS)

**TLS Version:** TLS 1.2+ (TLS 1.3 preferred)

**Purpose:** Secure communication with central server and HIS/EHR

**Key Features:**
- **QNetworkAccessManager:** Asynchronous HTTP client
- **QSslConfiguration:** mTLS client certificate configuration
- **Qt Concurrent:** Thread-safe network operations

**Endpoints:**
- `POST /api/v1/telemetry` - Telemetry batch submission (protobuf)
- `GET /api/v1/patients/{mrn}` - Patient lookup (JSON)
- `GET /api/v1/provisioning` - Device provisioning payload (JSON)

**Security:**
- Client certificate authentication (mTLS)
- Certificate pinning for server validation
- HMAC-SHA256 message signing

**Rationale:**
- Qt integration: Native async API, signal/slot callbacks
- mTLS: Meets IEC 62304 security requirements
- Standard HTTPS: Compatible with hospital infrastructure

**Alternatives Considered:**
- ❌ gRPC: Overkill for simple REST API
- ❌ Raw OpenSSL: Reinventing async I/O
- ❌ libcurl: C API, less Qt-friendly

---

### 2.6 Serialization

**Technology:** Protocol Buffers (protobuf-lite)

**Version:** protobuf 3.x (proto3 syntax)

**Purpose:** Compact, efficient telemetry serialization

**Key Messages:**
- `TelemetryBatch` - Top-level telemetry container
- `VitalRecord` - Single vital measurement
- `WaveformSample` - Single waveform sample
- `AlarmEvent` - Alarm event

**Code Generation:**
```cmake
find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS telemetry.proto)
```

**Why protobuf-lite:**
- Smaller binary size (~100-200 KB vs ~1 MB for full protobuf)
- No reflection overhead (not needed for telemetry)
- Compatible with full protobuf on server side

**Related:** [DOC-ARCH-014: Protocol Buffers Integration](./DOC-ARCH-014_protobuf_integration.md)

---

## 3. Development Tools

### 3.1 Version Control

**Technology:** Git + GitHub

**Branching Model:** Trunk-based development with feature branches

**Key Conventions:**
- `main` - Production-ready code
- `feature/*` - Feature development branches
- `hotfix/*` - Emergency production fixes

---

### 3.2 CI/CD

**Technology:** GitHub Actions

**Workflows:**
- **Build:** Compile on Linux, macOS, Windows
- **Test:** Run unit tests, integration tests
- **Lint:** clang-format, clang-tidy
- **Docs:** Validate documentation metadata, regenerate index

**Docker Integration:**
- Reproducible build environments
- Multi-stage builds for production images

---

### 3.3 Testing

**Framework:** Qt Test + Google Test + Google Benchmark

**Test Types:**
- **Unit Tests:** Qt Test for QObject-based classes
- **Integration Tests:** Google Test for cross-component tests
- **Benchmarks:** Google Benchmark for performance regression

**Coverage Target:** 80% line coverage for domain and application layers

---

### 3.4 Documentation

**Technologies:**
- **Doxygen:** API documentation from C++ comments
- **Mermaid:** Architecture diagrams (auto-converted to SVG)
- **Markdown:** Design documents with YAML frontmatter
- **Python Scripts:** Index generation, metadata validation

**Documentation Structure:**
```
project-dashboard/doc/
├── architecture/     # DOC-ARCH-* (system-level)
├── components/       # DOC-COMP-* (component-level)
└── processes/        # DOC-PROC-* (workflows)
```

---

## 4. Security Stack

### 4.1 Encryption

| Layer                 | Technology          | Purpose                               |
| --------------------- | ------------------- | ------------------------------------- |
| **Data at Rest**      | SQLCipher (AES-256) | Encrypt SQLite database               |
| **Data in Transit**   | TLS 1.2+ (mTLS)     | Encrypt HTTPS communication           |
| **Message Integrity** | HMAC-SHA256         | Sign telemetry batches                |
| **Key Storage**       | Platform Keychain   | Secure key storage (Keychain/Keyring) |

### 4.2 Authentication

**Device Authentication:** mTLS client certificates

**User Authentication:** Hospital user management server (REST API or LDAP)

**Session Management:** JWT tokens with 8-hour expiration

### 4.3 Authorization

**Model:** Role-Based Access Control (RBAC)

**Roles:**
- Nurse: Monitor vitals, acknowledge alarms
- Physician: All nurse permissions + modify settings
- Technician: Device diagnostics, network configuration
- Administrator: All permissions + user management

### 4.4 Audit Logging

**Technology:** SQLite audit_log table + file-based audit trail

**Events Logged:**
- User login/logout
- Patient admission/discharge
- Settings changes
- Alarm acknowledgments
- Data exports

**Retention:** 7 years (HIPAA requirement)

---

## 5. External Dependencies

| Library              | Version | Purpose       | License              |
| -------------------- | ------- | ------------- | -------------------- |
| **Qt**               | 6.x     | UI framework  | LGPL v3 / Commercial |
| **SQLite**           | 3.30+   | Database      | Public Domain        |
| **SQLCipher**        | 4.x     | Encryption    | BSD                  |
| **Protocol Buffers** | 3.x     | Serialization | BSD                  |
| **OpenSSL**          | 1.1.1+  | Cryptography  | Apache 2.0           |
| **Google Test**      | 1.10+   | Testing       | BSD                  |
| **Google Benchmark** | 1.5+    | Benchmarking  | Apache 2.0           |

---

## 6. Target Platforms

### 6.1 Primary Target

**OS:** Embedded Linux (Debian-based)

**Architecture:** ARM64 (aarch64) or x86_64

**Screen:** 8-inch 1280x800 touch screen

**Memory:** 2 GB RAM minimum

**Storage:** 8 GB eMMC/SD card

### 6.2 Development Platforms

**Supported:**
- Linux (Ubuntu 20.04+, Debian 11+)
- macOS (11+)
- Windows (10+) via WSL or native

---

## 7. Related Documents

- **[DOC-ARCH-001: Architecture Overview](./DOC-ARCH-001_architecture_overview.md)**
- **[DOC-ARCH-003: Design Decisions](./DOC-ARCH-003_design_decisions.md)**
- **[DOC-ARCH-013: Dependency Injection](./DOC-ARCH-013_dependency_injection.md)**
- **[DOC-ARCH-014: Protocol Buffers](./DOC-ARCH-014_protobuf_integration.md)**
- **Legacy:** `z-monitor/architecture_and_design/01_OVERVIEW.md`

---
**Status:** ✅ Synthesized from legacy architecture documentation

## 2. Architecture

![Architecture Diagram](diagrams/DOC-COMP-XXX_architecture.svg)

**Key Design Decisions:**
- **Decision 1:** {Rationale}
- **Decision 2:** {Rationale}
- **Decision 3:** {Rationale}

**Design Patterns Used:**
- {Pattern 1}: {Why and how it's applied}
- {Pattern 2}: {Why and how it's applied}

## 3. Public API

**Interface Specification:** See DOC-API-XXX for complete interface contract.

### 3.1 Key Classes

```cpp
class ComponentName {
public:
    // Primary methods with brief descriptions
    Result<void, Error> Initialize(const Config& config);
    Result<Data, Error> PerformOperation(const Input& input);
    void Shutdown();
};
```

### 3.2 Key Methods

**`Initialize(config)`**
- **Purpose:** {What it does}
- **Parameters:** {Description}
- **Returns:** {Return type and meaning}
- **Errors:** {Possible error conditions}

**`PerformOperation(input)`**
- **Purpose:** {What it does}
- **Parameters:** {Description}
- **Returns:** {Return type and meaning}
- **Errors:** {Possible error conditions}

## 4. Threading Model

**Thread Ownership:** {Main / Database I/O / Network / Custom Thread}

**Thread Safety:** {Thread-safe / Not thread-safe / Partially thread-safe}

**Synchronization Strategy:**
- {How concurrent access is managed}
- {What locks/mutexes are used}
- {Any lock-free structures}

**Thread Interaction Diagram:**

![Thread Model](diagrams/DOC-COMP-XXX_threading.svg)

## 5. Data Flow

**Typical Interaction Sequence:**

![Sequence Diagram](diagrams/DOC-COMP-XXX_sequence.svg)

**Key Data Paths:**
1. {Path 1}: {Description}
2. {Path 2}: {Description}
3. {Path 3}: {Description}

## 6. Configuration

### 6.1 Required Settings

| Setting    | Type    | Default     | Description   | Constraints        |
| ---------- | ------- | ----------- | ------------- | ------------------ |
| `setting1` | String  | `"default"` | {Description} | {Validation rules} |
| `setting2` | Integer | `100`       | {Description} | {Min/max values}   |

### 6.2 Environment Variables

| Variable   | Required | Description   |
| ---------- | -------- | ------------- |
| `VAR_NAME` | Yes/No   | {Description} |

### 6.3 Configuration Example

```cpp
Config config;
config.setting1 = "custom_value";
config.setting2 = 200;
```

## 7. Error Handling

### 7.1 Error Types

**Error Enum:**
```cpp
enum class ComponentError {
    INITIALIZATION_FAILED,
    INVALID_INPUT,
    OPERATION_TIMEOUT,
    DEPENDENCY_UNAVAILABLE
};
```

### 7.2 Error Scenarios

| Error                   | When It Occurs | How to Handle       |
| ----------------------- | -------------- | ------------------- |
| `INITIALIZATION_FAILED` | {Trigger}      | {Recovery strategy} |
| `INVALID_INPUT`         | {Trigger}      | {Recovery strategy} |

### 7.3 Error Propagation

{How errors are reported to callers - Result<T, Error>, exceptions, callbacks, etc.}

## 8. Performance Characteristics

### 8.1 Complexity Analysis

| Operation          | Time Complexity | Space Complexity |
| ------------------ | --------------- | ---------------- |
| `Initialize`       | O(1)            | O(n)             |
| `PerformOperation` | O(log n)        | O(1)             |

### 8.2 Resource Usage

**Memory:**
- Typical: {X MB}
- Peak: {Y MB}
- Allocations: {Static / Dynamic / Mixed}

**CPU:**
- Typical load: {Percentage or description}
- Peak load: {Percentage or description}

**I/O:**
- Disk: {Read/write patterns}
- Network: {Bandwidth requirements}

### 8.3 Latency Requirements

| Operation          | Target Latency | p99 Latency |
| ------------------ | -------------- | ----------- |
| `PerformOperation` | < 10ms         | < 50ms      |

## 9. Testing Strategy

### 9.1 Unit Tests

**Test Coverage Requirements:** ≥ {Percentage}%

**Key Test Scenarios:**
- {Scenario 1}: {What to verify}
- {Scenario 2}: {What to verify}
- {Scenario 3}: {What to verify}

**Test Files:**
- `tests/unit/component/ComponentNameTest.cpp`

### 9.2 Integration Tests

**Integration Points to Test:**
- {Component A} integration: {What to verify}
- {Component B} integration: {What to verify}

**Test Files:**
- `tests/integration/ComponentNameIntegrationTest.cpp`

### 9.3 Performance Tests

**Benchmarks:**
- {Operation}: Target {X ms}, measured {Y ms}

**Test Files:**
- `tests/benchmarks/ComponentNameBenchmark.cpp`

## 10. Implementation Notes

### 10.1 Key Implementation Details

**Detail 1:** {Description}
- {Sub-detail}
- {Sub-detail}

**Detail 2:** {Description}
- {Sub-detail}

### 10.2 Common Pitfalls

**Pitfall 1:** {Description}
- **How to Avoid:** {Solution}

**Pitfall 2:** {Description}
- **How to Avoid:** {Solution}

### 10.3 Platform-Specific Considerations

**macOS:**
- {Consideration}

**Linux:**
- {Consideration}

**Windows:**
- {Consideration}

## 11. Usage Examples

### 11.1 Basic Usage

```cpp
// Example 1: Basic initialization and usage
ComponentName component;
auto result = component.Initialize(config);
if (!result.isOk()) {
    // Handle error
    return result.error();
}

auto data = component.PerformOperation(input);
if (data.isOk()) {
    // Use data
} else {
    // Handle error
}
```

### 11.2 Error Handling

```cpp
// Example 2: Comprehensive error handling
auto result = component.PerformOperation(input);
if (!result.isOk()) {
    switch (result.error().code) {
        case ComponentError::INVALID_INPUT:
            // Validate and retry
            break;
        case ComponentError::OPERATION_TIMEOUT:
            // Retry with backoff
            break;
        default:
            // Fatal error
            return;
    }
}
```

### 11.3 Advanced Scenario

```cpp
// Example 3: Advanced usage pattern
// {Description of scenario}
```

## 12. Related Documentation

### Architecture
- [DOC-ARCH-XXX: System Architecture](../architecture/DOC-ARCH-XXX_system_overview.md)

### APIs
- [DOC-API-XXX: Interface Specification](../api/DOC-API-XXX_interface.md)

### Processes
- [DOC-PROC-XXX: Related Workflow](../processes/DOC-PROC-XXX_workflow.md)

### Guidelines
- [DOC-GUIDE-XXX: Coding Guidelines](../guidelines/DOC-GUIDE-XXX_guidelines.md)

### Requirements
- [REQ-XXX-XXX: Related Requirement](../requirements/REQ-XXX-XXX_requirement.md)

## 13. Changelog

### v1.0 (2025-12-01)
- Initial version
