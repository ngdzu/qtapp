---
doc_id: DOC-REF-002
title: Error Codes Reference
version: 1.0
status: Approved
created: 2025-12-01
updated: 2025-12-01
category: Reference
tags: [error-codes, error-handling, reference, troubleshooting]
related:
  - DOC-GUIDE-011
  - DOC-REF-001
source: Result.h (domain/common)
---

# Error Codes Reference

## Overview

This reference document catalogs all error codes used in the Z Monitor application through the `Result<T, Error>` error handling pattern. Error codes are defined in the `ErrorCode` enum in `src/domain/common/Result.h`.

## Error Code Enumeration

### ErrorCode Enum

```cpp
enum class ErrorCode {
    None = 0,         ///< No error
    Unknown,          ///< Unknown / unspecified error
    InvalidArgument,  ///< Invalid input parameter
    AlreadyExists,    ///< Resource already exists
    NotFound,         ///< Resource not found
    PermissionDenied, ///< Insufficient permissions
    Unavailable,      ///< Service or resource temporarily unavailable
    Timeout,          ///< Operation timed out
    Conflict,         ///< Conflicting state (e.g., concurrent update)
    DatabaseError,    ///< Database-related error
    Internal          ///< Internal error (bug, invariant violation)
};
```

## Error Code Catalog

### None (0)

**Description:** No error - operation succeeded.

**Usage:** This code should never be returned in an `Error` struct. It exists only for completeness.

**Recovery Strategy:** N/A (success case)

---

### Unknown

**Description:** Unknown or unspecified error occurred.

**When Used:**
- Unexpected error from third-party library
- Error condition not covered by other error codes
- Fallback for unhandled error cases

**Example:**
```cpp
// When catching unexpected exception
try {
    processData();
} catch (const std::exception& e) {
    return Error::create(ErrorCode::Unknown, 
                        QString("Unexpected error: %1").arg(e.what()));
}
```

**Recovery Strategy:**
- Log error details for debugging
- Notify user of unexpected error
- Retry operation if appropriate
- Consider it a bug if occurs frequently

---

### InvalidArgument

**Description:** Invalid input parameter provided to function.

**When Used:**
- Null pointer passed when non-null expected
- Out-of-range numeric value
- Invalid string format (e.g., malformed MRN)
- Empty collection when non-empty expected
- Invalid enum value

**Examples:**
```cpp
// Invalid MRN format
Result<Patient, Error> findByMrn(const QString& mrn) {
    if (mrn.isEmpty() || mrn.length() != 6) {
        return Error::create(ErrorCode::InvalidArgument,
                            "MRN must be 6 digits");
    }
    // ...
}

// Null pointer check
Result<void, Error> setRepository(IPatientRepository* repo) {
    if (!repo) {
        return Error::create(ErrorCode::InvalidArgument,
                            "Repository cannot be null");
    }
    // ...
}
```

**Recovery Strategy:**
- Validate input before calling function
- Display error message to user
- Prompt user to correct input
- Use defensive programming (validate all inputs)

---

### AlreadyExists

**Description:** Resource already exists when attempting to create it.

**When Used:**
- Inserting duplicate patient MRN
- Creating user with existing user ID
- Adding alarm that already exists
- Duplicate database record

**Example:**
```cpp
Result<void, Error> admitPatient(const QString& mrn) {
    if (m_admittedPatients.contains(mrn)) {
        return Error::create(ErrorCode::AlreadyExists,
                            QString("Patient %1 is already admitted").arg(mrn));
    }
    // ...
}
```

**Recovery Strategy:**
- Check if resource exists before creating
- Update existing resource instead of creating
- Display error to user
- Ask user if they want to update instead

---

### NotFound

**Description:** Requested resource was not found.

**When Used:**
- Patient MRN not in database
- Alarm ID not found
- Configuration setting not found
- User ID not found
- Database query returns no results

**Examples:**
```cpp
// Patient not found
Result<Patient, Error> findByMrn(const QString& mrn) {
    auto result = m_database->query("SELECT * FROM patients WHERE mrn = ?", mrn);
    if (!result.next()) {
        return Error::create(ErrorCode::NotFound,
                            QString("Patient %1 not found").arg(mrn));
    }
    // ...
}

// Cache miss
Result<VitalsData, Error> get(const QString& patientMrn) {
    auto it = m_cache.find(patientMrn);
    if (it == m_cache.end()) {
        return Error::create(ErrorCode::NotFound,
                            "Vitals not in cache");
    }
    // ...
}
```

**Recovery Strategy:**
- Check if resource exists before accessing
- Create resource if appropriate
- Display "not found" message to user
- Offer to search for similar resources

---

### PermissionDenied

**Description:** User lacks necessary permissions for operation.

**When Used:**
- User role doesn't have required permission
- Attempting to access restricted resource
- Session expired or invalid
- Authentication required

**Example:**
```cpp
Result<void, Error> dischargePatient(const QString& mrn, const User& user) {
    if (!user.hasPermission(Permission::DischargePatient)) {
        return Error::create(ErrorCode::PermissionDenied,
                            "User lacks permission to discharge patients",
                            {{"userId", user.id}, {"requiredRole", "PHYSICIAN"}});
    }
    // ...
}
```

**Recovery Strategy:**
- Display "permission denied" message
- Suggest logging in with different account
- Contact administrator for permission
- Log security audit event

---

### Unavailable

