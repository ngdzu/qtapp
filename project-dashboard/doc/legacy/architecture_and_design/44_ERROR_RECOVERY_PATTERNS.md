# Error Recovery Patterns: Implementation and Usage

**Document ID:** DESIGN-044  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-01-15

---

This document describes how Z Monitor implements and uses error recovery patterns (retry with exponential backoff and circuit breaker) to build resilient systems that handle transient failures and prevent cascading failures.

## 1. Overview

Z Monitor implements two complementary error recovery patterns:

1. **RetryPolicy** - Retry failed operations with exponential backoff
2. **CircuitBreaker** - Prevent calling persistently failing services

These patterns are implemented as pure C++ utilities in the domain layer (`domain/common/`) and can be used across all layers of the application.

**Related Documents:**
- `20_ERROR_HANDLING_STRATEGY.md` - Overall error handling strategy
- `../foundation/06_error_handling_and_resilience/03_circuit_breaker.md` - Circuit breaker foundational knowledge
- `../foundation/06_error_handling_and_resilience/04_retry_backoff.md` - Retry with exponential backoff foundational knowledge

---

## 2. RetryPolicy Implementation

### 2.1. Location and Design

**File:** `z-monitor/src/domain/common/RetryPolicy.h`

**Design decisions:**
- Pure C++ (no Qt dependencies) - can be used in domain layer
- Header-only implementation
- Template-based for type safety
- Configurable parameters (max retries, base delay, max delay)

### 2.2. API

```cpp
class RetryPolicy {
public:
    // Default: 3 retries, 100ms base, 5000ms max
    RetryPolicy();
    
    // Custom parameters
    RetryPolicy(int maxRetries, int baseDelayMs, int maxDelayMs);
    
    // Execute operation with retry
    template<typename T>
    Result<T> executeWithRetry(std::function<Result<T>()> operation);
};
```

### 2.3. Retryable Error Codes

The `RetryPolicy` automatically determines which errors are retryable:

**Retryable (transient failures):**
- `ErrorCode::Timeout` - Operation timed out
- `ErrorCode::Unavailable` - Service temporarily unavailable
- `ErrorCode::DatabaseError` - Database lock or connection issue

**Non-retryable (permanent failures):**
- `ErrorCode::InvalidArgument` - Invalid input
- `ErrorCode::NotFound` - Resource doesn't exist
- `ErrorCode::PermissionDenied` - Insufficient permissions
- `ErrorCode::AlreadyExists` - Resource already exists
- `ErrorCode::Conflict` - Conflicting state
- `ErrorCode::Internal` - Internal error (bug)

### 2.4. Exponential Backoff Calculation

**Formula:**
```
delay = baseDelay * (2 ^ attemptNumber)
delay = min(delay, maxDelay)  // Cap at max delay
```

**Example with baseDelay=100ms, maxDelay=5000ms:**
- Attempt 1 → 2: 100ms
- Attempt 2 → 3: 200ms
- Attempt 3 → 4: 400ms
- Attempt 4 → 5: 800ms
- Attempt 5 → 6: 1600ms
- Attempt 6 → 7: 3200ms
- Attempt 7 → 8: 5000ms (capped)

### 2.5. Usage Examples

#### Example 1: Network Service Call

```cpp
RetryPolicy retryPolicy(3, 100, 5000);  // 3 retries, 100ms base, 5s max

auto result = retryPolicy.executeWithRetry([&]() -> Result<ServerResponse> {
    return networkService.sendTelemetry(batch);
});

if (result.isError()) {
    // All retries exhausted or non-retryable error
    logError("Telemetry send failed after retries", result.error());
}
```

#### Example 2: Database Operation

```cpp
RetryPolicy dbRetryPolicy(5, 50, 2000);  // 5 retries, 50ms base, 2s max

auto result = dbRetryPolicy.executeWithRetry([&]() -> Result<void> {
    return patientRepo->save(patient);
});

if (result.isError()) {
    // Database lock or connection issue
    logError("Failed to save patient after retries", result.error());
}
```

### 2.6. Configuration Guidelines

**Network operations:**
- Max retries: 3-5
- Base delay: 100-500ms
- Max delay: 5-10 seconds

**Database operations:**
- Max retries: 5-10
- Base delay: 50-100ms
- Max delay: 2-5 seconds

**User-facing operations:**
- Max retries: 2-3 (fail fast for better UX)
- Base delay: 100-200ms
- Max delay: 1-2 seconds

---

## 3. CircuitBreaker Implementation

