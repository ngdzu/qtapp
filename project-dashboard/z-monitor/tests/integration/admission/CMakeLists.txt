add_executable(AdmissionWorkflowTest
    AdmissionWorkflowTest.cpp
    ${Z_MONITOR_ROOT}/src/interface/controllers/PatientController.cpp
)

target_link_libraries(AdmissionWorkflowTest
    PRIVATE
    z_monitor_application
    z_monitor_infrastructure
    zmon_test_mocks_infrastructure
    Qt6::Core
    Qt6::Sql
    Qt6::Test
    GTest::gtest
)

target_include_directories(AdmissionWorkflowTest
    PRIVATE
    ${Z_MONITOR_ROOT}
)

# Ensure schema is generated before building/running
add_dependencies(AdmissionWorkflowTest generate_schema)

add_test(NAME AdmissionWorkflowTest COMMAND AdmissionWorkflowTest)
