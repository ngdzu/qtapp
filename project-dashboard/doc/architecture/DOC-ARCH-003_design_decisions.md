---
doc_id: DOC-ARCH-003
title: Design Decisions
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

# DOC-ARCH-003: Design Decisions

## 1. Overview

This document captures the key architectural and design decisions made throughout the Z Monitor project. Each decision is documented with its context, rationale, alternatives considered, and consequences.

**Purpose:** Provide a historical record of why specific architectural choices were made, helping future contributors understand the reasoning behind the current implementation and avoid revisiting settled questions.

---

## 2. Domain-Driven Design (DDD)

### 2.1 Decision: Adopt Domain-Driven Design

**Context:** Medical device software requires clear separation of business logic from technical infrastructure.

**Decision:** Structure application using DDD principles with four layers:
- Domain Layer (pure business logic)
- Application Layer (use case orchestration)
- Infrastructure Layer (technical implementations)
- Interface Layer (UI integration)

**Rationale:**
- ✅ **Clear Boundaries:** Business rules isolated from Qt/SQL dependencies
- ✅ **Testability:** Domain logic testable without infrastructure
- ✅ **Maintainability:** Changes to persistence don't affect domain
- ✅ **Compliance:** Easier to audit business rules for medical device standards

**Alternatives Considered:**
- ❌ **Monolithic Architecture:** Tightly coupled business logic and infrastructure
- ❌ **Layered MVC:** Insufficient separation for complex medical workflows

**Consequences:**
- Initial learning curve for team members unfamiliar with DDD
- More directory structure complexity
- Clearer interfaces enable parallel development

**Related:** [DOC-ARCH-016: System Components](./DOC-ARCH-016_system_components.md)

---

### 2.2 Decision: Manual Dependency Injection (No Framework)

**Context:** Need dependency injection for testability and loose coupling.

**Decision:** Use manual constructor injection with lightweight AppContainer for bootstrapping.

**Rationale:**
- ✅ **Zero Dependencies:** No external DI framework required
- ✅ **Explicit:** All dependencies visible in constructors
- ✅ **Compile-Time Safety:** Type errors caught at compile time
- ✅ **Simple:** Easy to understand and debug

**Alternatives Considered:**
- ❌ **Boost.DI:** Template-heavy, cryptic error messages
- ❌ **Service Locator:** Hidden dependencies, testing difficulty
- ❌ **Qt Parent/Child:** Not a DI framework (memory management only)

**Consequences:**
- Manual wiring code in AppContainer
- No automatic dependency resolution
- Clear dependency graph enforces good design

**Related:** [DOC-ARCH-013: Dependency Injection](./DOC-ARCH-013_dependency_injection.md)

---

## 3. Data Management

### 3.1 Decision: In-Memory Caching with Periodic Persistence

**Context:** Real-time vitals arrive at high frequency (5 Hz), but disk I/O is slow.

**Decision:** Cache vitals in-memory (3-day capacity) with periodic SQLite persistence (every 10 minutes).

**Rationale:**
- ✅ **Performance:** Sub-50ms alarm detection requires in-memory access
- ✅ **Reliability:** Batch writes reduce disk I/O overhead
- ✅ **Capacity:** 3 days = ~1.3M records = ~39 MB (fits in RAM)
- ✅ **Recovery:** 10-minute persistence window acceptable data loss

**Alternatives Considered:**
- ❌ **Immediate Persistence:** 5 writes/sec would saturate disk I/O
- ❌ **No Persistence:** Unacceptable data loss on power failure
- ❌ **Smaller Cache:** Insufficient for 3-day trend visualization

**Consequences:**
- Up to 10 minutes of data loss on unexpected shutdown
- Memory usage: ~40 MB for vitals cache
- Requires daily cleanup to enforce 7-day retention

**Related:** Legacy `z-monitor/architecture_and_design/36_DATA_CACHING_STRATEGY.md`

---

### 3.2 Decision: SQLCipher for Database Encryption

**Context:** Patient data (PHI) must be encrypted at rest per HIPAA requirements.

**Decision:** Use SQLCipher (AES-256) for SQLite database encryption.

**Rationale:**
- ✅ **Compliance:** Meets HIPAA encryption requirements
- ✅ **Transparent:** Drop-in replacement for SQLite
- ✅ **Performance:** ~15% overhead acceptable for non-critical path
- ✅ **Key Management:** Integrates with platform keychain (Keychain/Keyring)

**Alternatives Considered:**
- ❌ **Application-Level Encryption:** Complex, error-prone
- ❌ **Full Disk Encryption:** Not sufficient for removable media
- ❌ **No Encryption:** Violates HIPAA requirements

**Consequences:**
- 15% database performance overhead
- Key management complexity (rotation, backup)
- Encrypted backups require decryption key

**Related:** Legacy `z-monitor/architecture_and_design/34_SQLCIPHER_INTEGRATION.md`

---

## 4. Concurrency & Performance

### 4.1 Decision: 6-Thread Module Architecture

**Context:** Need to isolate real-time critical path from I/O operations.

**Decision:** Organize components into 6 modules, each with dedicated thread:
1. Interface Module (Main/UI Thread)
2. Real-Time Processing (High-Priority Thread)
3. Application Services (Normal Priority)
4. Database I/O (I/O Priority)
5. Network I/O (Normal Priority)
6. Background Tasks (Low Priority)

