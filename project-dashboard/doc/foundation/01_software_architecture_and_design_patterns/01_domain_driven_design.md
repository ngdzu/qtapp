# Domain-Driven Design (DDD)

> **📚 Foundational Knowledge**  
> This is a general software engineering concept used in Z Monitor's design.  
> See `../00_FOUNDATIONAL_KNOWLEDGE_INDEX.md` for all foundational topics.

---

## Status: ✅ Complete

**Primary Reference:** `../../z-monitor/architecture_and_design/28_DOMAIN_DRIVEN_DESIGN.md`

This document provides foundational knowledge about Domain-Driven Design (DDD) principles and how they apply to the Z Monitor project.

---

## What is Domain-Driven Design?

Domain-Driven Design (DDD) is a software development approach introduced by Eric Evans in his 2003 book "Domain-Driven Design: Tackling Complexity in the Heart of Software." DDD focuses on:

1. **Modeling software based on the business domain** - The code structure reflects real-world business concepts
2. **Collaboration between technical and domain experts** - Developers and business stakeholders use a shared language
3. **Complexity management** - Breaking large systems into manageable, well-defined contexts
4. **Strategic design** - Organizing code around business boundaries, not technical boundaries

### Why DDD Matters

DDD helps teams build software that:
- **Stays aligned with business needs** - Code structure mirrors business concepts
- **Remains maintainable** - Clear boundaries and responsibilities
- **Scales effectively** - Well-defined contexts prevent coupling
- **Communicates clearly** - Ubiquitous language makes code self-documenting

---

## Core DDD Concepts

### 1. Ubiquitous Language

**Definition:** A shared vocabulary used by both developers and domain experts to describe the business domain. This language appears in code, documentation, and conversations.

**Key Principles:**
- Use business terms, not technical terms
- Avoid translation layers between business and code
- Keep the language consistent across the codebase
- Evolve the language as understanding deepens

**Example in Z Monitor:**
- ✅ **Good:** `PatientAggregate`, `admitPatient()`, `TelemetryBatch`, `AlarmPriority`
- ❌ **Bad:** `PatientData`, `insertPatient()`, `DataPacket`, `AlarmLevel`

The Z Monitor codebase uses medical device terminology directly:
- **Patient** (not "User" or "Subject")
- **Admission/Discharge/Transfer (ADT)** (not "Activate/Deactivate")
- **Telemetry** (not "Data Transmission")
- **Provisioning** (not "Setup" or "Configuration")

---

### 2. Bounded Contexts

**Definition:** A boundary within which a particular domain model is valid and consistent. Different bounded contexts can have different models of the same concept.

**Key Principles:**
- Each context has its own domain model
- Contexts communicate through well-defined interfaces
- Avoid sharing domain models across contexts
- Map between contexts when necessary

**Z Monitor Bounded Contexts:**

| Context | Purpose | Key Concepts |
|---------|---------|--------------|
| **Monitoring** | Real-time vitals, alarms, telemetry | `PatientAggregate`, `DeviceAggregate`, `TelemetryBatch`, `AlarmAggregate` |
| **Admission/ADT** | Patient admission, discharge, transfer | `AdmissionAggregate`, `PatientIdentity`, `BedAssignment` |
| **Provisioning** | Device pairing, certificate management | `ProvisioningSession`, `CredentialBundle`, `DeviceAggregate` |
| **Security** | Authentication, authorization, audit | `UserSession`, `PinCredential`, `AuditTrailEntry` |

**Why Separate Contexts:**
- **Monitoring** needs real-time performance and alarm handling
- **ADT** focuses on workflow and state transitions
- **Provisioning** handles device lifecycle and security
- **Security** manages authentication and compliance

Each context can evolve independently without affecting others.

---

### 3. Entities and Aggregates

#### Entities

**Definition:** Objects that have a unique identity that persists over time, even if their attributes change.

**Characteristics:**
- Have a unique identifier (ID)
- Can change state over time
- Identity matters more than attributes
- Examples: `Patient`, `Device`, `User`

