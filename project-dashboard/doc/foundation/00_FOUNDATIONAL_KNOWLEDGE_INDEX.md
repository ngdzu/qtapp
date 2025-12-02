# Foundational Knowledge Index

This document serves as an index to all foundational knowledge topics used in the Z Monitor project's design and architecture. These are general software engineering concepts, patterns, and best practices that underpin our technical decisions.

---

## Purpose

This index helps:
- **New developers** understand the foundational concepts used in the project
- **Experienced developers** quickly reference best practices and patterns
- **Code reviewers** verify implementations follow established patterns
- **Documentation maintainers** identify gaps in foundational knowledge coverage

---

## Coverage Status Legend

- ✅ **Complete** - Comprehensive documentation exists
- 🔶 **Partial** - Basic documentation exists, needs expansion
- ⏳ **Planned** - Should be documented
- ❌ **Missing** - Critical gap, needs documentation

---

## Folder Structure

All foundational knowledge documents are organized in category subfolders:

```
foundation/
├── 00_FOUNDATIONAL_KNOWLEDGE_INDEX.md (this file)
├── README.md (usage guide)
├── 01_software_architecture_and_design_patterns/
│   ├── 01_domain_driven_design.md
│   ├── 02_dependency_injection.md
│   └── ... (9 documents total)
├── 02_database_and_data_management/
│   ├── 01_database_normalization.md
│   ├── 02_sqlite_wal_mode.md
│   └── ... (8 documents total)
├── 03_security_and_cryptography/ (8 documents)
├── 04_concurrency_and_threading/ (6 documents)
├── 05_memory_and_performance/ (6 documents)
├── 06_error_handling_and_resilience/ (5 documents)
├── 07_logging_and_observability/ (4 documents)
├── 08_testing_strategies/ (5 documents)
├── 09_api_design_and_documentation/ (5 documents)
├── 10_qt_specific_knowledge/ (5 documents)
├── 11_medical_device_standards/ (4 documents)
├── 12_devops_and_deployment/ (4 documents)
├── 13_code_quality/ (4 documents)
└── 14_build_systems/ (3 documents)
```

**Total:** 75 topic documents + index + README = 77 markdown files

Each document follows a consistent format with:
- Status indicator (✅/🔶/⏳)
- Reference to Z Monitor-specific implementation (where applicable)
- Comprehensive coverage of the foundational concept

---

## 1. Software Architecture & Design Patterns

### 1.1 Domain-Driven Design (DDD)
- **Status:** ✅ Complete
- **Document:** `01_software_architecture_and_design_patterns/01_domain_driven_design.md`
 - **Z Monitor Reference:** `../architecture/DOC-ARCH-028_domain_driven_design.md`
- **Coverage:**
  - Bounded contexts
  - Aggregates and entities
  - Value objects
  - Repositories
  - Domain events
  - Application services
  - Infrastructure services
  - Layered architecture
  - Ubiquitous language
- **Applies to:** Entire codebase structure, class organization, dependency management

### 1.2 Dependency Injection (DI)
- **Status:** ✅ Complete
- **Document:** `01_software_architecture_and_design_patterns/02_dependency_injection.md`
- **Z Monitor Reference:** `../z-monitor/architecture_and_design/13_DEPENDENCY_INJECTION.md`
- **Coverage:**
  - Constructor injection
  - Interface-based dependencies
  - Service locator pattern (when to use/avoid)
  - Qt parent-child ownership integration
  - Testing with mock objects
- **Applies to:** Service instantiation, testing, modularity

### 1.3 Repository Pattern
- **Status:** ✅ Complete
- **Document:** `01_software_architecture_and_design_patterns/03_repository_pattern.md`
- **Z Monitor Reference:** `../z-monitor/architecture_and_design/30_DATABASE_ACCESS_STRATEGY.md` (Section 3-4)
- **Coverage:**
  - Interface definition in domain layer
  - Implementation in infrastructure layer
  - Separation of domain logic from data access
  - Manual mapping (no ORM)
  - Query optimization strategies
- **Applies to:** All database access, data persistence

### 1.4 Data Transfer Objects (DTOs)
- **Status:** ✅ Complete
- **Document:** `01_software_architecture_and_design_patterns/04_data_transfer_objects.md`
- **Z Monitor Reference:** `../z-monitor/architecture_and_design/31_DATA_TRANSFER_OBJECTS.md`
- **Coverage:**
  - DTO vs Value Object vs Entity
  - Immutability principles
  - Cross-thread communication
  - Serialization strategies
  - Validation at boundaries
