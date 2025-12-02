---
doc_id: DOC-GUIDE-011
title: Error Handling Strategy
version: 1.0
status: Approved
created: 2025-12-01
updated: 2025-12-01
category: Guideline
tags: [error-handling, result-monad, exceptions, logging, patterns]
related:
  - DOC-GUIDE-012
  - DOC-REF-002
  - DOC-COMP-010
  - DOC-COMP-011
source: 20_ERROR_HANDLING_STRATEGY.md
---

# Error Handling Strategy

## Overview

This guideline defines the error handling patterns, strategies, and best practices for the Z Monitor application. The system uses a combination of `Result<T, Error>` monads for synchronous operations, Qt signals for asynchronous errors, and limited exceptions for truly exceptional circumstances.

## Core Principles

1. **Make Errors Explicit:** Errors are part of the function signature (`Result<T, Error>`)
2. **Handle Errors at the Right Layer:** Different layers handle errors differently
3. **Fail Fast:** Detect and report errors as early as possible
4. **Provide Context:** Error messages should include enough information for debugging
5. **Distinguish Severity:** Critical errors, recoverable errors, and warnings are handled differently

## Error Categories

### Recoverable Errors (Non-Critical)

**Definition:** Errors that can be handled by the application without terminating or affecting other operations.

**Examples:**
- Network timeouts (can retry)
- Validation failures (can prompt user to correct input)
- File read/write failures (can notify user)
- Database query failures (can retry or use cache)

**Handling Strategy:**
- Return `Result<T, Error>`
- Log at appropriate level (WARN or ERROR)
- Provide user feedback if applicable
- Allow operation to continue

**Example:**
```cpp
Result<PatientData, Error> loadPatient(const QString& mrn) {
    auto result = m_database->query("SELECT * FROM patients WHERE mrn = ?", mrn);
    if (result.isError()) {
        LogService::warn("Failed to load patient {}: {}", mrn, result.error().message());
        return Error(ErrorCode::DatabaseQueryFailed, "Patient not found in database");
    }
    return PatientData::fromRow(result.value());
}
```

### Non-Recoverable Errors (Critical)

**Definition:** Errors that indicate a serious problem requiring application shutdown or major intervention.

**Examples:**
- Database corruption
- Out-of-memory conditions
- Critical resource unavailable (e.g., cannot open log file)
- Configuration file missing or invalid

**Handling Strategy:**
- Log at FATAL level
- Emit critical error signal to UI
- Show critical error dialog to user
- Shut down gracefully if necessary

**Example:**
```cpp
void DatabaseManager::initialize() {
    if (!openDatabase()) {
        LogService::fatal("Cannot open database: {}", m_lastError);
        emit criticalError("Database initialization failed. Application cannot continue.");
        QCoreApplication::exit(1);
    }
}
```

### Programming Errors (Bugs)

**Definition:** Errors caused by incorrect logic or assumptions in the code (assertions, null checks).

**Examples:**
- Null pointer dereferences
- Out-of-bounds array access
- Invalid state transitions
- Contract violations (preconditions, postconditions)

**Handling Strategy:**
- Use `Q_ASSERT` in debug builds
- Use `Q_CHECK_PTR` for null checks
- Log at ERROR or FATAL level
- Terminate in debug builds, handle gracefully in release

**Example:**
```cpp
void MonitoringService::processVitals(const TelemetryBatch* batch) {
    Q_CHECK_PTR(batch);  // Programming error if null
    Q_ASSERT(!batch->samples.empty());  // Precondition
    
    for (const auto& sample : batch->samples) {
        // Process sample
    }
}
```

## Result<T, Error> Pattern

### When to Use Result<T, Error>

✅ **Use Result<T, Error> when:**
- Operation can fail for expected reasons (validation, I/O, network)
- Caller should decide how to handle the error
- Error is recoverable
- Synchronous operation

❌ **Do NOT use Result<T, Error> when:**
- Operation is asynchronous (use Qt signals instead)
- Error is truly exceptional (use exceptions)
- Error is a programming bug (use assertions)

### Result<T, Error> Basics

