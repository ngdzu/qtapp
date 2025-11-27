# Database Access Strategy and ORM Plan

This document defines the database access strategy for the Z Monitor application, including ORM approach, repository pattern implementation, and query optimization.

> **📊 Architecture Diagram:**  
> [View Database Access Architecture (Mermaid)](./30_DATABASE_ACCESS_STRATEGY.mmd)  
> [View Database Access Architecture (SVG)](./30_DATABASE_ACCESS_STRATEGY.svg)
>
> **📊 Data Caching Strategy:**  
> [View Data Caching Architecture (36_DATA_CACHING_STRATEGY.md)](./36_DATA_CACHING_STRATEGY.md)

---

## 1. Overview

### **Question: Do we use ORM?**

**Answer: Lightweight ORM Acceptable - Repository Pattern with Qt SQL or QxOrm**

**Key Insight from Data Caching Architecture:**
- **Database operations are NOT in the critical path** (PRIORITY 3 - MEDIUM)
- **Critical path:** Sensor → In-Memory Cache → Alarms (< 50ms) — **does not touch database**
- **Database writes:** Background task, every 10 minutes (non-blocking)
- **Database reads:** Historical queries, trend analysis (non-critical)

**Rationale:**
- **Database is non-critical** - Persistence runs on low-priority background thread
- **In-memory cache handles real-time** - 3-day buffer for critical operations
- **SQLite + SQLCipher** for encryption
- **Qt framework** provides excellent SQL support
- **DDD architecture** with Repository pattern for abstraction
- **Lightweight ORM is acceptable** - No real-time constraints on database layer

---

## 2. Clarification: Database is NOT in the Critical Path

### **Architecture Priorities (from [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md)):**

```
PRIORITY 1 (CRITICAL - Real-Time Thread):
  Sensor → In-Memory Cache → Alarm Evaluation
  Target: < 50ms end-to-end
  ❌ NO DATABASE ACCESS

PRIORITY 2 (HIGH - Real-Time Thread):
  In-Memory Cache → Telemetry Batch → Network Transmission
  Target: Every 10 seconds (batched)
  ❌ NO DATABASE ACCESS

PRIORITY 3 (MEDIUM - Database Thread):
  In-Memory Cache → Database Persistence
  Target: Every 10 minutes (background)
  ✅ DATABASE OPERATIONS HERE (non-critical)

PRIORITY 4 (LOW - Database Thread):
  Database Cleanup (delete > 7 days)
  Target: Daily at 3 AM
  ✅ DATABASE OPERATIONS HERE (non-critical)
```

### **Previous Misunderstanding:**

❌ **OLD ASSUMPTION:** "Alarm latency must be < 50ms" → Need to avoid ORM for database performance  
✅ **CORRECT:** Alarm evaluation uses **in-memory cache only** (no database queries in critical path)

❌ **OLD ASSUMPTION:** "RT thread must avoid heap allocation" → Database must be optimized for RT  
✅ **CORRECT:** Database operations run on **dedicated Database I/O thread** (not RT thread)

### **Database Performance Requirements (Revised):**

|| Operation | Target Latency | Priority | Notes |
||-----------|----------------|----------|-------|
|| **Alarm Evaluation** | < 50ms | CRITICAL | ✅ Uses in-memory cache (no DB) |
|| **Real-time Display** | < 100ms | HIGH | ✅ Uses in-memory cache (no DB) |
|| **Database Persistence** | < 5 seconds | MEDIUM | ⚠️ Background, every 10 min |
|| **Historical Queries** | < 2 seconds | LOW | ⚠️ Trend analysis, not real-time |
|| **Database Cleanup** | < 30 seconds | LOW | ⚠️ Daily at 3 AM |

**Conclusion:** Database latency requirements are **relaxed** (seconds, not milliseconds). Lightweight ORM overhead is acceptable.

---

## 3. ORM Evaluation (Revised)

