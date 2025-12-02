---
doc_id: DOC-GUIDE-014
title: Database Access Strategy and Repository Pattern
version: 1.0
category: Guidelines
status: Approved
related_docs:
  - DOC-ARCH-017  # Database Design
  - DOC-ARCH-001  # Software Architecture
  - DOC-GUIDE-015 # Data Transfer Objects
  - DOC-COMP-032  # Query Registry
  - DOC-PROC-009  # Schema Management
tags:
  - database
  - repository-pattern
  - orm
  - ddd
  - qxorm
  - query-optimization
source:
  original_id: DESIGN-030
  file: 30_DATABASE_ACCESS_STRATEGY.md
  migrated_date: 2025-12-01
---

# Database Access Strategy and Repository Pattern

## Purpose

This document defines the database access strategy for the Z Monitor application, including ORM approach, repository pattern implementation, and query optimization. It establishes guidelines for how application code interacts with the database while maintaining clean architecture principles and performance requirements.

The strategy ensures:
1. **Clean separation** between domain logic and data persistence
2. **Performance optimization** for time-series data and real-time queries
3. **Type safety** through schema constants and query registry
4. **Testability** through dependency injection and repository interfaces
5. **DDD alignment** with repository pattern and aggregate boundaries

## Clarification: Database is NOT in the Critical Path

### Architecture Priorities

From DOC-COMP-026 (Data Caching Strategy):

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

### Previous Misunderstanding

**❌ OLD ASSUMPTION:** "Alarm latency must be < 50ms" → Need to avoid ORM for database performance

**✅ CORRECT:** Alarm evaluation uses **in-memory cache only** (no database queries in critical path)

**❌ OLD ASSUMPTION:** "RT thread must avoid heap allocation" → Database must be optimized for RT

**✅ CORRECT:** Database operations run on **dedicated Database I/O thread** (not RT thread)

### Database Performance Requirements (Revised)

| Operation                | Target Latency | Priority | Notes                           |
| ------------------------ | -------------- | -------- | ------------------------------- |
| **Alarm Evaluation**     | < 50ms         | CRITICAL | ✅ Uses in-memory cache (no DB)  |
| **Real-time Display**    | < 100ms        | HIGH     | ✅ Uses in-memory cache (no DB)  |
| **Database Persistence** | < 5 seconds    | MEDIUM   | ⚠️ Background, every 10 min      |
| **Historical Queries**   | < 2 seconds    | LOW      | ⚠️ Trend analysis, not real-time |
| **Database Cleanup**     | < 30 seconds   | LOW      | ⚠️ Daily at 3 AM                 |

**Conclusion:** Database latency requirements are **relaxed** (seconds, not milliseconds). Lightweight ORM overhead is acceptable.

## ORM Evaluation (Revised)

### Why a Lightweight ORM is Now Acceptable

✅ **Non-Critical Path:** Database operations don't affect real-time alarm latency  
✅ **Background Thread:** Database runs on dedicated thread (no RT constraints)  
✅ **Developer Productivity:** ORM reduces boilerplate mapping code  
✅ **Type Safety:** ORM provides compile-time type checking  
✅ **Schema Sync:** ORM can auto-generate schema from code (or vice versa)  
✅ **Query Building:** Type-safe query builders prevent SQL injection

### ORM Options for Qt/C++

| ORM                 | Pros                                       | Cons                         | Verdict           |
| ------------------- | ------------------------------------------ | ---------------------------- | ----------------- |
| **QxOrm**           | Qt-native, lightweight, supports SQLCipher | Learning curve, less popular | ✅ **RECOMMENDED** |
| **ODB**             | Mature, feature-rich, excellent docs       | Heavy, complex, large binary | ⚠️ Acceptable      |
| **Qt SQL (Manual)** | Zero overhead, full control                | More boilerplate code        | ✅ **FALLBACK**    |
| **sqlpp11**         | Modern C++, type-safe                      | Not Qt-native, complex setup | ❌ Not recommended |

### Recommended Approach: Hybrid QxOrm + Manual SQL

**Decision: Use QxOrm for simple CRUD, manual SQL for complex queries**

**Rationale:**
- **Lightweight:** Minimal overhead, small binary footprint
- **Qt-Native:** Integrates seamlessly with Qt types (QString, QDateTime, etc.)
- **SQLCipher Support:** Works with encrypted databases
- **Repository Pattern:** Natural fit for DDD
- **Non-Intrusive:** POCOs (Plain Old C++ Objects) - no inheritance required
- **Compile-Time Type Safety:** Catch errors at compile time
- **Schema Integration:** Can use schema constants for table/column names (see DOC-PROC-009)
- **Hybrid Flexibility:** Use ORM for simple operations, manual SQL for complex queries

