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
    ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME DomainEventDispatcherTest COMMAND DomainEventDispatcherTest)

set_tests_properties(DomainEventDispatcherTest PROPERTIES
    LABELS "unit;domain;events"
    TIMEOUT 30
)
