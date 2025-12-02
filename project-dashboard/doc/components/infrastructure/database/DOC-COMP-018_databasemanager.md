---
doc_id: DOC-COMP-018
title: DatabaseManager
version: v1.0
category: Component
subcategory: Infrastructure Layer / Persistence
status: Draft
owner: Infrastructure Team
reviewers: 
  - Architecture Team
last_reviewed: 2025-12-01
next_review: 2026-12-01
related_docs:
  - DOC-ARCH-002  # System architecture
  - DOC-COMP-014  # IPatientRepository
  - DOC-COMP-015  # ITelemetryRepository
related_tasks:
  - TASK-3C-001  # Phase 3C Migration
related_requirements:
  - REQ-DATA-001  # Database persistence
tags:
  - infrastructure
  - database
  - sqlite
  - connection-management
  - transaction
  - orm
diagram_files: []
---

# DOC-COMP-018: DatabaseManager

## 1. Overview

**Purpose:** Database connection manager providing SQLite connection management, transaction support, migration execution, and hybrid ORM + manual SQL capabilities. Central orchestrator for all database operations across the application.

**Responsibilities:**
- Manage SQLite database connections (read/write connection pooling)
- Transaction management (begin, commit, rollback)
- SQLCipher encryption support (optional encryption key)
- Database migration execution (schema versioning)
- QxOrm connection management (when USE_QXORM is enabled)
- Connection lifecycle (open, close, isOpen checks)
- Thread-safe database access coordination

**Layer:** Infrastructure Layer

**Module:** `z-monitor/src/infrastructure/persistence/DatabaseManager.h` (338 lines)

**Thread Affinity:** Database I/O Thread (dedicated single writer thread for SQLite)

**Dependencies:**
- **Qt SQL:** QSqlDatabase, QSqlQuery, QSqlError (Qt SQL framework)
- **QxOrm (Optional):** QxOrm ORM library (when USE_QXORM is enabled)
- **SQLCipher (Optional):** SQLite encryption extension

## 2. Architecture

**Key Design Decisions:**
- **Decision 1: Single Writer Thread** - All database write operations execute on dedicated Database I/O Thread to ensure SQLite thread safety and avoid SQLITE_BUSY errors
- **Decision 2: Hybrid ORM + Manual SQL** - Supports both QxOrm (simple CRUD) and manual SQL (complex queries) for flexibility and performance
- **Decision 3: Connection Pooling** - Maintains separate read and write connections for optimized concurrency (write connection is exclusive, read connections can be shared)
- **Decision 4: Optional Encryption** - SQLCipher encryption can be enabled via encryption key parameter (production use, disabled for testing)

**Design Patterns Used:**
- **Singleton Pattern:** Single DatabaseManager instance manages all database connections
- **Connection Pool Pattern:** Separate read/write connections for optimized access patterns
- **Transaction Pattern:** Explicit begin/commit/rollback for atomic operations
- **Adapter Pattern:** QxOrm adapter when USE_QXORM is enabled

## 3. Public API

### 3.1 Key Class

```cpp
class DatabaseManager : public QObject, public IDatabaseManager {
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    // === Connection Management ===

    /**
     * @brief Open database connection.
     * @param dbPath Path to SQLite database file
     * @param encryptionKey Optional encryption key for SQLCipher (empty = no encryption)
     * @return Result<void> Success or error details if connection fails
     */
    Result<void> open(const QString &dbPath, const QString &encryptionKey = QString()) override;

    /**
     * @brief Close database connection.
     */
    void close() override;

    /**
     * @brief Check if database is open.
     * @return true if database is open, false otherwise
     */
    bool isOpen() const override;

    /**
     * @brief Get main database connection.
     * @return Reference to QSqlDatabase connection
     */
    QSqlDatabase &getConnection() override;

    /**
     * @brief Get write connection (dedicated for writes).
     * @return Reference to QSqlDatabase write connection
     */
    QSqlDatabase &getWriteConnection();

    /**
     * @brief Get read connection (can be shared).
     * @return Reference to QSqlDatabase read connection
     */
    QSqlDatabase &getReadConnection();

    // === Transaction Management ===

    /**
     * @brief Begin database transaction.
     * @return Result<void> Success or error details if transaction fails
     */
    Result<void> beginTransaction() override;

    /**
     * @brief Commit current transaction.
     * @return Result<void> Success or error details if commit fails
     */
    Result<void> commit() override;

    /**
     * @brief Rollback current transaction.
     * @return Result<void> Success or error details if rollback fails
     */
    Result<void> rollback() override;

    // === Migration Management ===

    /**
     * @brief Run database migrations.
     * @param targetVersion Target schema version (0 = latest)
     * @return Result<void> Success or error details if migration fails
     */
    Result<void> runMigrations(int targetVersion = 0) override;

    /**
     * @brief Get current schema version.
     * @return Current schema version number
     */
    int getCurrentSchemaVersion() const override;

signals:
    void databaseOpened();
    void databaseClosed();
    void migrationCompleted(int version);
    void transactionStarted();
    void transactionCommitted();
    void transactionRolledBack();
};
```

