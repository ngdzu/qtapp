# Lesson 25: Deployment and Packaging

This lesson demonstrates Qt application deployment information, showing executable details, platform information, library paths, and platform-specific deployment guidance. The demo provides a comprehensive view of what developers need to know when preparing Qt applications for distribution.

## Prerequisites

For GUI applications on macOS, you need to set up X11 forwarding:
1. Install XQuartz: `brew install --cask xquartz`
2. Start XQuartz and enable "Allow connections from network clients" in Preferences > Security
3. Run: `xhost + localhost`

## Building

First, ensure the base images are built:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

Then build this lesson:

```bash
cd 25-deployment-and-packaging
docker build -t qt-lesson-25 .
```

## Running

### macOS
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-25
```

### Linux
```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-25
```

## Expected Behavior

The application displays a tabbed interface with deployment information:

**Executable Info Tab:**
- Application executable path and directory
- File size and executable status
- Qt runtime version vs compiled version
- Build type (Debug/Release)
- Qt modules used in the application

**Platform Tab:**
- Operating system and version
- Kernel type and version
- CPU architecture and build ABI
- Platform-specific requirements (DLLs, frameworks, plugins)
- Compiler information (MSVC, GCC, or Clang version)

**Library Paths Tab:**
- Qt library search paths
- Qt installation directories (prefix, libraries, plugins, binaries)
- Important plugin directories (platforms, imageformats, styles, sqldrivers)
- Explanation of required plugins for deployment

**Deployment Guide Tab:**
- Platform-specific deployment instructions
- Required files and directory structure
- Deployment tool usage (windeployqt/macdeployqt/linuxdeployqt)
- Code signing requirements and procedures
- Static vs dynamic linking comparison
- Best practices checklist

The window includes a "Refresh Information" button to update all tabs with current system state.

## Learning Objectives

After completing this lesson, you should understand:
- How to identify deployment dependencies for Qt applications
- Platform-specific deployment requirements (Windows, macOS, Linux)
- Using Qt's deployment tools effectively
- The difference between static and dynamic linking
- Code signing requirements for production applications
- Best practices for testing deployments on clean systems
- How to structure application bundles and installers
