---
doc_id: DOC-COMP-030
title: SQL Utilities - Reusable SQL Parsing Functions
version: v1.0
category: Component
subcategory: Infrastructure/Database
status: Published
owner: Infrastructure Team
reviewers:
  - Architecture Team
last_reviewed: 2025-12-04
next_review: 2026-03-04
related_docs:
  - DOC-COMP-033 # Database Connection Pattern
  - DOC-COMP-021 # Schema Management
  - DOC-TROUBLE-001 # Database Test Cleanup Issue
related_tasks:
  - TASK-DB-001 # DatabaseManager implementation (completed)
related_requirements:
  - REQ-CODE-001 # Code Reusability
tags:
  - database
  - utilities
  - sql
  - parsing
  - infrastructure
  - dry-principle
  - reusable-code
diagram_files: []
---

# SQL Utilities - Reusable SQL Parsing Functions

> **DOC-ID:** DOC-COMP-030 | **Version:** v1.0 | **Status:** Published | **Owner:** Infrastructure Team

## Overview

The `SqlUtils` module provides reusable SQL parsing and processing functions to eliminate code duplication across the codebase. These utilities handle common SQL tasks such as splitting multi-statement scripts and stripping comments.

**Following:** [DOC-GUIDE-004: Documentation Guidelines](../../.github/instructions/doc_guidelines.md)

## Location

- **Header**: `src/infrastructure/persistence/SqlUtils.h`
- **Implementation**: `src/infrastructure/persistence/SqlUtils.cpp`
- **Namespace**: `zmon::sql`

## Problem Solved

**Before**: SQL parsing logic was duplicated in every place that needed to process SQL files:
- DatabaseTestFixture
- Migration loaders
- Schema generators
- Any component loading `.sql` files

**After**: Centralized, well-documented, reusable functions in `zmon::sql` namespace.

## Functions

### `splitSqlStatements(const QString &sql)`

Splits a multi-statement SQL script into individual executable statements.

