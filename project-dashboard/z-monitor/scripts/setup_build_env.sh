#!/bin/bash
#
# setup_build_env.sh
# Complete build environment setup script for z-monitor
#
# This script configures the build environment including:
# - Qt path configuration
# - vcpkg setup (if needed)
# - Environment variable verification
#
# Usage:
#   source scripts/setup_build_env.sh
#   or
#   . scripts/setup_build_env.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=========================================="
echo "Z Monitor Build Environment Setup"
echo "=========================================="
echo ""

# 1. Configure Qt Path
echo "Step 1: Configuring Qt path..."
DEFAULT_QT_PATH="/Users/dustinwind/Qt/6.9.2/macos"

if [ -n "$QT6_DIR" ]; then
    QT_PATH="$QT6_DIR"
    echo "  Using QT6_DIR: $QT_PATH"
elif [ -n "$CMAKE_PREFIX_PATH" ] && [[ "$CMAKE_PREFIX_PATH" == *"Qt"* ]]; then
    QT_PATH=$(echo "$CMAKE_PREFIX_PATH" | cut -d: -f1)
    echo "  Using CMAKE_PREFIX_PATH: $QT_PATH"
elif [ -d "$DEFAULT_QT_PATH" ]; then
    QT_PATH="$DEFAULT_QT_PATH"
    echo "  Using default Qt path: $QT_PATH"
else
    echo "  Error: Qt installation not found."
    echo "  Please set QT6_DIR or CMAKE_PREFIX_PATH environment variable."
    echo "  Example: export QT6_DIR=\"/path/to/Qt/6.x.x/macos\""
    return 1 2>/dev/null || exit 1
fi

# Verify Qt installation
if [ ! -f "$QT_PATH/lib/cmake/Qt6/Qt6Config.cmake" ]; then
    echo "  Error: Qt6Config.cmake not found at: $QT_PATH/lib/cmake/Qt6/Qt6Config.cmake"
    return 1 2>/dev/null || exit 1
fi

export CMAKE_PREFIX_PATH="$QT_PATH:${CMAKE_PREFIX_PATH}"
echo "  ✓ Qt path configured: $QT_PATH"
echo ""

# 2. Check vcpkg (optional)
echo "Step 2: Checking vcpkg..."
if [ -n "$VCPKG_ROOT" ] && [ -d "$VCPKG_ROOT" ]; then
    echo "  ✓ vcpkg found at: $VCPKG_ROOT"
    if [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
        echo "  ✓ vcpkg toolchain file found"
        export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    else
        echo "  ⚠ vcpkg toolchain file not found"
    fi
else
    echo "  ℹ vcpkg not configured (optional)"
    echo "  To use vcpkg, set VCPKG_ROOT environment variable:"
    echo "    export VCPKG_ROOT=\"/path/to/vcpkg\""
fi
echo ""

# 3. Verify CMake
echo "Step 3: Verifying CMake..."
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "  ✓ CMake found: version $CMAKE_VERSION"
    
    # Check minimum version
    REQUIRED_VERSION="3.16"
    if [ "$(printf '%s\n' "$REQUIRED_VERSION" "$CMAKE_VERSION" | sort -V | head -n1)" != "$REQUIRED_VERSION" ]; then
        echo "  ⚠ Warning: CMake version $CMAKE_VERSION is older than required $REQUIRED_VERSION"
    fi
else
    echo "  ✗ CMake not found. Please install CMake 3.16 or higher."
    return 1 2>/dev/null || exit 1
fi
echo ""

# 4. Verify C++ Compiler
echo "Step 4: Verifying C++ compiler..."
if command -v clang++ &> /dev/null; then
    COMPILER_VERSION=$(clang++ --version | head -n1)
    echo "  ✓ C++ compiler found: $COMPILER_VERSION"
elif command -v g++ &> /dev/null; then
    COMPILER_VERSION=$(g++ --version | head -n1)
    echo "  ✓ C++ compiler found: $COMPILER_VERSION"
else
    echo "  ✗ C++ compiler not found. Please install clang++ or g++."
    return 1 2>/dev/null || exit 1
fi
echo ""

# 5. Verify Python (for schema generation)
echo "Step 5: Verifying Python..."
if command -v python3 &> /dev/null; then
    PYTHON_VERSION=$(python3 --version)
    echo "  ✓ Python found: $PYTHON_VERSION"
else
    echo "  ⚠ Python3 not found (required for schema generation)"
fi
echo ""

# 6. Summary
echo "=========================================="
echo "Environment Setup Complete"
echo "=========================================="
echo ""
echo "Environment variables set:"
echo "  CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"
if [ -n "$CMAKE_TOOLCHAIN_FILE" ]; then
    echo "  CMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE"
fi
echo ""
echo "Next steps:"
echo "  1. Navigate to project directory:"
echo "     cd $PROJECT_ROOT"
echo ""
echo "  2. Configure CMake:"
echo "     cmake -S . -B build"
echo ""
echo "  3. Build incrementally (recommended):"
echo "     # Phase 1: Domain common (header-only)"
echo "     cmake --build build --target zmon_domain_common"
echo ""
echo "     # Phase 2: Domain layer"
echo "     cmake --build build --target z_monitor_domain"
echo ""
echo "     # Phase 3: Application layer"
echo "     cmake --build build --target z_monitor_application"
echo ""
echo "     # Phase 4: Infrastructure layer"
echo "     cmake --build build --target z_monitor_infrastructure"
echo ""
echo "     # Phase 5: Main executable"
echo "     cmake --build build --target z-monitor"
echo ""
echo "  4. Or build everything:"
echo "     cmake --build build"
echo ""
echo "Note: These environment variables are only valid for the current shell session."
echo "To make them permanent, add to your ~/.zshrc or ~/.bashrc:"
echo "  export CMAKE_PREFIX_PATH=\"$QT_PATH:\$CMAKE_PREFIX_PATH\""
if [ -n "$VCPKG_ROOT" ]; then
    echo "  export VCPKG_ROOT=\"$VCPKG_ROOT\""
fi

