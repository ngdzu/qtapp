---
title: "Project Structure Architecture"
doc_id: DOC-ARCH-015
version: 1.0
category: Architecture
phase: 6D
status: Draft
created: 2025-12-01
author: migration-bot
related:
  - DOC-ARCH-001_system_architecture.md
---

# Project Structure Architecture

This document defines the project directory structure and file organization for the Z Monitor workspace, including both the main application (`z-monitor/`) and the project dashboard (`project-dashboard/`).

**Key Principles:**
- **Monorepo:** All projects in single repository
- **DDD Layers:** Domain, Application, Infrastructure, Interface separated in filesystem
- **Module Grouping:** Classes grouped by module (thread-assigned groups)
- **Clear Separation:** Source code, tests, documentation, build artifacts in distinct directories

## 1. Workspace Layout

### 1.1 Top-Level Structure

```
qtapp/                                    # Repository root
├── z-monitor/                            # Main Z Monitor application
│   ├── src/                              # Application source code (DDD layers)
│   ├── tests/                            # Unit tests
│   ├── docs/                             # Application-specific documentation
│   ├── resources/                        # QML, images, translations
│   ├── CMakeLists.txt                    # CMake build configuration
│   └── README.md                         # Application README
│
├── project-dashboard/                    # Dashboard and documentation hub
│   ├── doc/                              # Master documentation repository
│   │   ├── architecture/                 # Architecture documentation (DOC-ARCH-*)
│   │   ├── components/                   # Component documentation (DOC-COMP-*)
│   │   └── processes/                    # Process documentation (DOC-PROC-*)
│   ├── scripts/                          # Automation scripts
│   └── README.md                         # Dashboard README
│
├── build/                                # Build artifacts (gitignored)
├── .github/                              # GitHub Actions workflows
├── .vscode/                              # VS Code configuration
├── CMakeLists.txt                        # Top-level CMake
└── README.md                             # Workspace README
```

---

## 2. Z Monitor Application Structure

### 2.1 Source Directory (`z-monitor/src/`)

Organized by **Domain-Driven Design (DDD) layers**:

```
z-monitor/src/
├── domain/                               # Domain Layer (pure business logic)
│   ├── monitoring/                       # Monitoring bounded context
│   │   ├── aggregates/                   # PatientAggregate, DeviceAggregate, etc.
│   │   ├── value_objects/                # VitalRecord, WaveformSample, etc.
│   │   ├── events/                       # PatientAdmitted, AlarmRaised, etc.
│   │   └── repositories/                 # IPatientRepository, IVitalsRepository (interfaces)
│   ├── admission/                        # Admission/ADT bounded context
│   ├── provisioning/                     # Provisioning bounded context
│   └── security/                         # Security bounded context
│
├── application/                          # Application Layer (use case orchestration)
│   ├── services/                         # MonitoringService, AdmissionService, etc.
│   ├── dtos/                             # Data transfer objects
│   └── events/                           # Application-level event handlers
│
├── infrastructure/                       # Infrastructure Layer (technical implementations)
│   ├── persistence/                      # Repository implementations
│   │   ├── sqlite/                       # SQLitePatientRepository, etc.
│   │   └── memory/                       # MemoryRepository (for tests)
│   ├── networking/                       # Network adapters
│   │   ├── grpc/                         # gRPC client implementations
│   │   └── http/                         # HTTP/REST client implementations
│   ├── sensors/                          # Sensor data source adapters
│   │   ├── shared_memory/                # SharedMemorySensorDataSource
│   │   ├── simulator/                    # SimulatorDataSource
│   │   └── mock/                         # MockSensorDataSource
│   ├── security/                         # Cryptography, certificates, signing
│   ├── platform/                         # Qt-specific adapters (DatabaseManager, SettingsManager)
│   └── utils/                            # Utility classes (object pools, lock-free queues)
│
├── interface/                            # Interface Layer (UI integration)
│   ├── controllers/                      # QObject controllers (DashboardController, etc.)
│   └── qml/                              # QML views and components (symlink to resources/qml/)
│
└── main.cpp                              # Application entry point
```

