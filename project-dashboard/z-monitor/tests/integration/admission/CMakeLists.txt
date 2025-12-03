add_executable(AdmissionWorkflowTest
    AdmissionWorkflowTest.cpp
)

target_link_libraries(AdmissionWorkflowTest
    PRIVATE
    z_monitor_application
    z_monitor_infrastructure
    Qt6::Core
    Qt6::Sql
    GTest::gtest_main
)

target_include_directories(AdmissionWorkflowTest
    PRIVATE
    ${Z_MONITOR_ROOT}
)

# Ensure schema is generated before building/running
add_dependencies(AdmissionWorkflowTest generate_schema)

add_test(NAME AdmissionWorkflowTest COMMAND AdmissionWorkflowTest)
