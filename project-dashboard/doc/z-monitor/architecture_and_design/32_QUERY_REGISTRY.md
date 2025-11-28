# Database Query Registry and Management

**Document ID:** DESIGN-032  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document defines the strategy for managing, documenting, and organizing prepared SQL query strings in the Z Monitor application.

---

## 1. The Problem

**Anti-Pattern: Magic Strings Everywhere**

```cpp
// ❌ BAD: Magic strings scattered across codebase
QSqlQuery query = m_dbManager->getPreparedQuery("find_patient_by_mrn");
QSqlQuery query2 = m_dbManager->getPreparedQuery("find_patint_by_mrn");  // Typo!
QSqlQuery query3 = m_dbManager->getPreparedQuery("findPatientByMrn");    // Inconsistent naming!
```

**Problems:**
- ❌ No autocomplete (easy to make typos)
- ❌ No compile-time checking (typos discovered at runtime)
- ❌ Hard to find all usages (grep for string, not symbol)
- ❌ Hard to document (where is the list of all queries?)
- ❌ Inconsistent naming (camelCase vs. snake_case)
- ❌ Refactoring nightmare (renaming a query = find/replace strings)

---

## 2. Recommended Solution: Query Registry Pattern

### **Strategy:**

1. **Centralized Query IDs** (compile-time constants)
2. **Query Catalog** (single source of truth for all SQL)
3. **Type-safe API** (no string literals in calling code)
4. **Auto-documentation** (generated from catalog)

---

## 3. Implementation: Query Registry

### 3.1 Query ID Constants (Header)

**File:** `z-monitor/src/infrastructure/persistence/QueryRegistry.h`

