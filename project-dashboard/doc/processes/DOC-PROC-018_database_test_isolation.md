---
doc_id: DOC-PROC-018
title: Database Test Isolation and Cleanup Pattern
version: v1.0
category: Process
subcategory: Testing/Database
status: Published
owner: Testing Team
reviewers:
  - Architecture Team
  - Infrastructure Team
last_reviewed: 2025-12-04
next_review: 2026-03-04
related_docs:
  - DOC-COMP-020 # DatabaseManager
  - DOC-COMP-021 # Schema Management
  - DOC-GUIDE-004 # Documentation Guidelines
related_tasks:
  - TASK-TEST-001 # Test Infrastructure
related_requirements:
  - REQ-TEST-001 # Test Isolation
tags:
  - testing
  - database
  - sqlite
  - isolation
  - fixtures
  - in-memory
  - googletest
diagram_files: []
---

# Database Test Isolation and Cleanup Pattern

## Problem Description

When running multiple database tests sequentially (especially with GoogleTest fixtures), you may encounter:

1. **Warning messages**: `QSqlDatabasePrivate::removeDatabase: connection 'X' is still in use, all queries will cease to work`
2. **Segmentation faults** on subsequent tests after the first test passes
3. **Data persistence across tests** - second test sees data from first test (UPDATE instead of INSERT)

## Root Cause Analysis

The issue occurs because of TWO distinct problems:

### Problem 1: Shared In-Memory Database Cache (`file::memory:?cache=shared`)

- Using `file::memory:?cache=shared` creates a SHARED in-memory database
- This database persists ACROSS test runs and test instances
- When test 1 inserts data and closes, the data REMAINS in the shared cache
- When test 2 runs, it sees test 1's data, causing incorrect behavior
- The shared cache isn't properly cleared between tests

**Evidence**: Running the same test twice shows "UPDATE existing record, id: 1" instead of "INSERT"

### Problem 2: Query Object Lifecycle

- **QSqlQuery objects hold references to QSqlDatabase connections**
- When `DatabaseManager::close()` is called in the test `TearDown()`, it calls `QSqlDatabase::removeDatabase()`
- If ANY `QSqlQuery` objects still exist (even if finished), Qt considers the connection "still in use"
- This leaves the database in an invalid state for the next test
- The next test crashes when trying to use the invalidated connections

## The Solution: Reusable Pattern

### Primary Fix: Use Unique Database Names for Each Test

The `DatabaseTestFixture` generates a unique in-memory database URI for each test instance:

```cpp
void DatabaseTestFixture::SetUp()
{
    // Generate unique URI: file:test_<UUID>?mode=memory&cache=shared
    const QString uniqueDbUri = QString("file:test_%1?mode=memory&cache=shared")
                                    .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    
    m_dbManager = std::make_unique<DatabaseManager>();
    auto result = m_dbManager->open(uniqueDbUri);
    // ... rest of setup
}
```

**Why this works:**
- Each test gets its own uniquely-named in-memory database
- `cache=shared` allows the 3 connections (main, read, write) to share the same database
- The unique name ensures complete isolation between test instances
- Database is automatically cleaned up when connections close

**Evidence of fix:**
- First test: "Inserting new record, id: 1"
- Second test: "Inserting new record, id: 1" (not "UPDATE existing record, id: 1")
- Both tests pass without segfaults

### Secondary Fix: Proper Destruction Order

Even with unique database names, follow proper destruction order to avoid warnings:

**CRITICAL RULE**: Destroy all objects that might hold QSqlQuery objects BEFORE calling `DatabaseTestFixture::TearDown()`

```cpp
void TearDown() override
{
    // ✅ CORRECT ORDER:
    // 1. Delete repositories and managers (they may hold QSqlQuery objects)
    delete repo;
    mgr.reset();
    
    // 2. NOW safe to close database and destroy DatabaseManager
    zmon::test::DatabaseTestFixture::TearDown();
}
```

**WRONG ORDER** (causes crashes):
```cpp
void TearDown() override
{
    // ❌ WRONG: Close DB first, then delete objects holding queries
    zmon::test::DatabaseTestFixture::TearDown(); // Closes database
    delete repo; // QSqlQuery destructors run AFTER connections removed = CRASH
    mgr.reset();
}
```

### Tertiary Fix: Use Smart Pointers

Instead of raw pointers, use smart pointers that guarantee proper destruction order:

```cpp
class MyTestFixture : public zmon::test::DatabaseTestFixture
{
protected:
    void SetUp() override
    {
        zmon::test::DatabaseTestFixture::SetUp();
        repo = std::make_unique<SQLiteSomeRepository>(databaseManager());
        mgr = std::make_unique<SomeManager>(repo.get());
    }
    
    void TearDown() override
    {
        // Smart pointers destroyed in reverse order of declaration
        mgr.reset();  // Destroy manager first
        repo.reset(); // Then repository
        
        // Now safe to close database
        zmon::test::DatabaseTestFixture::TearDown();
    }
    
    std::unique_ptr<SQLiteSomeRepository> repo;
    std::unique_ptr<SomeManager> mgr;
};
```

