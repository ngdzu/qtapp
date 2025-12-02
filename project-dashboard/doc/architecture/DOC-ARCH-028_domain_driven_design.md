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

## 5. Testing Strategy
- Unit tests for dispatcher (sync + async behavior).
- Aggregate tests assert correct event emission (future: spy dispatcher/event collector).
- Performance tests ensure async queue latency < 20ms under normal load.

## 6. Future Enhancements
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
