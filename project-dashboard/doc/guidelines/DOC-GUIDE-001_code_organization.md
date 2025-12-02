---
doc_id: DOC-GUIDE-001
title: Code Organization and Module Structure
version: 1.0
status: Approved
created: 2025-12-01
updated: 2025-12-01
category: Guideline
tags: [ddd, architecture, code-organization, layers, namespaces]
related:
  - DOC-ARCH-002
  - DOC-COMP-010
  - DOC-COMP-011
  - DOC-COMP-012
source: 22_CODE_ORGANIZATION.md
---

# Code Organization and Module Structure

## Overview

This guideline defines the code organization, module structure, namespace conventions, and file organization patterns for the Z Monitor application. The structure follows Domain-Driven Design (DDD) principles with clear separation between domain, application, infrastructure, and interface layers.

## Guiding Principles

- **Separation of Concerns:** Clear boundaries between UI, business logic, and data access
- **Modularity:** Components should be loosely coupled and highly cohesive
- **Testability:** Code organization should facilitate unit testing and mocking
- **Maintainability:** Clear structure makes code easy to understand and modify
- **Scalability:** Structure should support growth without refactoring

## Directory Structure (DDD-Based)

### Top-Level Organization

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

### Domain Layer (`src/domain/`)

**Purpose:** Pure business logic, aggregates, value objects, domain events, repository interfaces

```
src/domain/
├── monitoring/                   # Monitoring bounded context
│   ├── PatientAggregate.h/cpp
│   ├── DeviceAggregate.h/cpp
│   ├── TelemetryBatch.h/cpp
│   ├── AlarmAggregate.h/cpp
│   ├── VitalRecord.h             # Value object
│   ├── WaveformSample.h          # Value object
│   └── events/                   # Domain events
│       ├── PatientAdmitted.h
│       ├── TelemetryQueued.h
│       └── AlarmRaised.h
├── admission/                    # Admission/ADT bounded context
│   ├── AdmissionAggregate.h/cpp
│   ├── PatientIdentity.h         # Value object
│   └── events/
├── provisioning/                 # Provisioning bounded context
│   ├── ProvisioningSession.h/cpp
│   ├── CredentialBundle.h        # Value object
│   └── events/
└── repositories/                 # Repository interfaces (domain)
    ├── IPatientRepository.h
    ├── ITelemetryRepository.h
    └── IAlarmRepository.h
```

**Dependencies Allowed:**
- Standard C++ library only
- **NO** Qt dependencies
- **NO** infrastructure dependencies

### Application Layer (`src/application/`)

**Purpose:** Use-case orchestration, coordinates domain objects and repositories

```
src/application/
├── services/                     # Application services
│   ├── MonitoringService.h/cpp
│   ├── AdmissionService.h/cpp
│   ├── ProvisioningService.h/cpp
│   ├── SecurityService.h/cpp
│   └── DataArchiveService.h/cpp
└── dto/                          # Data Transfer Objects
    ├── AdmitPatientCommand.h
    ├── DischargePatientCommand.h
    └── TelemetrySubmission.h
```

**Dependencies Allowed:**
- Domain layer (aggregates, repositories, events)
- Standard C++ library
- Qt Core (QObject, signals/slots for events)

**Dependencies NOT Allowed:**
- Infrastructure implementations (use interfaces only)
- Qt Widgets, Qt Quick, QML types

### Infrastructure Layer (`src/infrastructure/`)

**Purpose:** Technical implementations (persistence, networking, platform adapters, caching)

```
src/infrastructure/
├── persistence/                  # Repository implementations
│   ├── SQLitePatientRepository.h/cpp
│   ├── SQLiteTelemetryRepository.h/cpp
│   └── DatabaseManager.h/cpp
├── network/                      # Network adapters
│   ├── NetworkTelemetryServer.h/cpp
│   └── MockTelemetryServer.h/cpp
├── sensors/                      # Sensor data source adapters
│   ├── SharedMemorySensorDataSource.h/cpp
│   └── MockSensorDataSource.h/cpp
├── caching/                      # Data caching components
│   ├── VitalsCache.h/cpp
│   └── WaveformCache.h/cpp
├── security/                     # Security adapters
│   ├── CertificateManager.h/cpp
│   └── EncryptionService.h/cpp
├── logging/                      # Logging infrastructure
│   ├── LogService.h/cpp
│   └── ILogBackend.h
└── utils/                        # Shared utility classes
    ├── ObjectPool.h/cpp
    └── LockFreeQueue.h/cpp
```

