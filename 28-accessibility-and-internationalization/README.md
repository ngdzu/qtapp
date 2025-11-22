# Lesson 28: Accessibility and Internationalization

This lesson demonstrates Qt's accessibility and internationalization (i18n) features for making applications accessible to users with disabilities and translatable to multiple languages.

## Building and Running

### One-Time Setup

These steps only need to be done once per machine.

#### 1. Install X11 Server

**For macOS users:**
- Install XQuartz: `brew install --cask xquartz`
- Start XQuartz and enable "Allow connections from network clients" in Preferences > Security

**For Linux users:**
- X11 should be available by default

#### 2. Build the shared Qt base images

From the **root directory** of the repository:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime-nano -t qtapp-qt-runtime-nano:latest .
```

> **Note:** The dev environment is ~1.33 GB (used only for building) and the runtime is ~242 MB. All lessons share these base images, so each individual lesson only adds ~16 KB (just the executable). This keeps total storage minimal even with 28 lessons!

#### 3. Grant X11 access to Docker containers

From the **root directory** of the repository:

```bash
./scripts/xhost-allow-for-compose.sh allow
```

> **Note:** This disables X11 access control to allow Docker containers to display GUI applications. Run this once per session (after reboot, you'll need to run it again). To revoke access later, run `./scripts/xhost-allow-for-compose.sh revoke`.

### Build and Run This Lesson

#### Step 1: Build this lesson's image

From the **lesson directory** (`28-accessibility-and-internationalization`):

```bash
docker build -t qtapp-lesson28:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qtapp-lesson28:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson28:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson28
```

## What You'll See

A comprehensive accessibility and internationalization demonstration with tabbed interface:

**Main Window:**
- Title showing current lesson topic
- Language selection buttons (English, Español, Français)
- Four tabs demonstrating different aspects of a11y and i18n
- Full keyboard navigation support with logical tab order

**Translation Demo Tab:**
- Current language indicator
- Sample translated strings that change with language selection
- Plural form demonstrations (0 items, 1 item, 2 items, 5 items, 21 items)
- Explanation of Qt's translation workflow:
  1. Developer wraps strings in tr()
  2. lupdate extracts to .ts files
  3. Translator translates in Qt Linguist
  4. lrelease compiles to .qm files
  5. QTranslator loads at runtime
- Note: This demo simulates translations (actual .qm files not included)

**Locale Formatting Tab:**
- Current locale display (en_US, es_ES, fr_FR)
- Date formatting examples (short and long formats):
  - US: "11/21/2025"
  - Europe: "21.11.2025"
- Time formatting examples (short and long formats):
  - US: "2:30 PM"
  - Europe: "14:30"
- Number formatting with locale-specific separators:
  - US: "1,234,567.89"
  - Europe: "1.234.567,89"
- Currency formatting:
  - US: "$1,234.56"
  - Spain: "1.234,56 €"
  - France: "1 234,56 €"
- Measurement system (Metric vs Imperial)

**Accessibility Features Tab:**
- List of accessible names for UI elements
- Keyboard navigation instructions:
  - Tab/Shift+Tab for navigation
  - Space/Enter to activate buttons
  - Arrow keys within widgets
- Screen reader support information:
  - How NVDA, JAWS, VoiceOver announce widgets
  - Accessible names and descriptions
  - Widget types and states
- Focus policy explanations:
  - StrongFocus (Tab + Click) for buttons
  - NoFocus for labels
- Best practices checklist:
  - All interactive elements keyboard accessible
  - Logical tab order
  - Icon-only buttons have accessible names
  - Tooltips for visual users

**RTL Support Tab:**
- Current layout direction indicator (LTR or RTL)
- List of RTL languages (Arabic, Hebrew, Persian, Urdu)
- How Qt automatically mirrors layouts for RTL languages
- QHBoxLayout becomes right-to-left
- Icons and text alignment adjust automatically

Click language buttons to see translations and locale formatting change dynamically!

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast"). These can be safely ignored - the application runs perfectly without hardware acceleration.

## Requirements

- **Qt Modules:** Qt6::Widgets, Qt6::Core
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS

## Learning Objectives

- Using tr() to mark translatable strings in code
- Qt's translation workflow with lupdate, Qt Linguist, and lrelease
- Handling plural forms correctly across different languages
- Locale-aware formatting for dates, times, numbers, and currency
- Setting accessible names and descriptions for screen readers
- Implementing complete keyboard navigation
- Supporting right-to-left (RTL) languages with layout mirroring
- QLocale for culture-specific formatting
- QTranslator for runtime language switching
- Accessibility best practices for inclusive applications
- Testing applications with screen readers

## Notes

- The Dockerfile uses a multi-stage build: lessons use the `qt-runtime-nano` base (~242 MB) which contains only essential Qt libraries needed to run applications
- The dev environment (`qt-dev-env`) is only needed for building and is ~1.33 GB
- This demo simulates translations - production apps would include compiled .qm files
- Qt automatically handles many RTL layout adjustments
- Screen readers like NVDA (Windows), JAWS (Windows), and VoiceOver (macOS) work with Qt's accessibility API
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`
- Harmless GL/Mesa warnings about missing drivers can be ignored - the app works fine without hardware acceleration
- What gets mirrored in RTL mode:
  - Layout direction
  - Text alignment
  - Widget ordering
  - Scrollbars
  - Tab order
  - Icons
- Explanation of Qt's automatic RTL handling
- Toggle button to switch between LTR and RTL modes
- When toggled, the entire UI mirrors demonstrating RTL layout

**Interactive Features:**
- Click language buttons to see UI update (simulated translations)
- Toggle RTL button to see layout mirror
- Tab through controls to test keyboard navigation
- All widgets have accessible names for screen readers

## Learning Objectives

After completing this lesson, you should understand:
- How to use tr() to mark strings for translation
- Qt's translation workflow (lupdate, Linguist, lrelease)
- Loading translations at runtime with QTranslator
- Handling plural forms across different languages
- Using QLocale for locale-aware formatting (dates, times, numbers, currency)
- Setting accessible names and descriptions for screen readers
- Implementing proper keyboard navigation with tab order
- Qt's automatic RTL language support
- The difference between accessibility and internationalization
- Best practices for creating inclusive, globally-accessible applications

## Accessibility Testing

To test accessibility features:

**Screen Readers:**
- **Windows**: NVDA (free) or JAWS
- **macOS**: VoiceOver (built-in, Cmd+F5)
- **Linux**: Orca

**Keyboard Navigation:**
1. Launch app and unplug mouse
2. Use Tab to navigate through all controls
3. Verify tab order is logical
4. Test that Space/Enter activates buttons
5. Ensure all functionality accessible via keyboard

**High Contrast:**
- Qt automatically supports system high contrast themes
- Test on Windows with High Contrast mode enabled

## Internationalization Workflow

In a real application, you would:

1. **Mark strings for translation:**
   ```cpp
   QString msg = tr("Save file?");
   ```

2. **Extract translatable strings:**
   ```bash
   lupdate myapp.pro -ts translations/myapp_es.ts
   ```

3. **Translate in Qt Linguist:**
   ```bash
   linguist translations/myapp_es.ts
   ```

4. **Compile translations:**
   ```bash
   lrelease translations/myapp_es.ts -qm translations/myapp_es.qm
   ```

5. **Load at runtime:**
   ```cpp
   QTranslator translator;
   translator.load("myapp_es.qm", ":/translations");
   app.installTranslator(&translator);
   ```

This demo simulates the result of this workflow without actual .qm files.