### **3.1 Why a Lightweight ORM is Now Acceptable:**

✅ **Non-Critical Path**: Database operations don't affect real-time alarm latency  
✅ **Background Thread**: Database runs on dedicated thread (no RT constraints)  
✅ **Developer Productivity**: ORM reduces boilerplate mapping code  
✅ **Type Safety**: ORM provides compile-time type checking  
✅ **Schema Sync**: ORM can auto-generate schema from code (or vice versa)  
✅ **Query Building**: Type-safe query builders prevent SQL injection

### **3.2 ORM Options for Qt/C++:**

|| ORM | Pros | Cons | Verdict |
||-----|------|------|---------|
|| **QxOrm** | Qt-native, lightweight, supports SQLCipher | Learning curve, less popular | ✅ **RECOMMENDED** |
|| **ODB** | Mature, feature-rich, excellent docs | Heavy, complex, large binary | ⚠️ Acceptable |
|| **Qt SQL (Manual)** | Zero overhead, full control | More boilerplate code | ✅ **FALLBACK** |
|| **sqlpp11** | Modern C++, type-safe | Not Qt-native, complex setup | ❌ Not recommended |

### **3.3 Recommended Approach: QxOrm**

**Rationale:**
- **Lightweight**: Minimal overhead, small binary footprint
- **Qt-Native**: Integrates seamlessly with Qt types (QString, QDateTime, etc.)
- **SQLCipher Support**: Works with encrypted databases
- **Repository Pattern**: Natural fit for DDD
- **Non-Intrusive**: POCOs (Plain Old C++ Objects) - no inheritance required
- **Compile-Time Type Safety**: Catch errors at compile time

**Example with QxOrm:**

```cpp
// Domain aggregate (POCO - no QxOrm dependencies)
class PatientAggregate {
public:
    QString mrn;
    QString name;
    QDate dateOfBirth;
    QString sex;
    QString bedLocation;
    QDateTime admittedAt;
    QString admissionSource;
};

// QxOrm registration (in separate file)
QX_REGISTER_HPP_EXPORT(PatientAggregate, qx::trait::no_base_class_defined, 0)

namespace qx {
    template<> void register_class(QxClass<PatientAggregate>& t) {
        t.id(&PatientAggregate::mrn, "mrn");
        t.data(&PatientAggregate::name, "name");
        t.data(&PatientAggregate::dateOfBirth, "date_of_birth");
        t.data(&PatientAggregate::sex, "sex");
        t.data(&PatientAggregate::bedLocation, "bed_location");
        t.data(&PatientAggregate::admittedAt, "admitted_at");
        t.data(&PatientAggregate::admissionSource, "admission_source");
    }
}

// Repository using QxOrm
class SQLitePatientRepository : public IPatientRepository {
public:
    std::optional<PatientAggregate> findByMrn(const QString& mrn) override {
        PatientAggregate patient;
        patient.mrn = mrn;
        
        qx::dao::fetch_by_id(patient);  // ✅ Type-safe, no SQL strings
        
        if (patient.name.isEmpty()) {
            return std::nullopt;
        }
        return patient;
    }
    
    bool save(const PatientAggregate& patient) override {
        auto error = qx::dao::save(patient);  // ✅ Handles INSERT/UPDATE
        return !error.isValid();
    }
};
```

### **3.4 Alternative: Manual Qt SQL (No ORM)**

**If ORM complexity is not worth the benefits, use manual Qt SQL:**

> **🔗 Related Documentation:**  
> **Query Management:** See [32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md) for type-safe query string management using the Query Registry pattern. All repositories use `QueryId::` constants instead of magic strings for compile-time safety and autocomplete support.  
> **Schema Management:** See [33_SCHEMA_MANAGEMENT.md](./33_SCHEMA_MANAGEMENT.md) for schema definition and code generation workflow. All repositories use `Schema::Columns::` constants instead of hardcoded column names for compile-time safety and autocomplete support.