### Quaternary Fix: Enhanced DatabaseTestFixture Cleanup

The `DatabaseTestFixture::TearDown()` has been enhanced to clear query caches:

```cpp
void DatabaseTestFixture::TearDown()
{
    if (m_dbManager)
    {
        // Ensure all queries are finished before closing
        // This clears internal Qt query caches
        {
            QSqlQuery q(m_dbManager->getWriteConnection());
            q.finish();
        }
        {
            QSqlQuery q(m_dbManager->getReadConnection());
            q.finish();
        }
        
        m_dbManager->close();
        m_dbManager.reset();
    }
    m_app.reset();
}
```

## Implementation Patterns

### Pattern 1: Repository Tests

```cpp
class RepositoryTestFixture : public zmon::test::DatabaseTestFixture
{
protected:
    void SetUp() override
    {
        zmon::test::DatabaseTestFixture::SetUp();
        repo = new SQLiteMyRepository(databaseManager());
    }
    
    void TearDown() override
    {
        delete repo;  // ✅ Delete BEFORE base TearDown
        repo = nullptr;
        zmon::test::DatabaseTestFixture::TearDown();
    }
    
    SQLiteMyRepository *repo{nullptr};
};
```

### Pattern 2: Manager with Repository Tests

```cpp
class ManagerTestFixture : public zmon::test::DatabaseTestFixture
{
protected:
    void SetUp() override
    {
        zmon::test::DatabaseTestFixture::SetUp();
        repo = new SQLiteMyRepository(databaseManager());
        mgr = std::make_unique<MyManager>(repo);
    }
    
    void TearDown() override
    {
        // Destroy in reverse order of dependencies
        mgr.reset();      // ✅ Manager first (depends on repo)
        delete repo;      // ✅ Repository second (holds queries)
        repo = nullptr;
        zmon::test::DatabaseTestFixture::TearDown(); // ✅ Database last
    }
    
    SQLiteMyRepository *repo{nullptr};
    std::unique_ptr<MyManager> mgr;
};
```

## Verification Checklist

- [ ] All repository/manager objects are destroyed BEFORE `DatabaseTestFixture::TearDown()`
- [ ] Destruction order follows dependency chain (managers → repositories → database)
- [ ] Use smart pointers when possible to enforce RAII
- [ ] Test runs cleanly when executed multiple times: `./test --gtest_repeat=10`
- [ ] No "connection still in use" warnings
- [ ] No segmentation faults

## Debugging Guide

### Test in Isolation First
```bash
# Run single test
./test_name --gtest_filter=MyFixture.MyTest

# If it passes alone but fails with others, it's a cleanup issue
```

### Run Multiple Times
```bash
# Run same test multiple times to verify cleanup
./test_name --gtest_filter=MyFixture.MyTest --gtest_repeat=5
```

### Check for Warnings
Even if tests pass, check for the warning:
```
qt.sql.qsqldatabase: QSqlDatabasePrivate::removeDatabase: connection 'X' is still in use
```

This warning indicates a cleanup order bug, even if it hasn't caused a crash yet.

## Related Implementation Files

- `tests/fixtures/DatabaseTestFixture.h` - Base fixture with enhanced cleanup
- `tests/fixtures/DatabaseTestFixture.cpp` - Implementation with query finish logic
- `src/infrastructure/persistence/DatabaseManager.h` - Database manager interface
- `src/infrastructure/persistence/DatabaseManager.cpp` - Connection management

## Reference Guide

### The Problem
```
❌ file::memory:?cache=shared  → Shared across ALL tests → Data contamination
```

### The Solution  
```
✅ file:test_<UUID>?mode=memory&cache=shared  → Unique per test → Complete isolation
```

### Verification Test
```bash
# Both tests should show "Inserting new record, id: 1"
./test_your_test

# Run multiple times to verify no shared state
./test_your_test --gtest_repeat=5
```

### Key Principles
1. **Unique database per test** - Use UUID in database name
2. **Proper destruction order** - Repositories before DatabaseManager
3. **Shared cache within test** - `cache=shared` allows read/write/main connections to share
4. **Isolated between tests** - Unique names prevent cross-test contamination

## History and Revisions

- **v1.0 (2025-12-04)**: Initial documentation
  - Documented pattern after fixing DatabaseTest segfault
  - Root cause identified: Shared in-memory database cache
  - Solution implemented: Unique database URI per test instance
  - Applied to all integration tests (MonitoringWorkflowTest, AdmissionWorkflowTest)

## External References

- Qt Documentation: [QSqlDatabase::removeDatabase()](https://doc.qt.io/qt-6/qsqldatabase.html#removeDatabase)
- Qt Documentation: [QSqlQuery](https://doc.qt.io/qt-6/qsqlquery.html)
- Qt Documentation: [SQLite URI Filenames](https://www.sqlite.org/uri.html)
- Google Testing Framework: [GoogleTest Fixtures](https://google.github.io/googletest/primer.html#test-fixtures)

---

**Document ID:** DOC-PROC-018  
**Version:** v1.0  
**Status:** Published  
**Owner:** Testing Team  
**Last Updated:** 2025-12-04
