# C++ Implementation Guidelines

## Overview

This document defines the C++ coding standards and best practices for the Z Monitor project. These guidelines ensure code consistency, maintainability, and alignment with modern C++ and Qt 6 practices.

**Key Principles:**
- **C++17 Standard** - Target C++17 throughout the project
- **Qt 6 APIs** - Use modern Qt 6 features, avoid deprecated Qt 5 APIs
- **Domain-Driven Design** - Respect DDD layer boundaries (domain, application, infrastructure, interface)
- **Type Safety** - Prefer strong types, avoid `void*` and raw casts
- **RAII First** - Always prefer automatic resource management
- **Documentation Required** - All public APIs must have Doxygen comments
- **No Hardcoded Values** - Never hardcode runtime data in production code

---

## 1. Language Standard and Compiler Settings

### C++ Standard

- **Target C++17** throughout the project
- Use `CMAKE_CXX_STANDARD 17` and `CMAKE_CXX_STANDARD_REQUIRED ON` in CMake
- Do not use C++20 features (maintain C++17 compatibility)

### Compiler Warnings

- **Enable all warnings**: `-Wall -Wextra -Wpedantic` (or equivalent)
- **Treat warnings as errors in CI/CD**: `-Werror` (or `CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror"`)
- **Fix warnings immediately** - Do not suppress warnings with pragmas unless absolutely necessary

### Code Generation

- **Optimize for correctness first** - Use `-O0` or `-O1` for debug builds
- **Enable debug symbols**: `-g` for all debug builds
- **Use LTO for release builds** - Link-time optimization for performance

---

## 2. Code Organization and Structure

### File Organization

- **One class per header**: Each header file should define one primary class/interface
- **Keep translation units focused**: Each `.cpp` should correspond to a single class or cohesive feature
- **Follow DDD layer structure**: Organize files by layer (domain, application, infrastructure, interface)

### Header File Guidelines

- **Use `#pragma once`** in headers (preferred over include guards for simplicity)
- **Include order** (maintain consistency):
  1. Own header (for `.cpp` files)
  2. Project headers (domain, application, infrastructure, interface)
  3. Qt headers
  4. STL/system headers

### Include Paths

- **Avoid relative include paths**: Do not use relative include paths such as:
  ```cpp
  // ❌ Bad
  #include "../path/to/header.h"
  #include "../../other/Thing.h"
  ```
- **Use project-relative paths**: Add paths to include search path via CMake (`target_include_directories`), then use:
  ```cpp
  // ✅ Good
  #include "domain/monitoring/PatientAggregate.h"
  #include "infrastructure/persistence/DatabaseManager.h"
  ```
- **Benefits**: Centralizes path management, prevents breakage when files are moved

### Forward Declarations

- **Use forward declarations in headers** to reduce compile-time dependencies:
  ```cpp
  // ✅ Good: Forward declaration
  class DatabaseManager;
  class MyClass {
      DatabaseManager* m_db;  // Use pointer/reference, not value
  };
  
  // ❌ Bad: Full include in header
  #include "infrastructure/persistence/DatabaseManager.h"
  class MyClass {
      DatabaseManager m_db;  // Requires full definition
  };
  ```

---

## 3. Naming Conventions

### Namespaces

- **Use short, lowercase namespace names**: Keep namespace names concise
  ```cpp
  // ✅ Good
  namespace zmon { namespace infra { namespace sensors { ... } } }
  
  // ❌ Bad (too verbose)
  namespace ZMonitor { namespace Infrastructure { namespace Sensors { ... } } }
  ```

### Classes and Types

- **Class names**: Use `PascalCase` (e.g., `SecurityService`, `DatabaseManager`)
- **Type aliases**: Use `PascalCase` (e.g., `using ResultType = Result<T, Error>`)

### Functions and Methods

- **Function/method names**: Use `camelCase` (e.g., `getCurrentUserId()`, `logAction()`)
- **Use `[[nodiscard]]`** for functions returning values that must be handled:
  ```cpp
  [[nodiscard]] bool isValid() const;
  [[nodiscard]] std::unique_ptr<Data> loadData();
  ```

### Variables