---

## 4. Recommended Approach: Repository Pattern with Qt SQL

### **Architecture:**

```
┌──────────────────────────────────────────┐
│      Application Services Layer          │
│  (MonitoringService, AdmissionService)   │
└──────────────┬───────────────────────────┘
               │ Depends on
               ▼
┌──────────────────────────────────────────┐
│      Repository Interfaces (Domain)      │
│  IPatientRepository, ITelemetryRepository│
└──────────────┬───────────────────────────┘
               │ Implemented by
               ▼
┌──────────────────────────────────────────┐
│   Repository Implementations (Infra)     │
│  SQLitePatientRepository, etc.           │
│  - Uses QSqlQuery (prepared statements)  │
│  - Runs on Database I/O Thread           │
│  - Batch operations (every 10 min)       │
└──────────────┬───────────────────────────┘
               │ Uses
               ▼
┌──────────────────────────────────────────┐
│         DatabaseManager (Infra)          │
│  - QSqlDatabase connection management    │
│  - SQLCipher encryption                  │
│  - Connection pooling                    │
│  - Transaction management                │
└──────────────────────────────────────────┘
```

### **Benefits:**

✅ **Performance**: Direct SQL control, predictable latency  
✅ **Testability**: Mock repositories for testing  
✅ **Abstraction**: Application layer doesn't know about SQL  
✅ **Flexibility**: Can swap SQLite for another DB  
✅ **DDD Alignment**: Repository pattern is DDD best practice

---

## 5. Implementation Details

### 5.1 DatabaseManager (Infrastructure)

**Responsibilities:**
- QSqlDatabase connection management
- SQLCipher encryption setup
- Connection pooling (read/write connections)
- Transaction management
- Migration execution
- **Query Registry integration** (see [32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md))

**Example:**

```cpp
// z-monitor/src/infrastructure/persistence/DatabaseManager.h
class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(QObject* parent = nullptr);
    
    bool open(const QString& dbPath, const QString& encryptionKey);
    void close();
    
    QSqlDatabase& getConnection();           // Main connection
    QSqlDatabase& getWriteConnection();      // Dedicated write connection (DB thread)
    QSqlDatabase& getReadConnection();       // Read-only connection (can be shared)
    
    bool beginTransaction();
    bool commit();
    bool rollback();
    
    bool executeMigrations();
    
    // Query Registry integration (see doc/32_QUERY_REGISTRY.md)
    void registerPreparedQuery(const QString& queryId, const QString& sql);
    QSqlQuery getPreparedQuery(const QString& queryId);  // Use QueryId:: constants
    
signals:
    void connectionOpened();
    void connectionClosed();
    void transactionFailed(const QString& error);
    
private:
    QSqlDatabase m_writeDb;   // Single writer (DB thread)
    QSqlDatabase m_readDb;    // Read-only (can be cloned)
    QMap<QString, QSqlQuery> m_preparedQueries;  // Cached prepared statements
    QString m_encryptionKey;
};
```

### 5.2 Repository Interface (Domain Layer)

**Example: IPatientRepository**

```cpp
// z-monitor/src/domain/repositories/IPatientRepository.h
#include "../aggregates/PatientAggregate.h"
#include "../value_objects/PatientIdentity.h"

class IPatientRepository {
public:
    virtual ~IPatientRepository() = default;
    
    // Pure domain methods (no SQL leakage)
    virtual std::optional<PatientAggregate> findByMrn(const QString& mrn) = 0;
    virtual bool save(const PatientAggregate& patient) = 0;
    virtual bool remove(const QString& mrn) = 0;
    virtual QList<PatientIdentity> findAll() = 0;
    virtual QList<AdmissionEvent> getAdmissionHistory(const QString& mrn) = 0;
};
```