```cpp
// Success case
Result<int, Error> divide(int a, int b) {
    if (b == 0) {
        return Error(ErrorCode::DivisionByZero, "Cannot divide by zero");
    }
    return a / b;
}

// Usage
auto result = divide(10, 2);
if (result.isOk()) {
    qDebug() << "Result:" << result.value();
} else {
    qDebug() << "Error:" << result.error().message();
}
```

### Chaining with map() and flatMap()

```cpp
// Chain transformations
auto result = loadPatient(mrn)
    .map([](const PatientData& data) {
        return data.name;  // Transform success value
    })
    .flatMap([](const QString& name) {
        return validateName(name);  // Chain another Result
    });
```

### Early Return Pattern

```cpp
Result<void, Error> admitPatient(const AdmitPatientCommand& cmd) {
    // Validate input
    auto validationResult = validateCommand(cmd);
    if (validationResult.isError()) {
        return validationResult.error();  // Early return
    }
    
    // Load patient
    auto patientResult = m_repository->findByMrn(cmd.mrn);
    if (patientResult.isError()) {
        return patientResult.error();  // Early return
    }
    
    // Admit patient
    auto patient = patientResult.value();
    return patient.admit(cmd.identity);
}
```

## When to Return vs. Log vs. Emit Errors

### Decision Matrix

| Layer              | Recoverable Error                    | Non-Recoverable Error            | Programming Error             |
| ------------------ | ------------------------------------ | -------------------------------- | ----------------------------- |
| **Domain**         | Return `Result<T, Error>`            | Return `Result<T, Error>`        | `Q_ASSERT`                    |
| **Application**    | Return `Result<T, Error>` + log WARN | Log FATAL + emit signal          | `Q_ASSERT` + log ERROR        |
| **Infrastructure** | Return `Result<T, Error>` + log WARN | Log FATAL + emit signal          | `Q_ASSERT` + log ERROR        |
| **Interface**      | Show user feedback + log INFO        | Show critical dialog + log FATAL | Log ERROR + show error dialog |

### Return Errors (Always Explicit)

**When:**
- Synchronous operation that can fail
- Caller needs to decide how to handle the error
- Domain layer operations
- Repository operations

**Example:**
```cpp
// Domain layer
Result<void, Error> PatientAggregate::admit(const PatientIdentity& identity) {
    if (m_isAdmitted) {
        return Error(ErrorCode::InvalidState, "Patient already admitted");
    }
    m_identity = identity;
    m_isAdmitted = true;
    return Success();
}
```

### Log Errors (For Diagnostics)

**When:**
- Error should be recorded for debugging/audit
- Error severity is WARN, ERROR, or FATAL
- Infrastructure layer operations (database, network)
- Application service coordination failures

**Example:**
```cpp
// Infrastructure layer
Result<VitalsData, Error> VitalsCache::get(const QString& patientMrn) {
    auto it = m_cache.find(patientMrn);
    if (it == m_cache.end()) {
        LogService::warn("Cache miss for patient {}", patientMrn);
        return Error(ErrorCode::CacheMiss, "Vitals not in cache");
    }
    return it->second;
}
```

### Emit Errors (For Asynchronous Operations)

**When:**
- Asynchronous operation fails
- UI needs to be notified of error
- Error occurs in background thread
- Non-recoverable error requires user intervention

**Example:**
```cpp
// Application service
void MonitoringService::processTelemetryBatch(const TelemetryBatch& batch) {
    auto result = m_repository->save(batch);
    if (result.isError()) {
        LogService::error("Failed to save telemetry: {}", result.error().message());
        emit telemetryFailed(batch.patientMrn, result.error().message());
    }
}
```

## Layer-Specific Guidelines

### Domain Layer

**Responsibilities:**
- Return `Result<T, Error>` for all operations that can fail
- **NO** logging (domain layer is pure)
- **NO** Qt signals (domain layer is independent)
- Use `Q_ASSERT` for precondition violations (debug only)

**Example:**
```cpp
namespace zmon {
class PatientAggregate {
public:
    Result<void, Error> discharge() {
        if (!m_isAdmitted) {
            return Error(ErrorCode::InvalidState, "Patient not admitted");
        }
        m_isAdmitted = false;
        return Success();
    }
};
}
```

### Application Layer

