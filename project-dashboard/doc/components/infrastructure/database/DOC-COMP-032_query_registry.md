---
doc_id: DOC-COMP-032
title: Query Registry Pattern Implementation
version: 1.0
category: Components
status: Approved
related_docs:
  - DOC-GUIDE-014 # Database Access Strategy
  - DOC-PROC-009  # Schema Management
  - DOC-ARCH-017  # Database Design
tags:
  - database
  - query
  - type-safety
  - constants
  - prepared-statements
source:
  original_id: DESIGN-032
  file: 32_QUERY_REGISTRY.md
  migrated_date: 2025-12-01
---

# Query Registry Pattern Implementation

## Purpose

The **Query Registry** pattern centralizes SQL query identifiers as compile-time constants, eliminating magic strings and providing type-safe, autocomplete-friendly query access throughout the application.

**Problem:** Magic strings for query IDs are error-prone, lack autocomplete, and break silently at runtime.  
**Solution:** Use compile-time `constexpr` string constants in a namespace hierarchy for type-safe query identification.

## Problem with Magic Strings

### Current Issues

```cpp
// ❌ ANTI-PATTERN: Magic strings everywhere
void PatientRepository::findByMrn(const QString& mrn) {
    // Typo risk: "find_patient_by_mrn" vs "findPatientByMrn"
    auto query = db->getPreparedQuery("find_patient_by_mrn");
    
    // No autocomplete, no compile-time checking
    query.bindValue(":mrn", mrn);
    query.exec();
}

void AlarmService::getActiveAlarms() {
    // Same query, different string? Who knows!
    auto query = db->getPreparedQuery("get_active_alarms");  // Typo!
}
```

**Problems:**
- ❌ No autocomplete (must remember exact string)
- ❌ Typos detected only at runtime
- ❌ Hard to refactor (global search required)
- ❌ No IDE support for "Find Usages"
- ❌ No compile-time validation

## Query Registry Solution

### QueryId Namespace

Centralized query identifiers with hierarchical namespace:

```cpp
// include/database/QueryId.h
namespace QueryId {

// Patient queries
namespace Patient {
    constexpr const char* FIND_BY_MRN = "patient.find_by_mrn";
    constexpr const char* FIND_ALL_ACTIVE = "patient.find_all_active";
    constexpr const char* FIND_BY_DEVICE = "patient.find_by_device";
    constexpr const char* INSERT = "patient.insert";
    constexpr const char* UPDATE = "patient.update";
    constexpr const char* DISCHARGE = "patient.discharge";
}

// Vitals queries
namespace Vitals {
    constexpr const char* INSERT = "vitals.insert";
    constexpr const char* FIND_LATEST_BY_PATIENT = "vitals.find_latest_by_patient";
    constexpr const char* FIND_RANGE = "vitals.find_range";
    constexpr const char* DELETE_OLDER_THAN = "vitals.delete_older_than";
}

// Alarm queries
namespace Alarm {
    constexpr const char* INSERT = "alarm.insert";
    constexpr const char* FIND_ACTIVE = "alarm.find_active";
    constexpr const char* UPDATE_STATUS = "alarm.update_status";
    constexpr const char* FIND_BY_PATIENT = "alarm.find_by_patient";
}

// Telemetry Metrics queries
namespace TelemetryMetrics {
    constexpr const char* INSERT = "telemetry_metrics.insert";
    constexpr const char* FIND_BY_BATCH_ID = "telemetry_metrics.find_by_batch_id";
    constexpr const char* CALCULATE_AVG_LATENCY = "telemetry_metrics.calculate_avg_latency";
}

// User & Security queries
namespace User {
    constexpr const char* FIND_BY_USER_ID = "user.find_by_user_id";
    constexpr const char* VALIDATE_CREDENTIALS = "user.validate_credentials";
    constexpr const char* UPDATE_LAST_LOGIN = "user.update_last_login";
}

namespace SecurityAudit {
    constexpr const char* INSERT = "security_audit.insert";
    constexpr const char* FIND_BY_USER = "security_audit.find_by_user";
    constexpr const char* FIND_RECENT = "security_audit.find_recent";
}

// Action Log queries
namespace ActionLog {
    constexpr const char* INSERT = "action_log.insert";
    constexpr const char* FIND_BY_PATIENT = "action_log.find_by_patient";
    constexpr const char* FIND_BY_USER = "action_log.find_by_user";
}

// Settings queries
namespace Settings {
    constexpr const char* FIND_BY_KEY = "settings.find_by_key";
    constexpr const char* UPSERT = "settings.upsert";
    constexpr const char* FIND_ALL = "settings.find_all";
}

} // namespace QueryId
```