**Key Principles:**
- ✅ **Zero SQL in interface** (pure domain language)
- ✅ **Returns domain objects** (PatientAggregate, not QSqlRecord)
- ✅ **Virtual destructor** (for polymorphism)
- ✅ **std::optional** for "not found" case (no exceptions)

### 5.3 Repository Implementation (Infrastructure Layer)

**Example: SQLitePatientRepository**

```cpp
// z-monitor/src/infrastructure/persistence/SQLitePatientRepository.h
#include "../../domain/repositories/IPatientRepository.h"
#include "DatabaseManager.h"
#include "QueryRegistry.h"  // ✅ Include for QueryId constants

class SQLitePatientRepository : public IPatientRepository {
public:
    explicit SQLitePatientRepository(DatabaseManager* dbManager);
    
    std::optional<PatientAggregate> findByMrn(const QString& mrn) override;
    bool save(const PatientAggregate& patient) override;
    bool remove(const QString& mrn) override;
    QList<PatientIdentity> findAll() override;
    QList<AdmissionEvent> getAdmissionHistory(const QString& mrn) override;
    
private:
    DatabaseManager* m_dbManager;
    
    // Mapping helpers
    PatientAggregate mapToAggregate(const QSqlQuery& query);
    void bindAggregateToQuery(QSqlQuery& query, const PatientAggregate& patient);
};
```

**Implementation (C++ file):**

```cpp
// z-monitor/src/infrastructure/persistence/SQLitePatientRepository.cpp
#include "SQLitePatientRepository.h"
#include "QueryRegistry.h"  // ✅ Include for QueryId constants
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

std::optional<PatientAggregate> SQLitePatientRepository::findByMrn(const QString& mrn) {
    // ✅ Type-safe query ID with autocomplete (no magic strings)
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
    
    // Bind parameters
    query.bindValue(":mrn", mrn);
    
    if (!query.exec()) {
        qWarning() << "Failed to find patient:" << query.lastError().text();
        return std::nullopt;
    }
    
    if (!query.next()) {
        return std::nullopt;  // Not found
    }
    
    // Map QSqlQuery result to domain aggregate
    return mapToAggregate(query);
}

bool SQLitePatientRepository::save(const PatientAggregate& patient) {
    // ✅ Type-safe query ID with autocomplete
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::INSERT);
    
    // Bind domain object to SQL parameters
    bindAggregateToQuery(query, patient);
    
    if (!query.exec()) {
        qWarning() << "Failed to save patient:" << query.lastError().text();
        return false;
    }
    
    return true;
}

PatientAggregate SQLitePatientRepository::mapToAggregate(const QSqlQuery& query) {
    // ✅ Map SQL columns to domain objects using type-safe constants
    // Auto-generated from schema/database.yaml (see doc/33_SCHEMA_MANAGEMENT.md)
    using namespace Schema::Columns::Patients;
    
    PatientIdentity identity(
        query.value(MRN).toString(),
        query.value(NAME).toString(),
        query.value(DOB).toDate(),
        query.value(SEX).toString()
    );
    
    BedLocation bedLocation(query.value(BED_LOCATION).toString());
    
    PatientAggregate aggregate(identity, bedLocation);
    
    // Reconstruct admission state
    if (!query.value(ADMITTED_AT).isNull()) {
        aggregate.admit(
            query.value(ADMISSION_SOURCE).toString(),
            query.value(ADMITTED_AT).toDateTime()
        );
    }
    
    return aggregate;
}

void SQLitePatientRepository::bindAggregateToQuery(
    QSqlQuery& query, 
    const PatientAggregate& patient)
{
    // ✅ Map domain object to SQL parameters
    const auto& identity = patient.getIdentity();
    
    query.bindValue(":mrn", identity.getMrn());
    query.bindValue(":name", identity.getName());
    query.bindValue(":dob", identity.getDateOfBirth());
    query.bindValue(":sex", identity.getSex());
    query.bindValue(":bed_location", patient.getBedLocation().toString());
    query.bindValue(":admitted_at", patient.getAdmittedAt());
    query.bindValue(":admission_source", patient.getAdmissionSource());
}
```

