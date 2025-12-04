# Unit tests for infrastructure security

add_executable(test_certificate_manager
    CertificateManagerTest.cpp
)

target_link_libraries(test_certificate_manager
    PRIVATE
    test_fixtures
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    GTest::gtest_main
)

target_include_directories(test_certificate_manager
    PRIVATE
    ${Z_MONITOR_ROOT}/src
    ${Z_MONITOR_ROOT}/tests
)

add_test(NAME CertificateManagerTest COMMAND test_certificate_manager)