### Usage with Type Safety

```cpp
// ✅ GOOD: Type-safe with autocomplete
#include "database/QueryId.h"

void PatientRepository::findByMrn(const QString& mrn) {
    // Autocomplete suggests: QueryId::Patient::FIND_BY_MRN
    auto query = db->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
    
    query.bindValue(":mrn", mrn);
    query.exec();
    
    // Compile-time constant, IDE can find all usages
}

void AlarmService::getActiveAlarms() {
    // Different namespace, clear intent
    auto query = db->getPreparedQuery(QueryId::Alarm::FIND_ACTIVE);
    query.exec();
}
```

**Benefits:**
- ✅ Autocomplete after typing `QueryId::`
- ✅ Compile-time checking (typos = compiler error)
- ✅ Easy refactoring (IDE "Rename Symbol")
- ✅ "Find Usages" works in IDE
- ✅ Self-documenting (`Patient::FIND_BY_MRN` is clear)

## Query Catalog
## Alarm Queries (SQLite)

This project defines structured Alarm query IDs under `QueryId::Alarms` and registers SQL via the `QueryRegistry`. These cover insert, retrieval, active filtering, history-by-patient, and status updates.

### Query IDs

```cpp
namespace QueryId {
namespace Alarms {
        constexpr const char* INSERT = "alarms.insert";
        constexpr const char* FIND_BY_ID = "alarms.find_by_id";
        constexpr const char* GET_ACTIVE = "alarms.get_active";
        constexpr const char* GET_HISTORY_BY_PATIENT = "alarms.get_history_by_patient";
        constexpr const char* UPDATE_STATUS = "alarms.update_status";
}
}
```

### Registered SQL Shapes

```sql
-- INSERT
INSERT INTO alarms (
    alarm_id,
    alarm_type,
    priority,
    status,
    raw_value,
    threshold_value,
    start_time,
    patient_mrn,
    acknowledged_by,
    acknowledged_time
) VALUES (
    :alarm_id,
    :alarm_type,
    :priority,
    :status,
    :raw_value,
    :threshold_value,
    :start_time,
    :patient_mrn,
    :acknowledged_by,
    :acknowledged_time
);

-- FIND_BY_ID
SELECT * FROM alarms WHERE alarm_id = :alarm_id;

-- GET_ACTIVE
SELECT * FROM alarms WHERE status = 'ACTIVE' ORDER BY start_time DESC;

-- GET_HISTORY_BY_PATIENT
SELECT * FROM alarms
WHERE patient_mrn = :patient_mrn
    AND start_time BETWEEN :start_time AND :end_time
ORDER BY start_time DESC;

-- UPDATE_STATUS
UPDATE alarms
SET status = :status,
        acknowledged_by = :acknowledged_by,
        acknowledged_time = :acknowledged_time
WHERE alarm_id = :alarm_id;
```

### Prepared Query Retrieval Guidance

- Use `QSqlQuery q = db->getPreparedQuery(QueryId::Alarms::INSERT);` and validate with `if (q.lastQuery().isEmpty())` before binding.
- `QSqlQuery::isValid()` indicates cursor state after execution; for prepared statements, prefer `lastQuery().isEmpty()` to detect missing registrations.
- Register only the queries needed for a given test/scope to avoid dependencies on unrelated tables.

### Test Fixture Notes

- For integration tests, ensure `alarms` table exists (manual DDL is acceptable when global DDL loaders skip tables).
- Clear `alarms` in `SetUp()`/`TearDown()` to avoid `UNIQUE` collisions on `alarm_id` between tests.
- Keep query registration scoped to the fixture; avoid global initialization that can fail if some tables are absent.


The **QueryCatalog** maps query IDs to actual SQL definitions and manages prepared statements:

```cpp
// include/database/QueryCatalog.h
class QueryCatalog {
public:
    QueryCatalog();
    
    // Register query definition
    void registerQuery(const char* queryId, const QString& sql);
    
    // Get SQL definition
    QString getQuerySql(const char* queryId) const;
    
    // Check if query exists
    bool hasQuery(const char* queryId) const;
    
    // Get all query IDs (for documentation)
    QStringList getAllQueryIds() const;

private:
    QHash<QString, QString> m_queries;
};
```

### QueryCatalog Implementation