**Dependencies Allowed:**
- Domain layer (implements repository interfaces)
- Application layer (provides implementations)
- Qt modules (Core, Network, SQL, etc.)
- Third-party libraries (OpenSSL, SQLCipher, etc.)

**Dependencies NOT Allowed:**
- Interface layer (controllers, QML)

### Interface Layer (`src/interface/`)

**Purpose:** UI integration (QML controllers and QML UI)

```
src/interface/
├── controllers/                  # QML controllers (QObject bridges)
│   ├── DashboardController.h/cpp
│   ├── AlarmController.h/cpp
│   ├── PatientController.h/cpp
│   └── SettingsController.h/cpp
└── qml/                          # QML UI files
    ├── views/                    # Full-screen views
    ├── components/               # Reusable QML components
    └── Main.qml                  # Root QML file
```

**Dependencies Allowed:**
- Application layer (uses application services)
- Domain layer (reads domain events, value objects)
- Qt Quick (Q_PROPERTY, signals/slots)
- Qt Core

**Dependencies NOT Allowed:**
- Direct infrastructure access (must go through application services)

## Namespace Conventions

### Namespace Hierarchy

Use a flat `zmon::` namespace for all code:

```cpp
namespace zmon {
    // Domain Layer
    class PatientAggregate;
    class TelemetryBatch;
    class VitalRecord;  // Value object
    
    // Application Layer
    class MonitoringService;
    class AdmissionService;
    
    // Infrastructure Layer
    class SQLitePatientRepository;
    class DatabaseManager;
    
    // Interface Layer
    class DashboardController;
}
```

**Rationale:** Single business domain, no name collisions expected.

### Using Declarations

Prefer explicit namespace usage in headers, allow `using` in implementation files:

```cpp
// Header file
namespace zmon {
    class NetworkManager : public QObject {
        // ...
    };
}

// Implementation file
#include "NetworkManager.h"
using namespace zmon;

NetworkManager::NetworkManager() {
    // Can use NetworkManager without namespace prefix
}
```

## File Naming Conventions

### C++ Files

- **Headers:** `ClassName.h` (PascalCase)
- **Implementation:** `ClassName.cpp` (PascalCase)
- **One class per file:** Each class gets its own `.h` and `.cpp` file

### QML Files

- **Views:** `ViewNameView.qml` (PascalCase, "View" suffix)
- **Components:** `ComponentName.qml` (PascalCase)
- **Dialogs:** `DialogNameDialog.qml` (PascalCase, "Dialog" suffix)

### Test Files

- **Unit Tests:** `TestClassName.cpp` (prefix "Test")
- **Integration Tests:** `IntegrationTestFeature.cpp` (prefix "IntegrationTest")

## Layer Dependency Rules

### Domain Layer Rules

✅ **Allowed:**
- Standard C++ library
- No external dependencies

❌ **Prohibited:**
- Qt (any module)
- SQL/database libraries
- Network libraries
- Infrastructure code

**Example:**
```cpp
// ✅ GOOD: Domain layer with no external dependencies
namespace zmon {
class PatientAggregate {
private:
    std::string m_mrn;
    std::string m_name;
    bool m_isAdmitted;
    
public:
    Result<void, Error> admit(const PatientIdentity& identity);
    Result<void, Error> discharge();
};
}
```

### Application Layer Rules

✅ **Allowed:**
- Domain layer (aggregates, repositories, events)
- Standard C++ library
- Qt Core (QObject, signals/slots)

❌ **Prohibited:**
- Infrastructure implementations (use interfaces only)
- Qt Widgets, Qt Quick, QML types

**Example:**
```cpp
// ✅ GOOD: Application service using domain interfaces
namespace zmon {
class MonitoringService : public QObject {
private:
    IPatientRepository* m_patientRepo;  // ✅ Interface, not implementation
    ITelemetryRepository* m_telemetryRepo;
    
public:
    Result<void, Error> startMonitoring(const QString& patientMrn);
};
}
```

### Infrastructure Layer Rules