**Example:**
```cpp
class PatientAggregate {
private:
    QUuid patientId;  // Identity persists
    PatientIdentity identity;
    AdmissionState state;
    // ... can change over time
};
```

#### Aggregates

**Definition:** A cluster of related entities and value objects treated as a single unit. The aggregate root is the only entry point for accessing the aggregate.

**Key Principles:**
- Aggregate root controls access to internal objects
- Invariants are enforced within the aggregate boundary
- External references only point to the aggregate root
- Aggregates are consistency boundaries

**Z Monitor Aggregates:**

**PatientAggregate:**
- **Root:** `PatientAggregate`
- **Contains:** `PatientIdentity`, `AdmissionState`, `BedAssignment`, `CurrentVitals`
- **Invariants:** Only one active admission per patient; device label matches bed assignment
- **Methods:** `admit()`, `discharge()`, `transfer()`, `updateVitals()`

**DeviceAggregate:**
- **Root:** `DeviceAggregate`
- **Contains:** `DeviceId`, `DeviceLabel`, `ProvisioningState`, `Certificates`
- **Invariants:** Device must be provisioned before use; certificates must be valid
- **Methods:** `applyProvisioningPayload()`, `markProvisioned()`, `rotateCredentials()`

**TelemetryBatch:**
- **Root:** `TelemetryBatch`
- **Contains:** `PatientIdentity`, `DeviceSnapshot`, `std::vector<VitalRecord>`, `std::vector<AlarmSnapshot>`
- **Invariants:** All vitals must belong to same patient; batch must be signed before transmission
- **Methods:** `sign(QByteArray privateKey)`, `validate()`

**AlarmAggregate:**
- **Root:** `AlarmAggregate`
- **Contains:** `AlarmId`, `Priority`, `State`, `History`
- **Invariants:** Alarm state transitions must be valid; priority cannot be downgraded
- **Methods:** `raise()`, `acknowledge()`, `silence()`

---

### 4. Value Objects

**Definition:** Objects defined entirely by their attributes, with no identity. Two value objects with the same attributes are considered equal.

**Characteristics:**
- Immutable (cannot change after creation)
- No unique identifier
- Equality based on all attributes
- Can be replaced, not modified
- Examples: `PatientIdentity`, `MeasurementUnit`, `AlarmThreshold`

**Z Monitor Value Objects:**

```cpp
// PatientIdentity - immutable value object
struct PatientIdentity {
    QString mrn;           // Medical Record Number
    QString name;
    QDate dateOfBirth;
    QString sex;
    
    // Equality based on all fields
    bool operator==(const PatientIdentity& other) const {
        return mrn == other.mrn && 
               name == other.name && 
               dateOfBirth == other.dateOfBirth && 
               sex == other.sex;
    }
};

// DeviceSnapshot - immutable snapshot
struct DeviceSnapshot {
    QString deviceId;
    QString deviceLabel;
    QString firmwareVersion;
    ProvisioningStatus status;
    
    bool operator==(const DeviceSnapshot& other) const {
        return deviceId == other.deviceId && 
               deviceLabel == other.deviceLabel && 
               firmwareVersion == other.firmwareVersion && 
               status == other.status;
    }
};
```

**When to Use Value Objects:**
- ✅ Measurements, units, thresholds
- ✅ Addresses, locations
- ✅ Money, dates (if not entities)
- ✅ Configuration values
- ❌ Don't use for objects that need identity or change over time

---

### 5. Domain Events

**Definition:** Events that represent something significant that happened in the domain. They are immutable facts about the past.

**Key Principles:**
- Represent business events, not technical events
- Immutable (cannot be changed after creation)
- Named in past tense (something that happened)
- Can trigger side effects in other aggregates/contexts

**Z Monitor Domain Events:**

```cpp
// Patient admission event
struct PatientAdmitted {
    QUuid eventId;
    QDateTime occurredAt;
    PatientIdentity patient;
    BedAssignment bed;
    QString admittedBy;
};

// Telemetry transmission event
struct TelemetryQueued {
    QUuid eventId;
    QDateTime occurredAt;
    QUuid batchId;
    PatientIdentity patient;
    int vitalCount;
    int alarmCount;
};

// Alarm raised event
struct AlarmRaised {
    QUuid eventId;
    QDateTime occurredAt;
    QUuid alarmId;
    AlarmPriority priority;
    QString message;
    PatientIdentity patient;
};
```