```cpp
// src/database/QueryCatalog.cpp
QueryCatalog::QueryCatalog() {
    // Patient queries
    registerQuery(QueryId::Patient::FIND_BY_MRN, R"(
        SELECT patient_id, mrn, name, date_of_birth, sex, bed_location, 
               admission_source, admitted_at, discharged_at, device_label
        FROM patients
        WHERE mrn = :mrn
          AND discharged_at IS NULL
    )");
    
    registerQuery(QueryId::Patient::FIND_ALL_ACTIVE, R"(
        SELECT patient_id, mrn, name, date_of_birth, sex, bed_location,
               admission_source, admitted_at, device_label
        FROM patients
        WHERE discharged_at IS NULL
        ORDER BY admitted_at DESC
    )");
    
    registerQuery(QueryId::Patient::INSERT, R"(
        INSERT INTO patients (mrn, name, date_of_birth, sex, bed_location,
                              admission_source, admitted_at, device_label)
        VALUES (:mrn, :name, :date_of_birth, :sex, :bed_location,
                :admission_source, :admitted_at, :device_label)
    )");
    
    // Vitals queries
    registerQuery(QueryId::Vitals::INSERT, R"(
        INSERT INTO vitals (patient_mrn, device_label, parameter_type,
                            value, unit, timestamp_utc, quality_flag)
        VALUES (:patient_mrn, :device_label, :parameter_type,
                :value, :unit, :timestamp_utc, :quality_flag)
    )");
    
    registerQuery(QueryId::Vitals::FIND_LATEST_BY_PATIENT, R"(
        SELECT vital_id, patient_mrn, device_label, parameter_type,
               value, unit, timestamp_utc, quality_flag
        FROM vitals
        WHERE patient_mrn = :patient_mrn
          AND parameter_type = :parameter_type
        ORDER BY timestamp_utc DESC
        LIMIT 1
    )");
    
    // Alarm queries
    registerQuery(QueryId::Alarm::INSERT, R"(
        INSERT INTO alarms (patient_mrn, device_label, parameter_type,
                            alarm_type, severity, value, threshold_low,
                            threshold_high, triggered_at, acknowledged_at,
                            acknowledged_by)
        VALUES (:patient_mrn, :device_label, :parameter_type,
                :alarm_type, :severity, :value, :threshold_low,
                :threshold_high, :triggered_at, :acknowledged_at,
                :acknowledged_by)
    )");
    
    registerQuery(QueryId::Alarm::FIND_ACTIVE, R"(
        SELECT alarm_id, patient_mrn, device_label, parameter_type,
               alarm_type, severity, value, threshold_low, threshold_high,
               triggered_at
        FROM alarms
        WHERE acknowledged_at IS NULL
        ORDER BY severity DESC, triggered_at ASC
    )");
    
    // ... (more queries)
}

void QueryCatalog::registerQuery(const char* queryId, const QString& sql) {
    m_queries.insert(QString(queryId), sql.trimmed());
}

QString QueryCatalog::getQuerySql(const char* queryId) const {
    return m_queries.value(QString(queryId), QString());
}
```

## DatabaseManager Integration

The **DatabaseManager** uses the QueryCatalog to prepare and cache queries:

```cpp
// include/database/DatabaseManager.h
class DatabaseManager {
public:
    DatabaseManager(QObject* parent = nullptr);
    
    // Prepare query from QueryId constant
    QSqlQuery prepareQuery(const char* queryId);
    
    // Register all queries at startup
    void initializeQueries();

private:
    QueryCatalog m_catalog;
    QHash<QString, QSqlQuery> m_preparedQueries;
};
```

### Implementation

```cpp
// src/database/DatabaseManager.cpp
void DatabaseManager::initializeQueries() {
    // QueryCatalog constructor already registered all queries
    
    // Pre-prepare frequently used queries
    prepareQuery(QueryId::Patient::FIND_BY_MRN);
    prepareQuery(QueryId::Vitals::INSERT);
    prepareQuery(QueryId::Alarm::INSERT);
}

QSqlQuery DatabaseManager::prepareQuery(const char* queryId) {
    QString id = QString(queryId);
    
    // Check cache first
    if (m_preparedQueries.contains(id)) {
        return m_preparedQueries.value(id);
    }
    
    // Get SQL from catalog
    QString sql = m_catalog.getQuerySql(queryId);
    if (sql.isEmpty()) {
        qWarning() << "Unknown query ID:" << queryId;
        return QSqlQuery();  // Invalid query
    }
    
    // Prepare query
    QSqlQuery query(m_database);
    if (!query.prepare(sql)) {
        qWarning() << "Failed to prepare query:" << queryId 
                   << "Error:" << query.lastError().text();
        return QSqlQuery();
    }
    
    // Cache prepared query
    m_preparedQueries.insert(id, query);
    
    return query;
}
```