### 5.4 Query Registry Integration

**Strategy:** Use QueryRegistry pattern for type-safe query management

**See:** [32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md) for complete Query Registry implementation

```cpp
// DatabaseManager initialization (uses QueryCatalog)
void DatabaseManager::initialize() {
    // Initialize all queries from QueryCatalog
    QueryCatalog::initializeQueries(this);
    
    qInfo() << "Initialized" << m_preparedQueries.size() << "prepared queries";
}

// Repositories use type-safe QueryId constants:
QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
                                                 // ↑ Autocomplete works!
```

**Benefits of Query Registry:**
- ✅ **Type Safety:** Compile-time checking (no typos)
- ✅ **Autocomplete:** IDE suggests all available queries
- ✅ **Easy to Find:** Ctrl+Click to find all usages
- ✅ **Centralized:** All SQL in one place (QueryCatalog.cpp)
- ✅ **Auto-Documented:** Generates QUERY_REFERENCE.md

### 5.5 Batch Operations (For Periodic Persistence)

**Strategy:** Batch INSERT for time-series data (runs every 10 minutes on Database thread)

```cpp
// SQLiteTelemetryRepository
bool SQLiteTelemetryRepository::saveBatch(const TelemetryBatch& batch) {
    auto& db = m_dbManager->getWriteConnection();
    
    // Start transaction
    if (!db.transaction()) {
        return false;
    }
    
    // ✅ Use type-safe QueryId constant (no magic string)
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Vitals::INSERT);
    
    // Batch INSERT using prepared statement
    const auto& vitals = batch.getVitals();
    for (const auto& vital : vitals) {
        query.bindValue(":timestamp", vital.timestamp);
        query.bindValue(":patient_mrn", vital.patientMrn);
        query.bindValue(":heart_rate", vital.heartRate);
        query.bindValue(":spo2", vital.spo2);
        query.bindValue(":respiration_rate", vital.respirationRate);
        query.bindValue(":batch_id", batch.getBatchId());
        
        if (!query.exec()) {
            qWarning() << "Batch insert failed:" << query.lastError().text();
            db.rollback();
            return false;
        }
    }
    
    // Commit transaction
    if (!db.commit()) {
        db.rollback();
        return false;
    }
    
    return true;
}
```

**Performance:**
- ✅ Single transaction for batch (faster)
- ✅ Prepared statement reuse (no re-parse)
- ✅ Runs on background thread (no impact on critical path)
- ⚠️ 5-second target latency (relaxed requirement)

---

## 6. Query Optimization Strategies

### 6.1 Time-Series Optimizations

**Vitals Table:**

```sql
-- 1. Compound index for time-range queries
CREATE INDEX IF NOT EXISTS idx_vitals_patient_time 
ON vitals(patient_mrn, timestamp);

-- 2. Covering index for common SELECT
CREATE INDEX IF NOT EXISTS idx_vitals_patient_time_hr_spo2
ON vitals(patient_mrn, timestamp, heart_rate, spo2);

-- 3. Partial index for unsync'd data
CREATE INDEX IF NOT EXISTS idx_vitals_unsynced
ON vitals(is_synced, timestamp)
WHERE is_synced = 0;
```

### 6.2 WAL Mode (Write-Ahead Logging)

```cpp
// DatabaseManager::open()
void DatabaseManager::open(const QString& dbPath, const QString& key) {
    QSqlQuery query(m_writeDb);
    
    // Enable WAL mode (concurrent reads during writes)
    query.exec("PRAGMA journal_mode = WAL;");
    
    // Optimize for background operations (not real-time)
    query.exec("PRAGMA synchronous = NORMAL;");  // Safe with WAL
    query.exec("PRAGMA cache_size = -8000;");    // 8MB cache
    query.exec("PRAGMA temp_store = MEMORY;");   // Temp tables in memory
    
    // SQLCipher encryption
    query.exec(QString("PRAGMA key = '%1';").arg(key));
    query.exec("PRAGMA cipher_page_size = 4096;");
}
```