**Description:** Service or resource is temporarily unavailable.

**When Used:**
- Database connection lost
- Network unavailable
- Service is down
- Resource being maintained
- Temporary outage

**Example:**
```cpp
Result<void, Error> sendTelemetry(const TelemetryBatch& batch) {
    if (!m_networkManager->isConnected()) {
        return Error::create(ErrorCode::Unavailable,
                            "Network connection unavailable");
    }
    // ...
}
```

**Recovery Strategy:**
- Retry operation after delay
- Use cached data if available
- Queue for later processing
- Display "service unavailable" message

---

### Timeout

**Description:** Operation exceeded allowed time limit.

**When Used:**
- Network request timeout
- Database query timeout
- User input timeout
- Waiting for lock timeout

**Example:**
```cpp
Result<Response, Error> sendRequest(const Request& req) {
    auto future = m_network->send(req);
    if (!future.waitForFinished(5000)) {  // 5 second timeout
        return Error::create(ErrorCode::Timeout,
                            "Request timed out after 5 seconds");
    }
    // ...
}
```

**Recovery Strategy:**
- Retry with longer timeout
- Cancel operation
- Display timeout message to user
- Check network connectivity

---

### Conflict

**Description:** Conflicting state prevents operation (e.g., concurrent update).

**When Used:**
- Concurrent modification of same resource
- State transition conflict
- Version mismatch (optimistic locking)
- Resource locked by another process

**Example:**
```cpp
Result<void, Error> discharge PatientManager(const QString& mrn) {
    auto patient = findByMrn(mrn);
    if (!patient.isAdmitted()) {
        return Error::create(ErrorCode::Conflict,
                            "Patient is not admitted, cannot discharge");
    }
    // ...
}
```

**Recovery Strategy:**
- Refresh data and retry
- Display conflict message to user
- Use optimistic locking with version field
- Implement conflict resolution UI

---

### DatabaseError

**Description:** Database-related error occurred.

**When Used:**
- SQL syntax error
- Database connection failed
- Transaction failed
- Constraint violation (foreign key, unique)
- Database corruption

**Example:**
```cpp
Result<void, Error> save(const Patient& patient) {
    QSqlQuery query;
    query.prepare("INSERT INTO patients VALUES (?, ?)");
    query.addBindValue(patient.mrn);
    query.addBindValue(patient.name);
    
    if (!query.exec()) {
        return Error::create(ErrorCode::DatabaseError,
                            "Failed to save patient",
                            {{"sqlError", query.lastError().text()}});
    }
    // ...
}
```

**Recovery Strategy:**
- Log database error details
- Retry operation
- Rollback transaction
- Check database connectivity
- Notify administrator if corruption

---

### Internal

**Description:** Internal error (bug, invariant violation, programming error).

**When Used:**
- Unreachable code path reached
- Invariant assertion failed
- Unexpected null pointer
- Logic error (should never happen)
- State corruption

**Example:**
```cpp
Result<void, Error> processState(State state) {
    switch (state) {
        case State::Ready:
        case State::Running:
        case State::Stopped:
            // Handle states...
            break;
        default:
            // Should never reach here
            return Error::create(ErrorCode::Internal,
                                "Invalid state - this is a bug",
                                {{"state", QString::number(static_cast<int>(state))}});
    }
}
```

**Recovery Strategy:**
- Log error with full context
- Report as bug
- Attempt graceful shutdown if critical
- Use assertions in debug builds (`Q_ASSERT`)

---

## Usage Patterns

### Creating Errors

```cpp
// Simple error
return Error::create(ErrorCode::NotFound, "Patient not found");

// Error with context
return Error::create(
    ErrorCode::DatabaseError,
    "Failed to save patient",
    {{"mrn", patient.mrn}, {"sqlError", query.lastError().text()}}
);
```

### Checking Errors

```cpp
auto result = repository->findByMrn(mrn);
if (result.isError()) {
    switch (result.error().code) {
        case ErrorCode::NotFound:
            // Handle not found
            break;
        case ErrorCode::DatabaseError:
            // Handle database error
            break;
        default:
            // Handle other errors
            break;
    }
}
```

### Error Context

Error context provides structured key-value pairs for debugging:

```cpp
Error error = result.error();
qDebug() << "Error:" << error.message;
for (const auto& [key, value] : error.context) {
    qDebug() << "  " << key << ":" << value;
}
```

## Related Documents

- [DOC-GUIDE-011: Error Handling Strategy](../guidelines/DOC-GUIDE-011_error_handling.md) - Error handling patterns and best practices
- [DOC-REF-001: Glossary](./DOC-REF-001_glossary.md) - Project terminology and definitions

## Extending Error Codes

**Note:** The `ErrorCode` enum is intentionally kept generic. Components are free to define more specific domain-level error enums if needed and map them into these generic categories when exposing errors via `Result`.

**Example:**
```cpp
// Domain-specific error enum
enum class PatientError {
    InvalidMRN,
    AlreadyAdmitted,
    NotAdmitted
};

// Map to generic ErrorCode
ErrorCode toErrorCode(PatientError err) {
    switch (err) {
        case PatientError::InvalidMRN:
            return ErrorCode::InvalidArgument;
        case PatientError::AlreadyAdmitted:
            return ErrorCode::AlreadyExists;
        case PatientError::NotAdmitted:
            return ErrorCode::Conflict;
    }
}
```