```cpp
/**
 * @file QueryRegistry.h
 * @brief Centralized registry of all database query IDs and SQL statements.
 * 
 * This file is the single source of truth for all prepared SQL queries
 * in the Z Monitor application. Query IDs are compile-time constants
 * that provide type safety and enable autocomplete.
 * 
 * @note When adding a new query:
 *       1. Add constant to QueryId namespace
 *       2. Add SQL to QueryCatalog::initializeQueries()
 *       3. Add documentation to QueryCatalog section
 */

#ifndef QUERYREGISTRY_H
#define QUERYREGISTRY_H

#include <QString>
#include <QMap>

/**
 * @namespace QueryId
 * @brief Compile-time constants for all database query IDs.
 * 
 * Use these constants instead of string literals when requesting
 * prepared queries from DatabaseManager.
 * 
 * Example:
 * @code
 * QSqlQuery query = dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
 * @endcode
 */
namespace QueryId {
    
    /**
     * @namespace Patient
     * @brief Query IDs for patient-related operations.
     */
    namespace Patient {
        constexpr const char* FIND_BY_MRN = "patient.find_by_mrn";
        constexpr const char* FIND_ALL = "patient.find_all";
        constexpr const char* INSERT = "patient.insert";
        constexpr const char* UPDATE = "patient.update";
        constexpr const char* DELETE = "patient.delete";
        constexpr const char* FIND_ADMITTED = "patient.find_admitted";
        constexpr const char* GET_ADMISSION_HISTORY = "patient.get_admission_history";
    }
    
    /**
     * @namespace Vitals
     * @brief Query IDs for vitals time-series operations.
     */
    namespace Vitals {
        constexpr const char* INSERT = "vitals.insert";
        constexpr const char* INSERT_BATCH = "vitals.insert_batch";
        constexpr const char* FIND_BY_PATIENT = "vitals.find_by_patient";
        constexpr const char* FIND_BY_TIME_RANGE = "vitals.find_by_time_range";
        constexpr const char* DELETE_OLD = "vitals.delete_old";
        constexpr const char* COUNT_UNSYNCED = "vitals.count_unsynced";
        constexpr const char* GET_UNSYNCED = "vitals.get_unsynced";
        constexpr const char* MARK_SYNCED = "vitals.mark_synced";
    }
    
    /**
     * @namespace Alarms
     * @brief Query IDs for alarm operations.
     */
    namespace Alarms {
        constexpr const char* INSERT = "alarms.insert";
        constexpr const char* UPDATE_STATUS = "alarms.update_status";
        constexpr const char* FIND_ACTIVE = "alarms.find_active";
        constexpr const char* FIND_BY_PATIENT = "alarms.find_by_patient";
        constexpr const char* ACKNOWLEDGE = "alarms.acknowledge";
        constexpr const char* SILENCE = "alarms.silence";
        constexpr const char* GET_HISTORY = "alarms.get_history";
    }
    
    /**
     * @namespace Telemetry
     * @brief Query IDs for telemetry metrics operations.
     */
    namespace Telemetry {
        constexpr const char* INSERT_METRICS = "telemetry.insert_metrics";
        constexpr const char* UPDATE_METRICS = "telemetry.update_metrics";
        constexpr const char* GET_P95_LATENCY = "telemetry.get_p95_latency";
        constexpr const char* GET_SLOW_BATCHES = "telemetry.get_slow_batches";
        constexpr const char* GET_HOURLY_TRENDS = "telemetry.get_hourly_trends";
        constexpr const char* GET_LATENCY_BREAKDOWN = "telemetry.get_latency_breakdown";
    }
    
    /**
     * @namespace Provisioning
     * @brief Query IDs for provisioning operations.
     */
    namespace Provisioning {
        constexpr const char* INSERT_EVENT = "provisioning.insert_event";
        constexpr const char* FIND_BY_DEVICE = "provisioning.find_by_device";
        constexpr const char* GET_HISTORY = "provisioning.get_history";
    }
    
    /**
     * @namespace Security
     * @brief Query IDs for security and authentication operations.
     */
    namespace Security {
        constexpr const char* FIND_USER = "security.find_user";
        constexpr const char* UPDATE_USER = "security.update_user";
        constexpr const char* INSERT_AUDIT_LOG = "security.insert_audit_log";
        constexpr const char* GET_AUDIT_LOGS = "security.get_audit_logs";
        constexpr const char* UPDATE_FAILED_ATTEMPTS = "security.update_failed_attempts";
        constexpr const char* GET_LOCKOUT_STATUS = "security.get_lockout_status";
    }
    
    /**
     * @namespace Certificates
     * @brief Query IDs for certificate management operations.
     */
    namespace Certificates {
        constexpr const char* INSERT = "certificates.insert";
        constexpr const char* FIND_BY_SERIAL = "certificates.find_by_serial";
        constexpr const char* UPDATE_STATUS = "certificates.update_status";
        constexpr const char* GET_EXPIRING = "certificates.get_expiring";
        constexpr const char* MARK_REVOKED = "certificates.mark_revoked";
    }
    
    /**
     * @namespace Settings
     * @brief Query IDs for settings operations.
     */
    namespace Settings {
        constexpr const char* GET_VALUE = "settings.get_value";
        constexpr const char* SET_VALUE = "settings.set_value";
        constexpr const char* GET_ALL = "settings.get_all";
        constexpr const char* DELETE = "settings.delete";
    }
    
    /**
     * @namespace Admission
     * @brief Query IDs for admission/discharge/transfer operations.
     */
    namespace Admission {
        constexpr const char* INSERT_EVENT = "admission.insert_event";
        constexpr const char* GET_CURRENT = "admission.get_current";
        constexpr const char* GET_HISTORY = "admission.get_history";
        constexpr const char* GET_BY_DEVICE = "admission.get_by_device";
    }
}

/**
 * @class QueryCatalog
 * @brief Registry of all SQL query statements and their metadata.
 * 
 * This class maintains the mapping between query IDs and their SQL
 * statements, parameters, and documentation.
 */
class QueryCatalog {
public:
    /**
     * @brief Structure holding query metadata.
     */
    struct QueryDefinition {
        QString id;                     ///< Query ID (from QueryId namespace)
        QString sql;                    ///< SQL statement with named parameters
        QString description;            ///< Human-readable description
        QStringList parameters;         ///< List of parameter names (for documentation)
        QString exampleUsage;           ///< Code example (optional)
        bool isReadOnly;                ///< true if SELECT, false if INSERT/UPDATE/DELETE
    };
    
    /**
     * @brief Get all registered queries.
     * @return Map of query ID to query definition
     */
    static QMap<QString, QueryDefinition> getAllQueries();
    
    /**
     * @brief Get query definition by ID.
     * @param queryId Query ID constant
     * @return Query definition or empty if not found
     */
    static QueryDefinition getQuery(const QString& queryId);
    
    /**
     * @brief Initialize all queries in DatabaseManager.
     * @param dbManager DatabaseManager instance to populate
     */
    static void initializeQueries(DatabaseManager* dbManager);
    
    /**
     * @brief Generate documentation for all queries.
     * @return Markdown documentation of all queries
     */
    static QString generateDocumentation();
};

#endif // QUERYREGISTRY_H
```

