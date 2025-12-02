---
doc_id: DOC-API-XXX
title: {Interface Name} Interface Contract
version: v1.0
category: API
subcategory: {Domain/Infrastructure/Application}
status: Draft
owner: {Team Name}
reviewers: 
  - Architecture Team
  - Client Teams
last_reviewed: YYYY-MM-DD
next_review: YYYY-MM-DD
related_docs:
  - DOC-COMP-XXX # Component that implements this
  - DOC-ARCH-XXX # Architecture context
related_tasks:
  - TASK-XXX-XXX # Implementation task
related_requirements:
  - REQ-XXX-XXX  # Related requirement
tags:
  - interface
  - api
  - {keyword}
diagram_files:
  - DOC-API-XXX_class_diagram.mmd
  - DOC-API-XXX_class_diagram.svg
---

# {DOC-ID}: {Interface Name}

## 1. Overview

**Purpose:** {What this interface abstracts and why}

**Responsibilities:**
- {Key responsibility 1}
- {Key responsibility 2}

**Layer:** {Domain / Application / Infrastructure}

**Implementations:**
- {ImplementationClass1} - {Description}
- {ImplementationClass2} - {Description}
- {MockImplementation} - {For testing}

## 2. Interface Definition

### 2.1 Class Hierarchy

![Class Diagram](diagrams/DOC-API-XXX_class_diagram.svg)

### 2.2 Header File

**File:** `src/{layer}/interfaces/{InterfaceName}.h`

```cpp
#ifndef I{INTERFACE_NAME}_H
#define I{INTERFACE_NAME}_H

#include "common/Result.h"
#include "domain/types/ErrorTypes.h"

/**
 * @brief {Interface purpose}
 * 
 * {Detailed description}
 * 
 * @threadsafety {Thread-safe / Not thread-safe}
 * @ownership {Ownership semantics}
 */
class I{InterfaceName} {
public:
    virtual ~I{InterfaceName}() = default;

    /**
     * @brief {Method description}
     * @param param1 {Description}
     * @return {Return value description}
     * @throws {Exception conditions if any}
     */
    virtual Result<ReturnType, Error> Method1(ParamType param1) = 0;

    /**
     * @brief {Method description}
     * @param param1 {Description}
     * @param param2 {Description}
     * @return {Return value description}
     */
    virtual Result<void, Error> Method2(ParamType1 param1, ParamType2 param2) = 0;
};

#endif // I{INTERFACE_NAME}_H
```

## 3. Method Specifications

### 3.1 Method1

**Signature:**
```cpp
virtual Result<ReturnType, Error> Method1(ParamType param1) = 0;
```

**Purpose:** {What this method does}

**Parameters:**
- `param1` ({Type}): {Description, constraints, validation rules}

**Returns:**
- **Success:** `Result<ReturnType>` containing {description}
- **Failure:** `Result<Error>` with error code:
  - `ERROR_CODE_1`: {When and why}
  - `ERROR_CODE_2`: {When and why}

**Preconditions:**
- {Condition 1 that must be true before calling}
- {Condition 2}

**Postconditions:**
- {Condition 1 that will be true after successful execution}
- {Condition 2}

**Thread Safety:** {Thread-safe / Not thread-safe / Conditionally thread-safe}

**Performance:** {Expected time/space complexity}

**Example:**
```cpp
auto result = interface->Method1(param);
if (result.isOk()) {
    auto value = result.value();
    // Use value
} else {
    // Handle error: result.error()
}
```

### 3.2 Method2

{Repeat format for each method}

## 4. Error Codes

### 4.1 Error Enum

```cpp
enum class {InterfaceName}Error {
    INVALID_PARAMETER,
    OPERATION_FAILED,
    TIMEOUT,
    NOT_INITIALIZED
};
```

### 4.2 Error Descriptions

| Error Code          | Description      | Recovery Strategy |
| ------------------- | ---------------- | ----------------- |
| `INVALID_PARAMETER` | {When it occurs} | {How to handle}   |
| `OPERATION_FAILED`  | {When it occurs} | {How to handle}   |
| `TIMEOUT`           | {When it occurs} | {How to handle}   |

## 5. Lifecycle & Ownership

**Creation:** {How instances are created - factory, DI container, etc.}

**Initialization:** {Initialization requirements}

**Cleanup:** {Cleanup requirements, resource disposal}

**Ownership:** {Who owns instances, when they're destroyed}

## 6. Threading Model

**Thread Safety:** {Thread-safe / Not thread-safe}

**Concurrent Access:** {How concurrent calls are handled}

**Synchronization:** {What synchronization primitives are used}

## 7. Implementations

### 7.1 Production Implementation

**Class:** `{ImplementationClass}`

**File:** `src/{layer}/{module}/{ImplementationClass}.h/cpp`

**Description:** {What this implementation does}

**Documentation:** See DOC-COMP-XXX

### 7.2 Mock Implementation

**Class:** `Mock{InterfaceName}`

**File:** `tests/mocks/{layer}/Mock{InterfaceName}.h/cpp`

**Purpose:** {For testing, development, etc.}

**Key Features:**
- {Feature 1}
- {Feature 2}

## 8. Usage Patterns

### 8.1 Dependency Injection

```cpp
class ServiceClass {
public:
    ServiceClass(I{InterfaceName}* interface)
        : m_interface(interface) {}

private:
    I{InterfaceName}* m_interface;
};
```

### 8.2 Factory Pattern

```cpp
std::unique_ptr<I{InterfaceName}> CreateInterface() {
    if (productionMode) {
        return std::make_unique<ProductionImpl>();
    } else {
        return std::make_unique<MockImpl>();
    }
}
```

## 9. Testing

### 9.1 Interface Contract Tests

**Test File:** `tests/unit/interfaces/I{InterfaceName}Test.cpp`

**Test all implementations against contract:**
- {Test scenario 1}
- {Test scenario 2}

### 9.2 Mock Usage in Tests

```cpp
class MockImpl : public I{InterfaceName} {
    MOCK_METHOD(Result<ReturnType, Error>, Method1, (ParamType), (override));
    MOCK_METHOD(Result<void, Error>, Method2, (ParamType1, ParamType2), (override));
};
```

## 10. Related Documentation

### Components
- [DOC-COMP-XXX: Implementation](../components/DOC-COMP-XXX_component.md)

### Architecture
- [DOC-ARCH-XXX: Architecture Context](../architecture/DOC-ARCH-XXX_overview.md)

### Guidelines
- [DOC-GUIDE-XXX: Interface Design Guidelines](../guidelines/DOC-GUIDE-XXX_guidelines.md)

## 11. Changelog

### v1.0 (YYYY-MM-DD)
- Initial interface definition
