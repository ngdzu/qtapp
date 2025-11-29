#
# Domain common utilities - pure C++ helpers (no Qt, no framework deps)
#
# This target is header-only and can be reused by other targets (e.g. simulator)
# without pulling in the full domain library or any Qt dependencies.
#

add_library(zmon_domain_common INTERFACE)

target_include_directories(zmon_domain_common
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)


