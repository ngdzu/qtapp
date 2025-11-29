# Domain common utilities unit tests

# Result type tests
add_executable(result_test
    ResultTest.cpp
)

target_link_libraries(result_test
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        zmon_domain_common
)

target_include_directories(result_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/z-monitor/src
)

add_test(NAME ResultTest COMMAND result_test)

# RetryPolicy tests
add_executable(retry_policy_test
    RetryPolicyTest.cpp
)

target_link_libraries(retry_policy_test
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        zmon_domain_common
)

target_include_directories(retry_policy_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/z-monitor/src
)

add_test(NAME RetryPolicyTest COMMAND retry_policy_test)

# CircuitBreaker tests
add_executable(circuit_breaker_test
    CircuitBreakerTest.cpp
)

target_link_libraries(circuit_breaker_test
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        zmon_domain_common
)

target_include_directories(circuit_breaker_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/z-monitor/src
)

add_test(NAME CircuitBreakerTest COMMAND circuit_breaker_test)