### 3.1. Location and Design

**File:** `z-monitor/src/domain/common/CircuitBreaker.h`

**Design decisions:**
- Pure C++ (no Qt dependencies) - can be used in domain layer
- Header-only implementation
- Thread-safe (uses atomic operations and mutex)
- Three-state state machine (Closed, Open, HalfOpen)

### 3.2. API

```cpp
enum class CircuitState {
    Closed,    // Normal operation
    Open,      // Service failing - fail fast
    HalfOpen   // Testing recovery
};

class CircuitBreaker {
public:
    // Default: 5 failures, 60s timeout, 3 half-open requests
    CircuitBreaker();
    
    // Custom parameters
    CircuitBreaker(int failureThreshold, int resetTimeoutSeconds, int halfOpenMaxRequests);
    
    // Execute operation with circuit breaker protection
    template<typename T>
    Result<T> execute(std::function<Result<T>()> operation);
    
    // Get current state
    CircuitState getState() const;
    
    // Get failure count
    int getFailureCount() const;
    
    // Manual reset (for testing/recovery)
    void reset();
};
```

### 3.3. State Machine

```
┌─────────┐
│ Closed  │ ←──────────────┐
└────┬────┘                │
     │                     │
     │ (5 failures)        │ (3 successes)
     ▼                     │
┌─────────┐                │
│  Open   │                │
└────┬────┘                │
     │                     │
     │ (60s timeout)       │
     ▼                     │
┌─────────┐                │
│HalfOpen │ ───────────────┘
└─────────┘
     │
     │ (any failure)
     ▼
┌─────────┐
│  Open   │
└─────────┘
```

### 3.4. State Transitions

**Closed → Open:**
- Trigger: `failureCount >= failureThreshold` (default: 5)
- Behavior: All subsequent requests fail immediately

**Open → HalfOpen:**
- Trigger: `resetTimeoutSeconds` elapsed (default: 60s)
- Behavior: Limited requests allowed (default: 3) to test recovery

**HalfOpen → Closed:**
- Trigger: `successCount >= halfOpenMaxRequests` (default: 3)
- Behavior: Circuit closes, normal operation resumes

**HalfOpen → Open:**
- Trigger: Any failure in half-open state
- Behavior: Circuit opens immediately (service still failing)

### 3.5. Usage Examples

#### Example 1: Network Service with Circuit Breaker

```cpp
CircuitBreaker networkBreaker(5, 60, 3);  // 5 failures, 60s timeout, 3 half-open

auto result = networkBreaker.execute([&]() -> Result<ServerResponse> {
    return telemetryServer->sendBatch(batch);
});

if (result.isError()) {
    if (networkBreaker.getState() == CircuitState::Open) {
        // Circuit is open - service is down, don't retry
        logWarning("Circuit breaker is open - service unavailable");
    } else {
        // Operation failed but circuit is closed/half-open
        logError("Telemetry send failed", result.error());
    }
}
```

#### Example 2: Patient Lookup Service with Circuit Breaker + Retry

```cpp
CircuitBreaker lookupBreaker(5, 60, 3);
RetryPolicy retryPolicy(3, 100, 5000);

auto result = lookupBreaker.execute([&]() -> Result<PatientInfo> {
    return retryPolicy.executeWithRetry([&]() -> Result<PatientInfo> {
        return patientLookupService->lookupPatient(mrn);
    });
});

if (result.isError()) {
    if (lookupBreaker.getState() == CircuitState::Open) {
        // Use cached patient data as fallback
        return getCachedPatient(mrn);
    }
    return result;
}
```

### 3.6. Configuration Guidelines

**External network services:**
- Failure threshold: 5-10 consecutive failures
- Reset timeout: 60-120 seconds
- Half-open max requests: 3-5

**Critical services (patient lookup):**
- Failure threshold: 3-5 (fail fast for critical services)
- Reset timeout: 30-60 seconds (test recovery more frequently)
- Half-open max requests: 2-3 (fewer test requests)

**Non-critical services:**
- Failure threshold: 10-20 (more tolerant)
- Reset timeout: 120-300 seconds (longer recovery time)
- Half-open max requests: 5-10 (more test requests)

---

## 4. Combined Strategy: Retry + Circuit Breaker

### 4.1. When to Use Both

**Use both patterns for:**
- External service calls (network APIs, patient lookup)
- Services that can fail independently
- Services that need time to recover
- Operations that are expensive to retry

