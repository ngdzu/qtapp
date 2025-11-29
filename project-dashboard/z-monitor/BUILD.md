# Z Monitor Build Guide

This document provides detailed instructions for building z-monitor locally on macOS, including environment setup, incremental build strategy, and troubleshooting.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start](#quick-start)
3. [Environment Setup](#environment-setup)
4. [Incremental Build Strategy](#incremental-build-strategy)
5. [Build Options](#build-options)
6. [Troubleshooting](#troubleshooting)
7. [Common Errors and Solutions](#common-errors-and-solutions)

## Prerequisites

### Required

- **CMake 3.16 or higher**
  - Check version: `cmake --version`
  - Install: `brew install cmake` (macOS) or download from [cmake.org](https://cmake.org)

- **Qt 6.x** (Core, Gui, Qml, Quick, QuickControls2, Sql, Network)
  - Default location: `/Users/dustinwind/Qt/6.9.2/macos`
  - Verify installation: `ls /Users/dustinwind/Qt/6.9.2/macos/lib/cmake/Qt6/Qt6Config.cmake`

- **C++17 compatible compiler**
  - macOS: Xcode Command Line Tools (`xcode-select --install`)
  - Verify: `clang++ --version` or `g++ --version`

- **Python 3** (for schema generation)
  - Verify: `python3 --version`
  - Install: `brew install python3` (macOS)

### Optional

- **vcpkg** (if using vcpkg-managed dependencies)
  - Install: `git clone https://github.com/Microsoft/vcpkg.git`
  - Set `VCPKG_ROOT` environment variable

- **protobuf** (if not using FetchContent)
  - Install: `brew install protobuf` (macOS)

## Quick Start

### 1. Setup Environment

```bash
cd project-dashboard/z-monitor
source scripts/setup_build_env.sh
```

This script configures:
- Qt path (`CMAKE_PREFIX_PATH`)
- vcpkg toolchain (if available)
- Verifies all prerequisites

### 2. Configure CMake

```bash
cmake -S . -B build
```

### 3. Build

**Incremental (recommended):**
```bash
# Phase 1: Domain common (header-only, no dependencies)
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

**Build everything:**
```bash
cmake --build build
```

### 4. Run

```bash
./build/z-monitor
```

## Environment Setup

### Qt Path Configuration

Qt path can be configured via environment variable. CMake will automatically detect Qt if `CMAKE_PREFIX_PATH` or `QT6_DIR` is set.

#### Option 1: Use Setup Script (Recommended)

```bash
source scripts/setup_build_env.sh
```

#### Option 2: Manual Configuration

```bash
# Set Qt path
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"

# Or use QT6_DIR
export QT6_DIR="/Users/dustinwind/Qt/6.9.2/macos"
```

#### Option 3: Make Permanent

Add to `~/.zshrc` or `~/.bashrc`:

```bash
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"
```

Then reload shell:
```bash
source ~/.zshrc  # or source ~/.bashrc
```

#### Option 4: CMake Command Line

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos"
```

### vcpkg Configuration (if required)

If any dependencies require vcpkg:

```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg

# Set VCPKG_ROOT
export VCPKG_ROOT="$HOME/vcpkg"

# Bootstrap vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh  # macOS/Linux
# or
.\bootstrap-vcpkg.bat  # Windows

# Install required packages (if any)
./vcpkg install <package-name>
```

Then configure CMake with vcpkg toolchain:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

**Note:** Most dependencies use FetchContent (spdlog, QxOrm, protobuf), so vcpkg is typically not required.

## Incremental Build Strategy

Building incrementally helps identify and fix dependency issues systematically. Build in this order:

### Phase 1: Domain Common Library

**Target:** `zmon_domain_common`  
**Dependencies:** None (header-only)  
**Purpose:** Verify CMake configuration and compiler setup

```bash
cmake --build build --target zmon_domain_common
```

**Expected:** Should build without errors (header-only library)

### Phase 2: Domain Layer

**Target:** `z_monitor_domain`  
**Dependencies:** `zmon_domain_common`, Qt6::Core (minimal)  
**Purpose:** Verify domain layer compiles with minimal Qt dependency

```bash
cmake --build build --target z_monitor_domain
```

**Expected:** Should build successfully. May have warnings about unused code.

### Phase 3: Application Layer

**Target:** `z_monitor_application`  
**Dependencies:** `z_monitor_domain`, Qt6::Core  
**Purpose:** Verify application services compile

```bash
cmake --build build --target z_monitor_application
```

**Expected:** Should build successfully.

### Phase 4: Infrastructure Layer

**Target:** `z_monitor_infrastructure`  
**Dependencies:** `z_monitor_domain`, `z_monitor_application`, Qt6::Core, Qt6::Gui, Qt6::Sql, Qt6::Network  
**Purpose:** Verify infrastructure adapters compile with full Qt dependencies

```bash
cmake --build build --target z_monitor_infrastructure
```

**Expected:** Should build successfully. May take longer due to more dependencies.

### Phase 5: Interface Layer & Executable

**Target:** `z-monitor`  
**Dependencies:** All layers, Qt6::Qml, Qt6::Quick, Qt6::QuickControls2  
**Purpose:** Build main executable with QML resources

```bash
cmake --build build --target z-monitor
```

**Expected:** Should build successfully and produce executable.

### Phase 6: Tests (Optional)

**Targets:** Test executables in `tests/`  
**Dependencies:** All layers, GoogleTest  
**Purpose:** Build and run tests

```bash
# Build all tests
cmake --build build --target all

# Run tests
cd build
ctest
```

## Build Options

### CMake Options

- `BUILD_TESTING`: Enable/disable test building (default: ON)
  ```bash
  cmake -S . -B build -DBUILD_TESTING=OFF
  ```

- `Z_MONITOR_USE_SPDLOG`: Enable spdlog backend for logging (default: OFF)
  ```bash
  cmake -S . -B build -DZ_MONITOR_USE_SPDLOG=ON
  ```

- `USE_QXORM`: Enable QxOrm for ORM support (default: OFF)
  ```bash
  cmake -S . -B build -DUSE_QXORM=ON
  ```

- `ENABLE_SQLCIPHER`: Enable SQLCipher for database encryption (default: OFF)
  ```bash
  cmake -S . -B build -DENABLE_SQLCIPHER=ON
  ```

### Build Types

- **Debug** (default): Includes debug symbols, no optimizations
  ```bash
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
  ```

- **Release**: Optimized build, no debug symbols
  ```bash
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  ```

- **RelWithDebInfo**: Optimized build with debug symbols
  ```bash
  cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
  ```

## Troubleshooting

### CMake Cannot Find Qt6

**Error:**
```
CMake Error at CMakeLists.txt:15 (find_package):
  Could not find a package configuration file provided by "Qt6"
```

**Solution:**
1. Verify Qt installation exists:
   ```bash
   ls /Users/dustinwind/Qt/6.9.2/macos/lib/cmake/Qt6/Qt6Config.cmake
   ```

2. Set CMAKE_PREFIX_PATH:
   ```bash
   export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"
   ```

3. Reconfigure CMake:
   ```bash
   cmake -S . -B build
   ```

### Missing Qt6 Components

**Error:**
```
Could not find a configuration file for package "Qt6" that is compatible
with requested version "6.9.2".
```

**Solution:**
1. Verify Qt version matches:
   ```bash
   cat /Users/dustinwind/Qt/6.9.2/macos/lib/cmake/Qt6/Qt6ConfigVersion.cmake
   ```

2. Check installed components:
   ```bash
   ls /Users/dustinwind/Qt/6.9.2/macos/lib/cmake/Qt6/
   ```

3. Install missing components via Qt Maintenance Tool

### Include Path Errors

**Error:**
```
fatal error: 'domain/common/Result.h' file not found
```

**Solution:**
1. Verify CMake configuration:
   ```bash
   cmake -S . -B build
   ```

2. Check include directories in CMakeLists.txt

3. Clean and rebuild:
   ```bash
   rm -rf build
   cmake -S . -B build
   cmake --build build
   ```

### Linker Errors

**Error:**
```
Undefined symbols for architecture x86_64:
  "zmon::Result<void>::ok()"
```

**Solution:**
1. Verify all required libraries are linked in CMakeLists.txt

2. Check library dependencies:
   ```bash
   cmake --build build --verbose
   ```

3. Ensure libraries are built in correct order (use incremental build strategy)

### Schema Generation Errors

**Error:**
```
Error: Python script generate_schema.py failed
```

**Solution:**
1. Verify Python3 is installed:
   ```bash
   python3 --version
   ```

2. Install required Python packages:
   ```bash
   pip3 install -r scripts/requirements.txt
   ```

3. Run schema generation manually:
   ```bash
   python3 scripts/generate_schema.py
   ```

### Protobuf Generation Errors

**Error:**
```
protoc: command not found
```

**Solution:**
1. Protobuf is downloaded via FetchContent automatically

2. If using system protobuf, verify installation:
   ```bash
   protoc --version
   ```

3. Install if needed:
   ```bash
   brew install protobuf  # macOS
   ```

## Common Errors and Solutions

### Error: "zmon_domain_common" target not found

**Cause:** `zmon_domain_common` is an INTERFACE library (header-only), so it cannot be built directly.

**Solution:** Build the domain layer instead, which depends on it:
```bash
cmake --build build --target z_monitor_domain
```

### Error: "No rule to make target 'zmon_domain_common'"

**Cause:** CMake configuration not run or target name incorrect

**Solution:**
```bash
# Reconfigure CMake
cmake -S . -B build

# List available targets
cmake --build build --target help
```

### Error: "Qt6::Core not found"

**Cause:** Qt path not configured correctly

**Solution:**
```bash
# Use setup script
source scripts/setup_build_env.sh

# Or set manually
export CMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos:$CMAKE_PREFIX_PATH"

# Reconfigure
cmake -S . -B build
```

### Error: "Multiple definitions of symbol"

**Cause:** Source file included in multiple targets

**Solution:**
1. Check CMakeLists.txt - ensure source files are only listed once per target
2. Verify no duplicate source files across libraries
3. Clean and rebuild:
   ```bash
   rm -rf build
   cmake -S . -B build
   ```

### Error: "Cannot open include file: 'QObject'"

**Cause:** Qt headers not found or Qt not linked

**Solution:**
1. Verify Qt is found:
   ```bash
   cmake -S . -B build -DCMAKE_PREFIX_PATH="/Users/dustinwind/Qt/6.9.2/macos"
   ```

2. Check target_link_libraries includes Qt6::Core

3. Verify AUTOMOC is enabled in CMakeLists.txt

### Error: Build succeeds but executable not found

**Cause:** Executable built in different location

**Solution:**
```bash
# Find executable
find build -name "z-monitor" -type f

# Or check install location
cmake --install build --prefix install
ls install/opt/z-monitor/
```

## Build Verification

After building, verify each phase:

```bash
# Phase 1: Domain common
cmake --build build --target zmon_domain_common
echo "✓ Phase 1 complete"

# Phase 2: Domain layer
cmake --build build --target z_monitor_domain
echo "✓ Phase 2 complete"

# Phase 3: Application layer
cmake --build build --target z_monitor_application
echo "✓ Phase 3 complete"

# Phase 4: Infrastructure layer
cmake --build build --target z_monitor_infrastructure
echo "✓ Phase 4 complete"

# Phase 5: Main executable
cmake --build build --target z-monitor
echo "✓ Phase 5 complete"

# Verify executable exists
test -f build/z-monitor && echo "✓ Executable built successfully" || echo "✗ Executable not found"
```

## Next Steps

After successful build:

1. **Run the application:**
   ```bash
   ./build/z-monitor
   ```

2. **Run tests:**
   ```bash
   cd build
   ctest
   ```

3. **Generate documentation:**
   ```bash
   cmake --build build --target docs
   ```

4. **Install:**
   ```bash
   cmake --install build --prefix install
   ```

## Additional Resources

- **CMake Documentation:** https://cmake.org/documentation/
- **Qt6 Documentation:** https://doc.qt.io/qt-6/
- **Project Structure:** See `doc/27_PROJECT_STRUCTURE.md`
- **Code Organization:** See `doc/22_CODE_ORGANIZATION.md`

## Getting Help

If you encounter issues not covered in this guide:

1. Check the troubleshooting section above
2. Review CMake output for specific error messages
3. Verify all prerequisites are installed correctly
4. Try clean rebuild: `rm -rf build && cmake -S . -B build && cmake --build build`
5. Check project documentation in `doc/` directory