### 3.2 Query Catalog Implementation

**File:** `z-monitor/src/infrastructure/persistence/QueryCatalog.cpp`

```cpp
#include "QueryRegistry.h"
#include "DatabaseManager.h"

QMap<QString, QueryCatalog::QueryDefinition> QueryCatalog::getAllQueries() {
    static QMap<QString, QueryDefinition> queries;
    
    if (queries.isEmpty()) {
        // ═══════════════════════════════════════════════════════════
        // PATIENT QUERIES
        // ═══════════════════════════════════════════════════════════
        queries[QueryId::Patient::FIND_BY_MRN] = {
            .id = QueryId::Patient::FIND_BY_MRN,
            .sql = R"(
                SELECT mrn, name, dob, sex, bed_location, admitted_at, admission_source
                FROM patients
                WHERE mrn = :mrn
            )",
            .description = "Find patient by Medical Record Number",
            .parameters = {":mrn"},
            .exampleUsage = R"(
                query.bindValue(":mrn", "MRN-12345");
                query.exec();
            )",
            .isReadOnly = true
        };
        
        queries[QueryId::Patient::INSERT] = {
            .id = QueryId::Patient::INSERT,
            .sql = R"(
                INSERT INTO patients (mrn, name, dob, sex, bed_location, admitted_at, admission_source)
                VALUES (:mrn, :name, :dob, :sex, :bed_location, :admitted_at, :admission_source)
                ON CONFLICT(mrn) DO UPDATE SET
                    name = :name,
                    dob = :dob,
                    sex = :sex,
                    bed_location = :bed_location,
                    admitted_at = :admitted_at,
                    admission_source = :admission_source
            )",
            .description = "Insert or update patient record (UPSERT)",
            .parameters = {":mrn", ":name", ":dob", ":sex", ":bed_location", ":admitted_at", ":admission_source"},
            .isReadOnly = false
        };
        
        queries[QueryId::Patient::FIND_ADMITTED] = {
            .id = QueryId::Patient::FIND_ADMITTED,
            .sql = R"(
                SELECT mrn, name, bed_location, admitted_at, admission_source
                FROM patients
                WHERE admitted_at IS NOT NULL
                  AND discharged_at IS NULL
                ORDER BY admitted_at DESC
            )",
            .description = "Find all currently admitted patients",
            .parameters = {},
            .isReadOnly = true
        };
        
        // ═══════════════════════════════════════════════════════════
        // VITALS QUERIES
        // ═══════════════════════════════════════════════════════════
        queries[QueryId::Vitals::INSERT] = {
            .id = QueryId::Vitals::INSERT,
            .sql = R"(
                INSERT INTO vitals (timestamp, patient_mrn, heart_rate, spo2, respiration_rate, batch_id)
                VALUES (:timestamp, :patient_mrn, :heart_rate, :spo2, :respiration_rate, :batch_id)
            )",
            .description = "Insert single vital sign record",
            .parameters = {":timestamp", ":patient_mrn", ":heart_rate", ":spo2", ":respiration_rate", ":batch_id"},
            .isReadOnly = false
        };
        
        queries[QueryId::Vitals::FIND_BY_TIME_RANGE] = {
            .id = QueryId::Vitals::FIND_BY_TIME_RANGE,
            .sql = R"(
                SELECT id, timestamp, heart_rate, spo2, respiration_rate
                FROM vitals
                WHERE patient_mrn = :patient_mrn
                  AND timestamp BETWEEN :start_time AND :end_time
                ORDER BY timestamp ASC
            )",
            .description = "Find vitals for patient within time range (for trends)",
            .parameters = {":patient_mrn", ":start_time", ":end_time"},
            .isReadOnly = true
        };
        
        queries[QueryId::Vitals::GET_UNSYNCED] = {
            .id = QueryId::Vitals::GET_UNSYNCED,
            .sql = R"(
                SELECT id, timestamp, patient_mrn, heart_rate, spo2, respiration_rate
                FROM vitals
                WHERE is_synced = 0
                ORDER BY timestamp ASC
                LIMIT :limit
            )",
            .description = "Get unsynced vitals for batch transmission",
            .parameters = {":limit"},
            .isReadOnly = true
        };
        
        queries[QueryId::Vitals::MARK_SYNCED] = {
            .id = QueryId::Vitals::MARK_SYNCED,
            .sql = R"(
                UPDATE vitals
                SET is_synced = 1, batch_id = :batch_id
                WHERE id IN (:id_list)
            )",
            .description = "Mark vitals as synced after successful transmission",
            .parameters = {":batch_id", ":id_list"},
            .isReadOnly = false
        };
        
        // ═══════════════════════════════════════════════════════════
        // TELEMETRY METRICS QUERIES
        // ═══════════════════════════════════════════════════════════
        queries[QueryId::Telemetry::INSERT_METRICS] = {
            .id = QueryId::Telemetry::INSERT_METRICS,
            .sql = R"(
                INSERT INTO telemetry_metrics (
                    batch_id, device_id, device_label, patient_mrn,
                    data_created_at, batch_created_at, signed_at,
                    queued_for_tx_at, transmitted_at, server_received_at,
                    server_processed_at, server_ack_at,
                    record_count, batch_size_bytes, status, created_at
                )
                VALUES (
                    :batch_id, :device_id, :device_label, :patient_mrn,
                    :data_created_at, :batch_created_at, :signed_at,
                    :queued_for_tx_at, :transmitted_at, :server_received_at,
                    :server_processed_at, :server_ack_at,
                    :record_count, :batch_size_bytes, :status, :created_at
                )
            )",
            .description = "Insert telemetry timing metrics for benchmarking",
            .parameters = {
                ":batch_id", ":device_id", ":device_label", ":patient_mrn",
                ":data_created_at", ":batch_created_at", ":signed_at",
                ":queued_for_tx_at", ":transmitted_at", ":server_received_at",
                ":server_processed_at", ":server_ack_at",
                ":record_count", ":batch_size_bytes", ":status", ":created_at"
            },
            .isReadOnly = false
        };
        
        queries[QueryId::Telemetry::GET_P95_LATENCY] = {
            .id = QueryId::Telemetry::GET_P95_LATENCY,
            .sql = R"(
                SELECT 
                    PERCENTILE_CONT(0.95) WITHIN GROUP (ORDER BY end_to_end_latency_ms) as p95_latency_ms,
                    AVG(end_to_end_latency_ms) as avg_latency_ms,
                    COUNT(*) as batch_count
                FROM telemetry_metrics
                WHERE created_at > :since_timestamp
                  AND status = 'success'
            )",
            .description = "Calculate P95 and average latency for performance monitoring",
            .parameters = {":since_timestamp"},
            .isReadOnly = true
        };
        
        // Add more queries...
    }
    
    return queries;
}

void QueryCatalog::initializeQueries(DatabaseManager* dbManager) {
    auto queries = getAllQueries();
    
    for (const auto& def : queries) {
        dbManager->registerPreparedQuery(def.id, def.sql);
    }
    
    // Note: LogService should be injected into DatabaseManager
    // m_logService->info("Initialized prepared queries", {
    //     {"count", QString::number(queries.size())}
    // });
}

QString QueryCatalog::generateDocumentation() {
    QString doc;
    doc += "# Database Query Reference\n\n";
    doc += "This document is auto-generated from QueryRegistry.h\n\n";
    
    auto queries = getAllQueries();
    
    // Group by namespace
    QMap<QString, QList<QueryDefinition>> grouped;
    for (const auto& def : queries) {
        QString ns = def.id.split('.').first();
        grouped[ns].append(def);
    }
    
    for (auto it = grouped.begin(); it != grouped.end(); ++it) {
        doc += QString("## %1 Queries\n\n").arg(it.key());
        
        for (const auto& def : it.value()) {
            doc += QString("### %1\n\n").arg(def.id);
            doc += QString("**Description:** %1\n\n").arg(def.description);
            doc += QString("**Parameters:** %1\n\n").arg(def.parameters.join(", "));
            doc += QString("**SQL:**\n```sql\n%1\n```\n\n").arg(def.sql.trimmed());
            
            if (!def.exampleUsage.isEmpty()) {
                doc += QString("**Example:**\n```cpp\n%1\n```\n\n").arg(def.exampleUsage.trimmed());
            }
        }
    }
    
    return doc;
}
```

