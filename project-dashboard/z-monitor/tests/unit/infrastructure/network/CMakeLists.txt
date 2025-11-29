# Unit tests for infrastructure network layer

add_executable(test_network_retry
    network_retry_test.cpp
)

target_link_libraries(test_network_retry
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    z_monitor_infrastructure
    z_monitor_domain
    Qt6::Core
    Qt6::Network
)

target_include_directories(test_network_retry
    PRIVATE
    ${Z_MONITOR_ROOT}/src # For infrastructure includes
)

add_test(NAME NetworkRetryTest COMMAND test_network_retry)
