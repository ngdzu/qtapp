# Application config tests

add_executable(test_application_config
    ConfigLoaderTest.cpp
)

target_link_libraries(test_application_config
    PRIVATE
    GTest::gtest_main
    Qt6::Core
    z_monitor_application
)

# Register test with CTest
add_test(NAME ApplicationConfigTests COMMAND test_application_config)
