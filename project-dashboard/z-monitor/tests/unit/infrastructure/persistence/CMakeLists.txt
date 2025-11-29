# Unit tests for infrastructure persistence layer

add_executable(test_schema_generation
    test_schema_generation.cpp
)

target_link_libraries(test_schema_generation
    PRIVATE
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(test_schema_generation
    PRIVATE
        ${Z_MONITOR_ROOT}  # For generated/SchemaInfo.h
)

add_test(NAME SchemaGenerationTest COMMAND test_schema_generation)