**Event Usage:**
- **Audit Logging:** Record all domain events for compliance
- **UI Updates:** Notify controllers when events occur
- **Integration:** Trigger actions in other bounded contexts
- **Analytics:** Track business metrics

---

### 6. Repositories

**Definition:** An abstraction that provides access to aggregates. Repositories encapsulate data access logic and provide a domain-centric interface.

**Key Principles:**
- Interface defined in domain layer
- Implementation in infrastructure layer
- Methods use domain language, not SQL
- Hide persistence details from domain

**Z Monitor Repository Pattern:**

```cpp
// Domain layer - interface
class IPatientRepository {
public:
    virtual ~IPatientRepository() = default;
    
    // Domain language, not SQL
    virtual std::optional<PatientAggregate> findByMrn(const QString& mrn) = 0;
    virtual void save(const PatientAggregate& patient) = 0;
    virtual std::vector<PatientAggregate> findActiveAdmissions() = 0;
};

// Infrastructure layer - implementation
class SQLitePatientRepository : public IPatientRepository {
private:
    QSqlDatabase& database;
    
public:
    std::optional<PatientAggregate> findByMrn(const QString& mrn) override {
        // SQL implementation hidden here
        QSqlQuery query(database);
        query.prepare("SELECT * FROM patients WHERE mrn = ?");
        query.addBindValue(mrn);
        // ... map to PatientAggregate
    }
    
    void save(const PatientAggregate& patient) override {
        // SQL implementation hidden here
        // ... map from PatientAggregate to SQL
    }
};
```

**Benefits:**
- Domain code doesn't depend on SQL/database
- Easy to swap implementations (SQLite, PostgreSQL, in-memory for tests)
- Testable with mock repositories
- Clear separation of concerns

**Z Monitor Repositories:**
- `IPatientRepository` - Patient data access
- `ITelemetryRepository` - Telemetry batch storage
- `IAlarmRepository` - Alarm history
- `IProvisioningRepository` - Device provisioning data

---

### 7. Application Services

**Definition:** Services that orchestrate domain objects to fulfill use cases. They coordinate between domain layer and infrastructure.

**Key Principles:**
- Stateless (no domain state)
- Orchestrate domain objects and repositories
- Handle transactions and coordination
- Map between layers (DTOs ↔ Domain objects)
- Don't contain business logic (that's in domain layer)

**Z Monitor Application Services:**

**MonitoringService:**
```cpp
class MonitoringService {
private:
    IPatientRepository* patientRepo;
    ITelemetryRepository* telemetryRepo;
    ITelemetryServer* networkServer;
    
public:
    // Use case: Collect vitals and send telemetry
    Result<void> collectAndTransmitVitals(const QString& patientMrn) {
        // 1. Load patient aggregate
        auto patient = patientRepo->findByMrn(patientMrn);
        if (!patient) {
            return Result<void>::error("Patient not found");
        }
        
        // 2. Create telemetry batch (domain logic)
        auto batch = TelemetryBatch::create(
            patient->getIdentity(),
            collectVitals(),
            collectAlarms()
        );
        
        // 3. Sign batch (domain logic)
        batch.sign(getPrivateKey());
        
        // 4. Persist (infrastructure)
        telemetryRepo->save(batch);
        
        // 5. Transmit (infrastructure)
        auto result = networkServer->sendTelemetry(batch);
        
        // 6. Emit domain event
        emit telemetryQueued(TelemetryQueued{
            .batchId = batch.getId(),
            .patient = patient->getIdentity(),
            // ...
        });
        
        return result;
    }
};
```

**AdmissionService:**
- Orchestrates patient admission workflow
- Coordinates `PatientAggregate` and repositories
- Emits `PatientAdmitted` domain events
- Handles validation and error cases

