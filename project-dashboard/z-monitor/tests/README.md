# Z Monitor Test Infrastructure

## CMake Structure

The test infrastructure is integrated with the main project CMake build system:

```
z-monitor/
├── CMakeLists.txt          # Root CMake file (sets Z_MONITOR_ROOT variable)
├── src/                    # Source code
│   ├── CMakeLists.txt      # Builds libraries: z_monitor_domain, z_monitor_application, z_monitor_infrastructure
│   └── ...
└── tests/                  # Test code
    ├── CMakeLists.txt      # Test root (adds unit, integration, e2e)
    ├── mocks/              # Mock implementations
    │   └── CMakeLists.txt  # Builds mock libraries
    └── unit/               # Unit tests
        └── CMakeLists.txt  # Unit test executables
```

## CMake Best Practices

### ✅ Using Root Variables (Current Approach)

The root `CMakeLists.txt` sets variables that all subdirectories use:

```cmake
# In z-monitor/CMakeLists.txt
set(Z_MONITOR_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(Z_MONITOR_SOURCE_DIR ${Z_MONITOR_ROOT}/src)
```

Subdirectories use these variables instead of fragile relative paths:

```cmake
# ✅ GOOD: Uses root variable
target_include_directories(test_alarm_manager
    PRIVATE
        ${Z_MONITOR_ROOT}  # Clear, maintainable
)

# ❌ BAD: Fragile relative path
target_include_directories(test_alarm_manager
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..  # Breaks if structure changes
)
```

**Benefits:**
- Clear and maintainable
- Single source of truth
- Works regardless of directory depth
- Easy to refactor

## Test Location: Co-located vs. Separate

### Current Structure: Separate Tests Directory

```
z-monitor/
├── src/
│   └── domain/
│       └── common/
│           └── Result.h
└── tests/
    └── unit/
        └── domain/
            └── common/
                └── ResultTest.cpp
```

**Pros:**
- Clear separation of production code and test code
- Easy to exclude tests from production builds
- Works well for integration/e2e tests that need separate setup
- Matches current project structure

**Cons:**
- Tests are far from the code they test
- Harder to see what's tested when reading source
- More directory navigation
- Can lead to tests getting out of sync with source

### Alternative: Co-located Tests (Recommended for Unit Tests)

```
z-monitor/
└── src/
    └── domain/
        └── common/
            ├── Result.h
            ├── Result.cpp
            └── ResultTest.cpp  # Test next to source
```

**Pros:**
- ✅ Tests are immediately visible next to source
- ✅ Easy to see what's tested
- ✅ Tests stay in sync with source (harder to forget)
- ✅ Better for understanding code coverage
- ✅ Matches modern C++ best practices (Google, LLVM style)

**Cons:**
- Requires careful CMake configuration to exclude tests from production builds
- Integration/e2e tests may still need separate directory

### Hybrid Approach (Recommended)

Use co-located tests for **unit tests** and separate directory for **integration/e2e tests**:

```
z-monitor/
├── src/
│   └── domain/
│       └── common/
│           ├── Result.h
│           ├── Result.cpp
│           └── ResultTest.cpp  # Unit test co-located
└── tests/
    ├── integration/  # Integration tests (separate)
    └── e2e/          # E2E tests (separate)
```

**Benefits:**
- Unit tests close to source (easy to find, maintain)
- Integration/e2e tests separate (need different setup, fixtures)
- Best of both worlds

## Migration Path (If Desired)

To migrate to co-located unit tests:

1. **Move unit tests next to source:**
   ```bash
   # Example: Move ResultTest.cpp
   mv tests/unit/domain/common/ResultTest.cpp src/domain/common/ResultTest.cpp
   ```

2. **Update CMakeLists.txt in each source directory:**
   ```cmake
   # In src/domain/common/CMakeLists.txt
   if(BUILD_TESTING)
       add_executable(result_test ResultTest.cpp)
       target_link_libraries(result_test PRIVATE zmon_domain_common GTest::gtest)
       add_test(NAME ResultTest COMMAND result_test)
   endif()
   ```

3. **Keep integration/e2e tests in tests/ directory:**
   - Integration tests need separate fixtures
   - E2E tests need separate setup
   - These stay in `tests/integration/` and `tests/e2e/`

## How Tests Access Source Code

Tests access source code classes in two ways:

### 1. Linking to Libraries

Tests link to the libraries built from `src/`:

- **`z_monitor_domain`** - Domain layer (pure C++, business logic)
- **`z_monitor_application`** - Application layer (use-case orchestration)
- **`z_monitor_infrastructure`** - Infrastructure layer (technical implementations)
- **`zmon_domain_common`** - Common utilities (Result, RetryPolicy, CircuitBreaker)

Example:
```cmake
target_link_libraries(test_alarm_manager
    PRIVATE
        z_monitor_domain
        z_monitor_application
        z_monitor_infrastructure
        zmon_test_mocks
)
```

### 2. Include Directories

Tests use include directories that point to the `src/` directory:

```cmake
target_include_directories(test_alarm_manager
    PRIVATE
        ${Z_MONITOR_ROOT}  # Points to z-monitor/ root
)
```

This allows tests to include headers like:
```cpp
#include "domain/common/Result.h"
#include "infrastructure/interfaces/ITelemetryServer.h"
```

## Mock Objects

Mock objects are organized by layer to match the DDD structure:

- **`tests/mocks/domain/`** - Domain layer mocks (e.g., `MockPatientRepository`)
- **`tests/mocks/infrastructure/`** - Infrastructure layer mocks (e.g., `MockTelemetryServer`, `MockPatientLookupService`)

Mocks link to the corresponding source libraries to get interface definitions:

```cmake
# Infrastructure mocks link to infrastructure library
target_link_libraries(zmon_test_mocks_infrastructure
    PUBLIC
        z_monitor_infrastructure  # Provides ITelemetryServer, IPatientLookupService
        Qt6::Core
        Qt6::Network
)

# Domain mocks link to domain library
target_link_libraries(zmon_test_mocks_domain
    PUBLIC
        z_monitor_domain  # Provides IPatientRepository, PatientAggregate
        zmon_domain_common  # Provides Result type
)
```

## Running Tests

Tests are registered with CTest and can be run via:

```bash
# Build tests
cmake --build build --target test_alarm_manager

# Run all tests
ctest

# Run specific test
./build/tests/unit/core/test_alarm_manager
```

## Example Test

See `tests/unit/core/test_alarm_manager.cpp` for an example of:
- Using mock objects
- Testing success and failure scenarios
- Testing async operations with Qt signals
- Using GoogleTest framework
