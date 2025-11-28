# Code Organization & Module Structure

**Document ID:** DESIGN-022  
**Version:** 2.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document defines the code organization, module structure, namespace conventions, and file organization patterns for the Z Monitor application. The structure follows Domain-Driven Design (DDD) principles with clear separation between domain, application, infrastructure, and interface layers.

> **📋 Related Documents:**
> - [Architecture (02_ARCHITECTURE.md)](./02_ARCHITECTURE.md) - High-level architecture and DDD layer structure ⭐
> - [System Components & DDD Strategy (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md) - DDD strategy and component inventory ⭐
> - [Project Structure (27_PROJECT_STRUCTURE.md)](./27_PROJECT_STRUCTURE.md) - Directory layout reference

## 1. Guiding Principles

- **Separation of Concerns:** Clear boundaries between UI, business logic, and data access
- **Modularity:** Components should be loosely coupled and highly cohesive
- **Testability:** Code organization should facilitate unit testing and mocking
- **Maintainability:** Clear structure makes code easy to understand and modify
- **Scalability:** Structure should support growth without refactoring

## 2. Directory Structure (DDD-Based)

The Z Monitor application follows Domain-Driven Design (DDD) principles with a layered architecture. The directory structure reflects this organization.

### 2.1. Top-Level Organization

```
z-monitor/
├── src/                          # Source code (DDD layers)
│   ├── domain/                   # Domain layer (pure business logic)
│   ├── application/              # Application layer (use cases)
│   ├── infrastructure/           # Infrastructure layer (adapters)
│   ├── interface/                # Interface layer (UI controllers)
│   └── main.cpp                  # Application entry point
├── resources/                    # Resources (QML, images, fonts, etc.)
│   ├── qml/                      # QML UI files
│   ├── assets/                   # Images, icons
│   ├── i18n/                     # Translations
│   └── certs/                    # Certificates
├── tests/                        # Test code
│   ├── unit/                     # Unit tests
│   ├── integration/              # Integration tests
│   └── e2e/                      # End-to-end tests
├── scripts/                      # Build and utility scripts
├── docs/                         # Documentation
└── CMakeLists.txt               # Build configuration
```

### 2.2. Source Code Organization (`src/`) - DDD Layers

```
src/
├── domain/                       # Domain Layer (pure business logic)
│   ├── monitoring/               # Monitoring bounded context
│   │   ├── PatientAggregate.h/cpp
│   │   ├── DeviceAggregate.h/cpp
│   │   ├── TelemetryBatch.h/cpp
│   │   ├── AlarmAggregate.h/cpp
│   │   ├── VitalRecord.h         # Value object
│   │   ├── WaveformSample.h      # Value object
│   │   └── events/               # Domain events
│   │       ├── PatientAdmitted.h
│   │       ├── TelemetryQueued.h
│   │       └── AlarmRaised.h
│   ├── admission/                # Admission/ADT bounded context
│   │   ├── AdmissionAggregate.h/cpp
│   │   ├── PatientIdentity.h     # Value object
│   │   └── events/
│   ├── provisioning/             # Provisioning bounded context
│   │   ├── ProvisioningSession.h/cpp
│   │   ├── CredentialBundle.h     # Value object
│   │   └── events/
│   ├── security/                 # Security bounded context
│   │   ├── UserSession.h/cpp
│   │   ├── AuditTrailEntry.h/cpp
│   │   ├── PermissionRegistry.h/cpp
│   │   └── events/
│   └── repositories/             # Repository interfaces (domain)
│       ├── IPatientRepository.h
│       ├── ITelemetryRepository.h
│       ├── IVitalsRepository.h
│       ├── IAlarmRepository.h
│       └── IProvisioningRepository.h
│
├── application/                  # Application Layer (use-case orchestration)
│   ├── services/                 # Application services
│   │   ├── MonitoringService.h/cpp
│   │   ├── AdmissionService.h/cpp
│   │   ├── ProvisioningService.h/cpp
│   │   ├── SecurityService.h/cpp
│   │   ├── DataArchiveService.h/cpp
│   │   ├── FirmwareUpdateService.h/cpp
│   │   └── BackupService.h/cpp
│   └── dto/                      # Data Transfer Objects
│       ├── AdmitPatientCommand.h
│       ├── DischargePatientCommand.h
│       ├── TelemetrySubmission.h
│       └── ProvisioningPayload.h
│
├── infrastructure/               # Infrastructure Layer (adapters)
│   ├── persistence/              # Repository implementations
│   │   ├── SQLitePatientRepository.h/cpp
│   │   ├── SQLiteTelemetryRepository.h/cpp
│   │   ├── SQLiteVitalsRepository.h/cpp
│   │   ├── SQLiteAlarmRepository.h/cpp
│   │   ├── SQLiteProvisioningRepository.h/cpp
│   │   ├── SQLiteUserRepository.h/cpp
│   │   ├── SQLiteAuditRepository.h/cpp
│   │   └── DatabaseManager.h/cpp
│   ├── network/                  # Network adapters
│   │   ├── NetworkTelemetryServer.h/cpp    # ITelemetryServer impl
│   │   ├── MockTelemetryServer.h/cpp
│   │   ├── HISPatientLookupAdapter.h/cpp    # IPatientLookupService impl
│   │   ├── MockPatientLookupService.h/cpp
│   │   ├── CentralStationClient.h/cpp
│   │   └── HospitalUserManagementAdapter.h/cpp  # IUserManagementService impl
│   ├── sensors/                  # Sensor data source adapters
│   │   ├── SharedMemorySensorDataSource.h/cpp  # ISensorDataSource impl (memfd reader)
│   │   ├── SharedMemoryRingBuffer.h/cpp        # Shared buffer layout helpers
│   │   ├── SharedMemoryControlChannel.h/cpp    # Unix socket handshake utilities
│   │   ├── SimulatorDataSource.h/cpp
│   │   ├── MockSensorDataSource.h/cpp
│   │   ├── HardwareSensorAdapter.h/cpp
│   │   └── ReplayDataSource.h/cpp
│   ├── caching/                  # Data caching components
│   │   ├── VitalsCache.h/cpp
│   │   ├── WaveformCache.h/cpp
│   │   ├── PersistenceScheduler.h/cpp
│   │   └── DataCleanupService.h/cpp
│   ├── security/                 # Security adapters
│   │   ├── CertificateManager.h/cpp
│   │   ├── KeyManager.h/cpp
│   │   ├── EncryptionService.h/cpp
│   │   ├── SignatureService.h/cpp
│   │   └── SecureStorage.h/cpp
│   ├── logging/                  # Logging infrastructure
│   │   ├── LogService.h/cpp
│   │   ├── ILogBackend.h
│   │   ├── LogEntry.h
│   │   ├── backends/
│   │   │   ├── SpdlogBackend.h/cpp
│   │   │   ├── CustomBackend.h/cpp
│   │   │   └── GlogBackend.h/cpp (optional)
│   │   └── utils/
│   │       └── LogFormatter.h/cpp
│   ├── qt/                       # Qt-specific adapters
│   │   └── SettingsManager.h/cpp
│   ├── system/                   # System services
│   │   ├── HealthMonitor.h/cpp
│   │   ├── ClockSyncService.h/cpp
│   │   ├── FirmwareManager.h/cpp
│   │   └── WatchdogService.h/cpp
│   └── utils/                    # Shared utility classes
│       ├── ObjectPool.h/cpp      # Object pooling utility (see 23_MEMORY_RESOURCE_MANAGEMENT.md)
│       ├── LockFreeQueue.h/cpp   # Lock-free queue (or use external library)
│       ├── LogBuffer.h/cpp       # Pre-allocated log buffer
│       ├── MemoryPool.h/cpp       # Memory pool allocator
│       ├── CryptoUtils.h/cpp     # Cryptographic utilities
│       ├── DateTimeUtils.h/cpp   # Date/time utilities
│       ├── StringUtils.h/cpp     # String manipulation utilities
│       └── ValidationUtils.h/cpp # Input validation utilities
│
├── interface/                    # Interface Layer (UI integration)
│   ├── controllers/              # QML controllers (QObject bridges)
│   │   ├── DashboardController.h/cpp
│   │   ├── AlarmController.h/cpp
│   │   ├── TrendsController.h/cpp
│   │   ├── SystemController.h/cpp
│   │   ├── PatientController.h/cpp
│   │   ├── SettingsController.h/cpp
│   │   ├── ProvisioningController.h/cpp
│   │   ├── NotificationController.h/cpp
│   │   ├── DiagnosticsController.h/cpp
│   │   └── AuthenticationController.h/cpp
│   └── qml/                      # QML UI files (moved from resources/qml)
│       ├── views/                # Full-screen views
│       ├── components/           # Reusable QML components
│       ├── dialogs/              # Dialog components
│       └── Main.qml             # Root QML file
│
└── main.cpp
```

### 2.3. QML Organization (`src/interface/qml/`)

QML files are organized under the interface layer to reflect their role as part of the UI interface:

```
src/interface/qml/
├── views/                        # Full-screen views
│   ├── DashboardView.qml
│   ├── TrendsView.qml
│   ├── SettingsView.qml
│   ├── DiagnosticsView.qml
│   ├── LoginView.qml
│   └── PatientAdmissionModal.qml
├── components/                   # Reusable components
│   ├── StatCard.qml
│   ├── PatientBanner.qml
│   ├── AlarmIndicator.qml
│   ├── NotificationBell.qml
│   ├── Sidebar.qml
│   ├── TopBar.qml
│   ├── WaveformDisplay.qml
│   ├── TrendChart.qml
│   ├── SettingsRow.qml
│   ├── ConfirmDialog.qml
│   ├── LoadingSpinner.qml
│   └── QRCodeDisplay.qml
├── dialogs/                      # Dialog components
│   ├── ConfirmationDialog.qml
│   ├── ErrorDialog.qml
│   └── InfoDialog.qml
└── Main.qml                      # Root QML file
```

**Note:** QML files may also be placed in `resources/qml/` for resource embedding, but the logical organization follows the interface layer structure.

## 3. Namespace Conventions (DDD-Aligned)

### 3.1. Namespace Hierarchy

Namespaces align with DDD layers:

```cpp
namespace ZMonitor {
    // Domain Layer
    namespace Domain {
        namespace Monitoring {
            class PatientAggregate;
            class TelemetryBatch;
            class VitalRecord;  // Value object
        }
        namespace Admission {
            class AdmissionAggregate;
            class PatientIdentity;  // Value object
        }
        namespace Repositories {
            class IPatientRepository;
            class ITelemetryRepository;
        }
    }
    
    // Application Layer
    namespace Application {
        namespace Services {
            class MonitoringService;
            class AdmissionService;
            class SecurityService;
        }
        namespace DTO {
            struct AdmitPatientCommand;
            struct TelemetrySubmission;
        }
    }
    
    // Infrastructure Layer
    namespace Infrastructure {
        namespace Persistence {
            class SQLitePatientRepository;
            class DatabaseManager;
        }
        namespace Network {
            class NetworkTelemetryServer;
        }
        namespace Sensors {
            class SharedMemorySensorDataSource;
        }
        namespace Security {
            class CertificateManager;
            class EncryptionService;
        }
    }
    
    // Interface Layer
    namespace Interface {
        namespace Controllers {
            class DashboardController;
            class AlarmController;
        }
    }
}
```

### 3.2. Namespace Usage

- **Domain Layer:** `ZMonitor::Domain::` (with bounded context sub-namespaces)
- **Application Layer:** `ZMonitor::Application::`
- **Infrastructure Layer:** `ZMonitor::Infrastructure::` (with adapter sub-namespaces)
- **Interface Layer:** `ZMonitor::Interface::`

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

## 4. Layer Boundaries (DDD)

### 4.1. Domain Layer

**Purpose:** Pure business logic, aggregates, value objects, domain events, repository interfaces

**Dependencies:**
- Standard C++ library only
- No Qt dependencies
- No infrastructure dependencies

**Dependencies NOT Allowed:**
- Qt (any module)
- SQL/database libraries
- Network libraries
- Any infrastructure code

**Components:**
- All aggregates in `src/domain/`
- All value objects in `src/domain/`
- All domain events in `src/domain/*/events/`
- All repository interfaces in `src/domain/repositories/`
- All external service interfaces (e.g., `ISensorDataSource`, `ITelemetryServer`)

**Key Principle:** Domain layer is pure business logic with no external dependencies.

### 4.2. Application Layer

**Purpose:** Use-case orchestration, coordinates domain objects and repositories

**Dependencies:**
- Domain layer (aggregates, repositories, events)
- Standard C++ library
- Qt Core (QObject, signals/slots for events)

**Dependencies NOT Allowed:**
- Infrastructure implementations (use interfaces only)
- Qt Widgets
- Qt Quick
- QML types

**Components:**
- All application services in `src/application/services/`
- All DTOs in `src/application/dto/`

**Key Principle:** Application services orchestrate use cases but don't contain infrastructure details.

### 4.3. Infrastructure Layer

**Purpose:** Technical implementations (persistence, networking, Qt adapters, caching)

**Dependencies:**
- Domain layer (implements repository interfaces)
- Application layer (provides implementations)
- Qt modules (Core, Network, SQL, etc.)
- Third-party libraries (OpenSSL, SQLCipher, etc.)

**Dependencies NOT Allowed:**
- Interface layer (controllers, QML)

**Components:**
- Repository implementations in `src/infrastructure/persistence/`
- Network adapters in `src/infrastructure/network/`
- Sensor adapters in `src/infrastructure/sensors/`
- Caching components in `src/infrastructure/caching/`
- Security adapters in `src/infrastructure/security/`
- Qt adapters in `src/infrastructure/qt/`
- System services in `src/infrastructure/system/`
- Utility classes in `src/infrastructure/utils/` (ObjectPool, LockFreeQueue, LogBuffer, etc. - see [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md))

**Key Principle:** Infrastructure implements domain interfaces and provides technical capabilities.

### 4.4. Interface Layer

**Purpose:** UI integration (QML controllers and QML UI)

**Dependencies:**
- Application layer (uses application services)
- Domain layer (reads domain events, value objects)
- Qt Quick (Q_PROPERTY, signals/slots)
- Qt Core

**Dependencies NOT Allowed:**
- Direct infrastructure access (must go through application services)
- Qt Widgets

**Components:**
- All controllers in `src/interface/controllers/`
- All QML files in `src/interface/qml/`

**Key Principle:** Interface layer is the only layer that knows about UI. It delegates to application services.

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

#include "infrastructure/persistence/DatabaseManager.h"  // 4. Project headers (infrastructure)
#include "domain/repositories/ITelemetryRepository.h"    // Domain interfaces
#include "infrastructure/utils/CryptoUtils.h"            // Utility classes
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

## 8. Dependency Rules (DDD)

### 8.1. Dependency Direction

```
Interface → Application → Domain ← Infrastructure
```

**Layer Dependency Rules:**
- **Interface Layer** depends on **Application Layer** (controllers use application services)
- **Application Layer** depends on **Domain Layer** (services use aggregates and repository interfaces)
- **Infrastructure Layer** depends on **Domain Layer** (implements repository interfaces)
- **Domain Layer** has **no dependencies** (pure business logic)

**Key Principles:**
- Dependencies flow inward (toward domain)
- Domain layer is independent
- Infrastructure implements domain interfaces
- Application orchestrates domain objects
- Interface delegates to application services

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

- [02_ARCHITECTURE.md](./02_ARCHITECTURE.md) - High-level architecture and DDD layer structure ⭐
- [29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md) - DDD strategy and component inventory ⭐
- [27_PROJECT_STRUCTURE.md](./27_PROJECT_STRUCTURE.md) - Directory layout reference
- [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) - Memory management patterns and utility classes (ObjectPool, LockFreeQueue, LogBuffer) ⭐
- [13_DEPENDENCY_INJECTION.md](./13_DEPENDENCY_INJECTION.md) - Dependency management
- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Thread organization
- [18_TESTING_WORKFLOW.md](./18_TESTING_WORKFLOW.md) - Test organization
- [09_CLASS_DESIGNS_OVERVIEW.md](./09_CLASS_DESIGNS_OVERVIEW.md) - Module-based class architecture
- `26_API_DOCUMENTATION.md` - API documentation generation

