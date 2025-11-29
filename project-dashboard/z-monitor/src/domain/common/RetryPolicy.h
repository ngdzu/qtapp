/**
 * @file RetryPolicy.h
 * @brief Retry policy with exponential backoff for recoverable errors.
 *
 * Provides a reusable retry mechanism for operations that can fail with
 * recoverable errors (network timeouts, database locks, etc.). Uses exponential
 * backoff to avoid overwhelming failing services.
 *
 * This is a pure C++ utility (no Qt dependencies) suitable for use in the
 * domain layer or any layer that needs retry logic.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "domain/common/Result.h"
#include <functional>
#include <chrono>
#include <thread>

namespace zmon
{

    /**
     * @class RetryPolicy
     * @brief Retry policy with exponential backoff for recoverable errors.
     *
     * Executes an operation with automatic retry on recoverable errors, using
     * exponential backoff between attempts. Non-retryable errors are returned
     * immediately without retry.
     *
     * @note Pure C++ (no Qt dependencies) - can be used in domain layer
     * @note Thread-safe: Can be used from any thread
     */
    class RetryPolicy
    {
    public:
        /**
         * @brief Default constructor with standard retry parameters.
         *
         * Creates a retry policy with:
         * - Max retries: 3
         * - Base delay: 100ms
         * - Max delay: 5000ms
         */
        RetryPolicy()
            : m_maxRetries(3), m_baseDelayMs(100), m_maxDelayMs(5000)
        {
        }

        /**
         * @brief Constructor with custom retry parameters.
         *
         * @param maxRetries Maximum number of retry attempts (default: 3)
         * @param baseDelayMs Base delay in milliseconds for exponential backoff (default: 100ms)
         * @param maxDelayMs Maximum delay in milliseconds (default: 5000ms)
         */
        RetryPolicy(int maxRetries, int baseDelayMs, int maxDelayMs)
            : m_maxRetries(maxRetries), m_baseDelayMs(baseDelayMs), m_maxDelayMs(maxDelayMs)
        {
        }

        /**
         * @brief Execute operation with retry on recoverable errors.
         *
         * Executes the operation and retries on recoverable errors using
         * exponential backoff with jitter.
         *
         * @tparam Func Callable type that returns Result<T>
         * @param operation Operation to execute (returns Result<T, Error>)
         * @return Result<T, Error> - Success if operation succeeded (on any attempt),
         *         Error if all retries exhausted or non-retryable error occurred
         *
         * @note Retryable errors: Timeout, Unavailable, DatabaseError (for locks)
         * @note Non-retryable errors: InvalidArgument, NotFound, PermissionDenied, etc.
         */
        template <typename Func>
        auto executeWithRetry(Func operation) -> decltype(operation())
        {
            using ResultType = decltype(operation());
            int attempt = 0;

            while (attempt < m_maxRetries)
            {
                auto result = operation();

                // Success - return immediately
                if (result.isOk())
                {
                    return result;
                }

                // Check if error is retryable
                if (!isRetryable(result.error()))
                {
                    // Non-retryable error - return immediately
                    return result;
                }

                // Last attempt failed - return error
                if (attempt == m_maxRetries - 1)
                {
                    return ResultType::error(Error::create(
                        ErrorCode::Timeout,
                        "Operation failed after " + std::to_string(m_maxRetries) + " retry attempts",
                        {{"maxRetries", std::to_string(m_maxRetries)},
                         {"lastError", result.error().message}}));
                }

                // Calculate exponential backoff delay
                int delayMs = calculateBackoffDelay(attempt);

                // Sleep before retry (pure C++ - no Qt)
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

                attempt++;
            }

            // Should not reach here, but return error just in case
            return ResultType::error(Error::create(
                ErrorCode::Timeout,
                "Retry exhausted",
                {{"maxRetries", std::to_string(m_maxRetries)}}));
        }

    private:
        int m_maxRetries;
        int m_baseDelayMs;
        int m_maxDelayMs;

        /**
         * @brief Check if error is retryable.
         *
         * Determines if an error is recoverable and should be retried.
         *
         * @param error Error to check
         * @return true if error is retryable, false otherwise
         */
        bool isRetryable(const Error &error) const
        {
            switch (error.code)
            {
            case ErrorCode::Timeout:
            case ErrorCode::Unavailable:
            case ErrorCode::DatabaseError: // For database locks
                return true;
            case ErrorCode::InvalidArgument:
            case ErrorCode::NotFound:
            case ErrorCode::PermissionDenied:
            case ErrorCode::AlreadyExists:
            case ErrorCode::Conflict:
            case ErrorCode::Internal:
            case ErrorCode::Unknown:
            case ErrorCode::None:
            default:
                return false;
            }
        }

        /**
         * @brief Calculate exponential backoff delay.
         *
         * Calculates delay using exponential backoff: baseDelay * 2^attempt,
         * capped at maxDelayMs.
         *
         * @param attempt Current attempt number (0-based)
         * @return Delay in milliseconds
         */
        int calculateBackoffDelay(int attempt) const
        {
            // Exponential backoff: baseDelay * 2^attempt
            int delay = m_baseDelayMs * (1 << attempt);

            // Cap at max delay
            if (delay > m_maxDelayMs)
            {
                delay = m_maxDelayMs;
            }

            return delay;
        }
    };

} // namespace zmon