- **Applies to:** Layer boundaries, thread boundaries, network boundaries

### 1.5 Model-View-Controller (MVC) / Model-View-ViewModel (MVVM)
- **Status:** 🔶 Partial
- **Document:** `01_software_architecture_and_design_patterns/05_mvc_mvvm_patterns.md`
- **Z Monitor Reference:** Scattered across `../z-monitor/architecture_and_design/02_ARCHITECTURE.md`, `../z-monitor/architecture_and_design/09_CLASS_DESIGNS.md`
- **Coverage:**
  - QML Controllers
  - Service layer
  - View models for QML
- **Gaps:**
  - ⏳ Comprehensive MVC/MVVM pattern guide
  - ⏳ Qt-specific Model/View architecture patterns
  - ⏳ QML property binding best practices
- **Applies to:** UI layer, QML components, data binding

### 1.6 State Machine Pattern
- **Status:** ✅ Complete
- **Document:** `01_software_architecture_and_design_patterns/06_state_machine_pattern.md`
- **Z Monitor Reference:** `../z-monitor/architecture_and_design/05_STATE_MACHINES.md`
- **Coverage:**
  - Finite state machines
  - State transitions
  - Guards and actions
  - Hierarchical states
  - Qt State Machine framework
- **Applies to:** Device provisioning, patient admission workflow, alarm states

### 1.7 Observer Pattern / Signals & Slots
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Qt signals & slots mechanism
  - Event-driven architecture
  - Decoupling components
  - Signal connection patterns
  - Memory management with signals
  - Cross-thread signal safety
- **Applies to:** Component communication, event handling, UI updates

### 1.8 Strategy Pattern
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Algorithm abstraction
  - Runtime behavior selection
  - Interface-based strategies
- **Examples in project:**
  - `IPatientLookupService` (Mock vs Real)
  - `ITelemetryServer` (Mock vs Real)
  - Encryption strategies
- **Applies to:** Pluggable behaviors, testing

### 1.9 Factory Pattern
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Object creation abstraction
  - Factory methods
  - Abstract factories
  - Qt QObject factories
- **Applies to:** Service instantiation, test object creation

---

## 2. Database & Data Management

### 2.1 Database Normalization
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Normal forms (1NF, 2NF, 3NF, BCNF)
  - When to normalize vs denormalize
  - Performance trade-offs
  - Medical device context (audit trails, time-series)
- **Applies to:** Schema design, data integrity

### 2.2 SQLite Write-Ahead Logging (WAL)
- **Status:** 🔶 Partial
- **Document:** Mentioned in `../z-monitor/architecture_and_design/30_DATABASE_ACCESS_STRATEGY.md` (Section 5.1)
- **Coverage:**
  - Basic WAL mode enabling
  - Performance benefits
- **Gaps:**
  - ⏳ Detailed WAL internals
  - ⏳ WAL checkpoint strategies
  - ⏳ Concurrent reader/writer patterns
  - ⏳ WAL file size management
  - ⏳ Backup strategies with WAL
- **Applies to:** Database performance, concurrent access

### 2.3 Database Indexing Strategies
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/10_DATABASE_DESIGN.md`, `33_SCHEMA_MANAGEMENT.md`
- **Coverage:**
  - Index definitions in schema
  - Partial indices (WHERE clauses)
- **Gaps:**
  - ⏳ Index selection strategies
  - ⏳ Covering indices
  - ⏳ Index maintenance costs
  - ⏳ Query plan analysis
  - ⏳ Time-series index optimization
- **Applies to:** Query performance, schema design

### 2.4 Database Transactions & ACID
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - ACID properties
  - Transaction isolation levels
  - SQLite transaction behavior
  - Deadlock prevention
  - Transaction boundaries in application code
  - Long-running transaction strategies
- **Applies to:** Data integrity, concurrent access

### 2.5 Time-Series Data Management
- **Status:** 🔶 Partial
- **Document:** Scattered across `../z-monitor/architecture_and_design/10_DATABASE_DESIGN.md`, `30_DATABASE_ACCESS_STRATEGY.md`
- **Coverage:**
  - Vitals table structure
  - Retention policies
- **Gaps:**
  - ⏳ Time-series optimization strategies
  - ⏳ Downsampling techniques
  - ⏳ Archival strategies
  - ⏳ Query patterns for time-series
  - ⏳ Storage efficiency
- **Applies to:** Vitals data, ECG/Pleth samples, alarm history

### 2.6 Database Schema Versioning
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/34_DATA_MIGRATION_WORKFLOW.md`
- **Coverage:**
  - Migration file organization
  - Version tracking
  - Forward migrations
  - Rollback strategies
