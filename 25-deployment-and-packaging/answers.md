# Lesson 25 Quiz Answers: Deployment and Packaging

## 1. What does `windeployqt MyApp.exe` do, and when should you use it?

**Answer:** It copies all required Qt DLLs and plugins needed to run MyApp.exe on systems without Qt installed.

**Explanation:** `windeployqt` analyzes your executable, determines which Qt modules it uses (Widgets, Network, etc.), and copies the corresponding DLLs, platform plugins (qwindows.dll), and other dependencies to your application directory. Use it after building a release version and before distributing to end users. It saves hours of manually identifying dependencies. Example:

```bash
windeployqt --release --no-translations MyApp.exe
```

This creates a deployable folder with MyApp.exe, Qt6Core.dll, Qt6Widgets.dll, Qt6Gui.dll, platforms/qwindows.dll, and other required files.

## 2. What's the difference between static and dynamic linking in Qt applications?

**Answer:** Dynamic linking uses shared Qt libraries (.dll/.so); static linking compiles Qt into the executable.

**Explanation:** 

**Dynamic (default):**
- Smaller executable (50KB-5MB)
- Requires Qt libraries distributed with app
- Can update Qt libraries independently
- LGPL-friendly

**Static:**
- Large executable (15-50MB+)
- Single self-contained file
- No external dependencies
- Requires commercial license or LGPL compliance (must provide .o files for re-linking)

Build statically: configure Qt with `-static`, rebuild Qt from source, then build your app against static Qt. Most deployments use dynamic linking with bundled libraries.

## 3. Why might this deployment fail on a user's Windows machine?

```bash
# Just copying the executable
copy MyApp.exe C:\Deploy\
```

**Answer:** Missing Qt DLLs and plugins - the app won't find required libraries.

**Explanation:** Qt applications need:
1. **Qt libraries** - Qt6Core.dll, Qt6Widgets.dll, Qt6Gui.dll, etc.
2. **Platform plugin** - platforms/qwindows.dll (critical!)
3. **Compiler runtime** - MSVC redistributables (vcruntime140.dll, etc.)

Without these, the app shows errors like "Qt platform plugin could not be initialized" or "Missing DLL" errors. Proper deployment:

```bash
windeployqt --release MyApp.exe
# Now the entire folder can be distributed
```

## 4. What is an AppImage and what problem does it solve on Linux?

**Answer:** AppImage is a self-contained executable that runs on any Linux distribution without installation.

**Explanation:** Linux has fragmented package management (.deb for Debian/Ubuntu, .rpm for Fedora/Red Hat, etc.) and varying Qt versions across distros. AppImage solves this by bundling the application and all dependencies (including Qt libraries) into a single executable file. Users download it, make it executable (`chmod +x MyApp.AppImage`), and run it - no installation needed. It works on Ubuntu, Fedora, Arch, etc. because it brings its own libraries. Create with:

```bash
linuxdeployqt MyApp -appimage
```

Alternatives include Flatpak (sandboxed) and Snap (Ubuntu-focused).

## 5. What are the minimum files needed to deploy a Qt Widgets application on Windows?

**Answer:**

```
MyApp.exe
Qt6Core.dll
Qt6Gui.dll
Qt6Widgets.dll
platforms/qwindows.dll
```

**Explanation:** These are the absolute minimum for a basic Widgets app. Depending on features used, you might also need:
- Qt6Network.dll (networking)
- Qt6Sql.dll (databases)
- styles/ (if using non-default styles)
- imageformats/ (for PNG/JPG support)
- MSVC runtime DLLs (vcruntime140.dll, msvcp140.dll)

`windeployqt` automatically identifies and copies all required files. The `platforms/qwindows.dll` plugin is critical - without it, the app fails with "This application failed to start because no Qt platform plugin could be initialized."

## 6. Why is code signing important for macOS applications?

**Answer:** Unsigned apps are blocked by Gatekeeper; users see "cannot be opened because it is from an unidentified developer."

