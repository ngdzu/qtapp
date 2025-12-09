# Namespace Guidelines

## Overview

This document defines namespace organization and best practices for the Z Monitor project to ensure consistent code organization, prevent naming conflicts, and improve code maintainability.

## Namespace Hierarchy

### Rule: Follow DDD Layer Structure

**Namespaces MUST reflect Domain-Driven Design (DDD) layered architecture.**

### Root Namespace

All project code MUST be under the root namespace:

```cpp
namespace zmon {
    // All project code
}
```

### Layer-Based Namespace Structure

```cpp
namespace zmon {
    namespace dom { }       // Domain layer - entities, value objects, services, repositories
    namespace app { }       // Application layer - use cases, DTOs, commands, queries
    namespace infra { }     // Infrastructure layer - database, external services, logging, config
    namespace ui { }        // UI layer - UI components, controllers, presenters, views
    namespace share { }     // Shared kernel - common types, utilities, constants
}
```

**Rationale:**
- Keep namespaces flat and simple - additional nesting is unnecessary
- Each layer namespace contains all related components without further subdivision
- Easier to navigate and understand the codebase
- Class names should be descriptive enough to indicate their purpose
```

## Namespace Naming Conventions

### Rule: lowercase for Namespace Names

**All namespace names MUST use lowercase.**

✅ **Correct:**
```cpp
namespace zmon { }
namespace domain { }
namespace svc { }      // service
namespace repo { }     // repository
namespace app { }      // application
```

❌ **Incorrect:**
```cpp
namespace ZMon { }               // Wrong case - use lowercase
namespace Domain { }             // Wrong case - use lowercase
namespace ValueObjects { }       // PascalCase - wrong
namespace PatientManagement { }  // PascalCase - wrong
namespace value_objects { }      // Snake case - wrong
namespace patient_management { } // Snake case - wrong
```

### Rule: Prefer Short Namespace Names (3-5 Characters)

**Namespace names SHOULD be short (3-5 characters) to improve readability and reduce verbosity.**

✅ **Correct:**
```cpp
namespace zmon { }        // 4 chars - good
namespace dom { }         // 3 chars - good
namespace app { }         // 3 chars - good
namespace infra { }       // 5 chars - good
namespace ui { }          // 2 chars - acceptable for very common concepts
namespace svc { }         // 3 chars - good (service)
namespace repo { }        // 4 chars - good (repository)
```

❌ **Avoid (Too Long):**
```cpp
namespace zmonitor { }              // Too long - use 'zmon'
namespace application { }           // Too long - use 'app'
namespace infrastructure { }        // Too long - use 'infra'
namespace patientmanagement { }     // Too long - use 'patient' or 'patmgmt'
namespace vitalsignmonitoring { }   // Too long - use 'vital' or 'vitals'
```

**Rationale:**
- Reduces typing and improves code readability
- Makes fully qualified names more concise: `zmon::dom::Patient` vs `zmonitor::domain::Patient`
- Easier to read and scan in code reviews
- Common practice in industry (e.g., `std`, `fmt`, `spdlog`)

**Common Abbreviations:**
- `dom` - domain
- `app` - application
- `infra` - infrastructure
- `ui` - user interface
- `svc` - service
- `repo` - repository
- `dto` - data transfer object
- `cmd` - command
- `qry` - query
- `cfg` - configuration
- `log` - logging
- `db` - database
- `ext` - external

### Rule: Use Singular for Most Namespaces

**Use singular form for namespace names unless representing a collection concept.**

✅ **Correct:**
```cpp
namespace dom { }            // Singular (short form)
namespace svc { }            // Singular (short form)
namespace repo { }           // Singular (short form)
namespace entity { }         // Singular
```

✅ **Acceptable Plural (Collection Concepts):**
```cpp
namespace svc { }            // Multiple service classes (short form preferred)
namespace repo { }           // Multiple repository classes (short form preferred)
namespace entity { }         // Multiple entity classes
namespace util { }           // Multiple utility classes (short form preferred)
```

**Be consistent within a layer** - if you use `svc`, don't mix with `service`.

## Namespace Organization by Feature

### Rule: Group Related Features Together

For large domains, organize by feature/module within layers:

```cpp
namespace zmon::dom {
    class Patient;
    class PatientRegistrar;
    class VitalSignReading;
    class VitalSignAnalyzer;
    class Alert;
    class AlertGenerator;
    class MedicalDevice;
    class DeviceCalibrator;
}
```

**Or if you need feature separation, add one level only:**

```cpp
namespace zmon::dom {
    namespace patient {
        class Patient;
        class PatientRegistrar;
    }
    
    namespace vital {
        class VitalSignReading;
        class VitalSignAnalyzer;
    }
}
```

### Vertical Slice Organization (Alternative)

For feature-focused architecture, add one feature level:

```cpp
namespace zmon {
    namespace patient {
        // All patient-related code across layers
        class Patient;              // Domain
        class PatientService;       // Application
        class PatientRepository;    // Infrastructure
        class PatientController;    // Interface
    }
    