- **Applies to:** Schema changes, data migrations

### 2.7 Query Optimization
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/30_DATABASE_ACCESS_STRATEGY.md` (Section 5)
- **Coverage:**
  - Prepared statements
  - Statement caching
  - Batch operations
- **Gaps:**
  - ⏳ EXPLAIN QUERY PLAN usage
  - ⏳ Query rewriting techniques
  - ⏳ Index selection strategies
  - ⏳ JOIN optimization
  - ⏳ Subquery vs JOIN trade-offs
- **Applies to:** Query performance, database design

### 2.8 Database Connection Pooling
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/30_DATABASE_ACCESS_STRATEGY.md` (Section 4.1)
- **Coverage:**
  - Read vs Write connection separation
  - Single writer pattern
- **Gaps:**
  - ⏳ Connection lifecycle management
  - ⏳ Connection pool sizing
  - ⏳ Connection timeout strategies
- **Applies to:** Database performance, thread management

---

## 3. Security & Cryptography

### 3.1 Transport Layer Security (TLS/SSL)
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/06_SECURITY.md` (Section 6.1), `15_CERTIFICATE_PROVISIONING.md`
- **Coverage:**
  - mTLS (Mutual TLS)
  - Certificate management
  - Certificate lifecycle
- **Gaps:**
  - ⏳ TLS handshake process
  - ⏳ Cipher suite selection
  - ⏳ TLS version selection (1.2 vs 1.3)
  - ⏳ Perfect Forward Secrecy
  - ⏳ Certificate pinning strategies
- **Applies to:** Network communication, server authentication

### 3.2 Encryption at Rest
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/06_SECURITY.md` (Section 2)
- **Coverage:**
  - SQLCipher with AES-256
  - Key storage strategies
- **Gaps:**
  - ⏳ Encryption algorithm selection
  - ⏳ Key derivation functions (KDF)
  - ⏳ Salt and IV management
  - ⏳ Performance impact of encryption
  - ⏳ Key rotation procedures
- **Applies to:** Database security, sensitive data protection

### 3.3 Digital Signatures & Message Authentication
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/06_SECURITY.md` (Section 6.3)
- **Coverage:**
  - HMAC-SHA256 for data integrity
  - Signature verification
- **Gaps:**
  - ⏳ Digital signature algorithms (RSA, ECDSA)
  - ⏳ Hash function selection
  - ⏳ Signature verification workflows
  - ⏳ Non-repudiation
- **Applies to:** Telemetry data integrity, audit trails

### 3.4 Authentication & Authorization
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/06_SECURITY.md` (Section 3-4)
- **Coverage:**
  - PIN-based authentication
  - Role-based access control (RBAC)
  - Session management
- **Gaps:**
  - ⏳ Authentication protocols (OAuth2, JWT)
  - ⏳ Password hashing best practices (Argon2, bcrypt)
  - ⏳ Multi-factor authentication
  - ⏳ Token-based authentication
  - ⏳ Session timeout strategies
- **Applies to:** User login, API access, device authentication

### 3.5 Secure Key Management
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/06_SECURITY.md` (Section 2.3, 6.6)
- **Coverage:**
  - HSM (Hardware Security Module)
  - Memory protection
  - Key rotation
- **Gaps:**
  - ⏳ Key generation best practices
  - ⏳ Key storage strategies (filesystem, HSM, TPM)
  - ⏳ Key backup and recovery
  - ⏳ Key escrow
  - ⏳ Key lifecycle management
- **Applies to:** Encryption keys, signing keys, certificates

### 3.6 Security Audit Logging
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/06_SECURITY.md` (Section 6.7), `21_LOGGING_STRATEGY.md`
- **Coverage:**
  - Audit log requirements
  - Log security and immutability
  - Log retention
- **Applies to:** Compliance, incident response

### 3.7 Input Validation & Sanitization
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Input validation strategies
  - SQL injection prevention
  - XSS prevention (if applicable)
  - Data sanitization
  - Boundary validation
