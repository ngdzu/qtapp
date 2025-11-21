# Lesson 25: Deployment and Packaging

## Learning Goals

By the end of this lesson, you will:
- Understand Qt deployment tools: `windeployqt`, `macdeployqt`, `linuxdeployqt`
- Learn how to identify and bundle required Qt libraries
- Know how to create installers with Qt Installer Framework
- Understand static vs dynamic linking trade-offs
- Learn about code signing and notarization requirements

## Introduction to Qt Deployment

Deploying a Qt application means packaging your executable with all necessary dependencies (Qt libraries, plugins, resources) so it runs on target systems without requiring Qt installation. Each platform has unique requirements and tools.

### Platform-Specific Challenges

**Windows:** Applications need Qt DLLs, platform plugins (qwindows.dll), and potentially compiler runtime libraries (MSVC redistributables). Use `windeployqt` to automatically copy required files.

**macOS:** Applications are bundled as `.app` packages containing the executable, frameworks, and resources. Use `macdeployqt` to create self-contained bundles. Code signing and notarization are required for distribution outside the App Store.

**Linux:** The most complex platform due to distribution diversity. Applications typically rely on system Qt libraries or bundle everything with AppImage/Flatpak/Snap. Use `linuxdeployqt` or `linuxdeploy` with Qt plugin.

## Deployment Tools

### windeployqt (Windows)

```bash
windeployqt --release --no-translations MyApp.exe
```

This copies all required Qt DLLs, plugins (platforms/, imageformats/, etc.), and libraries to your application directory. Options:
- `--release` - Deploy release builds (omits debug libraries)
- `--no-translations` - Skip Qt translation files
- `--qmldir <dir>` - Scan QML files for dependencies
- `--dir <dir>` - Specify output directory

### macdeployqt (macOS)

```bash
macdeployqt MyApp.app -dmg
```

Creates a self-contained .app bundle and optionally packages it as a .dmg disk image. The tool:
- Copies Qt frameworks into `MyApp.app/Contents/Frameworks/`
- Fixes library paths using `install_name_tool`
- Copies platform plugins to `PlugIns/`
- Options: `-codesign=<identity>` for code signing

### linuxdeployqt (Linux)

```bash
linuxdeployqt MyApp -appimage
```

Creates an AppImage - a self-contained executable that runs on most Linux distributions. Alternatives include:
- **Flatpak** - Sandboxed distribution format
- **Snap** - Universal Linux packages
- **System packages** - .deb, .rpm with Qt dependencies

## Static vs Dynamic Linking

**Dynamic Linking (Default):**
- Application uses shared Qt libraries (.dll, .dylib, .so)
- Smaller executable size
- Easier updates (replace libraries without rebuilding)
- Deployment requires bundling or system installation of Qt

**Static Linking:**
- Qt libraries compiled into the executable
- Single-file deployment (no external dependencies)
- Larger executable (10-50 MB for simple apps)
- Requires commercial Qt license or LGPL compliance
- Build Qt from source with `-static` configuration

```bash
# Building Qt statically (simplified)
./configure -static -prefix /opt/qt-static -release -nomake examples
cmake --build . --parallel
cmake --install .
```

## Qt Installer Framework

Qt Installer Framework (IFW) creates professional installers for Windows, macOS, and Linux:

```xml
<!-- config.xml -->
<Installer>
    <Name>MyApplication</Name>
    <Version>1.0.0</Version>
    <Title>My Application Installer</Title>
    <Publisher>My Company</Publisher>
    <StartMenuDir>MyApp</StartMenuDir>
</Installer>
```

Create installer:
```bash
binarycreator -c config.xml -p packages MyAppInstaller
```

## Code Signing

**Windows:** Use `signtool` with a code signing certificate:
```bash
signtool sign /f certificate.pfx /p password /t http://timestamp.server MyApp.exe
```

**macOS:** Use Apple Developer certificate:
```bash
codesign --deep --force --verify --verbose --sign "Developer ID Application: Company Name" MyApp.app
# Notarization (required for macOS 10.15+)
xcrun notarytool submit MyApp.dmg --apple-id user@example.com --wait
```

**Linux:** AppImage supports GPG signing for verification.

## Example Walkthrough

Our demo application shows deployment information relevant to the current platform:

1. **Executable Info** - Shows application path, Qt version, build type
2. **Platform Details** - Displays OS, architecture, and compiler
3. **Library Paths** - Lists where Qt searches for plugins and libraries
4. **Deployment Checklist** - Platform-specific deployment guidance

The application itself demonstrates a properly structured Qt app ready for deployment - with resources, proper window handling, and clean architecture.

## Expected Output

The application displays detailed deployment information:
- Application executable path
- Qt version and configuration
- System information (OS, CPU architecture)
- Plugin search paths
- Platform-specific deployment instructions

This helps you understand what needs to be packaged with your application.

## Try It

**Exercise 1**: Use `windeployqt` (Windows), `macdeployqt` (macOS), or `linuxdeployqt` (Linux) on this lesson's executable. Examine what files are copied. Try running the deployed app on a system without Qt installed.

**Exercise 2**: Create a simple Qt Widgets app and package it as an AppImage (Linux) or .dmg (macOS). Test on a different machine.

**Exercise 3**: Research Qt's LGPL obligations. If you distribute a closed-source application with dynamic Qt libraries, what files must you provide to users?

## Key Takeaways

- Each platform has specific deployment requirements and tools
- `windeployqt`, `macdeployqt`, and `linuxdeployqt` automate dependency bundling
- Dynamic linking is default and easier; static linking creates standalone executables
- Code signing is essential for Windows/macOS distribution
- Qt Installer Framework provides cross-platform installer creation
- Always test deployed applications on clean systems without development tools
- AppImage, Flatpak, and Snap simplify Linux distribution
- Proper deployment ensures your application runs on end-user systems without Qt SDK installed
