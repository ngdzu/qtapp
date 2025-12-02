# Shared test fixtures library

add_library(test_fixtures STATIC
    DatabaseTestFixture.cpp
    RepositoryTestFixture.cpp
)

# Link against necessary project libraries and Qt
# GTest::gtest is linked PUBLIC so fixtures can derive from ::testing::Test
# without each target needing to link it manually (test executables still link gtest_main).
target_link_libraries(test_fixtures
    PUBLIC
    z_monitor_infrastructure
    z_monitor_domain
    Qt6::Core
    Qt6::Sql
    GTest::gtest
)

# Expose root include path so fixtures can include project headers
target_include_directories(test_fixtures
    PUBLIC
    ${Z_MONITOR_ROOT}
)

# Provide source dir macro for locating migrations on disk
target_compile_definitions(test_fixtures
    PUBLIC
    Z_MONITOR_SOURCE_DIR="${Z_MONITOR_ROOT}"
)

# Enable USE_QXORM propagation if defined
if(USE_QXORM)
    target_link_libraries(test_fixtures PUBLIC QxOrm)
    target_compile_definitions(test_fixtures PUBLIC USE_QXORM)
endif()