- **Applies to:** All user inputs, API inputs, network data

### 3.8 Secure Boot & Firmware Integrity
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/06_SECURITY.md` (Section 9)
- **Coverage:**
  - Basic secure boot concept
  - Firmware integrity verification
- **Gaps:**
  - ⏳ Secure boot chain of trust
  - ⏳ Bootloader security
  - ⏳ Firmware signing processes
  - ⏳ Rollback protection
- **Applies to:** Device startup, firmware updates

---

## 4. Concurrency & Threading

### 4.1 Thread Safety & Synchronization
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/12_THREAD_MODEL.md` (Section 8)
- **Coverage:**
  - Qt thread safety rules
  - QMutex usage
  - Lock-free programming guidelines
- **Gaps:**
  - ⏳ Mutex types (recursive, read-write)
  - ⏳ Lock ordering to prevent deadlock
  - ⏳ Lock-free data structures
  - ⏳ Atomic operations
  - ⏳ Memory barriers
  - ⏳ Thread-local storage
- **Applies to:** All multi-threaded code

### 4.2 Thread Priorities & Scheduling
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/12_THREAD_MODEL.md` (Section 10)
- **Coverage:**
  - Priority levels (6-level hierarchy)
  - Priority inversion prevention
  - Starvation prevention strategies
  - CPU budgeting
- **Applies to:** Real-time processing, alarm system

### 4.3 Qt Event Loop & Signal-Slot Threading
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/12_THREAD_MODEL.md` (Section 8.1)
- **Coverage:**
  - Cross-thread signal connections
  - Qt::QueuedConnection
- **Gaps:**
  - ⏳ Event loop internals
  - ⏳ Event prioritization
  - ⏳ Custom event handling
  - ⏳ Event filter patterns
  - ⏳ Blocking event loops (QEventLoop::exec())
- **Applies to:** All Qt components, UI updates

### 4.4 Producer-Consumer Patterns
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Queue-based communication
  - Bounded vs unbounded queues
  - Backpressure strategies
  - Work stealing
- **Examples in project:**
  - Vitals data generation → database insertion
  - Telemetry data → network transmission
- **Applies to:** Data processing pipelines

### 4.5 Real-Time Processing Constraints
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/12_THREAD_MODEL.md` (Section 4)
- **Coverage:**
  - Latency targets (alarm < 50ms)
  - Memory allocation constraints
  - Deterministic behavior
- **Gaps:**
  - ⏳ Real-time scheduling algorithms
  - ⏳ Worst-case execution time (WCET) analysis
  - ⏳ Jitter management
  - ⏳ Cache-aware programming
- **Applies to:** Real-time thread, alarm system

### 4.6 Deadlock & Race Condition Prevention
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Deadlock detection
  - Lock ordering strategies
  - Race condition patterns
  - Thread sanitizer usage
  - Debugging multi-threaded issues
- **Applies to:** All concurrent code

---

## 5. Memory Management & Performance

### 5.1 Memory Management in C++/Qt
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/23_MEMORY_RESOURCE_MANAGEMENT.md`
- **Coverage:**
  - Smart pointers (unique_ptr, shared_ptr)
  - Qt parent-child ownership
  - RAII pattern
  - Memory leak prevention
- **Applies to:** All C++ code