**Responsibilities:**
- Return `Result<T, Error>` for synchronous operations
- Log errors at appropriate level (WARN, ERROR, FATAL)
- Emit Qt signals for asynchronous errors
- Coordinate error handling between domain and infrastructure

**Example:**
```cpp
namespace zmon {
class AdmissionService : public QObject {
public:
    Result<void, Error> admitPatient(const AdmitPatientCommand& cmd) {
        // Validate
        auto validationResult = validateCommand(cmd);
        if (validationResult.isError()) {
            LogService::warn("Admission validation failed: {}", validationResult.error().message());
            return validationResult.error();
        }
        
        // Load patient
        auto patientResult = m_patientRepo->findByMrn(cmd.mrn);
        if (patientResult.isError()) {
            LogService::error("Patient lookup failed: {}", patientResult.error().message());
            return patientResult.error();
        }
        
        // Admit
        auto patient = patientResult.value();
        auto admitResult = patient.admit(cmd.identity);
        if (admitResult.isError()) {
            LogService::error("Patient admission failed: {}", admitResult.error().message());
            return admitResult.error();
        }
        
        // Save
        auto saveResult = m_patientRepo->save(patient);
        if (saveResult.isError()) {
            LogService::fatal("Failed to save admitted patient: {}", saveResult.error().message());
            emit criticalError("Database save failed");
            return saveResult.error();
        }
        
        LogService::info("Patient {} admitted successfully", cmd.mrn);
        return Success();
    }
    
signals:
    void criticalError(const QString& message);
};
}
```

### Infrastructure Layer

**Responsibilities:**
- Return `Result<T, Error>` for repository operations
- Log infrastructure failures (database, network, I/O)
- Handle retries and timeouts
- Convert low-level errors to domain errors

**Example:**
```cpp
namespace zmon {
class SQLitePatientRepository : public IPatientRepository {
public:
    Result<PatientAggregate, Error> findByMrn(const QString& mrn) override {
        QSqlQuery query;
        query.prepare("SELECT * FROM patients WHERE mrn = ?");
        query.addBindValue(mrn);
        
        if (!query.exec()) {
            QString dbError = query.lastError().text();
            LogService::error("Database query failed: {}", dbError);
            return Error(ErrorCode::DatabaseQueryFailed, "Failed to load patient");
        }
        
        if (!query.next()) {
            LogService::warn("Patient {} not found in database", mrn);
            return Error(ErrorCode::NotFound, "Patient not found");
        }
        
        return PatientAggregate::fromRow(query.record());
    }
};
}
```

### Interface Layer

**Responsibilities:**
- Display errors to user (dialogs, notifications)
- Log user-visible errors at INFO or WARN level
- Connect to application service error signals
- Provide retry/recovery options to user

**Example:**
```cpp
namespace zmon {
class PatientController : public QObject {
    Q_OBJECT
    
public:
    Q_INVOKABLE void admitPatient(const QString& mrn, const QString& name) {
        AdmitPatientCommand cmd{mrn, name};
        
        auto result = m_admissionService->admitPatient(cmd);
        if (result.isError()) {
            LogService::info("User notified of admission failure: {}", result.error().message());
            emit errorOccurred("Admission failed: " + result.error().message());
            return;
        }
        
        emit patientAdmitted(mrn);
    }
    
signals:
    void errorOccurred(const QString& message);
    void patientAdmitted(const QString& mrn);
};
}
```

## Common Patterns

### Pattern 1: Validate-Load-Process-Save

```cpp
Result<void, Error> processOperation(const Command& cmd) {
    // 1. Validate
    auto validationResult = validate(cmd);
    if (validationResult.isError()) return validationResult.error();
    
    // 2. Load
    auto loadResult = load(cmd.id);
    if (loadResult.isError()) return loadResult.error();
    
    // 3. Process
    auto entity = loadResult.value();
    auto processResult = entity.process(cmd);
    if (processResult.isError()) return processResult.error();
    
    // 4. Save
    auto saveResult = save(entity);
    if (saveResult.isError()) return saveResult.error();
    
    return Success();
}
```

### Pattern 2: Try-Retry-Fallback