**Features:**
- Respects string literals (doesn't split on semicolons inside quotes)
- Handles inline SQL comments (`-- comment`)
- Works around DDL generator bug where commas appear inside comments
- Returns clean, executable SQL statements

**Example:**
```cpp
#include "infrastructure/persistence/SqlUtils.h"

QString multiStatementSql = loadFromFile("create_tables.sql");
QStringList statements = zmon::sql::splitSqlStatements(multiStatementSql);

for (const QString &stmt : statements) {
    if (zmon::sql::isSqlComment(stmt.trimmed()))
        continue;
    
    QSqlQuery q(db);
    q.exec(stmt);
}
```

### `stripSqlComments(const QString &line)`

Removes inline SQL comments from a single line while preserving syntax.

**DDL Generator Bug Workaround:**
```cpp
// Input (from auto-generated DDL):
QString line = "id INTEGER PRIMARY KEY  -- Primary key,";

// Output (comma extracted and preserved):
QString cleaned = zmon::sql::stripSqlComments(line);
// Result: "id INTEGER PRIMARY KEY,"
```

### `isSqlComment(const QString &statement)`

Checks if a statement is empty, whitespace-only, or a comment line.

**Example:**
```cpp
QStringList statements = {...};
for (const QString &stmt : statements) {
    QString trimmed = stmt.trimmed();
    if (zmon::sql::isSqlComment(trimmed))
        continue; // Skip non-executable lines
    executeStatement(trimmed);
}
```

## Usage Pattern

### For Loading SQL Files

```cpp
#include "infrastructure/persistence/SqlUtils.h"

void loadSchemaFile(const QString &filePath, QSqlDatabase &db)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QString sql = QString::fromUtf8(file.readAll());
    file.close();
    
    // Split into individual statements
    QStringList statements = zmon::sql::splitSqlStatements(sql);
    
    for (const QString &stmt : statements) {
        QString trimmed = stmt.trimmed();
        
        // Skip comments and empty statements
        if (zmon::sql::isSqlComment(trimmed))
            continue;
        
        // Execute statement
        QSqlQuery q(db);
        if (!q.exec(trimmed)) {
            qWarning() << "SQL failed:" << q.lastError().text();
        }
    }
}
```

### For Testing

```cpp
// In test fixtures
void MyTestFixture::applyMigrations()
{
    QString ddl = loadDDL();
    QStringList statements = zmon::sql::splitSqlStatements(ddl);
    
    for (const QString &stmt : statements) {
        if (zmon::sql::isSqlComment(stmt.trimmed()))
            continue;
        QSqlQuery q(db());
        ASSERT_TRUE(q.exec(stmt)) << q.lastError().text().toStdString();
    }
}
```

## DDL Generator Bug

### The Problem

Auto-generated DDL files have trailing commas INSIDE comments:

```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY  -- Primary key,
    name TEXT NOT NULL      -- User full name,
    age INTEGER             -- User age in years
);
```

The commas should be BEFORE the `--` marker, not inside the comment.

### The Solution

`splitSqlStatements()` automatically detects this pattern and fixes it:

1. Scans each line for `--` comment marker
2. Checks if comment ends with `,`
3. Extracts comma and places it before the comment
4. Returns syntactically correct SQL

**Input:**
```sql
id INTEGER PRIMARY KEY  -- Primary key,
```

**Output:**
```sql
id INTEGER PRIMARY KEY,
```

## Where It's Used

Currently used in:
- ✅ `DatabaseTestFixture` - Test database setup
- 🔄 Future: Migration loader
- 🔄 Future: Schema validation tools
- 🔄 Future: SQL import utilities

## Migration Guide

### Before (Duplicated Code)

```cpp
// Every file had its own SQL splitter
QStringList MyClass::splitStatements(const QString &sql)
{
    // 50+ lines of duplicated logic
    QString cleanedSql;
    QStringList lines = sql.split('\n');
    for (const QString &line : lines) {
        // ... duplicate comment stripping logic ...
    }
    // ... duplicate semicolon splitting logic ...
    return out;
}
```

### After (Reusable Utility)

```cpp
#include "infrastructure/persistence/SqlUtils.h"

// One line!
QStringList statements = zmon::sql::splitSqlStatements(sql);
```

## Benefits

1. **DRY Principle** - Don't Repeat Yourself
   - Single source of truth for SQL parsing
   - Fix bugs once, benefit everywhere

2. **Well-Documented**
   - Comprehensive Doxygen comments
   - Usage examples in header
   - Explains DDL generator bug workaround

3. **Tested**
   - Used in production test fixtures
   - Handles real-world DDL files
   - Edge cases documented

4. **Reusable**
   - Stateless functions (thread-safe)
   - No dependencies beyond Qt
   - Easy to use anywhere

## Best Practices

### DO ✅
```cpp
// Use the utility
QStringList statements = zmon::sql::splitSqlStatements(sql);

// Check for comments
if (zmon::sql::isSqlComment(stmt))
    continue;

// Strip comments from single line
QString clean = zmon::sql::stripSqlComments(line);
```

### DON'T ❌
```cpp
// Don't duplicate SQL parsing logic
QStringList mySplit(const QString &sql) {
    // ... reimplementing splitSqlStatements ...
}

// Don't manually check for comments
if (stmt.trimmed().isEmpty() || stmt.startsWith("--"))
    // Use isSqlComment() instead!

// Don't forget to handle the DDL generator bug
// Use stripSqlComments() which handles it automatically
```

## Future Enhancements

Potential additions to `SqlUtils`:

- **Multi-line comment support** (`/* */` style)
- **Prepared statement parameter parsing**
- **SQL formatting/pretty-printing**
- **SQL validation before execution**
- **Transaction detection and grouping**

## References

- **Implementation**: `src/infrastructure/persistence/SqlUtils.cpp`
- **Test usage**: `tests/fixtures/DatabaseTestFixture.cpp`
- **Related documentation**: [DOC-COMP-033: Database Connection Pattern](../infrastructure/DOC-COMP-033_database_connection_pattern.md)
- **Related documentation**: [DOC-TROUBLE-001: Database Test Cleanup Issue](../troubleshooting/DATABASE_TEST_CLEANUP_ISSUE.md)

---

## Changelog

### v1.0 - 2025-12-04
- Initial documentation of SQL utilities module
- Documented all utility functions with examples
- Documented DDL generator bug workaround
- Added usage patterns and best practices
- Added migration guide from duplicated code
- Formatted per DOC-GUIDE-004 guidelines
- Recategorized from DOC-REF to DOC-COMP (utility components)

---

**Document ID:** DOC-COMP-030  
**Version:** v1.0  
**Status:** Published  
**Owner:** Infrastructure Team  
**Last Updated:** 2025-12-04