**Implementation Status:**
> **📋 Note:** ORM integration is planned but not yet implemented. See ZTODO.md task "Implement QxOrm Integration (Hybrid ORM + Stored Procedures)" for implementation plan. Schema management infrastructure is ready for ORM integration.

**Example with QxOrm + Schema Constants:**

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
// ✅ Uses schema constants instead of hardcoded strings
#include "generated/SchemaInfo.h"

QX_REGISTER_HPP_EXPORT(PatientAggregate, qx::trait::no_base_class_defined, 0)

namespace qx {
    template<> void register_class(QxClass<PatientAggregate>& t) {
        using namespace Schema::Columns::Patients;  // ✅ Import schema constants
        
        // ✅ Type-safe column names from schema
        t.id(&PatientAggregate::mrn, MRN);                           // "mrn"
        t.data(&PatientAggregate::name, NAME);                        // "name"
        t.data(&PatientAggregate::dateOfBirth, DOB);                  // "dob"
        t.data(&PatientAggregate::sex, SEX);                          // "sex"
        t.data(&PatientAggregate::bedLocation, BED_LOCATION);         // "bed_location"
        t.data(&PatientAggregate::admittedAt, ADMITTED_AT);           // "admitted_at"
        t.data(&PatientAggregate::admissionSource, ADMISSION_SOURCE); // "admission_source"
        
        // ✅ Table name from schema
        t.setName(Schema::Tables::PATIENTS);  // "patients"
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

**Benefits of Schema Integration:**
- ✅ **Single Source of Truth:** Column names defined in `database.yaml` only
- ✅ **Type Safety:** QxOrm + schema constants = double type safety
- ✅ **Refactoring:** Rename column in YAML → regenerate → compile error if ORM mapping outdated
- ✅ **No Duplication:** No hardcoded strings in ORM registration

### Hybrid Strategy: When to Use ORM vs Manual SQL

**Use ORM (QxOrm) for:**
- ✅ Simple CRUD operations (Patient, User, Settings aggregates)
- ✅ Single-record lookups (findByMrn, findById, findByUsername)
- ✅ Simple inserts/updates (save, update single record)
- ✅ Type-safe object mapping
- ✅ Operations that benefit from ORM's automatic INSERT/UPDATE detection

**Use Manual SQL/Query Registry for:**
- ✅ Time-series queries (vitals with date ranges, aggregations)
- ✅ Complex joins (multi-table queries)
- ✅ Performance-critical paths (real-time queries)
- ✅ Batch operations (bulk inserts, bulk updates)
- ✅ Custom queries that don't map well to ORM
- ✅ Aggregation queries (COUNT, SUM, AVG, GROUP BY)
- ✅ Complex WHERE clauses with multiple conditions

**Example Hybrid Repository:**

```cpp
class SQLitePatientRepository : public IPatientRepository {
public:
    // ✅ Use ORM for simple CRUD
    std::optional<PatientAggregate> findByMrn(const QString& mrn) override {
        PatientAggregate patient;
        patient.mrn = mrn;
        qx::dao::fetch_by_id(patient);  // ORM - simple lookup
        return patient.name.isEmpty() ? std::nullopt : std::make_optional(patient);
    }
    
    Result<void> save(const PatientAggregate& patient) override {
        auto error = qx::dao::save(patient);  // ORM - automatic INSERT/UPDATE
        return error.isValid() 
            ? Result<void>::error(Error::create(ErrorCode::DatabaseError, error.text().toStdString()))
            : Result<void>::ok();
    }
    
    // ✅ Use manual SQL for complex queries
    QList<PatientAggregate> findAdmittedPatientsInDateRange(
        const QDateTime& startDate, 
        const QDateTime& endDate) override {
        // Manual SQL with Query Registry + Schema constants
        QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_ADMITTED_IN_RANGE);
        query.bindValue(":start_date", startDate.toMSecsSinceEpoch());
        query.bindValue(":end_date", endDate.toMSecsSinceEpoch());
        // ... complex query logic with joins, aggregations ...
    }
};
```

> **🔗 Related Documentation:**  
> **Query Management:** See DOC-COMP-032 (Query Registry) for type-safe query string management. All repositories use `QueryId::` constants instead of magic strings for compile-time safety and autocomplete support.  
> **Schema Management:** See DOC-PROC-009 (Schema Management) for schema definition and code generation workflow. All repositories use `Schema::Columns::` constants instead of hardcoded column names.

## Repository Pattern with Qt SQL

### Architecture

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

**Benefits:**

✅ **Performance:** Direct SQL control, predictable latency  
✅ **Testability:** Mock repositories for testing  
✅ **Abstraction:** Application layer doesn't know about SQL  
✅ **Flexibility:** Can swap SQLite for another DB  
✅ **DDD Alignment:** Repository pattern is DDD best practice

### Implementation Details

**DatabaseManager (Infrastructure):**

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
    
    // Query Registry integration (see DOC-COMP-032)
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

**Repository Interface (Domain Layer):**

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

**Repository Implementation (Infrastructure Layer):**

```cpp
// z-monitor/src/infrastructure/persistence/SQLitePatientRepository.h
#include "../../domain/repositories/IPatientRepository.h"
#include "DatabaseManager.h"
#include "QueryRegistry.h"  // ✅ Include for QueryId constants

class SQLitePatientRepository : public IPatientRepository {
public:
    explicit SQLitePatientRepository(DatabaseManager* dbManager, LogService* logService);
    
    std::optional<PatientAggregate> findByMrn(const QString& mrn) override;
    bool save(const PatientAggregate& patient) override;
    bool remove(const QString& mrn) override;
    QList<PatientIdentity> findAll() override;
    QList<AdmissionEvent> getAdmissionHistory(const QString& mrn) override;
    
private:
    DatabaseManager* m_dbManager;
    LogService* m_logService;  // Injected dependency for structured logging
    
    // Mapping helpers
    PatientAggregate mapToAggregate(const QSqlQuery& query);
    void bindAggregateToQuery(QSqlQuery& query, const PatientAggregate& patient);
};
```

**Implementation Example:**

```cpp
// z-monitor/src/infrastructure/persistence/SQLitePatientRepository.cpp
#include "SQLitePatientRepository.h"
#include "QueryRegistry.h"  // ✅ Include for QueryId constants
#include "../../qt/LogService.h"  // ✅ Include for structured logging
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

// ✅ Constructor with dependency injection (LogService for structured logging)
SQLitePatientRepository::SQLitePatientRepository(DatabaseManager* dbManager, LogService* logService)
    : m_dbManager(dbManager), m_logService(logService) {
}

std::optional<PatientAggregate> SQLitePatientRepository::findByMrn(const QString& mrn) {
    // ✅ Type-safe query ID with autocomplete (no magic strings)
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
    
    // Bind parameters
    query.bindValue(":mrn", mrn);
    
    if (!query.exec()) {
        m_logService->warning("Failed to find patient", {
            {"mrn", mrn},
            {"error", query.lastError().text()}
        });
        return std::nullopt;
    }
    
    if (!query.next()) {
        return std::nullopt;  // Not found
    }
    
    // Map QSqlQuery result to domain aggregate
    return mapToAggregate(query);
}

bool SQLitePatientRepository::save(const PatientAggregate& patient) {
    // ✅ Type-safe query ID
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::INSERT);
    
    // Bind aggregate data to query
    bindAggregateToQuery(query, patient);
    
    if (!query.exec()) {
        m_logService->error("Failed to save patient", {
            {"mrn", patient.getIdentity().getMrn()},
            {"error", query.lastError().text()}
        });
        return false;
    }
    
    m_logService->debug("Patient saved", {
        {"mrn", patient.getIdentity().getMrn()}
    });
    
    return true;
}

// Private mapping helpers
PatientAggregate SQLitePatientRepository::mapToAggregate(const QSqlQuery& query) {
    using namespace Schema::Columns::Patients;  // ✅ Use schema constants
    
    PatientIdentity identity(
        query.value(MRN).toString(),
        query.value(NAME).toString(),
        query.value(DOB).toDate(),
        query.value(SEX).toString()
    );
    
    BedLocation bedLocation(query.value(BED_LOCATION).toString());
    
    PatientAggregate patient(identity, bedLocation);
    
    // Set admission data
    QDateTime admittedAt = QDateTime::fromMSecsSinceEpoch(
        query.value(ADMITTED_AT).toLongLong()
    );
    patient.admit(query.value(ADMISSION_SOURCE).toString(), admittedAt);
    
    return patient;
}

void SQLitePatientRepository::bindAggregateToQuery(
    QSqlQuery& query, 
    const PatientAggregate& patient) {
    
    using namespace Schema::Columns::Patients;  // ✅ Use schema constants
    
    query.bindValue(":" + QString(MRN), patient.getIdentity().getMrn());
    query.bindValue(":" + QString(NAME), patient.getIdentity().getName());
    query.bindValue(":" + QString(DOB), patient.getIdentity().getDateOfBirth());
    query.bindValue(":" + QString(SEX), patient.getIdentity().getSex());
    query.bindValue(":" + QString(BED_LOCATION), patient.getBedLocation().getValue());
    query.bindValue(":" + QString(ADMITTED_AT), patient.getAdmittedAt().toMSecsSinceEpoch());
    query.bindValue(":" + QString(ADMISSION_SOURCE), patient.getAdmissionSource());
}
```

## Query Optimization

### Prepared Statements

**All queries use prepared statements via Query Registry (see DOC-COMP-032):**

```cpp
// ✅ GOOD: Prepared statement with Query Registry
QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Vitals::FIND_BY_TIME_RANGE);
query.bindValue(":patient_mrn", mrn);
query.bindValue(":start_time", startTime.toMSecsSinceEpoch());
query.bindValue(":end_time", endTime.toMSecsSinceEpoch());

// ❌ BAD: Dynamic SQL (SQL injection risk, no prepare optimization)
QString sql = QString("SELECT * FROM vitals WHERE patient_mrn = '%1'").arg(mrn);
QSqlQuery query = db.exec(sql);
```

### Batch Operations

**Use transactions for batch inserts:**

```cpp
bool VitalsRepository::saveBatch(const QList<VitalRecord>& vitals) {
    if (!m_dbManager->beginTransaction()) {
        return false;
    }
    
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Vitals::INSERT);
    
    for (const auto& vital : vitals) {
        bindVitalToQuery(query, vital);
        
        if (!query.exec()) {
            m_dbManager->rollback();
            m_logService->error("Batch insert failed", {
                {"error", query.lastError().text()}
            });
            return false;
        }
    }
    
    return m_dbManager->commit();
}
```

### Connection Pooling

**Separate read/write connections:**

```cpp
// Write operations (exclusive, single thread)
QSqlDatabase writeDb = m_dbManager->getWriteConnection();

// Read operations (shared, multiple threads)
QSqlDatabase readDb = m_dbManager->getReadConnection();
```

## Verification Guidelines

### Repository Testing

```cpp
// Test repository with mock database
TEST_F(PatientRepositoryTest, TestFindByMrn) {
    SQLitePatientRepository repo(&m_dbManager, m_logService);
    
    // Create test patient
    PatientIdentity identity("MRN-12345", "John Doe", QDate(1980, 1, 1), "M");
    BedLocation bed("ICU-04");
    PatientAggregate patient(identity, bed);
    
    // Save patient
    ASSERT_TRUE(repo.save(patient));
    
    // Find patient
    auto found = repo.findByMrn("MRN-12345");
    ASSERT_TRUE(found.has_value());
    ASSERT_EQ(found->getIdentity().getName(), "John Doe");
}
```

### Query Performance Testing

```cpp
// Benchmark query performance
TEST_F(VitalsRepositoryTest, BenchmarkTimeRangeQuery) {
    VitalsRepository repo(&m_dbManager, m_logService);
    
    // Insert 10k test records
    QList<VitalRecord> testData = generateTestVitals(10000);
    repo.saveBatch(testData);
    
    // Benchmark query
    auto start = std::chrono::high_resolution_clock::now();
    auto vitals = repo.findByTimeRange("MRN-12345", startTime, endTime);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start
    ).count();
    
    // Verify performance target (< 2 seconds)
    ASSERT_LT(duration, 2000);
}
```

## Document Metadata

**Original Document ID:** DESIGN-030  
**Migration Date:** 2025-12-01  
**New Document ID:** DOC-GUIDE-014  
**Implementation Status:** Hybrid approach (manual SQL with Query Registry ready, QxOrm integration planned)

## Revision History

| Version | Date       | Changes                                                                                                                                                                         |
| ------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1.0     | 2025-12-01 | Initial migration from DESIGN-030 to DOC-GUIDE-014. Complete database access strategy with repository pattern, hybrid ORM approach, query optimization, and schema integration. |