- **Member variables**: Prefix with `m_` (e.g., `m_userId`, `m_settingsManager`)
- **Local variables**: Use `camelCase` (e.g., `userId`, `settingsManager`)
- **Constants**: Use `UPPER_SNAKE_CASE` (e.g., `MAX_RETRY_ATTEMPTS`, `DEFAULT_TIMEOUT_MS`)

### Private Members

- **Member variables**: Use `m_` prefix
- **Member functions**: No prefix needed (just make them private)

---

## 4. Constants and Magic Numbers

### No Hardcoded Values in Production Code

- **NEVER use hardcoded values in production code** - Always use variables, parameters, or constants that come from the actual runtime context
- **Hardcoded values hide bugs**, break functionality, make debugging impossible, and violate DRY principle

### When Hardcoded Values Are Acceptable

Hardcoded values are **ONLY acceptable** in:

1. **Test Code:** Test fixtures, mock data, unit test examples
   ```cpp
   // ✅ OK: Test code with hardcoded test data
   TEST(PatientLookup, Success) {
       MockPatientLookupService service;
       service.addMockPatient(PatientIdentity{
           .mrn = "MRN-12345",  // ✅ OK: Test fixture
           .name = "John Doe"
       });
   }
   ```

2. **Named Constants:** Configuration values with meaningful names
   ```cpp
   // ✅ OK: Named constant for configuration
   constexpr int MAX_RETRY_ATTEMPTS = 3;
   constexpr int DEFAULT_TIMEOUT_MS = 5000;
   ```

3. **Default Values:** Sensible defaults that can be overridden
   ```cpp
   // ✅ OK: Default value that can be overridden
   int timeout = getTimeoutSetting("network.timeout", 5000);  // Default: 5000ms
   ```

4. **Documentation Examples:** When clearly marked as example/test data
   ```cpp
   // ✅ OK: Documentation example (clearly marked)
   // Example: Lookup patient
   QString exampleMrn = "MRN-12345";  // Example value
   auto future = service->lookupPatient(exampleMrn);
   ```

### Common Hardcoded Value Patterns to Avoid

| Pattern               | ❌ Bad                            | ✅ Good                                     |
| --------------------- | -------------------------------- | ------------------------------------------ |
| **Logging**           | `{"mrn", "MRN-12345"}`           | `{"mrn", mrn}`                             |
| **Function Calls**    | `lookupPatient("MRN-12345")`     | `lookupPatient(mrn)`                       |
| **Object Properties** | `batch.patientMrn = "MRN-12345"` | `batch.patientMrn = m_currentPatientMrn`   |
| **Error Messages**    | `"Patient MRN-12345 not found"`  | `QString("Patient %1 not found").arg(mrn)` |
| **Database Queries**  | `WHERE mrn = 'MRN-12345'`        | `WHERE mrn = :mrn` (with bindValue)        |

### Examples of What NOT to Do

#### ❌ BAD: Hardcoded Values in Logging

```cpp
// ❌ BAD: Hardcoded MRN in logging
connect(lookupService, &IPatientLookupService::lookupCompleted,
        this, [](const QString& mrn, const Result<PatientIdentity>& result) {
    if (result.isSuccess()) {
        m_logService->info("Patient lookup succeeded", {
            {"mrn", "MRN-12345"},  // ❌ WRONG: Hardcoded value!
            {"name", patient.name}
        });
    } else {
        m_logService->warning("Patient lookup failed", {
            {"mrn", "MRN-12345"},  // ❌ WRONG: Hardcoded value!
            {"error", result.error().message}
        });
    }
});
```

**Problems:**
- Logs will always show "MRN-12345" even if the actual MRN is different
- Makes debugging impossible - logs don't reflect reality
- If the actual lookup fails for a different MRN, logs are misleading

#### ❌ BAD: Hardcoded Values in Function Calls

```cpp
// ❌ BAD: Hardcoded patient MRN
void MonitoringService::sendTelemetry() {
    TelemetryBatch batch;
    batch.patientMrn = "MRN-12345";  // ❌ WRONG: Hardcoded!
    batch.deviceId = "ZM-ICU-MON-04";  // ❌ WRONG: Hardcoded!
    
    m_telemetryServer->sendBatch(batch);
}
```

**Problems:**
- Always sends data for the same patient, regardless of actual patient
- Device ID is wrong if device changes
- Breaks multi-patient scenarios

