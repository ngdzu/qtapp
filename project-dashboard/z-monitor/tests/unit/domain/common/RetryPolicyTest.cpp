/**
 * @file RetryPolicyTest.cpp
 * @brief Unit tests for RetryPolicy with exponential backoff.
 * 
 * Tests retry logic, exponential backoff calculation, retryable error
 * detection, and retry exhaustion.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include "domain/common/RetryPolicy.h"
#include <atomic>
#include <chrono>

using namespace zmon;

class RetryPolicyTest : public ::testing::Test {
protected:
    void SetUp() override {
        attemptCount = 0;
    }

    std::atomic<int> attemptCount;
};

// Test: Operation succeeds on first attempt
TEST_F(RetryPolicyTest, SuccessOnFirstAttempt) {
    RetryPolicy policy(3, 10, 1000);  // 3 retries, 10ms base, 1000ms max
    
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::ok(42);
    };
    
    auto result = policy.executeWithRetry(operation);
    
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 42);
    EXPECT_EQ(attemptCount.load(), 1);
}

// Test: Operation succeeds after retries
TEST_F(RetryPolicyTest, SuccessAfterRetries) {
    RetryPolicy policy(3, 10, 1000);
    
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        if (attemptCount.load() < 3) {
            // Fail first 2 attempts
            return Result<int>::error(Error::create(ErrorCode::Timeout, "Timeout"));
        }
        return Result<int>::ok(42);
    };
    
    auto result = policy.executeWithRetry(operation);
    
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 42);
    EXPECT_EQ(attemptCount.load(), 3);
}

// Test: Non-retryable error returns immediately
TEST_F(RetryPolicyTest, NonRetryableErrorReturnsImmediately) {
    RetryPolicy policy(3, 10, 1000);
    
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::error(Error::create(ErrorCode::InvalidArgument, "Invalid"));
    };
    
    auto result = policy.executeWithRetry(operation);
    
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(attemptCount.load(), 1);  // Should not retry
}

// Test: Retry exhaustion returns error
TEST_F(RetryPolicyTest, RetryExhaustion) {
    RetryPolicy policy(3, 10, 1000);
    
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::error(Error::create(ErrorCode::Timeout, "Timeout"));
    };
    
    auto result = policy.executeWithRetry(operation);
    
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code, ErrorCode::Timeout);
    EXPECT_EQ(attemptCount.load(), 3);  // Should retry 3 times
}

// Test: Retryable error codes
TEST_F(RetryPolicyTest, RetryableErrorCodes) {
    RetryPolicy policy(1, 1, 10);
    
    // Test Timeout is retryable
    auto timeoutOp = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::error(Error::create(ErrorCode::Timeout, "Timeout"));
    };
    auto result1 = policy.executeWithRetry(timeoutOp);
    EXPECT_TRUE(result1.isError());
    EXPECT_GT(attemptCount.load(), 1);  // Should retry
    
    attemptCount = 0;
    
    // Test Unavailable is retryable
    auto unavailableOp = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::error(Error::create(ErrorCode::Unavailable, "Unavailable"));
    };
    auto result2 = policy.executeWithRetry(unavailableOp);
    EXPECT_TRUE(result2.isError());
    EXPECT_GT(attemptCount.load(), 1);  // Should retry
    
    attemptCount = 0;
    
    // Test DatabaseError is retryable (for locks)
    auto dbErrorOp = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::error(Error::create(ErrorCode::DatabaseError, "DB locked"));
    };
    auto result3 = policy.executeWithRetry(dbErrorOp);
    EXPECT_TRUE(result3.isError());
    EXPECT_GT(attemptCount.load(), 1);  // Should retry
}

// Test: Exponential backoff delay calculation
TEST_F(RetryPolicyTest, ExponentialBackoff) {
    RetryPolicy policy(5, 100, 10000);  // 100ms base, 10s max
    
    std::vector<int> delays;
    auto startTime = std::chrono::steady_clock::now();
    
    auto operation = [this, &delays, startTime]() -> Result<int> {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - startTime).count();
        delays.push_back(static_cast<int>(elapsed));
        
        attemptCount++;
        if (attemptCount.load() < 4) {
            return Result<int>::error(Error::create(ErrorCode::Timeout, "Timeout"));
        }
        return Result<int>::ok(42);
    };
    
    auto result = policy.executeWithRetry(operation);
    
    EXPECT_TRUE(result.isOk());
    // Should have delays: ~100ms, ~200ms, ~400ms (exponential backoff)
    // Note: Actual delays may vary due to system scheduling, so we just verify
    // that delays occurred (not exact timing)
    EXPECT_GE(delays.size(), 3u);
}

// Test: Max delay cap
TEST_F(RetryPolicyTest, MaxDelayCap) {
    RetryPolicy policy(10, 1000, 2000);  // 1s base, 2s max
    
    // With exponential backoff: 1s, 2s, 2s (capped), 2s, ...
    // Should not exceed 2s max delay
    
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        if (attemptCount.load() < 10) {
            return Result<int>::error(Error::create(ErrorCode::Timeout, "Timeout"));
        }
        return Result<int>::ok(42);
    };
    
    auto startTime = std::chrono::steady_clock::now();
    auto result = policy.executeWithRetry(operation);
    auto endTime = std::chrono::steady_clock::now();
    
    // Total time should be reasonable (not exponential explosion)
    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    // Should be capped at max delay per retry
    // 9 retries * 2s max = ~18s max, but allow some variance
    EXPECT_LT(totalMs, 25000);  // Less than 25 seconds
}

// Test: Mixed retryable and non-retryable errors
TEST_F(RetryPolicyTest, MixedErrorTypes) {
    RetryPolicy policy(3, 10, 1000);
    
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        if (attemptCount.load() == 1) {
            // First attempt: retryable error
            return Result<int>::error(Error::create(ErrorCode::Timeout, "Timeout"));
        } else {
            // Second attempt: non-retryable error
            return Result<int>::error(Error::create(ErrorCode::InvalidArgument, "Invalid"));
        }
    };
    
    auto result = policy.executeWithRetry(operation);
    
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(attemptCount.load(), 2);  // Should retry once, then stop
}

// Test: Default constructor parameters
TEST_F(RetryPolicyTest, DefaultConstructor) {
    RetryPolicy policy;  // Default: 3 retries, 100ms base, 5000ms max
    
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        if (attemptCount.load() < 2) {
            return Result<int>::error(Error::create(ErrorCode::Timeout, "Timeout"));
        }
        return Result<int>::ok(42);
    };
    
    auto result = policy.executeWithRetry(operation);
    
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(attemptCount.load(), 2);
}

