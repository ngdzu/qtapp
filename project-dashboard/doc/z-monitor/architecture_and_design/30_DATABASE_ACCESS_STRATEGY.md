# Database Access Strategy and ORM Plan

This document defines the database access strategy for the Z Monitor application, including ORM approach, repository pattern implementation, and query optimization.

> **📊 Architecture Diagram:**  
> [View Database Access Architecture (Mermaid)](./30_DATABASE_ACCESS_STRATEGY.mmd)  
> [View Database Access Architecture (SVG)](./30_DATABASE_ACCESS_STRATEGY.svg)

---

## 1. Overview

### **Question: Do we use ORM?**

**Answer: Hybrid Approach - Repository Pattern with Qt SQL (No Heavy ORM)**

**Rationale:**
- **Medical device performance requirements** (< 50ms alarm latency)
- **Real-time constraints** (100-500 Hz sample rate)
- **SQLite + SQLCipher** for encryption
- **Qt framework** already provides excellent SQL support
- **DDD architecture** with Repository pattern for abstraction

---

## 2. Why NOT Use a Heavy ORM (like QxOrm, ODB, etc.)?

### **Problems with Heavy ORMs in Real-Time Medical Devices:**

| Issue | Impact | Our Constraint |
|-------|--------|----------------|
| **Performance Overhead** | ORM query building, lazy loading, change tracking | Alarm latency must be < 50ms |
| **Memory Allocation** | Object creation per row, caching | RT thread must avoid heap allocation |
| **Unpredictable Latency** | Automatic lazy loading, cascade operations | P95 latency must be deterministic |
| **Complexity** | Learning curve, debugging, schema sync | Embedded device, limited resources |
| **Binary Size** | Large dependencies | Embedded device, storage constraints |
| **Control Loss** | Abstract SQL generation | Need precise query optimization |

### **Medical Device Requirements:**

✅ **Deterministic performance** (P95 < 100ms)  
✅ **Zero allocations in RT thread**  
✅ **Precise query control** (optimized for time-series)  
✅ **Small binary footprint**  
✅ **Explicit, auditable SQL** (for regulatory compliance)

---

## 3. Recommended Approach: Repository Pattern with Qt SQL

> **🔗 Related Documentation:**  
> **Query Management:** See [32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md) for type-safe query string management using the Query Registry pattern. All repositories use `QueryId::` constants instead of magic strings for compile-time safety and autocomplete support.  
> **Schema Management:** See [33_SCHEMA_MANAGEMENT.md](./33_SCHEMA_MANAGEMENT.md) for schema definition and code generation workflow. All repositories use `Schema::Columns::` constants instead of hardcoded column names for compile-time safety and autocomplete support.

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
│  - Pre-allocated buffers                 │
│  - Batch operations                      │
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

✅ **Performance**: Direct SQL control, no ORM overhead  
✅ **Testability**: Mock repositories for testing  
✅ **Abstraction**: Application layer doesn't know about SQL  
✅ **Flexibility**: Can swap SQLite for another DB  
✅ **DDD Alignment**: Repository pattern is DDD best practice

---

## 4. Implementation Details

### 4.1 DatabaseManager (Infrastructure)

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

### 4.2 Repository Interface (Domain Layer)

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
- ✅ **std::optional** for "not found" case (no exceptions in RT path)

### 4.3 Repository Implementation (Infrastructure Layer)

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
    
    // No need to cache queries - DatabaseManager handles it via QueryRegistry
    
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
    // Note: bindValue uses SQL parameter names (:param), not column names
    // SQL parameter names are defined in QueryCatalog (see doc/32_QUERY_REGISTRY.md)
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

### 4.4 Query Registry Integration

**Strategy:** Use QueryRegistry pattern for type-safe query management

**See:** `doc/32_QUERY_REGISTRY.md` for complete Query Registry implementation