### 5.2 Memory Pools & Pre-allocation
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/23_MEMORY_RESOURCE_MANAGEMENT.md` (Section 4)
- **Coverage:**
  - Pre-allocation strategies
  - Object pools
- **Gaps:**
  - ⏳ Custom allocators
  - ⏳ Memory pool sizing
  - ⏳ Memory fragmentation prevention
- **Applies to:** Real-time thread, performance-critical paths

### 5.3 Cache Optimization
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - CPU cache hierarchies (L1, L2, L3)
  - Cache line awareness
  - Data layout for cache efficiency
  - False sharing prevention
  - Prefetching strategies
- **Applies to:** Performance-critical loops, data structures

### 5.4 Memory Profiling & Leak Detection
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Valgrind usage
  - AddressSanitizer
  - LeakSanitizer
  - Qt memory profiling tools
  - Heap profiling
- **Applies to:** Debugging, testing, optimization

### 5.5 Performance Profiling
- **Status:** ⏳ Planned
- **Document:** Should create (could expand `27_PERFORMANCE_AND_PROFILING` from lessons)
- **Coverage needed:**
  - CPU profiling (perf, gprof)
  - Memory profiling
  - I/O profiling
  - Profiler-guided optimization
  - Benchmarking strategies
- **Applies to:** Performance optimization, bottleneck identification

### 5.6 Shared Memory IPC with memfd and Unix Domain Sockets
- **Status:** ✅ Complete
- **Document:** `05_memory_and_performance/07_shared_memory_ipc.md`
- **Z Monitor Reference:** `../z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md` (Section "Understanding memfd and Socket Handshake Architecture")
- **Coverage:**
  - `memfd_create` system call and advantages
  - Unix domain sockets with `SCM_RIGHTS` for file descriptor passing
  - Control channel + data channel architecture pattern
  - Performance comparison (shared memory vs. WebSocket vs. socket)
  - Security considerations for shared memory IPC
  - Code examples for handshake and data transfer
  - Ring buffer patterns for high-frequency data
  - Multiple reader support
  - Heartbeat mechanisms for stall detection
- **Applies to:** Sensor integration, high-performance IPC, real-time data transfer

---

## 6. Error Handling & Resilience

### 6.1 Error Handling Strategies
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/20_ERROR_HANDLING_STRATEGY.md`
- **Coverage:**
  - Error categories
  - Error propagation
  - Recovery patterns
  - Result<T, E> pattern
- **Applies to:** All error-prone operations

### 6.2 Exception Safety
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/20_ERROR_HANDLING_STRATEGY.md` (Section 2)
- **Coverage:**
  - No-throw guarantee
  - Exception avoidance in RT code
- **Gaps:**
  - ⏳ Exception safety levels (basic, strong, no-throw)
  - ⏳ RAII for exception safety
  - ⏳ noexcept specifications
- **Applies to:** All C++ code

### 6.3 Error Recovery Patterns (Retry, Exponential Backoff, Circuit Breaker)
- **Status:** ✅ Complete
- **Documents:** 
  - `06_error_handling_and_resilience/03_circuit_breaker.md` - Circuit breaker pattern
  - `06_error_handling_and_resilience/04_retry_backoff.md` - Retry with exponential backoff
- **Z Monitor Reference:** `../z-monitor/architecture_and_design/44_ERROR_RECOVERY_PATTERNS.md`
- **Coverage:**
  - Retry with exponential backoff
  - Circuit breaker pattern
  - Combined strategy (retry + circuit breaker) - see design document
  - When to retry vs. when not to retry
  - Circuit breaker state transitions (closed, open, half-open)
  - Exponential backoff calculation
  - Configuration and tuning guidelines
  - Best practices and common pitfalls
- **Applies to:** Network communication, external service calls, database operations

### 6.5 Graceful Degradation
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Fallback strategies
  - Reduced functionality modes
  - Service degradation levels
  - User communication during degradation
- **Applies to:** System resilience, availability

---

## 7. Logging, Monitoring & Observability

### 7.1 Logging Strategies
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/21_LOGGING_STRATEGY.md`
- **Coverage:**
  - Log levels
  - Structured logging
  - Log rotation
  - Performance considerations
- **Applies to:** Debugging, monitoring, compliance

### 7.2 Metrics & Telemetry
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/10_DATABASE_DESIGN.md` (telemetry_metrics table)
- **Coverage:**
  - Telemetry timing metrics
  - Latency tracking
- **Gaps:**
  - ⏳ Metrics aggregation strategies
  - ⏳ Time-series metrics databases
  - ⏳ Metric visualization
  - ⏳ Performance counters
  - ⏳ Application health metrics
- **Applies to:** Performance monitoring, diagnostics

### 7.3 Distributed Tracing
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Trace context propagation
  - Span creation
  - Correlation IDs
  - OpenTelemetry patterns
- **Applies to:** Multi-component debugging, performance analysis

### 7.4 Health Checks & Liveness Probes
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Health check endpoints
  - Dependency health
  - Self-healing mechanisms
  - Watchdog timers
- **Applies to:** System monitoring, deployment

---

## 8. Testing Strategies

### 8.1 Test-Driven Development (TDD)
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/18_TESTING_WORKFLOW.md`
- **Coverage:**
  - Testing pyramid
  - Unit/integration/E2E tests
- **Gaps:**
  - ⏳ TDD workflow (Red-Green-Refactor)
  - ⏳ Test-first development
  - ⏳ Refactoring with test coverage
