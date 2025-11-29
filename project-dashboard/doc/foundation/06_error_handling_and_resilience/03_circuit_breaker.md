# Circuit Breaker Pattern

> **📚 Foundational Knowledge**  
> This is a general software engineering concept used in Z Monitor's design.  
> See `../00_FOUNDATIONAL_KNOWLEDGE_INDEX.md` for all foundational topics.

---

## Status: ✅ Complete

**Reference:** `../../z-monitor/architecture_and_design/44_ERROR_RECOVERY_PATTERNS.md` (Z Monitor implementation)

This document provides foundational knowledge about the circuit breaker pattern, which prevents calls to failing services to avoid resource exhaustion and cascading failures.

---

## 1. What is a Circuit Breaker?

A **circuit breaker** is a design pattern that prevents calls to a failing service, allowing it to recover without being overwhelmed by continuous retry attempts. It's named after electrical circuit breakers that "trip" to prevent damage.

## 2. Why Use Circuit Breaker?

**Problems it solves:**
- **Cascading failures** - One failing service can bring down the entire system
- **Resource exhaustion** - Continuous retries consume CPU, memory, network
- **Slow failure detection** - Without circuit breaker, system keeps trying failing operations

**Benefits:**
- ✅ Prevents cascading failures
- ✅ Fast failure (fails immediately when circuit is open)
- ✅ Automatic recovery testing (half-open state)
- ✅ Protects downstream services from overload

## 3. Circuit Breaker States

A circuit breaker has three states:

### **Closed (Normal Operation)**
- Requests pass through to the service
- Failures are counted
- If failure threshold reached → transition to **Open**

### **Open (Failing)**
- Requests fail immediately without calling the service
- No load on the failing service
- After timeout period → transition to **HalfOpen**

### **HalfOpen (Testing Recovery)**
- Limited requests allowed (e.g., 3 requests)
- If all succeed → transition to **Closed** (service recovered)
- If any fail → transition to **Open** (service still failing)

## 4. State Transition Diagram

```
┌─────────┐
│ Closed  │ ←──────────────┐
└────┬────┘                │
     │                     │
     │ (failure threshold) │ (all succeed)
     ▼                     │
┌─────────┐                │
│  Open   │                │
└────┬────┘                │
     │                     │
     │ (timeout elapsed)   │
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

## 5. Key Parameters

- **Failure threshold:** Number of consecutive failures before opening circuit (typically 5)
- **Reset timeout:** Time to wait before attempting half-open (typically 60 seconds)
- **Half-open max requests:** Number of requests allowed in half-open state (typically 3)

## 6. When to Use Circuit Breaker

**✅ Use circuit breaker for:**
- External service calls (network APIs, patient lookup service)
- Services that can fail independently
- Services that need time to recover
- Operations that are expensive to retry

**❌ Don't use circuit breaker for:**
- Local operations (database, file I/O) - use retry instead
- Operations that must always be attempted
- Operations with very low failure rates

## 7. Implementation Considerations

**Recommended defaults:**
- Failure threshold: 5 consecutive failures
- Reset timeout: 60 seconds
- Half-open max requests: 3

**Adjust based on:**
- Service recovery time (faster recovery → shorter timeout)
- Criticality (critical services → higher threshold)
- Load characteristics (high load → more half-open requests)

## 8. Common Pitfalls

**❌ Opening circuit on first failure:**
```cpp
// BAD: Too sensitive
if (failureCount >= 1) {
    openCircuit();  // ❌ Opens on single transient failure
}
```

**✅ Use failure threshold:**
```cpp
// GOOD: Require multiple failures
if (failureCount >= failureThreshold) {
    openCircuit();  // ✅ Only opens on persistent failures
}
```

**❌ Never testing recovery:**
```cpp
// BAD: Circuit stays open forever
if (state == Open) {
    return Error("Circuit open");  // ❌ Never tests if service recovered
}
```

**✅ Test recovery periodically:**
```cpp
// GOOD: Transition to half-open after timeout
if (state == Open && shouldAttemptReset()) {
    state = HalfOpen;  // ✅ Tests if service recovered
}
```

## 9. Testing Circuit Breaker

**Test cases:**
- ✅ Circuit starts closed
- ✅ Opens after failure threshold
- ✅ Open circuit fails immediately (no service call)
- ✅ Transitions to half-open after timeout
- ✅ Half-open closes after success threshold
- ✅ Half-open opens on any failure
- ✅ Manual reset works

## 10. Related Patterns

- **Retry with Exponential Backoff** (`04_retry_backoff.md`) - Handles transient failures within operations
- **Timeout Pattern** (`06_timeout_pattern.md`) - Fail fast if operation takes too long
- **Health Checks** (`07_health_checks.md`) - Monitor service health to inform circuit breaker
- **Graceful Degradation** (`05_graceful_degradation.md`) - Continue operating with reduced functionality

## 11. References

- **Z Monitor Implementation:** `z-monitor/src/domain/common/CircuitBreaker.h`
- **Design Document:** `../../z-monitor/architecture_and_design/44_ERROR_RECOVERY_PATTERNS.md` (Section 3)
- **Related Pattern:** `04_retry_backoff.md` - Retry with exponential backoff
- **Martin Fowler - Circuit Breaker:** https://martinfowler.com/bliki/CircuitBreaker.html

---

## 12. Summary

**Circuit Breaker:**
- Prevents calling failing services
- Three states: Closed, Open, HalfOpen
- Automatic recovery testing
- Protects system from cascading failures

The circuit breaker pattern is essential for building resilient systems that can handle persistent service failures gracefully.
