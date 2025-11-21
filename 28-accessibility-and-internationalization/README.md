# Lesson 28: Accessibility and Internationalization

This lesson demonstrates Qt's accessibility and internationalization (i18n) features. The demo shows how to make applications accessible to users with disabilities and translatable to multiple languages. Students learn about tr(), QTranslator, QLocale, accessible names, keyboard navigation, and right-to-left (RTL) layout support.

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
cd 28-accessibility-and-internationalization
docker build -t qt-lesson-28 .
```

## Running

### macOS
```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" qt-lesson-28
```

### Linux
```bash
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix qt-lesson-28
```

## Expected Behavior

The application displays a comprehensive demo of accessibility and internationalization features:

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