### ✅ GOOD: Use Variables and Parameters

#### ✅ GOOD: Use Variables from Context

```cpp
// ✅ GOOD: Use variable from lambda parameter
connect(lookupService, &IPatientLookupService::lookupCompleted,
        this, [this](const QString& mrn, const Result<PatientIdentity>& result) {
    if (result.isSuccess()) {
        m_logService->info("Patient lookup succeeded", {
            {"mrn", mrn},  // ✅ Use variable from parameter
            {"name", patient.name}
        });
    } else {
        m_logService->warning("Patient lookup failed", {
            {"mrn", mrn},  // ✅ Use variable from parameter
            {"error", result.error().message}
        });
    }
});
```

#### ✅ GOOD: Use Member Variables or Injected Dependencies

```cpp
// ✅ GOOD: Use member variables from context
void MonitoringService::sendTelemetry() {
    TelemetryBatch batch;
    batch.patientMrn = m_currentPatientMrn;  // ✅ From current patient context
    batch.deviceId = m_deviceId;  // ✅ From settings/configuration
    
    m_telemetryServer->sendBatch(batch);
}
```

#### ✅ GOOD: Use Function Parameters

```cpp
// ✅ GOOD: Use function parameter
void MonitoringService::lookupPatient(const QString& mrn) {
    auto future = m_lookupService->lookupPatient(mrn);
    
    future.then([this, mrn](const Result<PatientIdentity>& result) {
        if (result.isSuccess()) {
            m_logService->info("Patient lookup succeeded", {
                {"mrn", mrn}  // ✅ Use parameter
            });
        }
    });
}
```

### Constant Organization

- **Use named constants**: Replace magic numbers and strings with named constants:
  ```cpp
  // ❌ Bad
  if (timeout > 5000) { ... }
  
  // ✅ Good
  constexpr int DEFAULT_TIMEOUT_MS = 5000;
  if (timeout > DEFAULT_TIMEOUT_MS) { ... }
  ```

- **Group related constants**: Use `namespace` or `class` with `static constexpr` members:
  ```cpp
  namespace Config {
      constexpr int MAX_RETRY_ATTEMPTS = 3;
      constexpr int DEFAULT_TIMEOUT_MS = 5000;
  }
  
  // Or in class
  class NetworkManager {
      static constexpr int MAX_RETRY_ATTEMPTS = 3;
  };
  ```

### String Constants

- **Avoid hardcoded strings**: Use constants or configuration:
  ```cpp
  // ❌ Bad
  if (role == "nurse") { ... }
  
  // ✅ Good
  namespace Roles {
      constexpr const char* NURSE = "nurse";
      constexpr const char* PHYSICIAN = "physician";
  }
  if (role == Roles::NURSE) { ... }
  ```

---

## 5. Memory Management

### RAII (Resource Acquisition Is Initialization)

- **Favor RAII**: Always prefer automatic resource management
- **Wrap ownership in smart pointers**: Use `std::unique_ptr` or `std::shared_ptr`
- **Avoid raw `new`/`delete`**: Encapsulate dynamic allocation in helpers or use stack-based objects

### Qt Parent-Child Ownership

- **Use Qt's parent-child relationships** for `QObject` hierarchies:
  ```cpp
  QWidget* parent = new QWidget;
  QPushButton* button = new QPushButton("Click", parent);
  // button automatically deleted when parent is deleted
  ```
- **Rule of thumb**: Use Qt parent-child for `QObject` hierarchies; use smart pointers for plain C++ objects

### Smart Pointers

- **`std::unique_ptr`**: For exclusive ownership
  ```cpp
  auto data = std::make_unique<MyClass>();
  ```
- **`std::shared_ptr`**: For shared ownership (use sparingly)
  ```cpp
  auto shared = std::make_shared<MyClass>();
  ```
- **Avoid `std::auto_ptr`**: Deprecated, use `std::unique_ptr` instead

### Memory Safety

- **No memory leaks**: Use RAII, smart pointers, or Qt parent-child
- **No use-after-free**: Ensure proper ownership semantics
- **No double-delete**: Use smart pointers or Qt parent-child

---

## 6. Modern C++ Features (C++17)

### Type Deduction