```cpp
// DatabaseManager initialization (uses QueryCatalog)
void DatabaseManager::initialize() {
    // Initialize all queries from QueryCatalog
    // This replaces manual prepared statement setup with centralized registry
    QueryCatalog::initializeQueries(this);
    
    // QueryCatalog registers all queries like:
    // - QueryId::Patient::FIND_BY_MRN
    // - QueryId::Patient::INSERT
    // - QueryId::Vitals::INSERT
    // - QueryId::Vitals::GET_UNSYNCED
    // - ... (all queries defined in QueryRegistry.h)
    
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

### 4.5 Batch Operations (Critical for Performance)

**Strategy:** Batch INSERT/UPDATE for time-series data

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
- ✅ Reduced fsync calls (WAL mode)

---

## 5. Query Optimization Strategies

### 5.1 Time-Series Optimizations

**Vitals Table (Hottest Path):**

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

### 5.2 WAL Mode (Write-Ahead Logging)

```cpp
// DatabaseManager::open()
void DatabaseManager::open(const QString& dbPath, const QString& key) {
    QSqlQuery query(m_writeDb);
    
    // Enable WAL mode (concurrent reads during writes)
    query.exec("PRAGMA journal_mode = WAL;");
    
    // Optimize for embedded device
    query.exec("PRAGMA synchronous = NORMAL;");  // Faster than FULL, safe with WAL
    query.exec("PRAGMA cache_size = -8000;");    // 8MB cache
    query.exec("PRAGMA temp_store = MEMORY;");   // Temp tables in memory
    
    // SQLCipher encryption
    query.exec(QString("PRAGMA key = '%1';").arg(key));
    query.exec("PRAGMA cipher_page_size = 4096;");
}
```

### 5.3 Connection Strategy

```
┌─────────────────────────┐
│   Write Connection      │  ← DB Thread (single writer)
│   (m_writeDb)           │     All INSERT/UPDATE/DELETE
└─────────────────────────┘