---

## 4. Usage Examples

### 4.1 Repository Using Query IDs

**Before (Magic Strings):**
```cpp
// ❌ BAD: Magic strings
QSqlQuery query = m_dbManager->getPreparedQuery("find_patient_by_mrn");
```

**After (Type-Safe Constants):**
```cpp
// ✅ GOOD: Type-safe constants with autocomplete
#include "QueryRegistry.h"

QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
```

### 4.2 Complete Repository Example

```cpp
// SQLitePatientRepository.cpp
#include "QueryRegistry.h"

std::optional<PatientAggregate> SQLitePatientRepository::findByMrn(const QString& mrn) {
    // Type-safe query ID (autocomplete works!)
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
    
    query.bindValue(":mrn", mrn);
    
    if (!query.exec()) {
        m_logService->warning("Query failed", {
            {"queryId", QueryId::Patient::FIND_BY_MRN},
            {"mrn", mrn},
            {"error", query.lastError().text()}
        });
        return std::nullopt;
    }
    
    if (!query.next()) {
        return std::nullopt;
    }
    
    return mapToAggregate(query);
}

bool SQLitePatientRepository::save(const PatientAggregate& patient) {
    // Type-safe query ID
    QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::INSERT);
    
    // Bind parameters
    const auto& identity = patient.getIdentity();
    query.bindValue(":mrn", identity.getMrn());
    query.bindValue(":name", identity.getName());
    query.bindValue(":dob", identity.getDateOfBirth());
    query.bindValue(":sex", identity.getSex());
    query.bindValue(":bed_location", patient.getBedLocation().toString());
    query.bindValue(":admitted_at", patient.getAdmittedAt());
    query.bindValue(":admission_source", patient.getAdmissionSource());
    
    if (!query.exec()) {
        m_logService->warning("Save failed", {
            {"queryId", QueryId::Patient::INSERT},
            {"mrn", identity.getMrn()},
            {"error", query.lastError().text()}
        });
        return false;
    }
    
    return true;
}
```