**Key Points:**
- **Domain Layer:** No Qt or SQL dependencies; pure C++ business logic
- **Application Layer:** Coordinates domain objects and repositories
- **Infrastructure Layer:** All third-party dependencies (Qt, SQLite, OpenSSL, gRPC)
- **Interface Layer:** Qt/QML integration only

> **📋 Detailed Structure:** For complete directory structure with all files and subdirectories, see **[DOC-ARCH-016: System Components](./DOC-ARCH-016_system_components.md)** Section 1.3 and **legacy z-monitor/architecture_and_design/22_CODE_ORGANIZATION.md**.

---

### 2.2 Tests Directory (`z-monitor/tests/`)

```
z-monitor/tests/
├── unit/                                 # Unit tests
│   ├── domain/                           # Domain layer tests
│   ├── application/                      # Application layer tests
│   └── infrastructure/                   # Infrastructure layer tests
│
├── integration/                          # Integration tests
│   ├── database/                         # Database integration tests
│   └── network/                          # Network integration tests
│
├── mocks/                                # Mock implementations
│   ├── MockDatabase.h
│   ├── MockSensorDataSource.h
│   └── MockTelemetryServer.h
│
└── CMakeLists.txt                        # Test build configuration
```

**Testing Strategy:**
- **Unit Tests:** Test individual classes in isolation (use mocks for dependencies)
- **Integration Tests:** Test interactions between components (use real implementations)
- **Mocks:** Implement domain interfaces for testing (e.g., `MockDatabase` implements `IDatabase`)

---

### 2.3 Resources Directory (`z-monitor/resources/`)

```
z-monitor/resources/
├── qml/                                  # QML UI files
│   ├── views/                            # Full-screen views
│   │   ├── DashboardView.qml
│   │   ├── TrendsView.qml
│   │   └── AlarmsView.qml
│   ├── components/                       # Reusable components
│   │   ├── WaveformChart.qml
│   │   ├── StatCard.qml
│   │   └── PatientBanner.qml
│   └── main.qml                          # QML entry point
│
├── images/                               # Image assets
│   ├── icons/
│   └── backgrounds/
│
├── translations/                         # Internationalization files
│   ├── zmonitor_en.ts
│   ├── zmonitor_es.ts
│   └── zmonitor_fr.ts
│
├── styles/                               # QSS stylesheets
│   └── default.qss
│
└── qml.qrc                               # Qt resource file
```

---

## 3. Project Dashboard Structure

### 3.1 Documentation Organization (`project-dashboard/doc/`)

```
project-dashboard/doc/
├── architecture/                         # Architecture documentation (DOC-ARCH-*)
│   ├── 00_INDEX.md                       # Auto-generated index
│   ├── DOC-ARCH-001_architecture_overview.md
│   ├── DOC-ARCH-012_configuration_management.md
│   ├── DOC-ARCH-013_dependency_injection.md
│   └── ...
│
├── components/                           # Component documentation (DOC-COMP-*)
│   ├── 00_INDEX.md                       # Auto-generated index
│   ├── domain/                           # Domain layer components
│   ├── application/                      # Application layer components
│   ├── infrastructure/                   # Infrastructure layer components
│   │   ├── database/
│   │   ├── networking/
│   │   ├── persistence/
│   │   └── ...
│   └── interface/                        # Interface layer components
│
├── processes/                            # Process documentation (DOC-PROC-*)
│   ├── 00_INDEX.md
│   └── ...
│
└── z-monitor/                            # Legacy documentation (being migrated)
    └── architecture_and_design/
```

**Documentation Standards:**
- **DOC-ID Schema:** `DOC-{CATEGORY}-{NUMBER}` (e.g., `DOC-ARCH-001`, `DOC-COMP-010`)
- **YAML Frontmatter:** Metadata for each document (doc_id, title, version, status, owner, reviewers, tags, related_docs)
- **Index Generation:** Auto-generated `00_INDEX.md` files via `scripts/generate_doc_index.py`
- **Version Control:** Track changes via Git; use pre-commit hooks for validation