```cpp
Result<Data, Error> fetchDataWithRetry(const QString& id) {
    // Try primary source
    auto result = m_primarySource->fetch(id);
    if (result.isOk()) return result;
    
    // Log and retry
    LogService::warn("Primary fetch failed, retrying: {}", result.error().message());
    result = m_primarySource->fetch(id);
    if (result.isOk()) return result;
    
    // Fallback to cache
    LogService::error("Primary fetch failed after retry, using cache");
    return m_cache->get(id);
}
```

### Pattern 3: Batch Processing with Partial Failures

```cpp
struct BatchResult {
    std::vector<Success> succeeded;
    std::vector<Failure> failed;
};

BatchResult processBatch(const std::vector<Item>& items) {
    BatchResult result;
    
    for (const auto& item : items) {
        auto processResult = processItem(item);
        if (processResult.isOk()) {
            result.succeeded.push_back(item.id);
        } else {
            LogService::warn("Item {} failed: {}", item.id, processResult.error().message());
            result.failed.push_back({item.id, processResult.error()});
        }
    }
    
    return result;
}
```

## Anti-Patterns to Avoid

### ❌ Anti-Pattern 1: Swallowing Errors

```cpp
// ❌ BAD: Error is ignored
void processData() {
    auto result = loadData();
    if (result.isOk()) {
        // Process data
    }
    // Error case is completely ignored!
}

// ✅ GOOD: Error is logged or propagated
Result<void, Error> processData() {
    auto result = loadData();
    if (result.isError()) {
        LogService::error("Failed to load data: {}", result.error().message());
        return result.error();
    }
    // Process data
    return Success();
}
```

### ❌ Anti-Pattern 2: Generic Error Messages

```cpp
// ❌ BAD: No context
return Error(ErrorCode::Failed, "Operation failed");

// ✅ GOOD: Specific context
return Error(ErrorCode::ValidationFailed, 
             QString("Patient MRN '%1' is invalid: must be 6 digits").arg(mrn));
```

### ❌ Anti-Pattern 3: Exceptions for Control Flow

```cpp
// ❌ BAD: Exception for expected error
try {
    auto patient = findPatient(mrn);
} catch (PatientNotFoundException& e) {
    // Expected case, should not use exception
}

// ✅ GOOD: Result for expected error
auto result = findPatient(mrn);
if (result.isError()) {
    // Handle expected case
}
```

### ❌ Anti-Pattern 4: Logging Every Error Multiple Times

```cpp
// ❌ BAD: Same error logged 3 times
Result<void, Error> savePatient(const Patient& patient) {
    auto result = m_db->save(patient);
    if (result.isError()) {
        LogService::error("Save failed");  // ❌ Log #1
        return result.error();
    }
    return Success();
}

void service() {
    auto result = savePatient(patient);
    if (result.isError()) {
        LogService::error("Save failed");  // ❌ Log #2 (duplicate)
        emit errorOccurred("Save failed"); // ❌ Log #3 (duplicate)
    }
}

// ✅ GOOD: Log once at the appropriate layer
Result<void, Error> savePatient(const Patient& patient) {
    return m_db->save(patient);  // Return error without logging
}

void service() {
    auto result = savePatient(patient);
    if (result.isError()) {
        LogService::error("Failed to save patient {}: {}", 
                         patient.mrn, result.error().message());  // ✅ Log once with context
        emit errorOccurred("Failed to save patient data");
    }
}
```

## Related Documents

- [DOC-GUIDE-012: Logging Strategy](./DOC-GUIDE-012_logging.md) - Logging guidelines and log levels
- [DOC-REF-002: Error Codes Reference](../reference/DOC-REF-002_error_codes.md) - Complete error code catalog
- [DOC-COMP-010: MonitoringService](../components/DOC-COMP-010_monitoringservice.md) - Example error handling in application service
- [DOC-GUIDE-001: Code Organization](./DOC-GUIDE-001_code_organization.md) - DDD layer boundaries

## Enforcement

Error handling is enforced through:
- **Code Review:** All PRs must use `Result<T, Error>` for recoverable errors
- **Static Analysis:** Clang-tidy checks for unchecked `Result` values
- **Unit Tests:** Test both success and error paths