---

## 5. Benefits of Query Registry Pattern

### **1. Type Safety** ✅
```cpp
// Autocomplete works!
QueryId::Patient::FIND_BY_MRN  // IDE suggests all Patient queries
QueryId::Vitals::INSERT        // IDE suggests all Vitals queries

// Typos caught at compile time
QueryId::Patient::FIND_BY_MNR  // ❌ Compile error!
```

### **2. Easy to Find** ✅
```cpp
// Find all usages: Ctrl+Click on constant
QueryId::Patient::FIND_BY_MRN

// Or grep for symbol (not string)
grep -r "QueryId::Patient::FIND_BY_MRN" src/
```

### **3. Refactoring Support** ✅
```cpp
// Rename query: Use IDE refactor (renames everywhere)
QueryId::Patient::FIND_BY_MRN  →  QueryId::Patient::GET_BY_MRN
// All usages updated automatically!
```

### **4. Self-Documenting** ✅
```cpp
// All queries documented in one place (QueryRegistry.h)
// Auto-generate query reference documentation
QString doc = QueryCatalog::generateDocumentation();
```

### **5. Consistent Naming** ✅
```cpp
// Enforced naming convention: namespace.action_object
QueryId::Patient::FIND_BY_MRN     // ✅ Consistent
QueryId::Vitals::INSERT           // ✅ Consistent
QueryId::Alarms::UPDATE_STATUS    // ✅ Consistent
```

---

## 6. Naming Conventions

### **Format:** `namespace.action_object`