**ProvisioningService:**
- Handles device provisioning workflow
- Coordinates `DeviceAggregate` and certificate management
- Emits `ProvisioningCompleted` events

**SecurityService:**
- Orchestrates authentication workflow
- Coordinates `UserSession` and audit logging
- Handles PIN policy and session management

---

### 8. Infrastructure Layer

**Definition:** Technical implementations that support the domain layer. Infrastructure handles external concerns like databases, networks, file systems.

**Key Principles:**
- Implements interfaces defined in domain/application layers
- Contains no business logic
- Handles technical concerns (SQL, HTTP, encryption)
- Can be swapped without affecting domain

**Z Monitor Infrastructure Components:**

**Persistence:**
- `SQLitePatientRepository` - Implements `IPatientRepository`
- `SQLiteTelemetryRepository` - Implements `ITelemetryRepository`
- `DatabaseManager` - SQLite/SQLCipher connection management

**Networking:**
- `NetworkTelemetryServer` - Implements `ITelemetryServer`
- Uses `QSslConfiguration` for mTLS
- Handles HTTPS communication

**Provisioning:**
- QR code generation
- Certificate installation
- Secure storage (HSM integration)

**Logging:**
- `LogService` - File-based logging
- Structured logging (JSON format)
- Log rotation and archival

---

### 9. Layered Architecture

**Definition:** Organizing code into layers with clear dependencies. Each layer has specific responsibilities and dependencies flow in one direction.

**Z Monitor Layer Structure:**

```
┌─────────────────────────────────────┐
│   Interface Layer (QML + Controllers)│
│   - QML views                        │
│   - QObject controllers               │
└──────────────┬──────────────────────┘
               │ depends on
┌──────────────▼──────────────────────┐
│   Application Layer (Services)       │
│   - MonitoringService                │
│   - AdmissionService                │
│   - ProvisioningService              │
│   - SecurityService                  │
└──────────────┬──────────────────────┘
               │ depends on
┌──────────────▼──────────────────────┐
│   Domain Layer (Business Logic)      │
│   - Aggregates (Patient, Device)    │
│   - Value Objects (PatientIdentity)  │
│   - Domain Events                    │
│   - Repository Interfaces            │
└──────────────┬──────────────────────┘
               │ depends on
┌──────────────▼──────────────────────┐
│   Infrastructure Layer (Adapters)   │
│   - SQLite repositories              │
│   - Network adapters                 │
│   - Platform adapters (Qt-based)     │
│   - Certificate management           │
└─────────────────────────────────────┘
```