> **📋 Documentation Governance:** See **[DOC-ARCH-000: Documentation Contribution Guide](./DOC-ARCH-000_documentation_contribution_guide.md)** for complete governance rules.

---

### 3.2 Scripts Directory (`project-dashboard/scripts/`)

```
project-dashboard/scripts/
├── generate_doc_index.py                 # Generate documentation indexes
├── create-doc.sh                         # Wizard for creating new docs (planned)
├── doc-dependency-checker.py             # Validate cross-references (planned)
└── ...
```

---

## 4. Build Artifacts

### 4.1 Build Directory (`build/`)

**Important:** `build/` is gitignored. All build artifacts go here.

```
build/
├── CMakeCache.txt
├── CMakeFiles/
├── z-monitor                             # Compiled executable
├── lib/                                  # Compiled libraries
└── ...
```

**Build Commands:**

```bash
# Configure
cmake -B build -S .

# Build
cmake --build build

# Run
./build/z-monitor
```

---

## 5. Configuration Files

### 5.1 CMake Configuration

- **`CMakeLists.txt`** (workspace root): Top-level CMake configuration
- **`z-monitor/CMakeLists.txt`**: Z Monitor application build
- **`z-monitor/tests/CMakeLists.txt`**: Test build configuration

### 5.2 Git Configuration

- **`.gitignore`**: Excludes `build/`, IDE files, generated files
- **`.gitattributes`**: Line ending normalization

### 5.3 CI/CD Configuration

- **`.github/workflows/docs.yml`**: Documentation validation workflow (planned)
- **`.github/workflows/build.yml`**: Build and test workflow (planned)

### 5.4 Editor Configuration

- **`.vscode/settings.json`**: VS Code workspace settings
- **`.vscode/tasks.json`**: Build tasks
- **`.vscode/launch.json`**: Debugger configuration

---

## 6. Naming Conventions

### 6.1 Files

- **C++ Headers:** `PascalCase.h` (e.g., `PatientAggregate.h`, `IDatabase.h`)
- **C++ Sources:** `PascalCase.cpp` (e.g., `PatientAggregate.cpp`)
- **QML Files:** `PascalCase.qml` (e.g., `DashboardView.qml`, `StatCard.qml`)
- **Documentation:** `DOC-{CATEGORY}-{NUMBER}_{snake_case}.md` (e.g., `DOC-ARCH-001_architecture_overview.md`)
- **Scripts:** `snake_case.py` or `kebab-case.sh` (e.g., `generate_doc_index.py`, `create-doc.sh`)

### 6.2 Directories

- **Source Directories:** `snake_case` (e.g., `value_objects/`, `aggregates/`)
- **Documentation Directories:** `snake_case` (e.g., `architecture/`, `components/`)

---

## 7. Repository Metadata

| Property             | Value                                           |
| -------------------- | ----------------------------------------------- |
| **Repository Name**  | `qtapp`                                         |
| **Primary Language** | C++ (Qt 6)                                      |
| **Build System**     | CMake                                           |
| **Main Projects**    | `z-monitor/` (app), `project-dashboard/` (docs) |
| **License**          | Proprietary                                     |
| **Version Control**  | Git (GitHub)                                    |

---

## 8. Related Documents

- **[DOC-ARCH-001: Architecture Overview](./DOC-ARCH-001_architecture_overview.md)** - High-level system architecture
- **[DOC-ARCH-016: System Components](./DOC-ARCH-016_system_components.md)** - Complete component inventory and DDD strategy
- **[DOC-ARCH-013: Dependency Injection](./DOC-ARCH-013_dependency_injection.md)** - DI patterns and AppContainer bootstrap
- **Legacy:** `z-monitor/architecture_and_design/22_CODE_ORGANIZATION.md` - Detailed code organization (being migrated)
- **Legacy:** `z-monitor/architecture_and_design/29_SYSTEM_COMPONENTS.md` - System components reference (being migrated)

---
**Status:** ✅ Migrated from legacy 27_PROJECT_STRUCTURE.md