┌─────────────────────────┐
│   Read Connection       │  ← Application Services threads
│   (m_readDb)            │     SELECT only (can be concurrent with WAL)
└─────────────────────────┘
```

---

## 6. Performance Benchmarks and Targets

| Operation | Target Latency | Implementation |
|-----------|----------------|----------------|
| **INSERT single vital** | < 5 ms | Prepared statement, no transaction |
| **INSERT batch (100 vitals)** | < 50 ms | Single transaction, prepared statement |
| **SELECT recent vitals (1 hour)** | < 20 ms | Covering index, patient_mrn + timestamp |
| **SELECT patient by MRN** | < 2 ms | Primary key or unique index |
| **UPDATE patient** | < 5 ms | Prepared statement, WHERE mrn |
| **Transaction commit (batch)** | < 100 ms | WAL mode, NORMAL synchronous |

---

## 7. Testing Strategy

### 7.1 Repository Unit Tests

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

### 7.2 Performance Tests

```cpp
// tests/benchmarks/TelemetryRepositoryBenchmark.cpp
TEST(TelemetryRepositoryBenchmark, InsertBatch_100Vitals_Under50ms) {
    auto batch = createTestBatch(100);  // 100 vitals
    
    auto start = std::chrono::high_resolution_clock::now();
    bool success = m_repository->saveBatch(batch);
    auto duration = std::chrono::high_resolution_clock::now() - start;
    
    ASSERT_TRUE(success);
    EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count(), 50);
}
```

---

## 8. Migration Strategy

**Schema migrations using numbered SQL files:**

```
project-dashboard/doc/migrations/
├── 0001_initial_schema.sql
├── 0002_add_telemetry_metrics.sql
├── 0003_adt_workflow.sql
└── README.md
```

**Migration Execution:**

```cpp
// DatabaseManager::executeMigrations()
bool DatabaseManager::executeMigrations() {
    // Create migrations table if not exists
    QSqlQuery query(m_writeDb);
    query.exec(
        "CREATE TABLE IF NOT EXISTS schema_migrations ("
        "  version INTEGER PRIMARY KEY,"
        "  applied_at INTEGER NOT NULL"
        ")"
    );
    
    // Get current version
    query.exec("SELECT MAX(version) FROM schema_migrations");
    int currentVersion = query.next() ? query.value(0).toInt() : 0;
    
    // Apply pending migrations
    QDir migrationsDir(":/migrations");  // Embedded in resources
    auto files = migrationsDir.entryList({"*.sql"}, QDir::Files, QDir::Name);
    
    for (const auto& file : files) {
        int version = extractVersion(file);
        if (version > currentVersion) {
            if (!applyMigration(file, version)) {
                return false;
            }
        }
    }
    
    return true;
}
```

---

## 9. Summary: Database Access Plan

### **ORM Strategy: NO Heavy ORM**

✅ **Approach:** Repository pattern with Qt SQL (QSqlQuery + prepared statements)

### **Key Decisions:**

| Aspect | Decision | Rationale |
|--------|----------|-----------|
| **ORM Framework** | None (use Qt SQL directly) | Performance, control, simplicity |
| **Abstraction** | Repository pattern (DDD) | Testability, decoupling |
| **Query Type** | Prepared statements (cached) | Performance, SQL injection prevention |
| **Transactions** | Manual control (batch operations) | Predictable latency |
| **Connection** | Single writer + read connections | SQLite write serialization + WAL reads |
| **Schema Migrations** | Numbered SQL files (embedded) | Explicit, auditable, version controlled |
| **Domain Mapping** | Manual (explicit mapping methods) | Control, performance, no magic |

### **Benefits:**

✅ **Performance**: Direct SQL, no ORM overhead  
✅ **Deterministic**: No lazy loading surprises  
✅ **Testability**: Mock repositories  
✅ **Regulatory**: Explicit, auditable SQL  
✅ **Small Binary**: No heavy ORM dependencies  
✅ **Qt Native**: Leverages Qt SQL (already in Qt)

### **Trade-offs:**

⚠️ **More Boilerplate**: Manual mapping code (but explicit and controllable)  
⚠️ **No Automatic Sync**: Schema changes require manual migration files  
⚠️ **Less "Magic"**: No automatic change tracking (but we don't need it)

---

## 10. Implementation Checklist

- [ ] Create `DatabaseManager` class (connection management)
- [ ] **Implement Query Registry pattern** (see [32_QUERY_REGISTRY.md](./32_QUERY_REGISTRY.md))
  - [ ] Create `QueryRegistry.h` with QueryId namespace constants
  - [ ] Create `QueryCatalog.cpp` with query definitions
  - [ ] Update `DatabaseManager` to support query registration
- [ ] Define repository interfaces in domain layer
- [ ] Implement SQLite repositories in infrastructure layer
  - [ ] Use `QueryId::` constants (no magic strings)
  - [ ] Include `QueryRegistry.h` in all repositories
- [ ] Implement batch INSERT for time-series data
- [ ] Enable WAL mode and optimize PRAGMAs
- [ ] Create migration system (numbered SQL files)
- [ ] Write unit tests for repositories
- [ ] Write performance benchmarks (< 50ms batch insert target)
- [ ] Document mapping conventions (domain ↔ SQL)
- [ ] Add to `ZTODO.md`

---

## 11. References

- **`doc/33_SCHEMA_MANAGEMENT.md`** – Schema definition and code generation (YAML → SchemaInfo.h)
- **`doc/32_QUERY_REGISTRY.md`** – Query string management and type-safe query IDs
- `doc/31_DATA_TRANSFER_OBJECTS.md` – DTOs for data transfer between layers
- `doc/10_DATABASE_DESIGN.md` – Database schema
- `doc/28_DOMAIN_DRIVEN_DESIGN.md` – DDD strategy and repository pattern
- `doc/29_SYSTEM_COMPONENTS.md` – Complete component list
- `doc/12_THREAD_MODEL.md` – Thread assignment (DB thread = single writer)
- Qt SQL Documentation: https://doc.qt.io/qt-6/sql-programming.html

---

*This document defines the database access strategy for Z Monitor. The approach prioritizes performance, determinism, and regulatory compliance over convenience.*