**Use retry only for:**
- Local operations (database, file I/O)
- Operations that must always be attempted
- Operations with very low failure rates

**Use circuit breaker only for:**
- Services that are known to fail persistently
- Services where fast failure is more important than retry

### 4.2. Implementation Pattern

```cpp
class NetworkService {
private:
    CircuitBreaker m_circuitBreaker;
    RetryPolicy m_retryPolicy;
    
public:
    Result<ServerResponse> sendTelemetry(const TelemetryBatch& batch) {
        // Check circuit breaker first
        return m_circuitBreaker.execute([this, &batch]() -> Result<ServerResponse> {
            // Use retry for transient failures
            return m_retryPolicy.executeWithRetry([this, &batch]() -> Result<ServerResponse> {
                return m_telemetryServer->sendBatch(batch);
            });
        });
    }
};
```

### 4.3. Flow Diagram

```
┌─────────────────┐
│  Request        │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Circuit Breaker │
│   State Check   │
└────────┬────────┘
         │
    ┌────┴────┐
    │         │
Closed      Open
    │         │
    │         └──→ Fail Immediately
    │
    ▼
┌─────────────────┐
│  Retry Policy   │
│  Execute Op     │
└────────┬────────┘
    ┌────┴────┐
    │         │
Success   Failure
    │         │
    │         └──→ Update Circuit Breaker
    │
    ▼
┌─────────────────┐
│  Return Result  │
└─────────────────┘
```

---

## 5. Integration with Z Monitor Services

### 5.1. NetworkManager Integration

**Location:** `z-monitor/src/infrastructure/network/NetworkManager.cpp`

**Usage:**
```cpp
class NetworkManager {
private:
    CircuitBreaker m_circuitBreaker;
    RetryPolicy m_retryPolicy;
    
public:
    Result<ServerResponse> sendTelemetry(const TelemetryBatch& batch) {
        return m_circuitBreaker.execute([this, &batch]() {
            return m_retryPolicy.executeWithRetry([this, &batch]() {
                return m_telemetryServer->sendBatch(batch);
            });
        });
    }
};
```

**Benefits:**
- Retry handles transient network failures (timeouts, temporary unavailability)
- Circuit breaker prevents calling server when it's down for extended periods
- Fast failure when circuit is open (no wasted retries)

### 5.2. PatientLookupService Integration

**Location:** `z-monitor/src/infrastructure/adapters/PatientLookupServiceAdapter.cpp`

**Usage:**
```cpp
class PatientLookupServiceAdapter {
private:
    CircuitBreaker m_circuitBreaker;
    RetryPolicy m_retryPolicy;
    
public:
    Result<PatientInfo> lookupPatient(const QString& mrn) {
        return m_circuitBreaker.execute([this, mrn]() {
            return m_retryPolicy.executeWithRetry([this, mrn]() {
                return m_httpClient->getPatient(mrn);
            });
        });
    }
};
```

**Benefits:**
- Retry handles transient network failures
- Circuit breaker prevents overwhelming failing HIS/EHR system
- Can fall back to cached patient data when circuit is open

### 5.3. Database Operations

**Location:** `z-monitor/src/infrastructure/persistence/*Repository.cpp`

**Usage:**
```cpp
// Use retry only (no circuit breaker for local database)
RetryPolicy dbRetryPolicy(5, 50, 2000);

Result<void> SQLitePatientRepository::save(const PatientAggregate& patient) {
    return dbRetryPolicy.executeWithRetry([this, &patient]() -> Result<void> {
        // Database operation that may fail due to locks
        return executeSaveQuery(patient);
    });
}
```

**Benefits:**
- Retry handles database locks (transient failures)
- No circuit breaker needed (local database, not external service)

---

## 6. Error Logging and Monitoring

### 6.1. Logging Retry Attempts

**Application layer guidelines:**
- Log infrastructure failures (before returning)
- Include retry attempt count in log context
- Log retry exhaustion

**Example:**
```cpp
auto result = retryPolicy.executeWithRetry([&]() {
    return networkService.send(batch);
});

if (result.isError()) {
    m_logService->error("Telemetry send failed after retries", {
        {"deviceId", batch.deviceId},
        {"patientMrn", batch.patientMrn},
        {"errorCode", QString::number(result.error().code)},
        {"retryExhausted", "true"}
    });
}
```

### 6.2. Monitoring Circuit Breaker State

**Metrics to track:**
- Circuit state transitions (Closed → Open → HalfOpen → Closed)
- Failure count
- Time in each state
- Success rate in half-open state