**Dependency Rules:**
- ✅ Interface → Application → Domain
- ✅ Infrastructure → Domain (implements interfaces)
- ❌ Domain → Infrastructure (domain defines interfaces)
- ❌ Domain → Application (domain is independent)
- ❌ Application → Interface (application doesn't know about UI)

**Z Monitor Directory Structure:**

```
z-monitor/src/
├── domain/              # Pure business logic
│   ├── monitoring/
│   │   ├── PatientAggregate.h
│   │   ├── TelemetryBatch.h
│   │   └── events/
│   ├── admission/
│   ├── provisioning/
│   └── security/
│
├── application/        # Use case orchestration
│   ├── services/
│   │   ├── MonitoringService.h
│   │   ├── AdmissionService.h
│   │   └── ...
│   └── dto/            # Data Transfer Objects
│
├── infrastructure/     # Technical implementations
│   ├── persistence/   # SQLite, repositories
│   ├── network/        # HTTPS/mTLS
│   ├── adapters/       # Platform adapters (Qt-based)
│   └── provisioning/   # Certificate management
│
└── interface/          # UI layer
    ├── controllers/    # QObject/QML bridges
    └── qml/            # QML resources
```

---

## DDD Patterns in Z Monitor

### Pattern 1: Aggregate Root Access

**Rule:** External code only accesses aggregates through the root.

```cpp
// ✅ Good: Access through aggregate root
auto patient = patientRepo->findByMrn("12345");
patient->admit(bedAssignment, admittedBy);  // Method on aggregate root
patientRepo->save(*patient);

// ❌ Bad: Direct access to internal objects
auto patient = patientRepo->findByMrn("12345");
patient->getIdentity().mrn = "99999";  // Bypassing aggregate root
```

### Pattern 2: Value Object Immutability

**Rule:** Value objects are immutable and can be replaced.

```cpp
// ✅ Good: Replace value object
auto patient = patientRepo->findByMrn("12345");
auto newIdentity = PatientIdentity{
    .mrn = "12345",
    .name = "Updated Name",  // Changed
    .dateOfBirth = patient->getIdentity().dateOfBirth,
    .sex = patient->getIdentity().sex
};
patient->updateIdentity(newIdentity);  // Replace, don't modify

// ❌ Bad: Modify value object
auto identity = patient->getIdentity();
identity.name = "Updated Name";  // Mutation not allowed
```

### Pattern 3: Domain Events for Side Effects

**Rule:** Use domain events to trigger side effects without coupling.

```cpp
// Application service emits event
void AdmissionService::admitPatient(const AdmitPatientCommand& cmd) {
    auto patient = patientRepo->findByMrn(cmd.mrn);
    patient->admit(cmd.bed, cmd.admittedBy);
    patientRepo->save(*patient);
    
    // Emit domain event (decoupled)
    emit patientAdmitted(PatientAdmitted{
        .patient = patient->getIdentity(),
        .bed = cmd.bed,
        .admittedBy = cmd.admittedBy,
        .occurredAt = QDateTime::currentDateTimeUtc()
    });
}

// Multiple handlers can react (audit, UI, analytics)
void AuditService::onPatientAdmitted(const PatientAdmitted& event) {
    auditRepo->log(event);
}

void DashboardController::onPatientAdmitted(const PatientAdmitted& event) {
    updatePatientBanner(event.patient);
}
```

### Pattern 4: Repository Interface Segregation

**Rule:** Define repository interfaces in domain layer, implement in infrastructure.

```cpp
// Domain layer - interface
namespace Domain::Monitoring {
    class IPatientRepository {
    public:
        virtual ~IPatientRepository() = default;
        virtual std::optional<PatientAggregate> findByMrn(const QString& mrn) = 0;
        virtual void save(const PatientAggregate& patient) = 0;
    };
}

// Infrastructure layer - implementation
namespace Infrastructure::Persistence {
    class SQLitePatientRepository : public Domain::Monitoring::IPatientRepository {
        // SQLite-specific implementation
    };
}

// Application layer - uses interface
class AdmissionService {
    Domain::Monitoring::IPatientRepository* patientRepo;  // Interface, not implementation
};
```

---

## Common DDD Pitfalls to Avoid

### ❌ Pitfall 1: Anemic Domain Model

**Problem:** Domain objects are just data containers with no behavior.

```cpp
// ❌ Bad: Anemic model
class Patient {
public:
    QString mrn;
    QString name;
    AdmissionState state;
    // No methods, just data
};

// ✅ Good: Rich domain model
class PatientAggregate {
public:
    void admit(const BedAssignment& bed, const QString& admittedBy);
    void discharge(const QString& dischargedBy);
    void transfer(const BedAssignment& newBed);
    // Behavior encapsulated in domain object
};
```

### ❌ Pitfall 2: Leaky Abstractions

**Problem:** Infrastructure details leak into domain layer.

```cpp
// ❌ Bad: SQL in domain layer
class PatientAggregate {
    void save() {
        QSqlQuery query;
        query.prepare("INSERT INTO patients ...");  // Infrastructure detail
    }
};

// ✅ Good: Repository abstraction
class PatientAggregate {
    // No persistence knowledge
};

class IPatientRepository {
    void save(const PatientAggregate& patient);  // Domain interface
};
```

### ❌ Pitfall 3: God Aggregates

**Problem:** Aggregates that are too large and contain too much.

```cpp
// ❌ Bad: God aggregate
class PatientAggregate {
    PatientIdentity identity;
    std::vector<VitalRecord> vitals;      // Too much!
    std::vector<AlarmSnapshot> alarms;   // Too much!
    std::vector<TelemetryBatch> batches; // Too much!
    DeviceAggregate device;               // Wrong aggregate!
};

// ✅ Good: Focused aggregates
class PatientAggregate {
    PatientIdentity identity;
    AdmissionState state;
    BedAssignment bed;
    // Only what's needed for admission workflow
};

class TelemetryBatch {
    PatientIdentity patient;  // Reference, not full aggregate
    std::vector<VitalRecord> vitals;
    // Focused on telemetry transmission
};
```

### ❌ Pitfall 4: Shared Domain Models

**Problem:** Same domain model used across multiple bounded contexts.

```cpp
// ❌ Bad: Shared model
// monitoring/Patient.h - used by Monitoring and ADT contexts
class Patient { /* ... */ };

// ✅ Good: Context-specific models
// monitoring/PatientAggregate.h
class PatientAggregate { /* monitoring-specific */ };

// admission/AdmissionAggregate.h
class AdmissionAggregate { /* ADT-specific */ };
```

---

## DDD Benefits in Z Monitor

### 1. **Medical Device Compliance**

DDD helps maintain compliance by:
- Clear audit trails through domain events
- Explicit business rules in domain layer
- Traceable state changes through aggregates
- Separation of concerns for security reviews

### 2. **Testability**

DDD improves testing by:
- Mock repositories for unit tests
- Isolated domain logic (no infrastructure dependencies)
- Value objects are easy to test (immutable, predictable)
- Application services can be tested with test doubles

### 3. **Maintainability**

DDD improves maintainability by:
- Clear boundaries between contexts
- Business logic centralized in domain layer
- Changes isolated to specific layers
- Self-documenting code (ubiquitous language)

### 4. **Scalability**

DDD supports scaling by:
- Independent bounded contexts
- Clear aggregate boundaries
- Repository pattern allows database optimization
- Event-driven architecture for decoupling

---

## When to Use DDD

**✅ Use DDD When:**
- Complex business domain with many rules
- Long-lived application (years of maintenance)
- Multiple teams working on different contexts
- Need for clear business-IT alignment
- Regulatory compliance requirements (medical devices)

**❌ Consider Alternatives When:**
- Simple CRUD application
- Short-lived prototype
- No complex business rules
- Team lacks domain expertise

**Z Monitor Context:**
- ✅ Complex medical device domain
- ✅ Long-term maintenance (regulatory requirements)
- ✅ Multiple bounded contexts (Monitoring, ADT, Provisioning, Security)
- ✅ Clear business rules (alarm priorities, admission workflows)
- ✅ Compliance requirements (IEC 62304, FDA regulations)

---

## Summary

Domain-Driven Design provides a structured approach to building complex software by:

1. **Modeling around business domain** - Code reflects real-world concepts
2. **Using ubiquitous language** - Shared vocabulary between developers and domain experts
3. **Defining bounded contexts** - Clear boundaries for different parts of the system
4. **Organizing with aggregates** - Consistency boundaries for business rules
5. **Separating concerns with layers** - Domain, application, infrastructure, interface
6. **Using repositories** - Abstraction for data access
7. **Emitting domain events** - Decoupled communication between components

In Z Monitor, DDD helps manage the complexity of a medical device application with multiple contexts (Monitoring, ADT, Provisioning, Security), clear business rules, and regulatory compliance requirements.

---

## Further Reading

### Books
- **"Domain-Driven Design" by Eric Evans** - The original DDD book
- **"Implementing Domain-Driven Design" by Vaughn Vernon** - Practical implementation guide

### Z Monitor References
- **`28_DOMAIN_DRIVEN_DESIGN.md`** - Z Monitor-specific DDD strategy
- **`02_ARCHITECTURE.md`** - Architecture overview with DDD layers
- **`09_CLASS_DESIGNS.md`** - Class designs organized by DDD layers

### Related Foundational Topics
- **`02_dependency_injection.md`** - How DI supports DDD
- **`03_repository_pattern.md`** - Repository pattern details
- **`04_data_transfer_objects.md`** - DTOs for layer boundaries

---

*This foundational knowledge document explains DDD concepts in general. For Z Monitor-specific implementation details, see the primary reference document.*
