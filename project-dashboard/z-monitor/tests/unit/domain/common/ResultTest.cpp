/**
 * @file ResultTest.cpp
 * @brief Unit tests for Result<T, Error> type.
 *
 * Tests the Result type functionality including success/error states,
 * error creation, context handling, and Result<void> specialization.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <gtest/gtest.h>
#include "domain/common/Result.h"
#include <string>

using namespace zmon;

// Test: Result<void> success
TEST(ResultTest, VoidSuccess)
{
    auto result = Result<void>::ok();
    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isError());
}

// Test: Result<void> error
TEST(ResultTest, VoidError)
{
    auto error = Error::create(ErrorCode::NotFound, "Resource not found");
    auto result = Result<void>::error(error);

    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code, ErrorCode::NotFound);
    EXPECT_EQ(result.error().message, "Resource not found");
}

// Test: Result<T> success with value
TEST(ResultTest, ValueSuccess)
{
    auto result = Result<int>::ok(42);

    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.value(), 42);
}

// Test: Result<T> error
TEST(ResultTest, ValueError)
{
    auto error = Error::create(ErrorCode::InvalidArgument, "Invalid input");
    auto result = Result<int>::error(error);

    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code, ErrorCode::InvalidArgument);
    EXPECT_EQ(result.error().message, "Invalid input");
}

// Test: Error creation with context
TEST(ResultTest, ErrorWithContext)
{
    ErrorContext context;
    context["key1"] = "value1";
    context["key2"] = "value2";

    auto error = Error::create(ErrorCode::DatabaseError, "Database error", context);

    EXPECT_EQ(error.code, ErrorCode::DatabaseError);
    EXPECT_EQ(error.message, "Database error");
    EXPECT_EQ(error.context.size(), 2u);
    EXPECT_EQ(error.context.at("key1"), "value1");
    EXPECT_EQ(error.context.at("key2"), "value2");
}

// Test: Error creation without context
TEST(ResultTest, ErrorWithoutContext)
{
    auto error = Error::create(ErrorCode::Timeout, "Operation timed out");

    EXPECT_EQ(error.code, ErrorCode::Timeout);
    EXPECT_EQ(error.message, "Operation timed out");
    EXPECT_TRUE(error.context.empty());
}

// Test: Result<T> valueOr with success
TEST(ResultTest, ValueOrSuccess)
{
    auto result = Result<int>::ok(42);
    EXPECT_EQ(result.valueOr(0), 42);
}

// Test: Result<T> valueOr with error
TEST(ResultTest, ValueOrError)
{
    auto error = Error::create(ErrorCode::NotFound, "Not found");
    auto result = Result<int>::error(error);
    EXPECT_EQ(result.valueOr(0), 0);
}

// Test: Result<T> valueOr rvalue fallback
TEST(ResultTest, ValueOrRvalueFallback)
{
    auto result = Result<std::string>::error(Error::create(ErrorCode::Unknown, "err"));
    // rvalue fallback should be accepted and returned
    auto s = result.valueOr(std::string("fallback"));
    EXPECT_EQ(s, "fallback");
}

// Test: Result<T> with string type
TEST(ResultTest, StringResult)
{
    auto result = Result<std::string>::ok("test string");

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), "test string");
}

// Test: Result<T> error access throws on success
TEST(ResultTest, ErrorAccessThrowsOnSuccess)
{
    auto result = Result<int>::ok(42);
    // Accessing error() on a success Result throws bad_optional_access
    EXPECT_THROW(result.error(), std::bad_optional_access);
}

// Test: Result<T> value access throws on error
TEST(ResultTest, ValueAccessThrowsOnError)
{
    auto error = Error::create(ErrorCode::NotFound, "Not found");
    auto result = Result<int>::error(error);
    // Accessing value() on an error Result throws bad_optional_access
    EXPECT_THROW(result.value(), std::bad_optional_access);
}

// Test: Error code enum values
TEST(ResultTest, ErrorCodeEnum)
{
    EXPECT_EQ(static_cast<int>(ErrorCode::None), 0);
    EXPECT_NE(ErrorCode::InvalidArgument, ErrorCode::NotFound);
    EXPECT_NE(ErrorCode::DatabaseError, ErrorCode::Timeout);
}

// Test: Result<void> specialization methods
TEST(ResultTest, VoidSpecialization)
{
    auto success = Result<void>::ok();
    EXPECT_TRUE(success.isOk());

    auto error = Result<void>::error(Error::create(ErrorCode::Internal, "Internal error"));
    EXPECT_TRUE(error.isError());
}

// Test: Error context is moveable
TEST(ResultTest, ErrorContextMoveable)
{
    ErrorContext context;
    context["test"] = "value";

    auto error1 = Error::create(ErrorCode::Unknown, "Error 1", std::move(context));
    EXPECT_EQ(error1.context.size(), 1u);
    EXPECT_EQ(error1.context.at("test"), "value");
}

// Test: Result can be moved
TEST(ResultTest, ResultMoveable)
{
    auto result1 = Result<int>::ok(42);
    auto result2 = std::move(result1);

    EXPECT_TRUE(result2.isOk());
    EXPECT_EQ(result2.value(), 42);
}
