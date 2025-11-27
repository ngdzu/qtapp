# Code Organization & Module Structure

This document defines the code organization, module structure, namespace conventions, and file organization patterns for the Z Monitor application.

## 1. Guiding Principles

- **Separation of Concerns:** Clear boundaries between UI, business logic, and data access
- **Modularity:** Components should be loosely coupled and highly cohesive
- **Testability:** Code organization should facilitate unit testing and mocking
- **Maintainability:** Clear structure makes code easy to understand and modify
- **Scalability:** Structure should support growth without refactoring

## 2. Directory Structure

### 2.1. Top-Level Organization

```
z-monitor/
├── src/                          # Source code
│   ├── core/                     # Core services (business logic)
│   ├── controllers/              # UI controllers (Qt/QML bridge)
│   ├── interfaces/               # Abstract interfaces
│   ├── models/                   # Data models
│   ├── utils/                    # Utility functions and helpers
│   └── main.cpp                  # Application entry point
├── qml/                          # QML UI files
│   ├── views/                    # Full-screen views
│   ├── components/               # Reusable QML components
│   ├── dialogs/                  # Dialog components
│   └── Main.qml                  # Root QML file
├── resources/                    # Resources (images, fonts, etc.)
├── tests/                        # Test code
│   ├── unit/                     # Unit tests
│   ├── integration/               # Integration tests
│   └── e2e/                      # End-to-end tests
├── scripts/                      # Build and utility scripts
├── docs/                         # Documentation
└── CMakeLists.txt               # Build configuration
```

### 2.2. Source Code Organization (`src/`)

```
src/
├── core/                         # Core services (no UI dependencies)
│   ├── DeviceSimulator.cpp/h
│   ├── AlarmManager.cpp/h
│   ├── NetworkManager.cpp/h
│   ├── DatabaseManager.cpp/h
│   ├── PatientManager.cpp/h
│   ├── SettingsManager.cpp/h
│   ├── AuthenticationService.cpp/h
│   ├── LogService.cpp/h
│   ├── DataArchiver.cpp/h
│   ├── ProvisioningService.cpp/h
│   └── AdmissionService.cpp/h
├── controllers/                  # UI controllers (Qt/QML bridge)
│   ├── DashboardController.cpp/h
│   ├── AlarmController.cpp/h
│   ├── TrendsController.cpp/h
│   ├── SystemController.cpp/h
│   ├── PatientController.cpp/h
│   ├── SettingsController.cpp/h
│   ├── NotificationController.cpp/h
│   ├── ProvisioningController.cpp/h
│   └── AdmissionController.cpp/h
├── interfaces/                   # Abstract interfaces
│   ├── IDeviceSimulator.h
│   ├── IPatientLookupService.h
│   ├── ITelemetryServer.h
│   └── IProvisioningService.h
├── models/                       # Data models
│   ├── Patient.h
│   ├── VitalSign.h
│   ├── Alarm.h
│   ├── TelemetryData.h
│   └── User.h
├── utils/                        # Utility functions
│   ├── CryptoUtils.cpp/h
│   ├── DateTimeUtils.cpp/h
│   ├── StringUtils.cpp/h
│   └── ValidationUtils.cpp/h
└── main.cpp
```

### 2.3. QML Organization (`qml/`)

```
qml/
├── views/                        # Full-screen views
│   ├── DashboardView.qml
│   ├── TrendsView.qml
│   ├── SettingsView.qml
│   ├── DiagnosticsView.qml
│   ├── LoginView.qml
│   └── AdmissionModal.qml
├── components/                   # Reusable components
│   ├── StatCard.qml
│   ├── PatientBanner.qml
│   ├── AlarmIndicator.qml
│   ├── NotificationBell.qml
│   ├── Sidebar.qml
│   ├── TopBar.qml
│   └── WaveformDisplay.qml
├── dialogs/                      # Dialog components
│   ├── ConfirmationDialog.qml
│   ├── ErrorDialog.qml
│   └── InfoDialog.qml
└── Main.qml                      # Root QML file
```

## 3. Namespace Conventions

### 3.1. Namespace Hierarchy

```cpp
namespace ZMonitor {
    // Core services
    namespace Core {
        class DeviceSimulator;
        class AlarmManager;
        class NetworkManager;
        // ...
    }
    
    // UI controllers
    namespace Controllers {
        class DashboardController;
        class AlarmController;
        // ...
    }
    
    // Interfaces
    namespace Interfaces {
        class IDeviceSimulator;
        class IPatientLookupService;
        // ...
    }
    
    // Models
    namespace Models {
        struct Patient;
        struct VitalSign;
        // ...
    }
    
    // Utilities
    namespace Utils {
        class CryptoUtils;
        class DateTimeUtils;
        // ...
    }
}
```