**Example:**
```cpp
if (circuitBreaker.getState() == CircuitState::Open) {
    m_logService->warning("Circuit breaker is open", {
        {"service", "TelemetryServer"},
        {"failureCount", QString::number(circuitBreaker.getFailureCount())}
    });
}
```

---

## 7. Testing Error Recovery Patterns

### 7.1. Unit Tests

**Location:** `z-monitor/tests/unit/domain/common/`

**Test files:**
- `RetryPolicyTest.cpp` - Tests retry logic, exponential backoff, retryable errors
- `CircuitBreakerTest.cpp` - Tests state transitions, failure threshold, recovery

**Coverage:**
- ✅ Success on first attempt
- ✅ Success after retries
- ✅ Non-retryable errors
- ✅ Retry exhaustion
- ✅ Circuit state transitions
- ✅ Failure threshold
- ✅ Recovery testing

### 7.2. Integration Tests

**Test scenarios:**
- Network service with retry + circuit breaker
- Patient lookup with fallback to cache
- Database operations with retry only

---

## 8. Configuration and Tuning

### 8.1. RetryPolicy Configuration

**Default values:**
- Max retries: 3
- Base delay: 100ms
- Max delay: 5000ms

**Tuning guidelines:**
- **Network operations:** Increase max retries (5) for unreliable networks
- **Database operations:** Increase max retries (10) for high-contention scenarios
- **User-facing operations:** Decrease max retries (2) for better UX

### 8.2. CircuitBreaker Configuration

**Default values:**
- Failure threshold: 5
- Reset timeout: 60 seconds
- Half-open max requests: 3

**Tuning guidelines:**
- **Critical services:** Lower failure threshold (3) for faster failure detection
- **Non-critical services:** Higher failure threshold (10) for more tolerance
- **Fast-recovering services:** Shorter reset timeout (30s)
- **Slow-recovering services:** Longer reset timeout (120s)

---

## 9. Best Practices

### 9.1. RetryPolicy Best Practices

**✅ Do:**
- Only retry retryable errors (Timeout, Unavailable, DatabaseError)
- Use exponential backoff (not linear or fixed delays)
- Cap maximum delay to prevent excessive wait times
- Log retry attempts for debugging
- Consider operation timeout (don't retry forever)

**❌ Don't:**
- Retry non-retryable errors (InvalidArgument, NotFound, etc.)
- Use fixed delays (causes thundering herd)
- Retry indefinitely (set max retries)
- Retry user-facing operations too many times (bad UX)

### 9.2. CircuitBreaker Best Practices

**✅ Do:**
- Use for external services (network, APIs)
- Monitor circuit state transitions
- Log circuit state changes
- Test recovery periodically (half-open state)
- Provide fallback behavior when circuit is open

**❌ Don't:**
- Use for local operations (database, file I/O)
- Open circuit on first failure (use failure threshold)
- Leave circuit open forever (test recovery)
- Ignore circuit state in error handling

### 9.3. Combined Strategy Best Practices

**✅ Do:**
- Use retry for transient failures within operation
- Use circuit breaker for persistent service failures
- Check circuit state before retrying
- Provide fallback when circuit is open
- Monitor both retry attempts and circuit state

**❌ Don't:**
- Retry when circuit is open (waste resources)
- Use circuit breaker for local operations
- Ignore circuit state in error messages
- Retry non-retryable errors even if circuit is closed

---

## 10. Related Documents

- `20_ERROR_HANDLING_STRATEGY.md` - Overall error handling strategy
- `../foundation/06_error_handling_and_resilience/03_circuit_breaker.md` - Circuit breaker foundational knowledge
- `../foundation/06_error_handling_and_resilience/04_retry_backoff.md` - Retry with exponential backoff foundational knowledge
- `05_GRACEFUL_DEGRADATION.md` - Graceful degradation patterns
- `12_THREAD_MODEL.md` - Thread model (error recovery on appropriate threads)

---

## 11. Summary

Z Monitor implements error recovery patterns as pure C++ utilities:

- **RetryPolicy** - Handles transient failures with exponential backoff
- **CircuitBreaker** - Prevents calling persistently failing services
- **Combined Strategy** - Use both for maximum resilience

These patterns are integrated into:
- NetworkManager (telemetry transmission)
- PatientLookupService (patient lookup)
- Database operations (repository saves)

All patterns follow the error handling strategy guidelines:
- Return `Result<T, Error>` for structured error information
- Log infrastructure failures appropriately
- Provide fallback behavior when possible