- **Use `auto`** when the type is obvious from context:
  ```cpp
  auto data = std::make_unique<MyClass>();  // ✅ Good
  auto count = items.size();                 // ✅ Good
  
  // ❌ Bad: Unclear type
  auto result = process();  // What type is result?
  ```

### Range-Based For Loops

- **Prefer range-based for loops** over traditional loops:
  ```cpp
  // ✅ Good
  for (const auto& item : items) {
      process(item);
  }
  
  // ❌ Bad (unless index needed)
  for (size_t i = 0; i < items.size(); ++i) {
      process(items[i]);
  }
  ```

### Structured Bindings

- **Use structured bindings** for unpacking pairs/tuples:
  ```cpp
  auto [key, value] = map.find(key);
  auto [first, second] = getPair();
  ```

### Optional Values

- **Use `std::optional`** for values that may or may not exist:
  ```cpp
  std::optional<QString> getValue();
  if (auto value = getValue()) {
      use(*value);
  }
  ```

### Lambda Expressions

- **Use lambdas** for callbacks, signal handlers, and small inline functions:
  ```cpp
  auto callback = [this, &data](int value) {
      processData(data, value);
  };
  ```

### Constexpr

- **Use `constexpr`** for compile-time constants and simple functions:
  ```cpp
  constexpr int MAX_RETRY_ATTEMPTS = 3;
  constexpr int calculate(int x) { return x * 2; }
  ```

### Noexcept

- **Use `noexcept`** for functions that guarantee not to throw:
  ```cpp
  void moveOperation() noexcept;
  ```

---

## 7. Const Correctness and Function Attributes

### Const Correctness

- **Mark methods that don't modify state as `const`**:
  ```cpp
  bool isValid() const;
  QString getName() const;
  ```
- **Mark parameters that aren't modified as `const`**:
  ```cpp
  void process(const QString& data);  // ✅ Good
  void process(QString& data);        // ❌ Bad if not modified
  ```

### Function Attributes

- **`[[nodiscard]]`**: For functions returning values that must be handled
- **`constexpr`**: For compile-time evaluation
- **`noexcept`**: For functions that don't throw exceptions
- **`override`**: Always use when overriding virtual functions
- **`final`**: Use when class/method should not be further derived/overridden

---

## 8. Error Handling

### Error Handling Strategy

- **Use `Result<T, Error>` pattern** for operations that can fail (see `domain/common/Result.h`)
- **Return error codes**: Use `bool`, `std::optional<T>`, or `Result<T, Error>` to indicate failure
- **Emit signals**: For async operations, emit error signals (e.g., `errorOccurred(const QString& message)`)
- **Use exceptions sparingly**: Prefer return codes for expected error conditions

### Do NOT Use Qt Debug Functions for Error Handling

- **`qWarning`, `qDebug`, `qInfo` do NOT handle errors** - they only log and continue execution
- **Instead, handle errors explicitly**:
  ```cpp
  // ❌ Bad: Error is logged but not handled
  if (!database.open()) {
      qWarning() << "Failed to open database";
      // Execution continues, but database is not open!
  }
  
  // ✅ Good: Error is returned to caller
  Result<void> openDatabase() {
      if (!database.open()) {
          return Result<void>::error(
              Error::create(ErrorCode::DatabaseError, "Cannot open database")
          );
      }
      return Result<void>::ok();
  }
  
  // ✅ Good: Error is emitted via signal (for async operations)
  void connectToServer() {
      if (!m_networkManager->connect()) {
          emit connectionFailed("Failed to connect to server");
          return;
      }
      emit connected();
  }
  
  // ✅ Good: Error is logged via proper logging infrastructure
  if (!saveToDatabase(data)) {
      m_auditRepo->logError("Database save failed", details);
      return false;
  }
  ```

### Error Propagation

- **Propagate errors to callers**: Don't swallow errors
- **Log errors at appropriate layer**: 
  - Domain layer: Return errors only (no logging)
  - Application layer: Log infrastructure failures (before returning)
  - Infrastructure layer: Log all failures (before returning)

### Qt Error Handling

- **Use Qt's error mechanisms** appropriately:
  - `QSqlError` for database errors
  - `QNetworkReply::error()` for network errors
  - Convert to `Result<T, Error>` for consistent error handling

---

## 9. Qt-Specific Guidelines

### Qt Version

