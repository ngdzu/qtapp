# Test targets - unit, integration, and e2e tests

# Enable testing
enable_testing()

# Find or download GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Unit tests
add_subdirectory(unit)

# Integration tests
add_subdirectory(integration)

# End-to-end tests (to be implemented)
# add_subdirectory(e2e)

# Benchmarks (to be implemented)
# add_subdirectory(benchmarks)