- **Applies to:** Development workflow

### 8.2 Test Doubles (Mocks, Stubs, Fakes)
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/18_TESTING_WORKFLOW.md`, `13_DEPENDENCY_INJECTION.md`
- **Coverage:**
  - Mock objects
  - Interface-based testing
- **Gaps:**
  - ⏳ Mock vs Stub vs Fake vs Spy
  - ⏳ When to use each type
  - ⏳ Over-mocking pitfalls
- **Applies to:** Unit testing, integration testing

### 8.3 Property-Based Testing
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Property-based testing concepts
  - QuickCheck-style testing
  - Property generation strategies
- **Applies to:** Algorithm testing, edge case discovery

### 8.4 Performance Testing & Benchmarking
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/18_TESTING_WORKFLOW.md` (Section 4)
- **Coverage:**
  - Google Benchmark
  - Benchmark organization
- **Gaps:**
  - ⏳ Benchmark analysis
  - ⏳ Regression detection
  - ⏳ Statistical significance
  - ⏳ Microbenchmarking pitfalls
- **Applies to:** Performance optimization

### 8.5 Integration Testing Strategies
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/18_TESTING_WORKFLOW.md` (Section 3)
- **Coverage:**
  - Integration test organization
  - Test databases
- **Gaps:**
  - ⏳ Test data management
  - ⏳ Test isolation
  - ⏳ Integration test patterns
- **Applies to:** Component integration

---

## 9. API Design & Documentation

### 9.1 API Design Principles (RESTful, RPC)
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - REST principles
  - gRPC vs REST trade-offs
  - API resource design
  - HTTP verb usage
  - Status code conventions
- **Applies to:** Central server API, telemetry API

### 9.2 API Versioning Strategies
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/25_API_VERSIONING.md`
- **Coverage:**
  - Versioning schemes
  - Backward compatibility
  - Deprecation process
- **Applies to:** API evolution

### 9.3 API Documentation Standards
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/26_API_DOCUMENTATION.md`
- **Coverage:**
  - Doxygen style
  - Documentation generation
  - API reference structure
- **Applies to:** All public APIs

### 9.4 OpenAPI / Swagger
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/14_PROTOCOL_BUFFERS.md` mentions OpenAPI
- **Coverage:**
  - Basic mention of OpenAPI
- **Gaps:**
  - ⏳ OpenAPI specification format
  - ⏳ Code generation from OpenAPI
  - ⏳ API documentation from OpenAPI
- **Applies to:** HTTP APIs, telemetry server

### 9.5 Protocol Buffers (Protobuf)
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/14_PROTOCOL_BUFFERS.md`
- **Coverage:**
  - Protobuf definition
  - Code generation
- **Gaps:**
  - ⏳ Protobuf vs JSON trade-offs
  - ⏳ Schema evolution
  - ⏳ Binary encoding details
- **Applies to:** Network communication, data serialization

---

## 10. Qt-Specific Knowledge

### 10.1 Qt Object Model & Meta-Object System
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - MOC (Meta-Object Compiler)
  - Q_OBJECT macro
  - Q_PROPERTY
  - QMetaObject reflection
  - Dynamic property system
- **Applies to:** All Qt classes, QML integration

### 10.2 Qt Signals & Slots Deep Dive
- **Status:** ⏳ Planned
- **Document:** Should create (related to 1.7)
- **Coverage needed:**
  - Signal/slot connection types
  - Queued vs direct connections
  - Lambda connections
  - Connection lifetime management
  - Performance considerations
- **Applies to:** All Qt event handling

### 10.3 QML/QtQuick Best Practices
- **Status:** 🔶 Partial
- **Document:** Lesson `12-qtquick-qml-intro`, `13-qtquick-controls`
- **Coverage:**
  - Basic QML syntax
  - Controls usage
- **Gaps:**
  - ⏳ QML performance optimization
  - ⏳ Property binding performance
  - ⏳ Loader vs dynamic object creation
  - ⏳ QML/C++ integration patterns
- **Applies to:** UI layer

### 10.4 Qt Graphics & Rendering
- **Status:** 🔶 Partial
- **Document:** Lesson `14-graphicsview`, `22-opengl-and-3d`
- **Coverage:**
  - Graphics View framework
  - OpenGL integration
- **Gaps:**
  - ⏳ QPainter optimization
  - ⏳ Scene graph rendering
  - ⏳ Custom QML items
  - ⏳ Rendering threads
- **Applies to:** ECG waveforms, real-time plotting

### 10.5 Qt Model/View Architecture
- **Status:** 🔶 Partial
- **Document:** Lesson `08-model-view-architecture`, `09-custom-models`
- **Coverage:**
  - Model/View separation
  - Custom models
- **Gaps:**
  - ⏳ Model/View optimization
  - ⏳ Lazy loading strategies
  - ⏳ Model change notifications
- **Applies to:** Data tables, list views

---

## 11. Medical Device Specific

### 11.1 IEC 62304 (Medical Device Software Lifecycle)
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Safety classification
  - Software development process
  - Risk management
  - Verification and validation
  - Documentation requirements
- **Applies to:** Entire development lifecycle

### 11.2 HL7 FHIR (Healthcare Data Standards)
- **Status:** ⏳ Planned
- **Document:** Should create (if applicable)
- **Coverage needed:**
  - FHIR resources
  - Data interchange formats
  - Patient data standards
- **Applies to:** Patient lookup service, data exchange

### 11.3 Alarm Management (IEC 60601-1-8)
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/04_ALARM_SYSTEM.md`
- **Coverage:**
  - Alarm priorities
  - Alarm states
