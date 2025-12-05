# Fixes Applied - December 4, 2025

## Fix 1: SQLite DDL Comment Parsing in Tests

**Issue**: Auto-generated DDL files have inline comments with trailing commas (e.g., `-- Primary key,`). The test fixture's SQL parser was removing these comments including the commas, breaking the SQL syntax.

**File**: `tests/fixtures/DatabaseTestFixture.cpp`

**Fix**: Enhanced `splitSqlStatements()` to:
- Detect inline SQL comments (`-- comment`)
- Extract trailing commas from within comments
- Place commas correctly before the comment marker
- Strip comments while preserving SQL syntax

**Result**: All tables (including `certificate_store`) are now created correctly in test databases.

---

## Fix 2: Shared In-Memory Database Contamination

**Issue**: Using `file::memory:?cache=shared` created a SHARED database that persisted across all test instances, causing:
- Data from test 1 visible in test 2
- Segmentation faults on subsequent tests
- "Parameter count mismatch" errors due to stale connections
- Lingering `file::memory:?cache=shared` file in project root after tests

**Files Updated**:
- `tests/fixtures/DatabaseTestFixture.cpp` ✅
- `tests/integration/application/MonitoringWorkflowTest.cpp` ✅
- `tests/integration/admission/AdmissionWorkflowTest.cpp` ✅

**Fix**: Generate unique database URI for each test instance:
```cpp
const QString uniqueDbUri = QString("file:test_%1?mode=memory&cache=shared")
                                .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
```

**Why this works**:
- Each test gets completely isolated database
- `cache=shared` still allows main/read/write connections to share within the test
- Unique UUID prevents cross-test contamination
- Auto-cleanup when connections close
- No files left behind in project directory

