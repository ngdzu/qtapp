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
- **Infrastructure Layer** (`src/infrastructure/`) – Technical implementations (persistence, network, sensors, caching, security, platform adapters, system services, utilities). Full Qt dependencies.
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

1. **Setup build environment:**
   ```bash
   cd project-dashboard/z-monitor
   source scripts/setup_build_env.sh
   ```

2. **Configure CMake:**
   ```bash
   cmake -S . -B build
   ```

3. **Build incrementally (recommended):**
   ```bash
   # Phase 1: Domain common (header-only)
   cmake --build build --target zmon_domain_common
   
   # Phase 2: Domain layer
   cmake --build build --target z_monitor_domain
   
   # Phase 3: Application layer
   cmake --build build --target z_monitor_application
   
   # Phase 4: Infrastructure layer
   cmake --build build --target z_monitor_infrastructure
   
   # Phase 5: Main executable
   cmake --build build --target z-monitor
   ```

   Or build everything at once:
   ```bash
   cmake --build build
   ```

#### Qt Path Configuration

Qt path must be configured for CMake to find Qt6. Default location: `/Users/dustinwind/Qt/6.9.2/macos`

**Option 1: Use setup script (recommended)**
```bash
source scripts/setup_build_env.sh
```

**Option 2: Manual configuration**
```bash
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"
```

**Option 3: Make permanent**
Add to `~/.zshrc` or `~/.bashrc`:
```bash
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"
```

**Option 4: CMake command line**
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos"
```

For detailed build instructions, troubleshooting, and incremental build strategy, see **[BUILD.md](BUILD.md)**.

### Build

From the `z-monitor/` directory:

```bash
cmake -S . -B build
cmake --build build
```

### Run

```bash
./build/z-monitor
```

Or install and run:

```bash
cmake --install build --prefix install
./install/opt/z-monitor/z-monitor
```

### Build Options

- `BUILD_TESTING`: Enable/disable test building (default: ON)
- `Z_MONITOR_USE_SPDLOG`: Enable spdlog backend for logging (default: OFF)
- `USE_QXORM`: Enable QxOrm for ORM support (default: OFF)
- `ENABLE_SQLCIPHER`: Enable SQLCipher for database encryption (default: OFF)

Example:

```bash
cmake -S . -B build -DZ_MONITOR_USE_SPDLOG=ON
cmake --build build
```

## Testing

Run tests using CTest:

```bash
cd build
ctest
```

Or run individual test executables:

```bash
./build/tests/unit/logging/test_ilog_backend
./build/tests/unit/logging/test_custom_backend
./build/tests/unit/logging/test_log_service
./build/tests/integration/logging/test_async_logging
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