| Pattern | Example | Description |
|---------|---------|-------------|
| `namespace.find_*` | `patient.find_by_mrn` | SELECT single record |
| `namespace.find_all` | `patient.find_all` | SELECT all records |
| `namespace.get_*` | `vitals.get_unsynced` | SELECT with filter/condition |
| `namespace.insert` | `patient.insert` | INSERT single record |
| `namespace.insert_batch` | `vitals.insert_batch` | INSERT multiple records |
| `namespace.update_*` | `alarms.update_status` | UPDATE specific field(s) |
| `namespace.delete` | `patient.delete` | DELETE record |
| `namespace.delete_*` | `vitals.delete_old` | DELETE with condition |
| `namespace.count_*` | `vitals.count_unsynced` | COUNT query |

### **Namespace Organization:**

```cpp
QueryId::
├── Patient::      // patients table
├── Vitals::       // vitals table
├── Alarms::       // alarms table
├── Telemetry::    // telemetry_metrics table
├── Provisioning:: // provisioning-related tables
├── Security::     // users, security_audit_log tables
├── Certificates:: // certificates table
├── Settings::     // settings table
└── Admission::    // admission_events table
```

---

## 7. DatabaseManager Integration

### 7.1 Enhanced DatabaseManager

```cpp
// DatabaseManager.h
class DatabaseManager : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Register a prepared query.
     * @param queryId Unique query identifier (use QueryId constants)
     * @param sql SQL statement with named parameters
     */
    void registerPreparedQuery(const QString& queryId, const QString& sql);
    
    /**
     * @brief Get a prepared query by ID.
     * @param queryId Query ID constant (from QueryId namespace)
     * @return Prepared QSqlQuery ready for parameter binding
     * 
     * Example:
     * @code
     * QSqlQuery query = dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
     * query.bindValue(":mrn", "MRN-12345");
     * query.exec();
     * @endcode
     */
    QSqlQuery getPreparedQuery(const QString& queryId);
    
    /**
     * @brief Check if query is registered.
     * @param queryId Query ID to check
     * @return true if registered, false otherwise
     */
    bool hasQuery(const QString& queryId) const;
    
    /**
     * @brief Get all registered query IDs.
     * @return List of all query IDs
     */
    QStringList getRegisteredQueries() const;
    
private:
    QMap<QString, QSqlQuery> m_preparedQueries;
};

// DatabaseManager.cpp
void DatabaseManager::registerPreparedQuery(const QString& queryId, const QString& sql) {
    QSqlQuery query(m_writeDb);
    
    if (!query.prepare(sql)) {
        m_logService->critical("Failed to prepare query", {
            {"queryId", queryId},
            {"error", query.lastError().text()},
            {"sql", sql}
        });
        return;
    }
    
    m_preparedQueries[queryId] = query;
    m_logService->debug("Registered prepared query", {
        {"queryId", queryId}
    });
}

QSqlQuery DatabaseManager::getPreparedQuery(const QString& queryId) {
    if (!m_preparedQueries.contains(queryId)) {
        m_logService->critical("Query not found", {
            {"queryId", queryId},
            {"availableQueries", m_preparedQueries.keys().join(", ")}
        });
        
        // Return invalid query
        return QSqlQuery();
    }
    
    // Clone the prepared query (resets bindings)
    QSqlQuery query(m_preparedQueries[queryId]);
    return query;
}
```

### 7.2 Initialization

```cpp
// Application startup
void initializeDatabase() {
    m_dbManager = new DatabaseManager(this);
    m_dbManager->open("/data/zmonitor.db", encryptionKey);
    
    // Initialize all queries from catalog
    QueryCatalog::initializeQueries(m_dbManager);
    
    // Now repositories can use QueryId constants
}
```

---

## 8. Auto-Generated Documentation

### 8.1 Generate Query Reference

```cpp
// Build-time or on-demand generation
int main() {
    QString doc = QueryCatalog::generateDocumentation();
    
    QFile file("doc/QUERY_REFERENCE.md");
    file.open(QIODevice::WriteOnly);
    file.write(doc.toUtf8());
    file.close();
    
    m_logService->info("Generated query documentation", {
        {"queryCount", QString::number(queries.size())}
    });
}
```

### 8.2 Generated Output Example

**File:** `doc/QUERY_REFERENCE.md` (auto-generated)

