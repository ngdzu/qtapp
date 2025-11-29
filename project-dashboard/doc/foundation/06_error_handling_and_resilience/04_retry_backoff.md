# Retry with Exponential Backoff

> **📚 Foundational Knowledge**  
> This is a general software engineering concept used in Z Monitor's design.  
> See `../00_FOUNDATIONAL_KNOWLEDGE_INDEX.md` for all foundational topics.

---

## Status: ✅ Complete

**Reference:** `../../z-monitor/architecture_and_design/44_ERROR_RECOVERY_PATTERNS.md` (Z Monitor implementation)

This document provides foundational knowledge about retry with exponential backoff, which automatically retries failed operations with increasing delays to handle transient failures.

---

## 1. What is Retry with Exponential Backoff?

**Retry** means automatically attempting a failed operation again. **Exponential backoff** means waiting progressively longer between retry attempts (e.g., 100ms, 200ms, 400ms, 800ms).

## 2. Why Use Retry with Exponential Backoff?

**Problems it solves:**
- **Transient failures** - Network hiccups, temporary service overload, database locks
- **Thundering herd** - Multiple clients retrying simultaneously can overwhelm a recovering service
- **Resource exhaustion** - Immediate retries can consume CPU/network resources

**Benefits:**
- ✅ Automatic recovery from transient failures
- ✅ Reduces load on recovering services (exponential backoff spreads out retries)
- ✅ Improves user experience (operations succeed without manual intervention)

## 3. How It Works

```
Attempt 1: Operation fails → Wait 100ms
Attempt 2: Operation fails → Wait 200ms
Attempt 3: Operation fails → Wait 400ms
Attempt 4: Operation succeeds → Return success
```

**Exponential backoff formula:**
```
delay = baseDelay * (2 ^ attemptNumber)
```

**Example with baseDelay = 100ms:**
- Attempt 1 → 2: 100ms * 2^0 = 100ms
- Attempt 2 → 3: 100ms * 2^1 = 200ms
- Attempt 3 → 4: 100ms * 2^2 = 400ms
- Attempt 4 → 5: 100ms * 2^3 = 800ms

**Max delay cap:** To prevent excessive delays, cap the delay at a maximum value (e.g., 5 seconds).

## 4. When to Retry vs. When Not to Retry

**✅ Retry these errors (transient/recoverable):**
- `Timeout` - Operation timed out (network may be slow)
- `Unavailable` - Service temporarily unavailable (may recover)
- `DatabaseError` - Database lock or connection issue (may clear)

**❌ Don't retry these errors (permanent/non-recoverable):**
- `InvalidArgument` - Invalid input (won't succeed on retry)
- `NotFound` - Resource doesn't exist (won't exist on retry)
- `PermissionDenied` - Insufficient permissions (won't change on retry)
- `AlreadyExists` - Resource already exists (won't change on retry)
- `Conflict` - Conflicting state (won't resolve on retry)

## 5. Implementation Considerations

**Key parameters:**
- **Max retries:** How many times to retry (typically 3-5)
- **Base delay:** Initial delay between retries (typically 100-500ms)
- **Max delay:** Maximum delay cap (typically 5-10 seconds)

**Best practices:**
- Use exponential backoff (not linear or fixed delays)
- Cap maximum delay to prevent excessive wait times
- Only retry retryable errors (don't retry validation errors)
- Log retry attempts for debugging
- Consider jitter (random variation) to prevent synchronized retries

## 6. Recommended Parameters

**Recommended defaults:**
- Max retries: 3-5
- Base delay: 100-500ms
- Max delay: 5-10 seconds

**Adjust based on:**
- Operation latency (faster operations → shorter delays)
- User experience requirements (UI operations → fewer retries)
- Service characteristics (database locks → more retries)

## 7. Common Pitfalls

**❌ Retrying non-retryable errors:**
```cpp
// BAD: Retrying validation error
if (result.isError() && result.error().code == ErrorCode::InvalidArgument) {
    retry();  // ❌ Will never succeed
}
```

**✅ Only retry retryable errors:**
```cpp
// GOOD: Only retry transient errors
if (result.isError() && isRetryable(result.error())) {
    retry();  // ✅ May succeed
}
```

**❌ No delay cap:**
```cpp
// BAD: Exponential backoff without cap
delay = baseDelay * (2 ^ attempt);  // Can grow to hours!
```

**✅ Cap maximum delay:**
```cpp
// GOOD: Cap at reasonable maximum
delay = std::min(baseDelay * (2 ^ attempt), maxDelay);
```

## 8. Testing Retry Policy

**Test cases:**
- ✅ Success on first attempt (no retry)
- ✅ Success after retries (retry works)
- ✅ Non-retryable error returns immediately (no retry)
- ✅ Retry exhaustion returns error (all retries failed)
- ✅ Exponential backoff delays are correct
- ✅ Max delay cap is enforced

## 9. Real-World Examples

### Example 1: Network API Call

**Scenario:** Calling patient lookup service over network

**Without retry:**
- Network hiccup causes immediate failure
- User sees error even though operation would succeed on retry

**With retry:**
- Network hiccup detected
- Retry with exponential backoff (100ms, 200ms, 400ms)
- Network recovers, operation succeeds
- User never sees error

### Example 2: Database Operation

**Scenario:** Saving patient data to database

**Without retry:**
- Database lock causes immediate failure
- User sees error even though operation would succeed on retry

**With retry:**
- Database lock detected
- Retry with exponential backoff (100ms, 200ms, 400ms)
- Lock clears, operation succeeds
- User never sees error

## 10. Related Patterns

- **Circuit Breaker** (`03_circuit_breaker.md`) - Prevents calling persistently failing services
- **Timeout Pattern** (`06_timeout_pattern.md`) - Fail fast if operation takes too long
- **Graceful Degradation** (`05_graceful_degradation.md`) - Continue operating with reduced functionality

## 11. References

- **Z Monitor Implementation:** `z-monitor/src/domain/common/RetryPolicy.h`
- **Design Document:** `../../z-monitor/architecture_and_design/44_ERROR_RECOVERY_PATTERNS.md` (Section 2)
- **Related Pattern:** `03_circuit_breaker.md` - Circuit breaker pattern
- **AWS - Exponential Backoff:** https://docs.aws.amazon.com/general/latest/gr/api-retries.html

---

## 12. Summary

**Retry with Exponential Backoff:**
- Handles transient failures automatically
- Uses increasing delays between retries
- Only retries retryable errors
- Improves user experience by hiding transient failures

The retry pattern with exponential backoff is essential for building resilient systems that can handle transient failures gracefully.
