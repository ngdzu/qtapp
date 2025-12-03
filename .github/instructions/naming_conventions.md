# Naming Conventions

## Overview

This document defines naming conventions for classes, files, and components in the Z Monitor project to ensure consistency and clarity.

## Mock Classes and Test Doubles

### Rule: Mock Classes Must Use "Mock" Prefix

**All mock implementations, test doubles, and fake objects MUST use the "Mock" prefix in their class name.**

### Examples

✅ **Correct:**
- `MockNetworkManager` - Mock implementation of network manager
- `MockTelemetryServer` - Mock implementation of telemetry server
- `MockPatientRepository` - Mock implementation of patient repository
- `MockUserManagementService` - Mock implementation of user management service

❌ **Incorrect:**
- `NetworkManager` - If it's a mock, it should be `MockNetworkManager`
- `TestNetworkManager` - Use "Mock" prefix, not "Test"
- `FakeNetworkManager` - Use "Mock" prefix for consistency

### Rationale

1. **Clarity:** The "Mock" prefix immediately indicates this is a test double, not a production implementation
2. **Consistency:** All mocks follow the same naming pattern, making them easy to identify
3. **Searchability:** Easy to find all mocks by searching for "Mock" prefix
4. **Documentation:** Clear distinction between production code and test code

### Location

- Mock classes should be placed in `tests/mocks/` directory structure
- If a mock is temporarily in `src/` for development, it MUST still use "Mock" prefix
- Production implementations should NOT use "Mock" prefix

### When to Use Mock Prefix

Use "Mock" prefix when:
- ✅ Implementing a test double for an interface
- ✅ Creating a fake implementation for testing
- ✅ Building a temporary mock implementation before real implementation
- ✅ Creating a development-only implementation

Do NOT use "Mock" prefix when:
- ❌ Implementing the actual production class
- ❌ Creating a real implementation (even if simplified)

## File Naming

### Rule: File Names Match Class Names

**File names MUST match the class name exactly (case-sensitive).**

### Examples

✅ **Correct:**
- Class: `MockNetworkManager` → Files: `MockNetworkManager.h`, `MockNetworkManager.cpp`
- Class: `NetworkManager` → Files: `NetworkManager.h`, `NetworkManager.cpp`

❌ **Incorrect:**
- Class: `MockNetworkManager` → Files: `NetworkManager.h`, `NetworkManager.cpp`
- Class: `MockNetworkManager` → Files: `mock_network_manager.h`, `mock_network_manager.cpp`

## Related Guidelines

- See `cpp_guidelines.mdc` for general C++ naming conventions
- See `api_documentation.mdc` for documentation requirements
- See `ztodo_verification.mdc` for testing requirements