```markdown
# Database Query Reference

This document is auto-generated from QueryRegistry.h

## patient Queries

### patient.find_by_mrn

**Description:** Find patient by Medical Record Number

**Parameters:** :mrn

**SQL:**
```sql
SELECT mrn, name, dob, sex, bed_location, admitted_at, admission_source
FROM patients
WHERE mrn = :mrn
```

**Example:**
```cpp
query.bindValue(":mrn", "MRN-12345");
query.exec();
```

### patient.insert

**Description:** Insert or update patient record (UPSERT)

**Parameters:** :mrn, :name, :dob, :sex, :bed_location, :admitted_at, :admission_source

**SQL:**
```sql
INSERT INTO patients (...)
VALUES (...)
ON CONFLICT(mrn) DO UPDATE SET ...
```

## vitals Queries

### vitals.insert

**Description:** Insert single vital sign record

...
```

---

## 9. Testing Strategy

### 9.1 Verify All Queries Compile

```cpp
// tests/unit/QueryRegistryTest.cpp
TEST(QueryRegistryTest, AllQueriesCompile) {
    DatabaseManager dbManager;
    dbManager.open(":memory:", "test-key");
    
    // Initialize catalog
    QueryCatalog::initializeQueries(&dbManager);
    
    // Verify all queries were registered
    auto queries = QueryCatalog::getAllQueries();
    for (const auto& def : queries) {
        EXPECT_TRUE(dbManager.hasQuery(def.id)) 
            << "Query not registered: " << def.id.toStdString();
    }
}

TEST(QueryRegistryTest, AllQueryIDsUnique) {
    auto queries = QueryCatalog::getAllQueries();
    QSet<QString> ids;
    
    for (const auto& def : queries) {
        EXPECT_FALSE(ids.contains(def.id)) 
            << "Duplicate query ID: " << def.id.toStdString();
        ids.insert(def.id);
    }
}
```

### 9.2 Verify Prepared Statements Work

```cpp
TEST(QueryRegistryTest, FindPatientByMrn_Works) {
    // Setup
    DatabaseManager dbManager;
    dbManager.open(":memory:", "test-key");
    QueryCatalog::initializeQueries(&dbManager);
    
    // Insert test data
    QSqlQuery insertQuery = dbManager.getPreparedQuery(QueryId::Patient::INSERT);
    insertQuery.bindValue(":mrn", "TEST-001");
    insertQuery.bindValue(":name", "Test Patient");
    insertQuery.exec();
    
    // Test find query
    QSqlQuery findQuery = dbManager.getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
    findQuery.bindValue(":mrn", "TEST-001");
    ASSERT_TRUE(findQuery.exec());
    ASSERT_TRUE(findQuery.next());
    EXPECT_EQ(findQuery.value("name").toString(), "Test Patient");
}
```

---

## 10. Alternative Approaches (Comparison)

### **Approach 1: Magic Strings** (Current Anti-Pattern)
```cpp
QSqlQuery query = db->getPreparedQuery("find_patient_by_mrn");
```
❌ No autocomplete  
❌ No compile-time checking  
❌ Hard to refactor  
❌ Easy to make typos

### **Approach 2: Enum Class**
```cpp
enum class QueryId {
    PATIENT_FIND_BY_MRN,
    VITALS_INSERT,
    // ...
};
QSqlQuery query = db->getPreparedQuery(QueryId::PATIENT_FIND_BY_MRN);
```
✅ Type-safe  
✅ Autocomplete  
❌ No namespaces (flat list)  
❌ Hard to map enum to string

### **Approach 3: constexpr Strings in Namespaces** (✅ Recommended)
```cpp
namespace QueryId::Patient {
    constexpr const char* FIND_BY_MRN = "patient.find_by_mrn";
}
QSqlQuery query = db->getPreparedQuery(QueryId::Patient::FIND_BY_MRN);
```
✅ Type-safe (const char*)  
✅ Autocomplete with namespaces  
✅ Easy to map to string  
✅ Organized by domain  
✅ Can use in switch/if statements  
✅ **RECOMMENDED for Z Monitor**

### **Approach 4: Full ORM** (Not Recommended)
```cpp
Patient patient = db.find<Patient>().where("mrn", "MRN-001").first();
```
✅ Very convenient  
❌ Heavy performance overhead  
❌ Unpredictable latency  
❌ Not suitable for medical device