    namespace vital {
        // All vital sign-related code across layers
        class VitalSignReading;     // Domain
        class VitalSignService;     // Application
    }
}
```
```

**Choose one organization style and be consistent across the project.**

## Namespace Usage Patterns

### Rule: Never Use `using namespace` in Headers

**NEVER use `using namespace` directives in header files.**

❌ **Forbidden in Headers:**
```cpp
// MyClass.h
#pragma once

using namespace std;           // NEVER do this
using namespace zmon;          // NEVER do this

class MyClass {
    string name;  // Pollutes namespace for all includers
};
```

✅ **Correct in Headers:**
```cpp
// MyClass.h
#pragma once

#include <string>

namespace zmon::dom {

class MyClass {
    std::string name;  // Fully qualified
};

} // namespace zmon::dom
```

### Rule: Prefer Specific Using Declarations

**In implementation files, prefer specific `using` declarations over blanket `using namespace`.**

✅ **Preferred:**
```cpp
// MyClass.cpp
#include "MyClass.h"

using std::string;
using std::vector;
using std::make_unique;

namespace zmon::dom {

void MyClass::doSomething() {
    string name = "test";           // Clear and concise
    vector<int> values;             // Clear and concise
    auto ptr = make_unique<int>(5); // Clear and concise
}

} // namespace zmon::dom
```

⚠️ **Acceptable but Less Preferred:**
```cpp
// MyClass.cpp
#include "MyClass.h"

namespace zmon::dom {

using namespace std;  // OK in .cpp, but be cautious

void MyClass::doSomething() {
    string name = "test";
    vector<int> values;
}

} // namespace zmon::dom
```

### Rule: Use Nested Namespace Definition (C++17)

**Use nested namespace definition syntax for cleaner code (C++17 and later).**

✅ **Correct (C++17+):**
```cpp
namespace zmon::dom {

class Patient {
    // ...
};

} // namespace zmon::dom
```

**Or with optional feature grouping:**
```cpp
namespace zmon::dom::patient {

class Patient {
    // ...
};

} // namespace zmon::dom::patient
```

❌ **Avoid (Old Style):**
```cpp
namespace zmon {
namespace dom {
namespace patient {

class Patient {
    // ...
};

} // namespace patient
} // namespace dom
} // namespace zmon
```

### Rule: Always Close Namespaces with Comments

**Always include closing comments for namespaces to improve readability.**

✅ **Correct:**
```cpp
namespace zmon::dom {

class Patient {
    // ...
};

} // namespace zmon::dom
```

❌ **Incorrect:**
```cpp
namespace zmon::dom {

class Patient {
    // ...
};

}  // No comment - unclear what this closes
```

## Namespace Aliases

### Rule: Use Aliases for Long Namespaces

**Use namespace aliases to improve readability of long namespace names.**

✅ **Correct:**
```cpp
// With short and flat namespaces, aliases are rarely needed
zmon::dom::Patient patient;  // Simple and clear

// Only use aliases if you're typing the same namespace repeatedly:
namespace dom = zmon::dom;

dom::Patient p;
dom::VitalSignReading reading;
```

✅ **In Implementation Files:**
```cpp
// PatientService.cpp
namespace {
    namespace dom = zmon::dom;
    namespace infra = zmon::infra;
}

void PatientService::registerPatient() {
    dom::Patient patient;
    infra::PatientRepository* repo = ...;
}
```

### Rule: Use Anonymous Namespaces for Internal Linkage

**Use anonymous namespaces for implementation-only helpers (instead of `static`).**

✅ **Correct:**
```cpp
// MyClass.cpp
namespace zmon::dom {

namespace {
    // Internal helper - not visible outside this translation unit
    bool isValidName(const std::string& name) {
        return !name.empty();
    }
    
    constexpr int MAX_RETRIES = 3;
}

void MyClass::doSomething() {
    if (isValidName(name_)) {
        // ...
    }
}

} // namespace zmon::dom
```

❌ **Avoid (Old C Style):**
```cpp
// MyClass.cpp
namespace zmon::dom {

// Static functions - old C style
static bool isValidName(const std::string& name) {
    return !name.empty();
}

} // namespace zmon::dom
```

## Namespace and File Organization

### Rule: One Primary Class Per Namespace Per File

**Each file should contain one primary class/interface in its own namespace.**

✅ **Correct:**
```cpp
// Patient.h
#pragma once

namespace zmon::dom {

/**
 * @brief Represents a patient in the medical monitoring system
 */
class Patient {
    // ...
};

} // namespace zmon::dom
```

### Rule: Match Directory Structure to Namespace Hierarchy

**Directory structure SHOULD reflect namespace hierarchy for easy navigation.**

✅ **Correct Structure:**
```
src/
  domain/
    Patient.h
    Patient.cpp
    BloodPressure.h
    BloodPressure.cpp
    VitalSignAnalyzer.h
    VitalSignAnalyzer.cpp
  application/
    PatientRegistrationService.h
    PatientRegistrationService.cpp
    RegisterPatientRequest.h
    PatientDetailsResponse.h
  infrastructure/
    PostgreSqlPatientRepository.h
    PostgreSqlPatientRepository.cpp
  ui/
    PatientController.h
    PatientController.cpp
```