### 3.2. Namespace Usage

- **Core Services:** `ZMonitor::Core::`
- **Controllers:** `ZMonitor::Controllers::`
- **Interfaces:** `ZMonitor::Interfaces::`
- **Models:** `ZMonitor::Models::`
- **Utilities:** `ZMonitor::Utils::`

### 3.3. Using Declarations

Prefer explicit namespace usage in headers, allow `using` in implementation files:

```cpp
// Header file
namespace ZMonitor {
namespace Core {
    class NetworkManager : public QObject {
        // ...
    };
}
}

// Implementation file
#include "NetworkManager.h"
using namespace ZMonitor::Core;

NetworkManager::NetworkManager() {
    // Can use NetworkManager without namespace prefix
}
```

## 4. Module Boundaries

### 4.1. Core Module

**Purpose:** Business logic, no UI dependencies

**Dependencies:**
- Qt Core (QObject, QThread, etc.)
- Qt Network (for NetworkManager)
- Qt SQL (for DatabaseManager)
- Standard C++ library

**Dependencies NOT Allowed:**
- Qt Widgets
- Qt Quick
- QML types

**Components:**
- All services in `src/core/`
- All interfaces in `src/interfaces/`
- All models in `src/models/`

### 4.2. Controller Module

**Purpose:** Bridge between Core and QML UI

**Dependencies:**
- Core module
- Qt Core
- Qt Quick (for Q_PROPERTY, signals/slots)

**Dependencies NOT Allowed:**
- Qt Widgets
- Direct QML file access

**Components:**
- All controllers in `src/controllers/`

### 4.3. UI Module (QML)

**Purpose:** User interface

**Dependencies:**
- Controllers (via QML property bindings)
- Qt Quick
- Qt Quick Controls

**Dependencies NOT Allowed:**
- Direct C++ class access (except via controllers)
- Core services (must go through controllers)

## 5. File Naming Conventions

### 5.1. C++ Files

- **Headers:** `ClassName.h` (PascalCase)
- **Implementation:** `ClassName.cpp` (PascalCase)
- **One class per file:** Each class gets its own `.h` and `.cpp` file

### 5.2. QML Files

- **Views:** `ViewNameView.qml` (PascalCase, "View" suffix)
- **Components:** `ComponentName.qml` (PascalCase)
- **Dialogs:** `DialogNameDialog.qml` (PascalCase, "Dialog" suffix)

### 5.3. Test Files

- **Unit Tests:** `TestClassName.cpp` (prefix "Test")
- **Integration Tests:** `IntegrationTestFeature.cpp` (prefix "IntegrationTest")
- **E2E Tests:** `E2ETestScenario.cpp` (prefix "E2ETest")

## 6. Include Organization

### 6.1. Header File Includes

Order of includes:

1. Corresponding header file (for `.cpp` files)
2. Qt headers (grouped by module)
3. Standard library headers
4. Third-party library headers
5. Project headers (grouped by module)

```cpp
// NetworkManager.cpp
#include "NetworkManager.h"           // 1. Corresponding header

#include <QNetworkAccessManager>       // 2. Qt headers
#include <QSslConfiguration>
#include <QJsonDocument>

#include <memory>                      // 3. Standard library
#include <chrono>

#include "core/DatabaseManager.h"      // 4. Project headers (core)
#include "interfaces/ITelemetryServer.h"
#include "utils/CryptoUtils.h"
```

### 6.2. Include Guards

Use `#pragma once` (preferred) or traditional include guards:

```cpp
#pragma once

#include <QObject>

namespace ZMonitor {
namespace Core {
    class NetworkManager : public QObject {
        // ...
    };
}
}
```

## 7. Class Organization

### 7.1. Class Member Order

1. **Public types and enums**
2. **Public constructors/destructor**
3. **Public methods**
4. **Public slots** (Qt-specific)
5. **Signals** (Qt-specific)
6. **Public properties** (Q_PROPERTY)
7. **Protected members**
8. **Private members**

```cpp
class NetworkManager : public QObject {
    Q_OBJECT
    
public:
    // 1. Types
    enum class ConnectionStatus { ... };
    
    // 2. Constructors
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager();
    
    // 3. Public methods
    void connectToServer();
    void sendTelemetry(const TelemetryData& data);
    
public slots:
    // 4. Public slots
    void onSettingsChanged();
    
signals:
    // 5. Signals
    void connectionStatusChanged(ConnectionStatus status);
    
    // 6. Properties
    Q_PROPERTY(ConnectionStatus status READ status NOTIFY statusChanged)
    
protected:
    // 7. Protected members
    
private:
    // 8. Private members
    ITelemetryServer* m_server;
};
```

## 8. Dependency Rules

### 8.1. Dependency Direction