---

## 11. Query Naming Best Practices

### **DO:**
✅ Use namespace hierarchy (`QueryId::Patient::FIND_BY_MRN`)  
✅ Use lowercase with underscores (`find_by_mrn`, not `findByMRN`)  
✅ Use consistent action verbs (`find`, `get`, `insert`, `update`, `delete`)  
✅ Include filter criteria in name (`find_by_mrn`, `get_unsynced`)  
✅ Document all queries in QueryCatalog  
✅ Keep SQL in one place (QueryCatalog.cpp)

### **DON'T:**
❌ Don't use magic strings in calling code  
❌ Don't duplicate SQL across files  
❌ Don't use inconsistent naming (camelCase vs. snake_case)  
❌ Don't skip documentation (description, parameters, example)  
❌ Don't create ad-hoc queries (register them in catalog)

---

## 12. Maintenance Workflow

### **Adding a New Query:**

1. **Add constant to `QueryRegistry.h`:**
   ```cpp
   namespace QueryId::Patient {
       constexpr const char* FIND_BY_NAME = "patient.find_by_name";
   }
   ```

2. **Add query to `QueryCatalog.cpp`:**
   ```cpp
   queries[QueryId::Patient::FIND_BY_NAME] = {
       .id = QueryId::Patient::FIND_BY_NAME,
       .sql = "SELECT ... WHERE name LIKE :name",
       .description = "Find patients by name (partial match)",
       .parameters = {":name"},
       .isReadOnly = true
   };
   ```

3. **Use in repository:**
   ```cpp
   QSqlQuery query = m_dbManager->getPreparedQuery(QueryId::Patient::FIND_BY_NAME);
   ```

4. **Regenerate documentation:**
   ```bash
   # Build-time hook or script
   ./generate_query_docs.sh
   ```

### **Removing a Query:**

1. Mark as deprecated in `QueryRegistry.h`
2. Find all usages (Ctrl+Click on constant)
3. Remove usages
4. Remove from catalog
5. Remove constant

---

## 13. Integration with Build System

### 13.1 CMakeLists.txt

```cmake
# Add QueryRegistry to library
set(INFRASTRUCTURE_SOURCES
    src/infrastructure/persistence/DatabaseManager.cpp
    src/infrastructure/persistence/QueryRegistry.cpp
    src/infrastructure/persistence/QueryCatalog.cpp
    # ...
)

# Optional: Generate query documentation at build time
add_custom_target(generate_query_docs
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_query_docs.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating query documentation"
)
```

### 13.2 Documentation Generation Script

```bash
#!/bin/bash
# scripts/generate_query_docs.sh

# Build query doc generator
cmake --build build --target query_doc_generator

# Run generator
./build/query_doc_generator > doc/QUERY_REFERENCE.md

echo "Generated doc/QUERY_REFERENCE.md"
```

---

## 14. Query Registry Diagram

[View Query Registry Architecture (Mermaid)](./32_QUERY_REGISTRY.mmd)  
[View Query Registry Architecture (SVG)](./32_QUERY_REGISTRY.svg)

---

## 15. Implementation Checklist

- [ ] Create `QueryRegistry.h` with QueryId namespace
- [ ] Create `QueryCatalog.cpp` with SQL definitions
- [ ] Update `DatabaseManager` to support registration
- [ ] Update all repositories to use QueryId constants
- [ ] Remove all magic string queries
- [ ] Add unit tests for query compilation
- [ ] Generate query reference documentation
- [ ] Add to build system (CMakeLists.txt)
- [ ] Document in this file
- [ ] Add to `ZTODO.md`

---

## 16. References

- [30_DATABASE_ACCESS_STRATEGY.md](./30_DATABASE_ACCESS_STRATEGY.md) – Repository pattern and database access
- [20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md) – Error handling guidelines (when to return vs. log vs. emit errors)
- [10_DATABASE_DESIGN.md](./10_DATABASE_DESIGN.md) – Database schema
- [29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md) – DDD repository pattern
- Qt SQL Documentation: https://doc.qt.io/qt-6/qsqlquery.html

---

*This document defines the query registry pattern for Z Monitor. All SQL queries must be registered in the QueryCatalog, not scattered as magic strings.*