✅ **Allowed:**
- Domain layer (implements repository interfaces)
- Application layer
- Qt modules (Core, Network, SQL, etc.)
- Third-party libraries

❌ **Prohibited:**
- Interface layer (controllers, QML)

**Example:**
```cpp
// ✅ GOOD: Infrastructure implementing domain interface
namespace zmon {
class SQLitePatientRepository : public IPatientRepository {
private:
    DatabaseManager* m_dbManager;  // ✅ Infrastructure dependency
    
public:
    Result<PatientAggregate, Error> findByMrn(const QString& mrn) override;
};
}
```

### Interface Layer Rules

✅ **Allowed:**
- Application layer (uses application services)
- Domain layer (reads domain events)
- Qt Quick (Q_PROPERTY, signals/slots)

❌ **Prohibited:**
- Direct infrastructure access

**Example:**
```cpp
// ✅ GOOD: Controller using application service
namespace zmon {
class PatientController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString patientName READ patientName NOTIFY patientChanged)
    
private:
    AdmissionService* m_admissionService;  // ✅ Application service
    
public slots:
    void admitPatient(const QString& mrn, const QString& name);
};
}
```

## Best Practices

### 1. Keep Domain Pure

The domain layer should contain only pure business logic with no external dependencies:

```cpp
// ✅ GOOD: Pure domain logic
class PatientAggregate {
public:
    Result<void, Error> admit(const PatientIdentity& identity) {
        if (m_isAdmitted) {
            return Error("Patient already admitted");  // ✅ Pure business rule
        }
        m_identity = identity;
        m_isAdmitted = true;
        return Success();
    }
};
```

### 2. Use Dependency Injection

Inject dependencies through constructors:

```cpp
// ✅ GOOD: Constructor injection
class MonitoringService : public QObject {
public:
    MonitoringService(
        IPatientRepository* patientRepo,
        ITelemetryRepository* telemetryRepo,
        QObject* parent = nullptr
    ) : QObject(parent),
        m_patientRepo(patientRepo),
        m_telemetryRepo(telemetryRepo) {}
};
```

### 3. One Responsibility Per Class

Each class should have a single, well-defined responsibility:

```cpp
// ✅ GOOD: Single responsibility (patient admission)
class AdmissionService {
public:
    Result<void, Error> admitPatient(const AdmitPatientCommand& cmd);
    Result<void, Error> dischargePatient(const QString& mrn);
    Result<void, Error> transferPatient(const QString& mrn, const QString& newBed);
};

// ❌ BAD: Multiple responsibilities
class PatientService {
    Result<void, Error> admitPatient(...);
    Result<void, Error> recordVitals(...);  // Should be in MonitoringService
    Result<void, Error> sendTelemetry(...); // Should be in TelemetryService
};
```

### 4. Prefer Interfaces for Cross-Layer Communication

Use interfaces for dependencies that cross layer boundaries:

```cpp
// ✅ GOOD: Interface for repository
class IPatientRepository {
public:
    virtual Result<PatientAggregate, Error> findByMrn(const QString& mrn) = 0;
    virtual Result<void, Error> save(const PatientAggregate& patient) = 0;
};

// Application service depends on interface, not implementation
class AdmissionService {
private:
    IPatientRepository* m_patientRepo;  // ✅ Interface
};
```

## Related Documents

- [DOC-ARCH-002: System Overview](../architecture/DOC-ARCH-001_system_overview.md) - High-level architecture and DDD layer structure
- [DOC-COMP-010: MonitoringService](../components/DOC-COMP-010_monitoringservice.md) - Example application service
- [DOC-COMP-011: AdmissionService](../components/DOC-COMP-011_admissionservice.md) - Example application service
- [DOC-COMP-012: ProvisioningService](../components/DOC-COMP-012_provisioningservice.md) - Example application service
- [DOC-GUIDE-011: Error Handling Strategy](./DOC-GUIDE-011_error_handling.md) - Error handling patterns
- [DOC-GUIDE-012: Logging Strategy](./DOC-GUIDE-012_logging.md) - Logging guidelines

## Enforcement

Code organization is enforced through:
- **Code Review:** All PRs must follow DDD layer boundaries
- **Architecture Tests:** Automated tests verify layer dependencies
- **CMake Structure:** Build system reflects layer organization