## 4. Implementation Highlights

**Connection Opening:**
1. Create QSqlDatabase connection with "QSQLITE" driver
2. Set database file path
3. If encryption key provided, execute SQLCipher PRAGMA commands
4. Open connection and verify
5. Initialize QxOrm connection (if USE_QXORM enabled)
6. Emit databaseOpened() signal

**Transaction Management:**
- `beginTransaction()`: Starts transaction on write connection
- `commit()`: Commits transaction, makes changes permanent
- `rollback()`: Rolls back transaction, discards changes
- **Thread Safety:** All transaction operations on Database I/O Thread

**Migration Execution:**
- Read current schema version from `schema_version` table
- Compare with target version
- Execute migration scripts sequentially
- Update `schema_version` table
- Emit migrationCompleted() signals

## 5. Usage Examples

### 5.1 Open Database with Encryption

```cpp
DatabaseManager* dbManager = new DatabaseManager();
QString dbPath = "/var/lib/zmonitor/data.db";
QString encryptionKey = "my-secret-key"; // Production: load from secure storage

auto result = dbManager->open(dbPath, encryptionKey);
if (result.isSuccess()) {
    qInfo() << "Database opened successfully";
} else {
    qCritical() << "Database open failed:" << result.error().message();
}
```

### 5.2 Transaction Usage

```cpp
auto txResult = dbManager->beginTransaction();
if (!txResult.isSuccess()) {
    qCritical() << "Transaction begin failed";
    return;
}

// Perform database operations
auto saveResult1 = repository1->save(data1);
auto saveResult2 = repository2->save(data2);

if (saveResult1.isSuccess() && saveResult2.isSuccess()) {
    dbManager->commit();
} else {
    dbManager->rollback();
    qWarning() << "Transaction rolled back due to errors";
}
```

### 5.3 Run Migrations

```cpp
auto migResult = dbManager->runMigrations(); // Migrate to latest version
if (migResult.isSuccess()) {
    int version = dbManager->getCurrentSchemaVersion();
    qInfo() << "Database migrated to version" << version;
}
```

## 6. Testing

**Unit Tests:**
- `test_Open_ValidPath_Success()` - Verify database opens successfully
- `test_Open_InvalidPath_ReturnsError()` - Verify error handling for invalid path
- `test_Transaction_CommitSuccess()` - Verify transaction commit
- `test_Transaction_RollbackOnError()` - Verify transaction rollback
- `test_Migration_UpgradeToLatest()` - Verify migration execution

**Integration Tests:**
- End-to-end database lifecycle (open → migrate → transact → close)
- Multi-repository transaction coordination
- SQLCipher encryption verification

## 7. Performance & Security

**Performance:**
- Connection open: <100ms (first time), <10ms (reopen)
- Transaction begin: <1ms
- Transaction commit: <10ms (depends on data volume)
- Migration: Variable (depends on schema changes)

**Security:**
- SQLCipher encryption support (AES-256 encryption)
- Encryption key never logged or persisted
- Database file permissions (600 - owner read/write only)
- Prepared statements prevent SQL injection

**Thread Safety:**
- All write operations on Database I/O Thread
- Read connections can be shared (SQLite read concurrency)
- QReadWriteLock for connection access control

## 8. Related Documentation

- IDatabaseManager Interface - Database manager contract
- SQLitePatientRepository - Repository using DatabaseManager
- Schema Migration Documentation - Migration strategy and versioning

## 9. Changelog

| Version | Date       | Author      | Changes                                      |
| ------- | ---------- | ----------- | -------------------------------------------- |
| v1.0    | 2025-12-01 | Dustin Wind | Initial documentation from DatabaseManager.h |
