# Logging unit tests

# Mock backend (used by tests)
add_library(mock_log_backend STATIC
    MockLogBackend.h
    MockLogBackend.cpp
)

target_include_directories(mock_log_backend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mock_log_backend
    PUBLIC
        z_monitor_infrastructure
        Qt6::Core
)

# ILogBackend tests
add_executable(test_ilog_backend
    ILogBackendTest.cpp
)

target_include_directories(test_ilog_backend
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

target_link_libraries(test_ilog_backend
    PRIVATE
        mock_log_backend
        z_monitor_infrastructure
        Qt6::Core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME ILogBackendTests COMMAND test_ilog_backend)

# CustomBackend tests
add_executable(test_custom_backend
    CustomBackendTest.cpp
)

target_include_directories(test_custom_backend
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

target_link_libraries(test_custom_backend
    PRIVATE
        z_monitor_infrastructure
        Qt6::Core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME CustomBackendTests COMMAND test_custom_backend)

# LogService tests
add_executable(test_log_service
    LogServiceTest.cpp
)

target_include_directories(test_log_service
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_log_service
    PRIVATE
        mock_log_backend
        z_monitor_infrastructure
        Qt6::Core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME LogServiceTests COMMAND test_log_service)