**Verification**:
```bash
# Before fix: Test 1 inserts id:1, Test 2 updates id:1 (shared state)
# After fix: Both tests insert id:1 (isolated)
./test_certificate_manager --gtest_repeat=3
---

## Documentation Created

1. **[DOC-PROC-018: Database Test Isolation and Cleanup Pattern](../processes/DOC-PROC-018_database_test_isolation.md)**
   - Comprehensive guide on database test isolation
   - Root cause analysis of shared database issues
   - Reusable patterns for all database tests
   - Debugging tips and verification steps
   - Implementation examples and reference guide

---

## Tests Fixed and Verified

**Database Isolation - All tests use unique URI pattern**:
- ✅ `tests/fixtures/DatabaseTestFixture` - Base fixture using unique URIs
- ✅ `tests/integration/application/MonitoringWorkflowTest` - Updated to use unique URI
- ✅ `tests/integration/admission/AdmissionWorkflowTest` - Updated to use unique URI
- ✅ `tests/integration/persistence/test_sqlite_alarm_repository.cpp` - Uses DatabaseTestFixture
- ✅ `tests/integration/persistence/test_sqlite_vitals_repository.cpp` - Uses MockDatabaseManager
- ✅ `tests/integration/persistence/test_sqlite_telemetry_repository.cpp` - Uses MockDatabaseManager
- ✅ `tests/integration/db_smoke_test.cpp` - Uses `:memory:` (simple smoke tests, no cross-test contamination)

All tests now:
- Pass when run individually
- Pass when run together
- Pass when run multiple times (`--gtest_repeat=N`)
- Have complete database isolation
- Leave no files behind in project directory

---

## Lessons Learned

### 1. Shared In-Memory Databases Need Unique Names in Tests
- **Always use unique database names** with `cache=shared`: `file:test_<UUID>?mode=memory&cache=shared`
- **Never use** a shared name like `file::memory:?cache=shared` across multiple tests
- The `cache=shared` parameter is **required** to allow read/write/main connections to share the same database
- Unique names provide test isolation while maintaining connection sharing within each test
- Stray files in project directory indicate a test is using fixed database names

### 2. DDL Comment Handling Matters
- Auto-generated DDL may have non-standard comment formatting
- SQL parsers must preserve syntax while stripping comments
- Test the parser with actual DDL, not idealized examples

### 3. Document Patterns for Reuse
- Same bug occurred multiple times before documentation
- Creating comprehensive troubleshooting guides prevents recurrence
- Include verification steps so fixes can be validated

### 4. Test Consistency Requires Uniform Patterns
- All standalone tests creating their own databases should use the same pattern
- Using fixtures (like DatabaseTestFixture) is preferred for consistency
- Mock implementations (MockDatabaseManager) are appropriate for unit tests
- Simple `:memory:` is acceptable only for smoke tests that don't persist data between tests

---

## Code Quality Improvement: Eliminated Code Duplication

**Issue**: SQL parsing logic was duplicated in `DatabaseTestFixture::splitSqlStatements()` and would need to be duplicated anywhere else SQL files are processed.

**Fix**: Created reusable `SqlUtils` module:

### New Files Created:

1. **`src/infrastructure/persistence/SqlUtils.h`**
   - Namespace: `zmon::sql`
   - Functions: `splitSqlStatements()`, `stripSqlComments()`, `isSqlComment()`
   - Full Doxygen documentation
   - Thread-safe, stateless functions

2. **`src/infrastructure/persistence/SqlUtils.cpp`**
   - Implements SQL parsing logic
   - Handles DDL generator bug (commas in comments)
   - Respects string literals and SQL comment syntax

3. **`doc/SQL_UTILS.md`**
   - Comprehensive usage guide
   - Examples and best practices
   - Migration guide from duplicated code

### Files Refactored:

- **`tests/fixtures/DatabaseTestFixture.cpp`**
  - Removed 60+ lines of duplicated SQL parsing code
  - Now uses `zmon::sql::splitSqlStatements()`
  - Now uses `zmon::sql::isSqlComment()`
  - Cleaner, more maintainable code

- **`tests/fixtures/DatabaseTestFixture.h`**
  - Removed `splitSqlStatements()` private method declaration
  - Reduced class complexity

- **`src/infrastructure/CMakeLists.txt`**
  - Added SqlUtils.h and SqlUtils.cpp to build

### Benefits:

1. **Single Source of Truth** - SQL parsing logic in one place
2. **Better Documentation** - Comprehensive Doxygen comments explain DDL bug
3. **Easier Maintenance** - Fix bugs once, benefit everywhere
4. **Reusability** - Any code needing SQL parsing can use `zmon::sql::`
5. **Testability** - Centralized logic easier to unit test

### Usage Example:

```cpp
#include "infrastructure/persistence/SqlUtils.h"

// Before (duplicated):
QStringList statements = this->splitSqlStatements(sql); // Local implementation

// After (reusable):
QStringList statements = zmon::sql::splitSqlStatements(sql); // Shared utility
```

### Verification:

✅ All tests pass with refactored code  
✅ No functionality changed, pure refactoring  
✅ Code is cleaner and more maintainable

---

## How to Prevent This in the Future

### For All Database Tests:

1. **Use `DatabaseTestFixture`** - It now has the fix built-in
2. **Follow destruction order** - Repositories before DatabaseManager
3. **Verify isolation** - Run with `--gtest_repeat=3` to catch shared state
4. **Check for warnings** - "connection still in use" indicates lifecycle issues

### For New Tests:

```cpp
class MyTestFixture : public zmon::test::DatabaseTestFixture
{
protected:
    void SetUp() override
    {
        zmon::test::DatabaseTestFixture::SetUp();  // Gets unique DB automatically
        repo = new SQLiteMyRepository(databaseManager());
    }
    
    void TearDown() override
    {
        delete repo;  // BEFORE base TearDown
        zmon::test::DatabaseTestFixture::TearDown();
    }
    
    SQLiteMyRepository *repo{nullptr};
};
```

---

## Verification Commands

```bash
# Run tests once
cd build/tests/unit/infrastructure/security
./test_certificate_manager

# Run multiple times to verify no shared state
./test_certificate_manager --gtest_repeat=5

# Check for "Inserting new record" (not "Updating")
./test_certificate_manager 2>&1 | grep -i "insert\|updat"
```

All commands should show consistent "Inserting" behavior with no "Updating".
