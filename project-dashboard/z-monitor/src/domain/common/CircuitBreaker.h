/**
 * @file CircuitBreaker.h
 * @brief Circuit breaker pattern for external service calls.
 *
 * Provides a circuit breaker implementation to prevent cascading failures
 * when external services are unavailable. The circuit breaker has three states:
 * - Closed: Normal operation (requests pass through)
 * - Open: Service is failing (requests fail immediately)
 * - HalfOpen: Testing if service recovered (limited requests allowed)
 *
 * This is a pure C++ utility (no Qt dependencies) suitable for use in the
 * domain layer or any layer that needs circuit breaker protection.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/common/Result.h"
#include <chrono>
#include <atomic>
#include <mutex>

namespace zmon {

/**
 * @enum CircuitState
 * @brief Circuit breaker state.
 */
enum class CircuitState {
    Closed,    ///< Normal operation - requests pass through
    Open,      ///< Service failing - requests fail immediately
    HalfOpen   ///< Testing recovery - limited requests allowed
};

/**
 * @class CircuitBreaker
 * @brief Circuit breaker for external service calls.
 *
 * Prevents cascading failures by opening the circuit when a service
 * is failing, and gradually testing recovery in half-open state.
 *
 * @note Pure C++ (no Qt dependencies) - can be used in domain layer
 * @note Thread-safe: Uses atomic operations and mutex for state management
 */
class CircuitBreaker {
public:
    /**
     * @brief Constructor with default parameters.
     *
     * Creates a circuit breaker with:
     * - Failure threshold: 5 consecutive failures
     * - Reset timeout: 60 seconds
     * - Half-open max requests: 3
     */
    CircuitBreaker()
        : m_state(CircuitState::Closed)
        , m_failureCount(0)
        , m_successCount(0)
        , m_failureThreshold(5)
        , m_resetTimeoutSeconds(60)
        , m_halfOpenMaxRequests(3)
        , m_lastFailureTime(std::chrono::steady_clock::now())
    {}

    /**
     * @brief Constructor with custom parameters.
     *
     * @param failureThreshold Number of consecutive failures before opening circuit
     * @param resetTimeoutSeconds Time to wait before attempting half-open state
     * @param halfOpenMaxRequests Max requests allowed in half-open state
     */
    CircuitBreaker(int failureThreshold, int resetTimeoutSeconds, int halfOpenMaxRequests)
        : m_state(CircuitState::Closed)
        , m_failureCount(0)
        , m_successCount(0)
        , m_failureThreshold(failureThreshold)
        , m_resetTimeoutSeconds(resetTimeoutSeconds)
        , m_halfOpenMaxRequests(halfOpenMaxRequests)
        , m_lastFailureTime(std::chrono::steady_clock::now())
    {}

    /**
     * @brief Execute operation with circuit breaker protection.
     *
     * Executes the operation if circuit is closed or half-open.
     * If circuit is open, returns error immediately without calling operation.
     *
     * @param operation Operation to execute (returns Result<T, Error>)
     * @return Result<T, Error> - Success if operation succeeded,
     *         Error if circuit is open or operation failed
     */
    template<typename T>
    Result<T> execute(std::function<Result<T>()> operation) {
        // Check if circuit should transition from open to half-open
        if (m_state.load() == CircuitState::Open) {
            if (shouldAttemptReset()) {
                transitionToHalfOpen();
            } else {
                // Circuit is open - fail immediately
                return Result<T>::error(Error::create(
                    ErrorCode::Unavailable,
                    "Circuit breaker is open - service unavailable",
                    {{"failureCount", std::to_string(m_failureCount.load())},
                     {"lastFailureTime", std::to_string(
                         std::chrono::duration_cast<std::chrono::seconds>(
                             m_lastFailureTime.time_since_epoch()).count())}}
                ));
            }
        }

        // Check half-open request limit
        if (m_state.load() == CircuitState::HalfOpen) {
            if (m_successCount.load() >= m_halfOpenMaxRequests) {
                // Too many requests in half-open - reject
                return Result<T>::error(Error::create(
                    ErrorCode::Unavailable,
                    "Circuit breaker half-open request limit reached",
                    {}
                ));
            }
        }

        // Execute operation
        auto result = operation();

        // Update circuit state based on result
        if (result.isOk()) {
            onSuccess();
        } else {
            onFailure();
        }

        return result;
    }

    /**
     * @brief Get current circuit state.
     *
     * @return Current circuit state
     */
    CircuitState getState() const {
        return m_state.load();
    }

    /**
     * @brief Get current failure count.
     *
     * @return Number of consecutive failures
     */
    int getFailureCount() const {
        return m_failureCount.load();
    }

    /**
     * @brief Reset circuit breaker to closed state.
     *
     * Manually reset the circuit breaker (useful for testing or manual recovery).
     */
    void reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state = CircuitState::Closed;
        m_failureCount = 0;
        m_successCount = 0;
    }

private:
    std::atomic<CircuitState> m_state;
    std::atomic<int> m_failureCount;
    std::atomic<int> m_successCount;
    int m_failureThreshold;
    int m_resetTimeoutSeconds;
    int m_halfOpenMaxRequests;
    mutable std::mutex m_mutex;
    std::chrono::steady_clock::time_point m_lastFailureTime;

    /**
     * @brief Check if circuit should transition from open to half-open.
     *
     * @return true if reset timeout has elapsed, false otherwise
     */
    bool shouldAttemptReset() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_lastFailureTime).count();
        return elapsed >= m_resetTimeoutSeconds;
    }

    /**
     * @brief Transition circuit to half-open state.
     */
    void transitionToHalfOpen() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_state.load() == CircuitState::Open) {
            m_state = CircuitState::HalfOpen;
            m_successCount = 0;
        }
    }

    /**
     * @brief Handle successful operation.
     */
    void onSuccess() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_state.load() == CircuitState::HalfOpen) {
            // In half-open: count successes
            int successes = m_successCount.fetch_add(1) + 1;
            
            // If enough successes, close circuit
            if (successes >= m_halfOpenMaxRequests) {
                m_state = CircuitState::Closed;
                m_failureCount = 0;
                m_successCount = 0;
            }
        } else if (m_state.load() == CircuitState::Closed) {
            // In closed: reset failure count on success
            m_failureCount = 0;
        }
    }

    /**
     * @brief Handle failed operation.
     */
    void onFailure() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_lastFailureTime = std::chrono::steady_clock::now();
        int failures = m_failureCount.fetch_add(1) + 1;
        
        if (m_state.load() == CircuitState::HalfOpen) {
            // In half-open: any failure opens circuit
            m_state = CircuitState::Open;
            m_successCount = 0;
        } else if (m_state.load() == CircuitState::Closed) {
            // In closed: check if threshold reached
            if (failures >= m_failureThreshold) {
                m_state = CircuitState::Open;
            }
        }
    }
};

} // namespace zmon