**Corresponding Namespaces:**
```cpp
// src/domain/Patient.h
namespace zmon::dom { class Patient; }

// src/domain/BloodPressure.h
namespace zmon::dom { class BloodPressure; }

// src/application/PatientRegistrationService.h
namespace zmon::app { class PatientRegistrationService; }

// src/infrastructure/PostgreSqlPatientRepository.h
namespace zmon::infra { class PostgreSqlPatientRepository; }
```

## Namespace Versioning

### Rule: Use Inline Namespaces for API Versioning

**Use inline namespaces for versioned APIs to support backward compatibility.**

✅ **Correct:**
```cpp
namespace zmon::api {
    inline namespace v2 {
        class PatientController {
            // Current version
        };
    }
    
    namespace v1 {
        class PatientController {
            // Legacy version
        };
    }
}

// Users get v2 by default
zmon::api::PatientController controller;  // Uses v2

// Can explicitly use v1 if needed
zmon::api::v1::PatientController legacyController;
```

## Common Anti-Patterns to Avoid

### ❌ Anti-Pattern 1: Namespace Pollution

**Don't dump everything into the root namespace:**

```cpp
namespace zmon {
    class Patient;           // Should be in dom
    class PatientRepository; // Should be in infra
    class PatientController; // Should be in ui
}
```

### ❌ Anti-Pattern 2: Over-Nesting

**Avoid excessive namespace nesting that makes code harder to read:**

```cpp
// Too deep! Keep it to 2-3 levels maximum
namespace zmon::dom::patient::detail::impl { }
```

**Limit to 2-3 levels maximum:**
- ✅ `zmon::dom` - Good (2 levels)
- ✅ `zmon::dom::patient` - Acceptable (3 levels, for feature organization)
- ✅ `zmon::dom::test` - Acceptable (3 levels, for testing)
- ❌ `zmon::dom::patient::detail` - Too deep (4 levels)

### ❌ Anti-Pattern 3: Inconsistent Naming

**Don't mix naming styles within the same project:**

```cpp
namespace zmon {
    namespace dom { }           // lowercase - correct
    namespace Application { }   // PascalCase - wrong
    namespace INFRASTRUCTURE { } // UPPERCASE - wrong
}
```

**Pick one style (lowercase) and stick with it.**

### ❌ Anti-Pattern 4: Using Namespace for Access Control

**Don't rely on namespaces for access control - use class access specifiers:**

```cpp
namespace zmon::internal {  // Don't do this
    class PrivateHelper { };     // Use private members instead
}
```

**Use `private`, `protected`, `public` and friend declarations instead.**

## Testing Namespace Organization

### Rule: Mirror Production Namespace Structure in Tests

**Test namespaces SHOULD mirror production code namespaces with a `Tests` suffix.**

✅ **Correct:**
```cpp
// Production: src/domain/Patient.h
namespace zmon::dom {
    class Patient;
}

// Test: tests/domain/PatientTests.cpp
namespace zmon::dom::test {
    class PatientTests : public ::testing::Test { };
}
```

### Rule: Place Mocks in Separate Namespace

**Mock classes SHOULD be in a `mock` namespace:**

```cpp
// tests/mocks/MockPatientRepository.h
namespace zmon::dom::mock {
    class MockPatientRepository : public IPatientRepository {
        // Mock implementation
    };
}
```

## Documentation Requirements

### Rule: Document Namespace Purpose

**Each namespace SHOULD have a comment explaining its purpose (in a representative header).**

✅ **Correct:**
```cpp
/**
 * @namespace zmon::dom
 * @brief Contains core domain logic and business rules
 * 
 * This namespace contains all domain entities, value objects, domain services,
 * and repository interfaces. Code in this namespace should not depend on
 * infrastructure or application layers.
 */
namespace zmon::dom {
    // ...
}

/**
 * @namespace zmon::app
 * @brief Application layer containing use cases and orchestration logic
 * 
 * Application services coordinate domain objects to fulfill use cases.
 * This layer depends on the domain layer but not on infrastructure.
 */
namespace zmon::app {
    // ...
}
```

## Quick Reference

### Namespace Checklist

- [ ] Namespaces follow DDD layer structure
- [ ] Namespaces use lowercase naming
- [ ] No `using namespace` in header files
- [ ] Nested namespace syntax used (C++17+)
- [ ] All namespaces have closing comments
- [ ] Directory structure mirrors namespace hierarchy
- [ ] Namespace aliases used for long names
- [ ] Anonymous namespaces used for internal helpers
- [ ] Test namespaces mirror production structure
- [ ] Namespace purpose documented

## Related Guidelines

- See `naming_conventions.md` for class and file naming
- See `cpp_guidelines.md` for general C++ coding standards
- See `api_documentation.md` for documentation requirements
- See `DRY.md` for code organization principles
