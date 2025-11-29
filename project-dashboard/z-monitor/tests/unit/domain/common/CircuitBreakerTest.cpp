/**
 * @file CircuitBreakerTest.cpp
 * @brief Unit tests for CircuitBreaker pattern.
 * 
 * Tests circuit breaker state transitions (Closed → Open → HalfOpen → Closed),
 * failure threshold, reset timeout, and half-open request limits.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include "domain/common/CircuitBreaker.h"
#include <thread>
#include <chrono>

using namespace zmon;

class CircuitBreakerTest : public ::testing::Test {
protected:
    void SetUp() override {
        attemptCount = 0;
    }

    std::atomic<int> attemptCount;
};

// Test: Circuit starts in Closed state
TEST_F(CircuitBreakerTest, StartsClosed) {
    CircuitBreaker breaker;
    EXPECT_EQ(breaker.getState(), CircuitState::Closed);
    EXPECT_EQ(breaker.getFailureCount(), 0);
}

// Test: Successful operations keep circuit closed
TEST_F(CircuitBreakerTest, SuccessKeepsCircuitClosed) {
    CircuitBreaker breaker(5, 60, 3);  // 5 failures threshold, 60s timeout, 3 half-open requests
    
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::ok(42);
    };
    
    // Execute multiple successful operations
    for (int i = 0; i < 10; i++) {
        auto result = breaker.execute(operation);
        EXPECT_TRUE(result.isOk());
        EXPECT_EQ(breaker.getState(), CircuitState::Closed);
        EXPECT_EQ(breaker.getFailureCount(), 0);
    }
}

// Test: Circuit opens after failure threshold
TEST_F(CircuitBreakerTest, OpensAfterFailureThreshold) {
    CircuitBreaker breaker(3, 60, 3);  // 3 failures threshold
    
    auto failingOp = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::error(Error::create(ErrorCode::Unavailable, "Service unavailable"));
    };
    
    // First 2 failures - circuit still closed
    for (int i = 0; i < 2; i++) {
        auto result = breaker.execute(failingOp);
        EXPECT_TRUE(result.isError());
        EXPECT_EQ(breaker.getState(), CircuitState::Closed);
        EXPECT_EQ(breaker.getFailureCount(), i + 1);
    }
    
    // Third failure - circuit should open
    auto result = breaker.execute(failingOp);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(breaker.getState(), CircuitState::Open);
    EXPECT_EQ(breaker.getFailureCount(), 3);
}

// Test: Open circuit fails immediately
TEST_F(CircuitBreakerTest, OpenCircuitFailsImmediately) {
    CircuitBreaker breaker(3, 60, 3);
    
    // Force circuit open
    for (int i = 0; i < 3; i++) {
        auto failingOp = []() -> Result<int> {
            return Result<int>::error(Error::create(ErrorCode::Unavailable, "Fail"));
        };
        breaker.execute(failingOp);
    }
    
    EXPECT_EQ(breaker.getState(), CircuitState::Open);
    
    // Operation should not be called when circuit is open
    attemptCount = 0;
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::ok(42);
    };
    
    auto result = breaker.execute(operation);
    
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code, ErrorCode::Unavailable);
    EXPECT_EQ(attemptCount.load(), 0);  // Operation should not be called
}

// Test: Circuit transitions to half-open after timeout
TEST_F(CircuitBreakerTest, TransitionsToHalfOpenAfterTimeout) {
    CircuitBreaker breaker(3, 1, 3);  // 1 second timeout (short for testing)
    
    // Force circuit open
    for (int i = 0; i < 3; i++) {
        auto failingOp = []() -> Result<int> {
            return Result<int>::error(Error::create(ErrorCode::Unavailable, "Fail"));
        };
        breaker.execute(failingOp);
    }
    
    EXPECT_EQ(breaker.getState(), CircuitState::Open);
    
    // Wait for timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    // Next request should transition to half-open
    attemptCount = 0;
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::ok(42);
    };
    
    auto result = breaker.execute(operation);
    
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(breaker.getState(), CircuitState::HalfOpen);
    EXPECT_EQ(attemptCount.load(), 1);
}

// Test: Half-open circuit closes after success threshold
TEST_F(CircuitBreakerTest, HalfOpenClosesAfterSuccess) {
    CircuitBreaker breaker(3, 1, 3);  // 3 half-open requests needed
    
    // Force circuit open, then wait for timeout
    for (int i = 0; i < 3; i++) {
        auto failingOp = []() -> Result<int> {
            return Result<int>::error(Error::create(ErrorCode::Unavailable, "Fail"));
        };
        breaker.execute(failingOp);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    // Execute successful operations in half-open state
    auto successOp = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::ok(42);
    };
    
    // First 2 successes - still half-open
    for (int i = 0; i < 2; i++) {
        auto result = breaker.execute(successOp);
        EXPECT_TRUE(result.isOk());
        EXPECT_EQ(breaker.getState(), CircuitState::HalfOpen);
    }
    
    // Third success - should close circuit
    auto result = breaker.execute(successOp);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(breaker.getState(), CircuitState::Closed);
    EXPECT_EQ(breaker.getFailureCount(), 0);
}

// Test: Half-open circuit opens on any failure
TEST_F(CircuitBreakerTest, HalfOpenOpensOnFailure) {
    CircuitBreaker breaker(3, 1, 3);
    
    // Force circuit open, then wait for timeout
    for (int i = 0; i < 3; i++) {
        auto failingOp = []() -> Result<int> {
            return Result<int>::error(Error::create(ErrorCode::Unavailable, "Fail"));
        };
        breaker.execute(failingOp);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    // First success in half-open
    auto successOp = []() -> Result<int> {
        return Result<int>::ok(42);
    };
    breaker.execute(successOp);
    EXPECT_EQ(breaker.getState(), CircuitState::HalfOpen);
    
    // Failure in half-open - should open immediately
    auto failingOp = []() -> Result<int> {
        return Result<int>::error(Error::create(ErrorCode::Unavailable, "Fail"));
    };
    auto result = breaker.execute(failingOp);
    
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(breaker.getState(), CircuitState::Open);
}

// Test: Half-open request limit
TEST_F(CircuitBreakerTest, HalfOpenRequestLimit) {
    CircuitBreaker breaker(3, 1, 2);  // Only 2 requests allowed in half-open
    
    // Force circuit open, then wait for timeout
    for (int i = 0; i < 3; i++) {
        auto failingOp = []() -> Result<int> {
            return Result<int>::error(Error::create(ErrorCode::Unavailable, "Fail"));
        };
        breaker.execute(failingOp);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    // First 2 requests should succeed
    attemptCount = 0;
    auto operation = [this]() -> Result<int> {
        attemptCount++;
        return Result<int>::ok(42);
    };
    
    auto result1 = breaker.execute(operation);
    EXPECT_TRUE(result1.isOk());
    
    auto result2 = breaker.execute(operation);
    EXPECT_TRUE(result2.isOk());
    
    // Third request should be rejected (limit reached)
    auto result3 = breaker.execute(operation);
    EXPECT_TRUE(result3.isError());
    EXPECT_EQ(result3.error().code, ErrorCode::Unavailable);
    EXPECT_EQ(attemptCount.load(), 2);  // Only 2 operations executed
}

// Test: Reset manually closes circuit
TEST_F(CircuitBreakerTest, ManualReset) {
    CircuitBreaker breaker(3, 60, 3);
    
    // Force circuit open
    for (int i = 0; i < 3; i++) {
        auto failingOp = []() -> Result<int> {
            return Result<int>::error(Error::create(ErrorCode::Unavailable, "Fail"));
        };
        breaker.execute(failingOp);
    }
    
    EXPECT_EQ(breaker.getState(), CircuitState::Open);
    
    // Manual reset
    breaker.reset();
    
    EXPECT_EQ(breaker.getState(), CircuitState::Closed);
    EXPECT_EQ(breaker.getFailureCount(), 0);
    
    // Should work normally after reset
    auto operation = []() -> Result<int> {
        return Result<int>::ok(42);
    };
    
    auto result = breaker.execute(operation);
    EXPECT_TRUE(result.isOk());
}

// Test: Default constructor parameters
TEST_F(CircuitBreakerTest, DefaultConstructor) {
    CircuitBreaker breaker;  // Default: 5 failures, 60s timeout, 3 half-open requests
    
    EXPECT_EQ(breaker.getState(), CircuitState::Closed);
    EXPECT_EQ(breaker.getFailureCount(), 0);
    
    // Should work normally
    auto operation = []() -> Result<int> {
        return Result<int>::ok(42);
    };
    
    auto result = breaker.execute(operation);
    EXPECT_TRUE(result.isOk());
}

// Test: Failure count resets on success in closed state
TEST_F(CircuitBreakerTest, FailureCountResetsOnSuccess) {
    CircuitBreaker breaker(5, 60, 3);
    
    // Two failures
    for (int i = 0; i < 2; i++) {
        auto failingOp = []() -> Result<int> {
            return Result<int>::error(Error::create(ErrorCode::Unavailable, "Fail"));
        };
        breaker.execute(failingOp);
    }
    
    EXPECT_EQ(breaker.getFailureCount(), 2);
    
    // Success should reset failure count
    auto successOp = []() -> Result<int> {
        return Result<int>::ok(42);
    };
    breaker.execute(successOp);
    
    EXPECT_EQ(breaker.getFailureCount(), 0);
    EXPECT_EQ(breaker.getState(), CircuitState::Closed);
}