### 6.3 Connection Strategy

```
┌─────────────────────────┐
│   Write Connection      │  ← Database Thread (single writer)
│   (m_writeDb)           │     All INSERT/UPDATE/DELETE
└─────────────────────────┘     Runs every 10 minutes (batch)

┌─────────────────────────┐
│   Read Connection       │  ← Application Services threads
│   (m_readDb)            │     SELECT only (historical queries)
└─────────────────────────┘     Non-critical (trend analysis)
```

---

## 7. Performance Benchmarks and Targets (Revised)

|| Operation | Target Latency | Thread | Notes |
||-----------|----------------|--------|-------|
|| **INSERT batch (1000 vitals)** | < 5 seconds | Database I/O | Every 10 minutes |
|| **SELECT historical (1 hour)** | < 2 seconds | Database I/O | Trend analysis |
|| **SELECT patient by MRN** | < 500 ms | Database I/O | Non-critical |
|| **UPDATE patient** | < 500 ms | Database I/O | Non-critical |
|| **Database cleanup (7 days)** | < 30 seconds | Database I/O | Daily at 3 AM |

**Key Difference from Previous Version:**
- ❌ **OLD:** Sub-millisecond targets (unrealistic for non-critical operations)
- ✅ **NEW:** Seconds-scale targets (appropriate for background operations)

---

## 8. Testing Strategy

### 8.1 Repository Unit Tests

```cpp
// tests/unit/SQLitePatientRepositoryTest.cpp
TEST_F(SQLitePatientRepositoryTest, FindByMrn_ReturnsPatient) {
    // Arrange
    auto patient = createTestPatient("MRN-001");
    m_repository->save(patient);
    
    // Act
    auto result = m_repository->findByMrn("MRN-001");
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getIdentity().getMrn(), "MRN-001");
}

TEST_F(SQLitePatientRepositoryTest, FindByMrn_NotFound_ReturnsNullopt) {
    // Act
    auto result = m_repository->findByMrn("DOES-NOT-EXIST");
    
    // Assert
    EXPECT_FALSE(result.has_value());
}
```

### 8.2 Performance Tests (Revised Targets)

```cpp
// tests/benchmarks/TelemetryRepositoryBenchmark.cpp
TEST(TelemetryRepositoryBenchmark, InsertBatch_1000Vitals_Under5Seconds) {
    auto batch = createTestBatch(1000);  // 1000 vitals
    
    auto start = std::chrono::high_resolution_clock::now();
    bool success = m_repository->saveBatch(batch);
    auto duration = std::chrono::high_resolution_clock::now() - start;
    
    ASSERT_TRUE(success);
    EXPECT_LT(std::chrono::duration_cast<std::chrono::seconds>(duration).count(), 5);
}
```

---

## 9. Migration Strategy

**Schema migrations using numbered SQL files:**

```
project-dashboard/doc/migrations/
├── 0001_initial_schema.sql
├── 0002_add_telemetry_metrics.sql
├── 0003_adt_workflow.sql
└── README.md
```

**See:** [34_DATA_MIGRATION_WORKFLOW.md](./34_DATA_MIGRATION_WORKFLOW.md) for complete migration strategy

---

## 10. Summary: Database Access Plan (Revised)

### **ORM Strategy: Lightweight ORM Acceptable**

✅ **Recommended Approach:** QxOrm (lightweight, Qt-native)  
✅ **Fallback Approach:** Repository pattern with Qt SQL (no ORM)

### **Key Decisions (Revised):**

