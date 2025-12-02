doc_id: DOC-ARCH-028
category: Architecture
owner: Architecture Team
status: Active
version: 1.0
last_updated: 2025-12-01
related:
  - DOC-ARCH-001_software_architecture.md
  - DOC-ARCH-015_project_structure.md
  - DOC-ARCH-019_class_designs_overview.md
  - DOC-ARCH-005_data_flow_and_caching.md
  - DOC-ARCH-017_database_design.md
tags: [architecture, ddd, domain-driven-design, aggregates, events]

# Domain-Driven Design Strategy (Z Monitor)

## 1. Aggregates
### PatientAggregate
- Enforces admission lifecycle: admit → transfer → discharge.
- Protects invariants (cannot admit twice, cannot discharge if not admitted).
- Emits domain events: `PatientAdmitted`, `PatientTransferred`, `PatientDischarged`.
- Uses immutable value objects (`PatientIdentity`, `BedLocation`, `VitalRecord`).

### AlarmAggregate
- Enforces alarm lifecycle: raise → acknowledge/silence/escalate → resolve.
- Protects invariants:
  - Duplicate suppression window prevents rapid re-raise of same alarm type for same patient (10s default).
  - Only active alarms can be acknowledged, silenced, or escalated.
  - Resolved alarms are moved to history and removed from active set.
- Emits domain events: `AlarmRaised`, `AlarmAcknowledged`, `AlarmSilenced`, `AlarmCleared`.
- Uses immutable value objects (`AlarmSnapshot`, `AlarmThreshold`).
- Business rules:
  - Escalation raises priority (LOW → MEDIUM → HIGH).
  - Role-based silence duration limits enforced at application layer (>60s requires supervisor).
  - History preserved for audit trail and regulatory compliance (HIPAA).

## 2. Value Objects
- Immutable (const members, deleted assignment).
- Provide validation and semantic clarity (MRN, bed location, vital sign samples).

## 3. Domain Events
### Principles
- Immutable, timestamped, include aggregate identifier.
- Derived from `IDomainEvent` interface providing: `aggregateId()`, `occurredAtMs()`, `eventType()`, `clone()`.
- Events are simple data carriers; no business logic.

### Dispatcher Infrastructure
`DomainEventDispatcher` supports:
- Synchronous handlers (inline execution, same transaction boundary)
- Asynchronous handlers (queued, worker thread for eventual consistency)
- Thread-safe registration and dispatch.

### Usage Pattern
1. Aggregate creates event after state change.
2. Aggregate or application service calls `dispatcher.dispatch(event)`.
3. Sync handlers perform immediate side-effects (audit logging, in-memory projections).
4. Async handlers perform deferred work (notifications, telemetry enrichment).

### Extensibility Guidelines
- New events MUST implement `clone()` for safe async dispatch.
- Handlers SHOULD be idempotent (avoid duplicate side-effects on retries).
- Prefer small focused handlers over monolithic processing.

## 4. Event Handling Recommendations
- Keep domain pure: aggregates do not depend on infrastructure details.
- Application services wire dispatcher and register handlers at startup.
- Avoid long-running work in sync handlers (> 5ms); move to async.

## 5. Application Services

Application services orchestrate business workflows by coordinating between domain aggregates, repositories, and infrastructure services. They belong to the application layer and are responsible for use case implementation.

### MonitoringService

**Purpose:** Coordinates real-time vital sign monitoring, alarm detection, and telemetry transmission.

**Responsibilities:**
- Receives vital signs from sensor data source (ISensorDataSource)
- Evaluates alarm conditions using configurable thresholds
- Raises alarms via AlarmAggregate when thresholds violated
- Persists vitals and alarms via repositories
- Batches telemetry data for transmission
- Emits Qt signals for UI updates

**Key Features:**
- **Configurable Alarm Thresholds:** Uses AlarmThreshold value objects for threshold management
- **Performance Measurement:** Tracks alarm detection latency to verify < 50ms requirement (REQ-PERF-LATENCY-001)
- **Error Handling:** Gracefully handles sensor errors, repository failures, and invalid data
- **Dependency Injection:** All dependencies injected via constructor (repositories, caches, sensor source)

**Alarm Detection Workflow:**
1. Receive vital sign from sensor
2. Start latency timer
3. Look up threshold configuration for vital type
4. Check if vital violates threshold (low or high)
5. If violated, raise alarm via AlarmAggregate
6. Persist alarm to IAlarmRepository
7. Emit alarmRaised signal for UI
8. Record latency measurement

**Performance Characteristics:**
- Alarm detection latency: < 50ms (measured and verified)
- Normal load: 60 Hz vital signs
- High load: 250 Hz waveforms + 60 Hz vitals
- Burst traffic: Multiple simultaneous alarms handled within latency requirement

**Testing:**
- Unit tests: Threshold configuration, alarm detection logic, error handling
- Integration tests: End-to-end workflow (sensor → alarm → repository)
- Performance benchmarks: Latency verification under various load conditions

**Dependencies:**
- Domain: PatientAggregate, AlarmAggregate, TelemetryBatch, value objects
- Repositories: IPatientRepository, IAlarmRepository, IVitalsRepository, ITelemetryRepository
- Infrastructure: ISensorDataSource, VitalsCache, WaveformCache

## 6. Testing Strategy
- Unit tests for dispatcher (sync + async behavior).
- Aggregate tests assert correct event emission (future: spy dispatcher/event collector).
- Performance tests ensure async queue latency < 20ms under normal load.
- Application service tests verify use case orchestration, error handling, and performance requirements.

## 7. Future Enhancements
- Correlation IDs for cross-service tracing.
- Persistent event store (append-only) for replay / audit.
- Retry + backoff for failed async handlers.
- Metrics instrumentation (count/events by type, handler timing).

## 7. Mapping to Requirements
- Audit trail (HIPAA): structured event inputs to logging pipeline.
- Extensibility: decoupled handlers enable feature additions without touching aggregates.
- Reliability: async channel isolates slow consumers from domain transaction path.

---
Generated baseline architecture for TASK-DOM-006 & TASK-DOM-007.
