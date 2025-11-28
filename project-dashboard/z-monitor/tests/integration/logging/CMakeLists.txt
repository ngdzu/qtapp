# Logging integration tests

# Async logging integration tests
add_executable(test_async_logging
    AsyncLoggingTest.cpp
)

target_include_directories(test_async_logging
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../unit/logging
        ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

target_link_libraries(test_async_logging
    PRIVATE
        mock_log_backend  # Defined in unit/logging/CMakeLists.txt
        z_monitor_infrastructure
        Qt6::Core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME AsyncLoggingTests COMMAND test_async_logging)

