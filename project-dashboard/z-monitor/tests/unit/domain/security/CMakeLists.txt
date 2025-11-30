# PermissionRegistry unit tests

add_executable(test_permission_registry
    PermissionRegistryTest.cpp
)

target_link_libraries(test_permission_registry PRIVATE
    z_monitor_domain
    Qt6::Core
    GTest::gtest_main
)

# Add test to CTest
add_test(NAME PermissionRegistryTest COMMAND test_permission_registry)