- **Gaps:**
  - ⏳ IEC 60601-1-8 requirements
  - ⏳ Alarm fatigue prevention
  - ⏳ Alarm escalation
  - ⏳ Audio/visual alarm specifications
- **Applies to:** Alarm system

### 11.4 Cybersecurity (IEC 62443, FDA Guidance)
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/06_SECURITY.md`
- **Coverage:**
  - mTLS, encryption, authentication
  - Security audit logging
- **Gaps:**
  - ⏳ IEC 62443 compliance
  - ⏳ FDA premarket cybersecurity guidance
  - ⏳ Threat modeling (STRIDE)
  - ⏳ Security risk assessment
- **Applies to:** Security architecture

---

## 12. DevOps & Deployment

### 12.1 Continuous Integration / Continuous Deployment (CI/CD)
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/18_TESTING_WORKFLOW.md` (Section 7), `../z-monitor/architecture_and_design/34_DATA_MIGRATION_WORKFLOW.md` (Section 12)
- **Coverage:**
  - GitHub Actions workflows
  - Automated testing
  - Migration CI checks
- **Gaps:**
  - ⏳ Deployment strategies (blue-green, canary)
  - ⏳ Rollback procedures
  - ⏳ Deployment verification
- **Applies to:** Build and deployment

### 12.2 Containerization (Docker)
- **Status:** ✅ Complete (for lessons)
- **Document:** Individual lesson Dockerfiles, `DOCKER_SETUP.md`
- **Coverage:**
  - Multi-stage builds
  - Base images
  - Container optimization
- **Gaps:**
  - ⏳ Production containerization for Z Monitor
  - ⏳ Container orchestration (Kubernetes)
  - ⏳ Container security
- **Applies to:** Deployment, testing environments

### 12.3 Configuration Management
- **Status:** ✅ Complete
- **Document:** `../z-monitor/architecture_and_design/24_CONFIGURATION_MANAGEMENT.md`
- **Coverage:**
  - Configuration categories
  - Type-safe accessors
  - Migration and validation
- **Applies to:** Application configuration

### 12.4 Semantic Versioning
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/25_API_VERSIONING.md` mentions SemVer
- **Coverage:**
  - Version number scheme
- **Gaps:**
  - ⏳ Version bump criteria
  - ⏳ Breaking change management
  - ⏳ Changelog maintenance
- **Applies to:** Software releases, API versioning

---

## 13. Code Quality & Maintainability

### 13.1 Static Code Analysis
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/18_TESTING_WORKFLOW.md` (Section 5)
- **Coverage:**
  - clang-tidy
  - clang-format
  - cppcheck
- **Gaps:**
  - ⏳ Analysis rule configuration
  - ⏳ Custom checks
  - ⏳ Technical debt tracking
- **Applies to:** Code quality, bug prevention

### 13.2 Code Review Best Practices
- **Status:** 🔶 Partial
- **Document:** `../z-monitor/architecture_and_design/22_CODE_ORGANIZATION.md` (Section 8)
- **Coverage:**
  - Basic review guidelines