- **Target Qt 6 APIs**: Use modern Qt 6 features
- **Avoid deprecated Qt 5 APIs**: Check Qt 6 migration guide for deprecated features

### Signals and Slots

- **Use new-style signals/slots**: Prefer function/lambda overloads with context objects:
  ```cpp
  // ✅ Good: Type-safe, compile-time checked
  QObject::connect(&button, &QPushButton::clicked, this, &MyClass::onButtonClicked);
  QObject::connect(&button, &QPushButton::clicked, [this]() { handleClick(); });
  
  // ❌ Bad: String-based (runtime errors, no type checking)
  connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
  ```

### QObject Macros

- **Always include `Q_OBJECT`** in classes that use signals/slots or Qt's meta-object system
- **Use `Q_PROPERTY`** for exposing C++ properties to QML
- **Use `Q_INVOKABLE`** for methods callable from QML

### Qt Containers

- **Qt containers are copy-on-write**: Qt containers (`QString`, `QList`, etc.) use implicit sharing, so copying is cheap
- **Still prefer moves** for clarity and when transferring ownership:
  ```cpp
  QStringList list;
  list.append(std::move(largeString));  // Move, don't copy
  ```

### Qt Parent-Child Ownership

- **Leverage parent-child ownership**: Use Qt's parent-child relationships instead of manual deletes for `QObject` hierarchies
- **Automatic cleanup**: Child objects are automatically deleted when parent is deleted

---

## 10. Thread Safety

### Thread Safety Documentation

- **Document thread safety**: Clearly document which methods are thread-safe and which require specific thread context
- **Use Doxygen `@thread` tag**: Document thread requirements in API documentation

### Threading Primitives

- **Use Qt's threading primitives**: Prefer `QThread`, `QMutex`, `QReadWriteLock`, `QSemaphore` over raw `std::thread` when working with Qt objects
- **Signal-slot thread safety**: Use `Qt::QueuedConnection` for cross-thread communication

### Thread Safety Patterns

- **Immutable objects**: Make objects immutable for thread safety
- **Thread-local storage**: Use thread-local storage when appropriate
- **Lock-free data structures**: Use lock-free queues for high-performance scenarios

---

## 11. Performance

### Performance Philosophy

- **Avoid premature optimization**: Write clear, correct code first; optimize only when profiling shows it's necessary
- **Measure before optimizing**: Use profiling tools to identify bottlenecks

### Move Semantics

- **Prefer moving large objects** instead of copying:
  ```cpp
  QStringList list;
  list.append(std::move(largeString));  // Move, don't copy
  ```

### Container Optimization

- **Reserve capacity**: Use `reserve()` for containers when you know the approximate size:
  ```cpp
  QList<Item> items;
  items.reserve(1000);  // Avoid reallocations
  ```

### Hot Path Optimization

- **Optimize hot paths**: Focus optimization efforts on frequently executed code
- **Avoid allocations in hot paths**: Pre-allocate buffers, use stack allocation when possible

---

## 12. Documentation

### Code Comments

- **Document tricky logic**: Use concise comments explaining the "why," not the "what"
- **Self-documenting code**: Prefer clear variable and function names over comments:
  ```cpp
  // ❌ Bad
  int x = 5;  // timeout in seconds
  
  // ✅ Good
  int timeoutSeconds = 5;
  ```

### API Documentation (CRITICAL)

- **ALL public classes, methods, properties, and enums MUST have Doxygen-style comments (`/** */`)**
- **See `.github/instructions/api_documentation.mdc`** for complete API documentation requirements
- **Documentation is required from the beginning** - not optional
- **Code reviews will reject code without proper API documentation**

### Doxygen Comments

- **Use Doxygen-style comments** for public APIs:
  ```cpp
  /**
   * @brief Opens a database connection.
   * 
   * @param dbPath Path to SQLite database file
   * @param encryptionKey Optional encryption key for SQLCipher
   * @return Result<void> Success or error details
   * 
   * @note Connection is asynchronous. Monitor connectionStatusChanged()
   *       signal for connection state updates.
   * 
   * @see close()
   * @see connectionStatusChanged()
   */
  Result<void> open(const QString& dbPath, const QString& encryptionKey = QString());
  ```

### Documentation Tags

