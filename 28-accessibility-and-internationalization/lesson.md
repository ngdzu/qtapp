# Lesson 28: Accessibility and Internationalization

## Learning Goals
- Make Qt applications accessible to users with disabilities
- Implement internationalization (i18n) for multi-language support
- Use Qt's translation system (tr() and QTranslator)
- Apply accessibility best practices (keyboard navigation, screen readers)
- Handle right-to-left (RTL) languages and locale-specific formatting

## Introduction

Accessible, internationalized applications reach the widest possible audience. This lesson covers Qt's built-in support for accessibility features (screen readers, keyboard navigation) and internationalization (multiple languages, locale-aware formatting). You'll learn to use `tr()` for translatable strings, create translation files with Qt Linguist, and make your UI accessible to users with visual, motor, or cognitive disabilities.

## Key Concepts

**Translatable Strings with tr()**

Wrap user-visible strings in `tr()` to make them translatable:

```cpp
// Good: Translatable
QPushButton *btn = new QPushButton(tr("Save"));
QLabel *label = new QLabel(tr("File saved successfully!"));

// Bad: Hardcoded, not translatable
QPushButton *btn = new QPushButton("Save");
QLabel *label = new QLabel("File saved successfully!");
```

The `tr()` function marks strings for translation. At runtime, Qt looks up translations in loaded `.qm` files.

**Creating and Loading Translations**

Generate `.ts` files, translate them, then load at runtime:

```bash
# 1. Extract translatable strings
lupdate myapp.pro -ts translations/myapp_es.ts translations/myapp_fr.ts

# 2. Translate with Qt Linguist (GUI tool)
linguist translations/myapp_es.ts

# 3. Compile to binary .qm files
lrelease translations/myapp_es.ts -qm translations/myapp_es.qm
```

```cpp
// 4. Load at runtime
QTranslator translator;
if (translator.load("myapp_es.qm", ":/translations")) {
    app.installTranslator(&translator);
}
```

**Context-Aware Translation**

Provide context for ambiguous strings:

```cpp
// "Close" can mean different things
tr("Close"); // Close window? Close file?

// Better: Provide context
QCoreApplication::translate("FileMenu", "Close"); // Close file
QCoreApplication::translate("WindowMenu", "Close"); // Close window
```

**Plurals and Number Formatting**

Handle singular/plural forms correctly:

```cpp
int count = 5;
QString msg = tr("You have %n file(s)", "", count);
// English: "You have 5 files"
// Polish: "Masz 5 plików" (different plural rules)
```

Use locale-aware formatting:

```cpp
QLocale locale(QLocale::French);
double price = 1234.56;
QString priceStr = locale.toCurrencyString(price);
// French: "1 234,56 €"
// US: "$1,234.56"
```

**Accessibility with QAccessible**

Provide accessible names and descriptions:

```cpp
QPushButton *btn = new QPushButton();
btn->setAccessibleName(tr("Save Document"));
btn->setAccessibleDescription(tr("Saves the current document to disk"));

// For custom widgets, implement QAccessibleInterface
widget->setAccessibleRole(QAccessible::Button);
```

Screen readers (NVDA, JAWS, VoiceOver) use these properties to describe UI to visually impaired users.

**Keyboard Navigation**

Ensure full keyboard accessibility:

```cpp
// Set tab order
setTabOrder(nameEdit, emailEdit);
setTabOrder(emailEdit, submitBtn);

// Keyboard shortcuts
QAction *saveAction = new QAction(tr("&Save"), this);
saveAction->setShortcut(QKeySequence::Save); // Ctrl+S

// Focus indicators (automatically styled by Qt)
btn->setFocusPolicy(Qt::StrongFocus);
```

Users should be able to navigate and operate your app entirely via keyboard.

**Right-to-Left (RTL) Language Support**

Qt automatically handles RTL layout for Arabic, Hebrew, etc.:

```cpp
// Enable RTL support
QApplication app(argc, argv);
app.setLayoutDirection(Qt::RightToLeft);

// Or auto-detect from translation
QTranslator translator;
translator.load("myapp_ar.qm"); // Arabic translation
app.installTranslator(&translator);
// Layout direction set automatically based on language
```

Layouts and widgets automatically mirror in RTL mode.

**Locale-Specific Date/Time Formatting**

Format dates according to user's locale:

```cpp
QLocale locale;
QDate date = QDate::currentDate();

// US: "11/21/2025"
// Europe: "21.11.2025"
// ISO: "2025-11-21"
QString dateStr = locale.toString(date, QLocale::ShortFormat);
```

## Example Walkthrough

The demo application demonstrates:

1. **Language switching** - Buttons to switch between English, Spanish, French
2. **Translatable UI** - All strings wrapped in `tr()`
3. **Locale formatting** - Date, time, numbers formatted per locale
4. **Accessibility features** - Accessible names, keyboard shortcuts, tab order
5. **RTL support** - Toggle RTL layout to simulate Arabic/Hebrew

When you switch languages, all UI elements update immediately using Qt's retranslateUi mechanism.

## Expected Output

A Qt application that displays:
- Language selection buttons (English, Spanish, French)
- Sample text that changes based on selected language
- Locale-formatted dates, times, and numbers
- Accessibility information (accessible names, roles)
- RTL layout toggle to demonstrate right-to-left languages
- Keyboard shortcuts working correctly

Switching languages updates all UI elements in real-time without restarting the app.

## Try It

1. **Switch languages** - Click language buttons to see translations
2. **Toggle RTL** - See how layout mirrors for right-to-left languages
3. **Test keyboard navigation** - Tab through controls, use shortcuts
4. **Inspect accessibility** - Use screen reader (if available) to hear UI descriptions
5. **Check locale formatting** - Observe how dates/numbers change with language
6. **Add a new translation** - Create your own `.ts` file and translate strings

## Key Takeaways

- **tr() marks strings for translation** - Wrap all user-visible text
- **QTranslator loads translations** at runtime from `.qm` files
- **lupdate/lrelease workflow** - Extract strings, translate, compile
- **Accessibility is essential** - Provide accessible names, keyboard navigation, screen reader support
- **QLocale handles formatting** - Dates, times, numbers, currency per locale
- **RTL support is automatic** - Qt mirrors layouts for right-to-left languages
- **Context matters** - Use translation context for ambiguous strings
- **Test with real users** - Accessibility and i18n require testing with diverse users
- **Plurals vary by language** - Use `%n` notation for correct plural forms
