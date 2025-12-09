# Unit tests

# Mocks library (for use in tests)
# Mocks are added via tests/CMakeLists.txt, so they're available here

# Domain layer tests
add_subdirectory(domain)

# Logging unit tests
add_subdirectory(logging)

# Core service tests
add_subdirectory(core)

# Infrastructure persistence tests
add_subdirectory(infrastructure/persistence)

# Infrastructure network tests
add_subdirectory(infrastructure/network)

# Infrastructure security tests
add_subdirectory(infrastructure/security)

# Interface controller tests
add_subdirectory(ui/controllers)

# Application tests (services + config)
add_subdirectory(application)