**Rationale:**
- ✅ **Isolation:** Real-time path isolated from disk/network I/O
- ✅ **Predictability:** Dedicated RT thread for sub-50ms alarm detection
- ✅ **Resource Efficiency:** Fewer threads than one-thread-per-component
- ✅ **Simplified Lifecycle:** Modules start/stop as units

**Alternatives Considered:**
- ❌ **Single-Threaded:** Disk I/O blocks alarm detection
- ❌ **Thread-Per-Component:** Excessive context switching (100+ threads)
- ❌ **Thread Pool:** Unpredictable scheduling for real-time path

**Consequences:**
- Cross-thread communication requires Qt::QueuedConnection
- Real-time thread must avoid blocking operations
- 6 OS threads (acceptable for embedded hardware)

**Related:** [DOC-ARCH-019: Class Designs Overview](./DOC-ARCH-019_class_designs_overview.md), Legacy `z-monitor/architecture_and_design/12_THREAD_MODEL.md`

---

### 4.2 Decision: Protocol Buffers (protobuf-lite) for Telemetry

**Context:** Need compact, efficient serialization for telemetry transmission.

**Decision:** Use protobuf-lite for telemetry serialization (not full protobuf).

**Rationale:**
- ✅ **Compact:** Smaller binary size (~100-200 KB overhead vs ~1 MB for full protobuf)
- ✅ **Performance:** No reflection overhead
- ✅ **Compatibility:** Server can use full protobuf
- ✅ **Versioning:** Explicit schema versioning for backward compatibility

**Alternatives Considered:**
- ❌ **JSON:** Verbose, slower parsing, larger payloads
- ❌ **Custom Binary:** Reinventing the wheel, no tooling
- ❌ **Full Protobuf:** Unnecessary overhead for embedded device

**Consequences:**
- No reflection API (must handle schema changes manually)
- Requires protoc compiler in build pipeline
- 100-200 KB binary overhead acceptable

**Related:** [DOC-ARCH-014: Protocol Buffers Integration](./DOC-ARCH-014_protobuf_integration.md)

---

## 5. Security

### 5.1 Decision: mTLS for All External Communication

**Context:** Medical device must authenticate both client and server.

**Decision:** Use mutual TLS (mTLS) with client certificates for all HTTPS communication.

**Rationale:**
- ✅ **Compliance:** Meets IEC 62304 security requirements
- ✅ **Authentication:** Server validates device certificate
- ✅ **Encryption:** TLS 1.2+ encrypts all data in transit
- ✅ **Standard:** Industry-standard protocol

**Alternatives Considered:**
- ❌ **Server-Only TLS:** Device not authenticated
- ❌ **API Keys:** Less secure, key rotation complexity
- ❌ **Custom Protocol:** Security through obscurity

**Consequences:**
- Certificate management complexity (provisioning, rotation, revocation)
- Requires PKI infrastructure (CA, CRL)
- Certificate expiration handling required

---

### 5.2 Decision: Role-Based Access Control (RBAC)

**Context:** Different users (nurses, physicians, technicians) require different permissions.

**Decision:** Implement RBAC with four roles: Nurse, Physician, Technician, Administrator.

**Rationale:**
- ✅ **Compliance:** Meets HIPAA minimum necessary access principle
- ✅ **Simplicity:** Fixed roles simpler than per-user permissions
- ✅ **Audit:** Role-based logging for compliance

**Alternatives Considered:**
- ❌ **No Access Control:** Violates HIPAA requirements
- ❌ **Per-User Permissions:** Excessive complexity for 4 roles
- ❌ **Single Role:** Insufficient granularity

**Consequences:**
- Permission checks at UI and service layers
- Role definitions must be maintained
- Hospital user management integration required

**Related:** Legacy `z-monitor/architecture_and_design/29_SYSTEM_COMPONENTS.md` (SecurityService)

---

## 6. User Interface

### 6.1 Decision: QML for Declarative UI

**Context:** Need responsive, touch-friendly UI with real-time waveform rendering.

**Decision:** Use QML for declarative UI with Canvas API for waveforms.

**Rationale:**
- ✅ **Declarative:** Easier to maintain than imperative Qt Widgets
- ✅ **Touch-Friendly:** Native touch support
- ✅ **Performance:** Hardware-accelerated rendering
- ✅ **Separation:** QML views separate from C++ controllers

**Alternatives Considered:**
- ❌ **Qt Widgets:** Less suited for touch interfaces
- ❌ **Web Technologies (Electron):** Excessive overhead for embedded
- ❌ **Custom OpenGL:** Reinventing the wheel

**Consequences:**
- QML learning curve for Qt Widgets developers
- Canvas API requires JavaScript for waveform rendering
- QML/C++ boundary requires careful data binding

---

## 7. Related Documents

- **[DOC-ARCH-001: Architecture Overview](./DOC-ARCH-001_architecture_overview.md)**
- **[DOC-ARCH-004: Technology Stack](./DOC-ARCH-004_technology_stack.md)**
- **[DOC-ARCH-013: Dependency Injection](./DOC-ARCH-013_dependency_injection.md)**
- **[DOC-ARCH-014: Protocol Buffers](./DOC-ARCH-014_protobuf_integration.md)**
- **[DOC-ARCH-016: System Components](./DOC-ARCH-016_system_components.md)**
- **[DOC-ARCH-019: Class Designs](./DOC-ARCH-019_class_designs_overview.md)**

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
