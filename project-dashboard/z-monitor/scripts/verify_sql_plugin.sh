#!/bin/bash
# Diagnostic script to verify SQL plugin deployment and configuration

set -e

echo "======================================"
echo "SQL Plugin Deployment Verification"
echo "======================================"
echo ""

# Find z-monitor executable
EXECUTABLE=$(find build -name "z-monitor" -type f 2>/dev/null | head -1)
if [ -z "$EXECUTABLE" ]; then
    echo "❌ ERROR: z-monitor executable not found"
    echo "   Run: cmake --build build --target z-monitor"
    exit 1
fi

EXECUTABLE_DIR=$(dirname "$EXECUTABLE")
echo "✅ Found executable: $EXECUTABLE"
echo "   Directory: $EXECUTABLE_DIR"
echo ""

# Check if plugin exists
PLUGIN_PATH="$EXECUTABLE_DIR/sqldrivers/libqsqlite.dylib"
if [ ! -f "$PLUGIN_PATH" ]; then
    echo "❌ ERROR: SQLite plugin not found at expected location"
    echo "   Expected: $PLUGIN_PATH"
    echo ""
    echo "Searching for plugin..."
    FOUND_PLUGIN=$(find build -name "libqsqlite.dylib" -type f 2>/dev/null | head -1)
    if [ -n "$FOUND_PLUGIN" ]; then
        echo "   Found plugin at: $FOUND_PLUGIN"
        echo "   This location doesn't match applicationDirPath() + '/sqldrivers'"
    else
        echo "   Plugin not found anywhere in build directory"
    fi
    exit 1
fi

echo "✅ SQLite plugin found: $PLUGIN_PATH"
echo ""

# Check plugin file info
echo "Plugin file information:"
file "$PLUGIN_PATH"
echo ""

# Verify plugin is loadable (check dependencies)
echo "Plugin dependencies:"
otool -L "$PLUGIN_PATH" | grep -v "$PLUGIN_PATH:" || true
echo ""

# Check if Qt installation has the plugin
QT_DIR=$(qmake -query QT_INSTALL_PREFIX 2>/dev/null || echo "/Users/dustinwind/Qt/6.9.2/macos")
QT_PLUGIN_PATH="$QT_DIR/plugins/sqldrivers/libqsqlite.dylib"
echo "Qt installation plugin path:"
echo "   $QT_PLUGIN_PATH"
if [ -f "$QT_PLUGIN_PATH" ]; then
    echo "   ✅ Plugin exists in Qt installation"
else
    echo "   ⚠️  Plugin not found in Qt installation"
fi
echo ""

# Summary
echo "======================================"
echo "Summary"
echo "======================================"
echo "Executable directory: $EXECUTABLE_DIR"
echo "Plugin directory:     $EXECUTABLE_DIR/sqldrivers"
echo ""
echo "Expected runtime behavior:"
echo "1. QCoreApplication::applicationDirPath() returns: $EXECUTABLE_DIR"
echo "2. addLibraryPath(applicationDirPath()) adds: $EXECUTABLE_DIR to search paths"
echo "3. addLibraryPath(applicationDirPath() + '/sqldrivers') adds: $EXECUTABLE_DIR/sqldrivers"
echo "4. Qt should find plugin at: $PLUGIN_PATH"
echo ""

if [ -f "$PLUGIN_PATH" ]; then
    echo "✅ Plugin deployment verified - should work correctly"
    exit 0
else
    echo "❌ Plugin deployment failed"
    exit 1
fi
