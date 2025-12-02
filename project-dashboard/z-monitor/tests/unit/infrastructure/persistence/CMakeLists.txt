# Unit tests for infrastructure persistence layer
# These are TRUE unit tests - no database connections, no file I/O

add_executable(test_schema_generation
    test_schema_generation.cpp
)

target_link_libraries(test_schema_generation
    PRIVATE
    GTest::gtest_main
)

target_include_directories(test_schema_generation
    PRIVATE
    ${Z_MONITOR_ROOT}/src # For generated/SchemaInfo.h and infrastructure includes
)

add_test(NAME SchemaGenerationTest COMMAND test_schema_generation)