```
QML → Controllers → Core → Interfaces
```

- **QML** depends on **Controllers** (via property bindings)
- **Controllers** depend on **Core** (use core services)
- **Core** depends on **Interfaces** (program to interfaces)
- **Interfaces** have no dependencies

### 8.2. Circular Dependencies

**Rule:** No circular dependencies between modules

**How to Break Cycles:**
- Use interfaces/abstractions
- Use dependency injection
- Use signals/slots for loose coupling
- Extract shared code to common module

### 8.3. Forward Declarations

Use forward declarations in headers when possible:

```cpp
// NetworkManager.h
#pragma once

#include <QObject>

// Forward declarations
class ITelemetryServer;
class DatabaseManager;

class NetworkManager : public QObject {
    // Use pointers/references only
    ITelemetryServer* m_server;
    DatabaseManager* m_database;
};
```

Include full headers only in implementation files:

```cpp
// NetworkManager.cpp
#include "NetworkManager.h"
#include "interfaces/ITelemetryServer.h"  // Full include
#include "core/DatabaseManager.h"          // Full include
```

## 9. Testing Organization

### 9.1. Test Structure

```
tests/
├── unit/                          # Unit tests
│   ├── core/                      # Tests for core services
│   │   ├── TestNetworkManager.cpp
│   │   ├── TestAlarmManager.cpp
│   │   └── ...
│   ├── controllers/               # Tests for controllers
│   └── utils/                     # Tests for utilities
├── integration/                   # Integration tests
│   ├── TestNetworkIntegration.cpp
│   ├── TestDatabaseIntegration.cpp
│   └── ...
└── e2e/                           # End-to-end tests
    ├── TestPatientAdmission.cpp
    ├── TestAlarmWorkflow.cpp
    └── ...
```

### 9.2. Test File Organization

- Mirror source directory structure
- One test file per source file (for unit tests)
- Group related tests in same file

## 10. Build System Organization

### 10.1. CMake Structure

```
CMakeLists.txt                    # Root CMake file
├── src/CMakeLists.txt           # Source code build
├── qml/CMakeLists.txt           # QML resources
├── tests/CMakeLists.txt          # Test build
└── cmake/                        # CMake modules
    ├── FindQt6.cmake
    └── ...
```

### 10.2. Library Organization

- **Core Library:** `zmonitor_core` (static or shared)
- **Controller Library:** `zmonitor_controllers` (static or shared)
- **Main Executable:** `z-monitor`

## 11. Documentation Organization

### 11.1. Code Documentation

- **Header Files:** Document public API (classes, methods, parameters)
- **Implementation Files:** Document complex algorithms and non-obvious logic
- **Use Doxygen-style comments** for API documentation

```cpp
/**
 * @brief Manages secure network connectivity to the central server.
 * 
 * This class handles mTLS connections, certificate validation, and
 * telemetry data transmission. It integrates with ITelemetryServer
 * for server communication.
 * 
 * @note All network operations are asynchronous and non-blocking.
 */
class NetworkManager : public QObject {
    /**
     * @brief Connects to the central server using mTLS.
     * 
     * @param url Server URL (defaults to configured server URL)
     * @return true if connection initiated successfully, false otherwise
     * 
     * @note Connection is asynchronous. Monitor connectionStatusChanged()
     *       signal for connection state updates.
     */
    bool connectToServer(const QString& url = QString());
};
```

### 11.2. Module Documentation

Each module should have a README documenting:
- Purpose and responsibilities
- Key classes and interfaces
- Dependencies
- Usage examples

## 12. Code Review Guidelines

### 12.1. Organization Checks

- ✅ Files are in correct directories
- ✅ Namespaces are used correctly
- ✅ Includes are organized properly
- ✅ No circular dependencies
- ✅ Dependencies follow module boundaries
- ✅ Forward declarations used where appropriate

### 12.2. Common Issues

- ❌ Files in wrong directory
- ❌ Missing or incorrect namespaces
- ❌ Circular dependencies
- ❌ Unnecessary includes in headers
- ❌ Missing forward declarations

## 13. Migration and Refactoring

### 13.1. When to Refactor

- Code organization violates module boundaries
- Circular dependencies detected
- Files are in wrong locations
- Namespace usage is inconsistent

### 13.2. Refactoring Strategy

1. Identify violations
2. Plan refactoring (minimize breaking changes)
3. Create interfaces to break dependencies
4. Move files incrementally
5. Update includes and namespaces
6. Update tests
7. Verify build and tests pass

## 14. Related Documents

- [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md) - Dependency management
- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Thread organization
- [18_TESTING_WORKFLOW.md](./18_TESTING_WORKFLOW.md) - Test organization
- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - Class structure
- `26_API_DOCUMENTATION.md` - API documentation generation