## Repository Example with Query Registry

```cpp
// include/database/SQLitePatientRepository.h
class SQLitePatientRepository : public IPatientRepository {
public:
    explicit SQLitePatientRepository(DatabaseManager* dbManager);
    
    std::optional<Patient> findByMrn(const QString& mrn) override;
    QList<Patient> findAllActive() override;
    bool save(const Patient& patient) override;
    bool discharge(const QString& mrn, const QDateTime& dischargedAt) override;

private:
    DatabaseManager* m_dbManager;
};
```

### Implementation

```cpp
// src/database/SQLitePatientRepository.cpp
std::optional<Patient> SQLitePatientRepository::findByMrn(const QString& mrn) {
    // ✅ Type-safe query ID with autocomplete
    auto query = m_dbManager->prepareQuery(QueryId::Patient::FIND_BY_MRN);
    
    query.bindValue(":mrn", mrn);
    
    if (!query.exec()) {
        qWarning() << "Query failed:" << query.lastError().text();
        return std::nullopt;
    }
    
    if (!query.next()) {
        return std::nullopt;  // Not found
    }
    
    // Map SQL result to domain object
    Patient patient;
    patient.setMrn(query.value("mrn").toString());
    patient.setName(query.value("name").toString());
    patient.setBedLocation(query.value("bed_location").toString());
    // ... more mapping
    
    return patient;
}

bool SQLitePatientRepository::save(const Patient& patient) {
    // ✅ Clear intent, autocomplete suggests correct query
    auto query = m_dbManager->prepareQuery(QueryId::Patient::INSERT);
    
    query.bindValue(":mrn", patient.getMrn());
    query.bindValue(":name", patient.getName());
    query.bindValue(":date_of_birth", patient.getDateOfBirth());
    query.bindValue(":sex", patient.getSex());
    query.bindValue(":bed_location", patient.getBedLocation());
    query.bindValue(":admission_source", patient.getAdmissionSource());
    query.bindValue(":admitted_at", patient.getAdmittedAt());
    query.bindValue(":device_label", patient.getDeviceLabel());
    
    if (!query.exec()) {
        qWarning() << "Insert failed:" << query.lastError().text();
        return false;
    }
    
    return true;
}
```

## Query Naming Conventions

Use the format: `namespace.action_object`

| Pattern           | Example                                   | When to Use                            |
| ----------------- | ----------------------------------------- | -------------------------------------- |
| **`find_by_*`**   | `patient.find_by_mrn`                     | Retrieve entity by specific field      |
| **`find_all_*`**  | `patient.find_all_active`                 | Retrieve multiple entities with filter |
| **`insert`**      | `vitals.insert`                           | Insert single record                   |
| **`update_*`**    | `alarm.update_status`                     | Update specific field(s)               |
| **`delete_*`**    | `vitals.delete_older_than`                | Delete with filter                     |
| **`upsert`**      | `settings.upsert`                         | Insert or update (if exists)           |
| **`calculate_*`** | `telemetry_metrics.calculate_avg_latency` | Aggregation/calculation                |

## Auto-Documentation Generation

Generate documentation from QueryCatalog:

```cpp
// tools/generate-query-docs.cpp
void QueryCatalog::generateDocumentation(const QString& outputPath) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream out(&file);
    
    out << "# Query Registry Documentation\n\n";
    out << "Auto-generated from QueryCatalog\n\n";
    
    // Group by namespace
    QHash<QString, QStringList> groups;
    for (const QString& queryId : getAllQueryIds()) {
        QString ns = queryId.split('.').first();
        groups[ns].append(queryId);
    }
    
    // Generate markdown
    for (const QString& ns : groups.keys()) {
        out << "## " << ns.toUpper() << " Queries\n\n";
        
        for (const QString& queryId : groups[ns]) {
            out << "### `" << queryId << "`\n\n";
            out << "```sql\n";
            out << getQuerySql(queryId.toUtf8().constData());
            out << "\n```\n\n";
        }
    }
}
```

**Generated output:** `doc/QUERY_REGISTRY.md`

## Testing Strategy

### Test: Verify All Queries Compile

```cpp
// tests/database/QueryRegistryTest.cpp
TEST_F(QueryRegistryTest, AllQueriesHaveValidSql) {
    QueryCatalog catalog;
    
    // Verify patient queries
    EXPECT_FALSE(catalog.getQuerySql(QueryId::Patient::FIND_BY_MRN).isEmpty());
    EXPECT_FALSE(catalog.getQuerySql(QueryId::Patient::INSERT).isEmpty());
    
    // Verify vitals queries
    EXPECT_FALSE(catalog.getQuerySql(QueryId::Vitals::INSERT).isEmpty());
    
    // ... more verifications
}