- **Gaps:**
  - ⏳ Review checklists
  - ⏳ Review size limits
  - ⏳ Constructive feedback
- **Applies to:** Development workflow

### 13.3 Refactoring Techniques
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Extract method/class
  - Rename safely
  - Move code between layers
  - Refactoring with tests
- **Applies to:** Code maintenance

### 13.4 Technical Debt Management
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Debt identification
  - Debt tracking (TODO, FIXME)
  - Debt prioritization
  - Paydown strategies
- **Applies to:** Long-term maintainability

---

## 14. Build Systems & Tooling

### 14.1 CMake Best Practices
- **Status:** 🔶 Partial
- **Document:** Individual lesson CMakeLists.txt
- **Coverage:**
  - Basic CMake patterns
  - Target-based builds
  - Qt integration
- **Gaps:**
  - ⏳ Modern CMake (3.x+)
  - ⏳ Generator expressions
  - ⏳ Export/install configurations
  - ⏳ Cross-compilation
- **Applies to:** Build system

### 14.2 Package Management (Conan, vcpkg)
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Dependency management
  - Package versioning
  - Reproducible builds
- **Applies to:** Dependency management

### 14.3 Cross-Compilation
- **Status:** ⏳ Planned
- **Document:** Should create
- **Coverage needed:**
  - Target platform configuration
  - Toolchain files
  - Sysroot management
  - Qt cross-compilation
- **Applies to:** Embedded device deployment

---

## Document Organization

All 74 foundational topics are organized into 14 categories under `foundation/`:

| Category                                    | Documents   | Files Created                                               |
| ------------------------------------------- | ----------- | ----------------------------------------------------------- |
| 01. Software Architecture & Design Patterns | 9 docs      | ✅ All placeholders created                                  |
| 02. Database & Data Management              | 8 docs      | ✅ All placeholders created                                  |
| 03. Security & Cryptography                 | 8 docs      | ✅ All placeholders created                                  |
| 04. Concurrency & Threading                 | 6 docs      | ✅ All placeholders created                                  |
| 05. Memory Management & Performance         | 5 docs      | ✅ All placeholders created                                  |
| 06. Error Handling & Resilience             | 5 docs      | ✅ All placeholders created                                  |
| 07. Logging, Monitoring & Observability     | 4 docs      | ✅ All placeholders created                                  |
| 08. Testing Strategies                      | 5 docs      | ✅ All placeholders created                                  |
| 09. API Design & Documentation              | 5 docs      | ✅ All placeholders created                                  |
| 10. Qt-Specific Knowledge                   | 5 docs      | ✅ All placeholders created (1 complete: Qt Signals & Slots) |
| 11. Medical Device Specific                 | 4 docs      | ✅ All placeholders created                                  |
| 12. DevOps & Deployment                     | 4 docs      | ✅ All placeholders created                                  |
| 13. Code Quality & Maintainability          | 4 docs      | ✅ All placeholders created                                  |
| 14. Build Systems & Tooling                 | 3 docs      | ✅ All placeholders created                                  |
| **Total**                                   | **74 docs** | **74 placeholders + 1 complete = 75 ready to populate**     |

**Note:** All documents are placeholder files with proper structure and cross-references. Content population is the next phase.

---

## How to Use This Index

### **For New Developers:**
1. Start with ✅ Complete documents in areas you'll be working
2. Review 🔶 Partial documents for context
3. Ask about ⏳ Planned topics that are relevant to your work

### **For Documentation Maintainers:**
1. Use this index to identify documentation gaps
2. Prioritize ⏳ Planned topics based on project needs
3. Expand 🔶 Partial documents with missing sections

### **For Code Reviewers:**
1. Reference relevant foundational documents during reviews
2. Suggest additions to ⏳ Planned topics when patterns emerge
3. Update this index when new patterns are documented

---

## Related Documents

- `README.md` - Foundation folder usage guide and structure
- `../z-monitor/architecture_and_design/27_PROJECT_STRUCTURE.md` - Repository layout and file organization
- `../z-monitor/architecture_and_design/22_CODE_ORGANIZATION.md` - Code structure within source files
- `../z-monitor/architecture_and_design/01_OVERVIEW.md` - High-level project overview
- `../z-monitor/architecture_and_design/02_ARCHITECTURE.md` - Z Monitor architecture (references foundation docs)

---

*This index is a living document. Update it as new foundational knowledge is documented or gaps are identified.*

