# Code Quality Rules - Don't Repeat Yourself (DRY)

## The Rule

**Never copy-paste code logic.** If you find yourself writing the same logic twice, create a reusable function instead.

## Example from This Project

### ❌ WRONG - Code Duplication

```cpp
// In DatabaseTestFixture.cpp
QStringList DatabaseTestFixture::splitSqlStatements(const QString &sql) const
{
    // 60+ lines of SQL parsing logic
    QString cleanedSql;
    QStringList lines = sql.split('\n');
    for (const QString &line : lines) {
        // ... comment stripping logic ...
    }
    // ... semicolon splitting logic ...
    return out;
}

// In MigrationLoader.cpp - DUPLICATED!
QStringList MigrationLoader::splitSqlStatements(const QString &sql) const
{
    // Exact same 60+ lines of logic copied!
    QString cleanedSql;
    QStringList lines = sql.split('\n');
    // ... same logic again ...
}
```

**Problems:**
- Bug fixes must be applied in multiple places
- Easy to forget to update all copies
- Code bloat
- Maintenance nightmare

### ✅ CORRECT - Reusable Function

```cpp
// In SqlUtils.h (single source of truth)
namespace zmon::sql {
    QStringList splitSqlStatements(const QString &sql);
}

// In SqlUtils.cpp
QStringList splitSqlStatements(const QString &sql)
{
    // Logic implemented ONCE
    // ...
}

// In DatabaseTestFixture.cpp - USE IT
#include "infrastructure/persistence/SqlUtils.h"
QStringList statements = zmon::sql::splitSqlStatements(sql);

// In MigrationLoader.cpp - USE IT
#include "infrastructure/persistence/SqlUtils.h"
QStringList statements = zmon::sql::splitSqlStatements(sql);
```

**Benefits:**
- ✅ Fix bugs once, benefit everywhere
- ✅ Easy to maintain
- ✅ Self-documenting (function name describes purpose)
- ✅ Easier to test

## When to Create a Reusable Function

### Rule of Three

**If you use the same logic 3 times, it should be a reusable function.**

1. **First time**: Write it inline (OK)
2. **Second time**: Copy-paste (acceptable, but watch for third)
3. **Third time**: STOP! Refactor into reusable function

### Signs You Need a Reusable Function

- ✋ You're about to copy-paste code
- ✋ You found similar logic in 2+ places
- ✋ A comment says "same as in File X"
- ✋ Bug fix required editing multiple files

## How to Create a Reusable Function

### Step 1: Identify the Abstraction

```cpp
// Specific implementation
QStringList DatabaseTestFixture::splitSqlStatements(const QString &sql) const

// Generic abstraction (remove class-specific context)
QStringList splitSqlStatements(const QString &sql)
```

### Step 2: Choose the Right Location

**Utility namespace** for general-purpose functions:
```cpp
namespace zmon::sql {
    QStringList splitSqlStatements(const QString &sql);
}
```

**Static class method** for class-related utilities:
```cpp
class SqlParser {
public:
    static QStringList splitStatements(const QString &sql);
};
```

### Step 3: Document It

```cpp
/**
 * @brief Split multi-statement SQL script into individual statements.
 * 
 * This function intelligently splits SQL on semicolons while:
 * - Respecting string literals
 * - Handling inline comments
 * - Working around DDL generator bugs
 * 
 * @param sql Multi-statement SQL script
 * @return List of individual executable statements
 */
QStringList splitSqlStatements(const QString &sql);
```

### Step 4: Update CMakeLists.txt

```cmake
set(SOURCES
    ...
    persistence/SqlUtils.h
    persistence/SqlUtils.cpp
)
```

### Step 5: Replace All Duplicates

Find all places using the duplicated logic and replace with function call:

```cpp
// Before
QStringList statements = this->splitSqlStatements(sql);

// After
#include "infrastructure/persistence/SqlUtils.h"
QStringList statements = zmon::sql::splitSqlStatements(sql);
```

## Common Patterns

### String Utilities
```cpp
namespace zmon::string {
    QString trim(const QString &str);
    QString removeWhitespace(const QString &str);
    QStringList split(const QString &str, const QString &sep);
}
```

### File Utilities
```cpp
namespace zmon::file {
    QString readFile(const QString &path);
    bool writeFile(const QString &path, const QString &content);
    bool exists(const QString &path);
}
```

### SQL Utilities  
```cpp
namespace zmon::sql {
    QStringList splitSqlStatements(const QString &sql);
    QString stripSqlComments(const QString &line);
    bool isSqlComment(const QString &statement);
}
```

### Date/Time Utilities
```cpp
namespace zmon::time {
    QDateTime now();
    QString toIso8601(const QDateTime &dt);
    QDateTime fromIso8601(const QString &str);
}
```

## Checklist Before Commit

- [ ] No copy-pasted logic (search for similar code)
- [ ] Reusable functions in appropriate namespace/class
- [ ] Functions have Doxygen documentation
- [ ] CMakeLists.txt updated if new files added
- [ ] All duplicates replaced with reusable function
- [ ] Tests pass after refactoring

## Examples in This Project

✅ **Good Examples:**
- `zmon::sql::splitSqlStatements()` - SQL parsing
- `zmon::sql::stripSqlComments()` - Comment removal
- `zmon::sql::isSqlComment()` - Comment detection

❌ **Past Violations (Fixed):**
- SQL splitting logic was duplicated in DatabaseTestFixture
- Now centralized in `SqlUtils`

## References

- "The Pragmatic Programmer" - DRY Principle
- "Clean Code" by Robert Martin - Functions
- [SQL Utils Documentation](../SQL_UTILS.md)

---

**Remember**: Every time you copy-paste code, ask yourself:  
**"Should this be a reusable function instead?"**

Most of the time, the answer is YES! ✅
