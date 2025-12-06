# Z Monitor Application

**Version:** 0.1.0  
**Status:** Development

This directory contains the Z Monitor executable, organized according to Domain-Driven Design (DDD) principles with a layered architecture.

## Overview

The Z Monitor is a patient monitoring device application built with Qt/C++ and QML. It follows DDD principles with clear separation between domain logic, application services, infrastructure adapters, and UI components.

## Directory Structure

The project follows a DDD-aligned structure:

```
z-monitor/
├── CMakeLists.txt              # Root build file
├── README.md                   # This file
├── src/                        # C++ source code (DDD layers)
│   ├── domain/                 # Domain layer (pure business logic)
│   │   ├── admission/          # Admission/ADT bounded context
│   │   ├── monitoring/         # Monitoring bounded context
│   │   ├── provisioning/       # Provisioning bounded context
│   │   └── repositories/       # Repository interfaces
│   ├── application/            # Application layer (use cases)
│   │   └── services/           # Application services
│   ├── infrastructure/         # Infrastructure layer (adapters)
│   │   ├── adapters/           # Platform adapters (Qt-based)
│   │   ├── logging/            # Logging infrastructure
│   │   └── ...                 # Other infrastructure components
│   ├── interface/              # Interface layer (UI integration)
│   │   └── controllers/        # QML controllers
│   └── main.cpp                # Application entry point
├── resources/                  # Resources
│   ├── qml/                    # QML UI files
│   ├── assets/                 # Images, icons
│   ├── i18n/                   # Translation files
│   └── certs/                  # Certificates (mTLS)
├── tests/                      # Test code
│   ├── unit/                   # Unit tests
│   ├── integration/            # Integration tests
│   ├── e2e/                    # End-to-end tests
│   └── benchmarks/             # Performance benchmarks
├── schema/                     # Database schema
│   └── migrations/             # SQL migration files
├── proto/                      # Protobuf schema definitions
├── openapi/                    # OpenAPI specifications
├── doc/                        # Application-specific documentation
│   └── migrations/             # Migration documentation
└── central-server-simulator/   # Central server simulator (for testing)
```

## Layer Overview

- **Domain Layer** (`src/domain/`) – Pure business logic, aggregates, value objects, domain events, repository interfaces. No Qt or infrastructure dependencies.
- **Application Layer** (`src/application/`) – Use-case orchestration, application services, DTOs. Uses Qt Core for signals/slots.
- **Infrastructure Layer** (`src/infrastructure/`) – Technical implementations (persistence, network, sensors, caching, security, platform adapters, system services, utilitiesx). Full Qt dependencies.
- **Interface Layer** (`src/interface/`) – UI integration (QML controllers and QML UI). Qt Quick dependencies.

## Build and Run

### Prerequisites

- CMake 3.16 or higher
- Qt 6 (Core, Gui, Qml, Quick, QuickControls2, Sql, Network)
- C++17 compatible compiler
- Python 3 (for schema generation)
- SQLite (for database)

### Local Build Setup

#### Quick Start

1. **Configure Qt installation path:**
   
   Set the Qt installation path as an environment variable. Replace with your Qt6 path:
   ```bash
   export DEFAULT_QT_PATH="/Users/<username>/Qt/6.9.2/macos"
   ```
   
   Replace `username` with your actual username and adjust the Qt version if needed.

2. **Setup build environment:**
   ```bash
   cd project-dashboard/z-monitor
   source scripts/setup_build_env.sh
   ```

3. **Configure CMake:**
   ```bash
   cmake -S . -B build
   ```

4. **Build the project:**
   ```bash
   cmake --build build --target z-monitor -j8
   ```

   Or build everything:
   ```bash
   cmake --build build -j8
   ```

For detailed build instructions, troubleshooting, and incremental build strategy, see **[BUILD.md](BUILD.md)**.

### Run

```bash
./build/src/z-monitor
```

### Build Options

- `BUILD_TESTING`: Enable/disable test building (default: ON)
- `Z_MONITOR_USE_SPDLOG`: Enable spdlog backend for logging (default: OFF)
- `USE_QXORM`: Enable QxOrm for ORM support (default: OFF)
- `ENABLE_SQLCIPHER`: Enable SQLCipher for database encryption (default: OFF)

Example:

```bash
cmake -S . -B build -DZ_MONITOR_USE_SPDLOG=ON -DUSE_QXORM=ON
cmake --build build
```

## Testing

Build and run all tests:

```bash
cmake --build build --target all -j8
cd build
ctest --output-on-failure
```

Run specific test:

```bash
ctest -R test_name --output-on-failure
```

## Development Status

### Implemented

- ✅ DDD-aligned source structure
- ✅ CMake build system with subdirectory organization
- ✅ Logging infrastructure (LogService, CustomBackend, SpdlogBackend)
- ✅ Settings management (SettingsManager, SettingsController)
- ✅ ADT workflow (AdmissionService)
- ✅ Database migration system
- ✅ Test infrastructure (GoogleTest)
- ✅ Domain aggregates and value objects

### In Progress

- Database persistence layer
- Network adapters
- Sensor data sources
- UI controllers and QML views

## Documentation

For detailed documentation, see:

- **Architecture:** `project-dashboard/doc/z-monitor/architecture_and_design/02_ARCHITECTURE.md`
- **Code Organization:** `project-dashboard/doc/z-monitor/architecture_and_design/22_CODE_ORGANIZATION.md`
- **Project Structure:** `project-dashboard/doc/z-monitor/architecture_and_design/27_PROJECT_STRUCTURE.md`
- **System Components:** `project-dashboard/doc/z-monitor/architecture_and_design/29_SYSTEM_COMPONENTS.md`
- **Database Design:** `project-dashboard/doc/z-monitor/architecture_and_design/10_DATABASE_DESIGN.md`

## Related Projects

- **Sensor Simulator:** `sensor-simulator/` – Simulator for sensor data
- **Central Server Simulator:** `central-server-simulator/` – Mock central server for testing

## License

[License information to be added]
