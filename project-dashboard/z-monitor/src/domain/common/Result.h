/**
 * @file Result.h
 * @brief Generic result type for operations that can fail.
 *
 * Provides a lightweight Result<T> wrapper with a structured Error type
 * so callers can distinguish success from failure and inspect rich error
 * information (code, message, and optional context).
 *
 * This header is intended for use across all layers (domain, application,
 * infrastructure, interface) to standardize error handling.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace zmon {

/**
 * @enum ErrorCode
 * @brief Generic error codes for operation results.
 *
 * These codes are intentionally broad. Components are free to define
 * more specific domain-level enums if needed and map them into these
 * generic categories when exposing errors via Result.
 */
enum class ErrorCode {
    None = 0,          ///< No error
    Unknown,           ///< Unknown / unspecified error
    InvalidArgument,   ///< Invalid input parameter
    AlreadyExists,     ///< Resource already exists
    NotFound,          ///< Resource not found
    PermissionDenied,  ///< Insufficient permissions
    Unavailable,       ///< Service or resource temporarily unavailable
    Timeout,           ///< Operation timed out
    Conflict,          ///< Conflicting state (e.g., concurrent update)
    DatabaseError,     ///< Database-related error
    Internal           ///< Internal error (bug, invariant violation)
};

/**
 * @using ErrorContext
 * @brief Key-value map for structured error context.
 */
using ErrorContext = std::unordered_map<std::string, std::string>;

/**
 * @struct Error
 * @brief Structured error information for failed operations.
 */
struct Error {
    ErrorCode   code = ErrorCode::Unknown; ///< Error code
    std::string message;                   ///< Human-readable message
    ErrorContext context;                  ///< Optional structured context (key-value pairs)

    /**
     * @brief Factory helper to create an Error.
     *
     * @param code Error code
     * @param message Human-readable message
     * @param context Optional structured context
     * @return Error instance
     */
    static Error create(ErrorCode code,
                        std::string message,
                        ErrorContext context = {})
    {
        Error e;
        e.code = code;
        e.message = std::move(message);
        e.context = std::move(context);
        return e;
    }
};

/**
 * @class Result
 * @brief Represents the result of an operation that can succeed or fail.
 *
 * @tparam T Success value type.
 *
 * A Result<T> is either:
 * - Success: contains a value of type T
 * - Error: contains an Error with rich error information
 */
template <typename T>
class Result {
public:
    /// Creates a successful result with the given value (copy).
    static Result<T> ok(const T& value)
    {
        Result<T> r;
        r.m_value = value;
        return r;
    }

    /// Creates a successful result with the given value (move).
    static Result<T> ok(T&& value)
    {
        Result<T> r;
        r.m_value = std::move(value);
        return r;
    }

    /// Creates an error result with the given Error.
    static Result<T> error(const Error& error)
    {
        Result<T> r;
        r.m_error = error;
        return r;
    }

    /// Returns true if the result represents success.
    [[nodiscard]] bool isOk() const { return m_value.has_value(); }

    /// Returns true if the result represents an error.
    [[nodiscard]] bool isError() const { return m_error.has_value(); }

    /// Returns the contained value (const). Precondition: isOk() == true.
    [[nodiscard]] const T& value() const { return m_value.value(); }

    /// Returns the contained value (mutable). Precondition: isOk() == true.
    [[nodiscard]] T& value() { return m_value.value(); }

    /// Returns the contained error. Precondition: isError() == true.
    [[nodiscard]] const Error& error() const { return m_error.value(); }

private:
    std::optional<T> m_value;
    std::optional<Error> m_error;
};

/**
 * @brief Specialization of Result for void (no value).
 *
 * Represents success or error without an associated value.
 */
template <>
class Result<void> {
public:
    /// Creates a successful result (no value).
    static Result<void> ok()
    {
        return Result<void>(true, std::nullopt);
    }

    /// Creates an error result with the given Error.
    static Result<void> error(const Error& error)
    {
        return Result<void>(false, error);
    }

    /// Returns true if the result represents success.
    [[nodiscard]] bool isOk() const { return m_ok; }

    /// Returns true if the result represents an error.
    [[nodiscard]] bool isError() const { return !m_ok; }

    /// Returns the contained error. Precondition: isError() == true.
    [[nodiscard]] const Error& error() const { return m_error.value(); }

private:
    explicit Result(bool ok, std::optional<Error> error)
        : m_ok(ok)
        , m_error(std::move(error))
    {
    }

    bool m_ok = true;
    std::optional<Error> m_error;
};

} // namespace zmon