**Explanation:** Since macOS 10.15 (Catalina), Apple requires:
1. **Code signing** - Developer ID certificate from Apple ($99/year)
2. **Notarization** - Apple scans app for malware, issues ticket

Without these, macOS blocks the app unless users right-click and choose "Open" (scary for non-technical users). Sign with:

```bash
codesign --deep --force --sign "Developer ID Application: Your Name" MyApp.app
```

Then notarize:
```bash
xcrun notarytool submit MyApp.dmg --apple-id user@example.com --wait
xcrun stapler staple MyApp.app
```

Notarization can take 5-60 minutes. Apps distributed via Mac App Store skip this (Apple signs them).

## 7. If you see this error on a deployed Windows app: "The application was unable to start correctly (0xc000007b)", what's likely wrong?

**Answer:** Mixing 32-bit and 64-bit binaries, or missing MSVC runtime libraries.

**Explanation:** Error 0xc000007b specifically indicates architecture mismatch. Common causes:

1. **64-bit exe with 32-bit Qt DLLs** (or vice versa)
2. **Missing MSVC redistributables** - Install vcredist_x64.exe
3. **Wrong Qt build** - Debug app with release DLLs

Fix by ensuring all components match:
- 64-bit executable → 64-bit Qt libraries → 64-bit MSVC runtime
- Built with MSVC 2019 → Deploy MSVC 2019 runtime

Use `dumpbin /dependents MyApp.exe` to check dependencies. `windeployqt` usually gets this right if you use matching Qt build.

## 8. What Qt license considerations exist when statically linking Qt?

**Answer:** Commercial license or LGPL requires providing object files for users to re-link.

**Explanation:** Qt is dual-licensed:

**Commercial:** Allows static linking with no source/object file obligations. Costs $5,000+/year per developer.

**LGPL v3:** Allows static linking BUT requires:
- Provide your application's object files (.o/.obj)
- Allow users to re-link your app with modified Qt
- Document how to rebuild

This is impractical for closed-source apps. Most LGPL users choose dynamic linking - just distribute Qt DLLs alongside the exe. This satisfies LGPL (users can replace Qt libraries) without exposing your code.

**GPL:** Requires releasing your entire source code. Rarely used for commercial apps.

## 9. How do you determine which Qt plugins your application needs?

**Answer:** Run the app and check plugin paths, or use deployment tools to analyze dependencies.

**Explanation:** Qt loads plugins from:

```cpp
qDebug() << QCoreApplication::libraryPaths();
```

Common plugins:
- **platforms/** - Required (qwindows.dll, qcocoa.dylib, qxcb.so)
- **imageformats/** - If loading PNG/JPG: qjpeg.dll, qpng.dll
- **sqldrivers/** - If using QSql: qsqlite.dll, qpsql.dll
- **styles/** - For custom styles

The easiest method: use `windeployqt`/`macdeployqt`/`linuxdeployqt` - they analyze your exe and copy needed plugins. Or run your app with:

```bash
QT_DEBUG_PLUGINS=1 ./MyApp
```

This shows which plugins Qt attempts to load.

## 10. What's the purpose of `install_name_tool` in macOS deployment?

**Answer:** It fixes library paths in macOS binaries to reference bundled frameworks.

**Explanation:** macOS executables store absolute paths to libraries they link against. If you build against `/usr/local/Qt-6.5.0/lib/QtCore.framework/QtCore`, the deployed app looks there - which doesn't exist on user machines.

`install_name_tool` changes these paths to relative ones:

```bash
# Before: /usr/local/Qt-6.5.0/lib/QtCore.framework/QtCore
# After: @executable_path/../Frameworks/QtCore.framework/QtCore

install_name_tool -change /usr/local/Qt-6.5.0/lib/QtCore.framework/QtCore \
    @executable_path/../Frameworks/QtCore.framework/QtCore \
    MyApp.app/Contents/MacOS/MyApp
```

`macdeployqt` does this automatically. The `@executable_path` makes the path relative to the executable's location, so libraries are found inside the .app bundle. Similar tools: `otool -L` (inspect dependencies), `@rpath` (runtime search paths).