|| Aspect | Decision | Rationale |
||--------|----------|-----------|
|| **ORM Framework** | QxOrm (or Qt SQL) | Non-critical path allows ORM overhead |
|| **Critical Path** | In-memory cache only | Database not used for real-time operations |
|| **Database Thread** | Dedicated background thread | No RT constraints |
|| **Persistence Schedule** | Every 10 minutes (batch) | Non-blocking, low priority |
|| **Performance Target** | < 5 seconds (batch insert) | Relaxed requirements |
|| **Abstraction** | Repository pattern (DDD) | Testability, decoupling |
|| **Query Type** | Prepared statements (cached) | Performance, SQL injection prevention |
|| **Transactions** | Manual control (batch operations) | Predictable latency |
|| **Connection** | Single writer + read connections | SQLite write serialization + WAL reads |
|| **Schema Migrations** | Numbered SQL files (embedded) | Explicit, auditable, version controlled |

### **Benefits:**

✅ **Simplified Development**: ORM reduces boilerplate (if using QxOrm)  
✅ **No Performance Impact**: Database not in critical path  
✅ **Testability**: Mock repositories  
✅ **Regulatory**: Explicit, auditable persistence  
✅ **Qt Native**: Leverages Qt SQL

### **Trade-offs:**

⚠️ **ORM Learning Curve**: QxOrm requires initial learning (if chosen)  
⚠️ **Binary Size**: Small increase if using ORM  
⚠️ **Less Control**: ORM generates SQL (but acceptable for non-critical operations)

---

## 11. Implementation Checklist

- [ ] **Decide on ORM:** QxOrm vs Manual Qt SQL
  - [ ] If QxOrm: Add dependency, configure build
  - [ ] If Manual: Implement Query Registry pattern (see [32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md))
- [ ] Create `DatabaseManager` class (connection management)
- [ ] Define repository interfaces in domain layer
- [ ] Implement SQLite repositories in infrastructure layer
  - [ ] Run on Database I/O thread (not RT thread)
  - [ ] Use batch INSERT for periodic persistence (every 10 min)
- [ ] Enable WAL mode and optimize PRAGMAs
- [ ] Create migration system (see [34_DATA_MIGRATION_WORKFLOW.md](./34_DATA_MIGRATION_WORKFLOW.md))
- [ ] Write unit tests for repositories
- [ ] Write performance benchmarks (< 5 second batch insert target)
- [ ] Document mapping conventions (domain ↔ SQL)
- [ ] Add to `ZTODO.md`

---

## 12. References

- **[36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md)** – Data caching architecture (critical path explanation)
- **[33_SCHEMA_MANAGEMENT.md](./33_SCHEMA_MANAGEMENT.md)** – Schema definition and code generation (YAML → SchemaInfo.h)
- **[32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md)** – Query string management and type-safe query IDs
- **[34_DATA_MIGRATION_WORKFLOW.md](./34_DATA_MIGRATION_WORKFLOW.md)** – Database migration strategy
- **[31_DATA_TRANSFER_OBJECTS.md](./31_DATA_TRANSFER_OBJECTS.md)** – DTOs for data transfer between layers
- **[10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md)** – Database schema
- **[28_DOMAIN_DRIVEN_DESIGN.md](./28_DOMAIN_DRIVEN_DESIGN.md)** – DDD strategy and repository pattern
- **[29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md)** – Complete component list
- **[12_THREAD_MODEL.md](./12_THREAD_MODEL.md)** – Thread assignment (Database I/O thread)
- Qt SQL Documentation: https://doc.qt.io/qt-6/sql-programming.html
- QxOrm Documentation: https://www.qxorm.com/

---

**Document Version:** 2.0  
**Last Updated:** 2025-11-27  
**Status:** Revised based on [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md)

*This document defines the database access strategy for Z Monitor. The revised approach recognizes that database operations are non-critical and run on a background thread, making lightweight ORM overhead acceptable.*