TEST_F(QueryRegistryTest, PreparedStatementsBindCorrectly) {
    DatabaseManager dbManager;
    
    // Test patient query
    auto query = dbManager.prepareQuery(QueryId::Patient::FIND_BY_MRN);
    EXPECT_TRUE(query.isValid());
    
    // Bind value and execute
    query.bindValue(":mrn", "TEST123");
    EXPECT_TRUE(query.exec());
}
```

## Alternative Approaches Comparison

| Approach                | Type Safety | Autocomplete | Refactoring | Performance | Verdict           |
| ----------------------- | ----------- | ------------ | ----------- | ----------- | ----------------- |
| **Magic Strings**       | ❌           | ❌            | ❌           | ✅           | ❌ Avoid           |
| **Enum Class**          | ⚠️           | ⚠️            | ✅           | ✅           | ⚠️ Limited         |
| **`constexpr` Strings** | ✅           | ✅            | ✅           | ✅           | ✅ **Recommended** |

### Why Not Enum Class?

```cpp
// ❌ Enum class is verbose and not human-readable
enum class QueryId {
    PATIENT_FIND_BY_MRN,
    VITALS_INSERT,
    ALARM_FIND_ACTIVE
};

// Need manual toString() conversion
QString toString(QueryId id) {
    switch (id) {
        case QueryId::PATIENT_FIND_BY_MRN: return "patient.find_by_mrn";
        // ... many more cases
    }
}

// Usage is more verbose
auto query = db->prepareQuery(toString(QueryId::PATIENT_FIND_BY_MRN));
```

**vs. constexpr (recommended):**

```cpp
// ✅ Direct string constant, no conversion needed
auto query = db->prepareQuery(QueryId::Patient::FIND_BY_MRN);
```

## Maintenance Workflow

### Adding a New Query

1. **Add constant to QueryId.h:**
   ```cpp
   namespace QueryId::Patient {
       constexpr const char* FIND_BY_BED_LOCATION = "patient.find_by_bed_location";
   }
   ```

2. **Register SQL in QueryCatalog constructor:**
   ```cpp
   registerQuery(QueryId::Patient::FIND_BY_BED_LOCATION, R"(
       SELECT * FROM patients WHERE bed_location = :bed_location
   )");
   ```

3. **Use in repository:**
   ```cpp
   auto query = db->prepareQuery(QueryId::Patient::FIND_BY_BED_LOCATION);
   ```

4. **Regenerate documentation:**
   ```bash
   ./tools/generate-query-docs
   ```

### Removing a Query

1. **Find all usages** (IDE "Find Usages" on constant)
2. **Remove from QueryId.h**
3. **Remove from QueryCatalog constructor**
4. **Verify build** (any missed usage = compile error)

## Verification Guidelines

### Query Registry Verification

1. **Type Safety:** Verify all query usages use `QueryId::*` constants (no magic strings)
2. **Coverage:** Verify all SQL queries in codebase are registered in QueryCatalog
3. **Naming:** Verify query IDs follow `namespace.action_object` convention
4. **Documentation:** Verify auto-generated query docs are up-to-date

### Build-Time Checks

1. **Compile Test:** Verify all `QueryId::*` constants compile without warnings
2. **Prepare Test:** Verify all registered queries can be prepared without errors
3. **Binding Test:** Verify placeholder names match between SQL and bind calls

## Document Metadata

**Original Document ID:** DESIGN-032  
**Migration Date:** 2025-12-01  
**New Document ID:** DOC-COMP-032

## Revision History

| Version | Date       | Changes                                                                                                                                                                 |
| ------- | ---------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1.0     | 2025-12-01 | Initial migration from DESIGN-032 to DOC-COMP-032. Complete Query Registry pattern with QueryId constants, QueryCatalog, DatabaseManager integration, testing strategy. |