- **Required tags**: `@brief`, `@param`, `@return`, `@note`
- **Optional tags**: `@see`, `@sa`, `@code`, `@endcode`, `@example`
- **Thread safety**: Use `@thread` tag to document thread requirements

---

## 13. Code Examples and Patterns

### Example Quality

- **Keep examples focused**: Avoid unnecessary complexity
- **Each example should demonstrate one concept clearly**
- **Use consistent style**: Follow the same patterns throughout the codebase

### Common Patterns

- **RAII pattern**: Always use RAII for resource management
- **Factory pattern**: Use factories for object creation when appropriate
- **Dependency injection**: Inject dependencies via constructor
- **Interface segregation**: Prefer small, focused interfaces

---

## 14. Best Practices Summary

### Top 10 Rules

1. **RAII first**: Always prefer automatic resource management
2. **Const correctness**: Mark everything that can be `const` as `const`
3. **Type safety**: Use strong types, avoid `void*` and raw casts
4. **Modern C++**: Leverage C++17 features (auto, lambdas, structured bindings, optional)
5. **Qt idioms**: Use Qt's parent-child ownership, new-style signals/slots, and Qt 6 APIs
6. **Clear naming**: Use descriptive names that explain intent
7. **No magic numbers**: Use named constants for all literal values
8. **No hardcoded runtime data**: Always use variables, parameters, or injected dependencies
9. **Documentation**: Document public APIs and non-obvious logic
10. **Thread awareness**: Document and respect thread safety requirements

### Quick Reference

| Topic                | Guideline                                                       |
| -------------------- | --------------------------------------------------------------- |
| **C++ Standard**     | C++17                                                           |
| **Memory**           | RAII, smart pointers, Qt parent-child                           |
| **Naming**           | `PascalCase` classes, `camelCase` functions, `m_` members       |
| **Constants**        | No magic numbers, use named constants                           |
| **Hardcoded Values** | Never hardcode runtime data (IDs, patient MRN, device ID)       |
| **Error Handling**   | `Result<T, Error>`, signals for async, no `qWarning` for errors |
| **Documentation**    | Doxygen comments required for all public APIs                   |
| **Qt Version**       | Qt 6 APIs only                                                  |
| **Thread Safety**    | Document thread requirements, use Qt primitives                 |
| **Performance**      | Correctness first, optimize hot paths only                      |

### Code Review Checklist

When reviewing code, verify:

- [ ] No hardcoded IDs (patient MRN, device ID, user ID) in production code
- [ ] No hardcoded values in logging calls (use variables from context)
- [ ] No hardcoded values in function calls (use parameters or member variables)
- [ ] All runtime data comes from variables, parameters, or injected dependencies
- [ ] Test code clearly separated (hardcoded test data is OK in tests)
- [ ] Doxygen comments on all public APIs
- [ ] Const correctness (methods and parameters marked `const` where appropriate)
- [ ] RAII used for resource management
- [ ] Error handling uses `Result<T, Error>` or signals
- [ ] No use of `qWarning`/`qDebug`/`qInfo` for error handling

---

## Related Guidelines

- **API Documentation**: See `.github/instructions/api_documentation.mdc` for complete API documentation requirements
- **Domain-Driven Design**: See `doc/architecture/DOC-ARCH-028_domain_driven_design.md` for DDD principles
- **Error Handling**: See `doc/20_ERROR_HANDLING_STRATEGY.md` for error handling strategy
- **Code Organization**: See `doc/22_CODE_ORGANIZATION.md` for code organization guidelines
- **Logging Strategy**: See `doc/21_LOGGING_STRATEGY.md` for structured logging with context
- **Dependency Injection**: See `doc/13_DEPENDENCY_INJECTION.md` for using DI for runtime values

---

## Enforcement

- **Code Review**: All code must follow these guidelines
- **Linting**: Automated linting checks enforce style and best practices
- **Documentation**: Public APIs without Doxygen comments will be rejected
- **Hardcoded Values**: PRs with hardcoded runtime data in production code will be rejected
- **CI/CD**: Builds fail on warnings (when `-Werror` enabled)

---

**Remember:** If you find yourself typing a literal value that represents runtime data (patient ID, device ID, user ID, etc.), **STOP** and use a variable instead. Hardcoded values in production code are bugs waiting to happen.
