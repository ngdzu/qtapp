# Domain events unit tests

add_executable(DomainEventDispatcherTest
    DomainEventDispatcherTest.cpp
)

target_link_libraries(DomainEventDispatcherTest
    PRIVATE
    z_monitor_domain
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(DomainEventDispatcherTest
    PRIVATE
    ${Z_MONITOR_SOURCE_DIR}
)

add_test(NAME DomainEventDispatcherTest COMMAND DomainEventDispatcherTest)

set_tests_properties(DomainEventDispatcherTest PROPERTIES
    LABELS "unit;domain;events"
    TIMEOUT 30
)
