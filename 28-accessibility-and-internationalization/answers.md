# Lesson 28 Quiz Answers: Accessibility and Internationalization

## 1. What's the difference between tr() and QCoreApplication::translate(), and when would you use each?

**Answer:** `tr()` is a convenience method on QObject; `translate()` allows specifying context explicitly.

**Explanation:**

**tr()** is a member function of QObject that uses the class name as context:

```cpp
class MyWidget : public QWidget {
    void setupUI() {
        // Context automatically set to "MyWidget"
        QPushButton *btn = new QPushButton(tr("Save"));
    }
};
```

**QCoreApplication::translate()** lets you specify context manually:

```cpp
// Useful for non-QObject code or specific context
QString msg = QCoreApplication::translate("FileDialog", "Save");
QString msg2 = QCoreApplication::translate("ExitDialog", "Save");
// Different translations possible for same word in different contexts
```

**When to use each:**
- **tr()**: Default choice in QObject-derived classes (90% of cases)
- **translate()**: When you need specific context, in non-QObject code, or when class name isn't meaningful context

Context helps translators understand usage: "Close" in file menu vs window title needs different translations in some languages.

## 2. What's wrong with this approach to translations?

```cpp
QString msg;
if (language == "English") {
    msg = "Save file?";
} else if (language == "Spanish") {
    msg = "¿Guardar archivo?";
}
```

**Answer:** Hardcoded translations are unmaintainable; use Qt's translation system instead.

**Explanation:** This approach has serious problems:

**Problems:**
- Translations embedded in code = hard to update
- No support for translators (they need .ts files, not C++ code)
- Doesn't scale (imagine 100 strings × 10 languages = 1000 if statements)
- No context for translators
- Can't use Qt Linguist tool
- Plurals, formatting impossible to handle

**Correct approach:**

```cpp
// In code: Just mark for translation
QString msg = tr("Save file?");

// Workflow:
// 1. lupdate extracts to .ts file
// 2. Translator translates in Qt Linguist
// 3. lrelease compiles to .qm
// 4. Load at runtime:

QTranslator translator;
if (translator.load("myapp_es.qm")) {
    app.installTranslator(&translator);
}
// msg automatically shows Spanish version
```

This separates code from translations, enabling professional translation workflows.

## 3. How do you handle plural forms correctly in Qt for languages with complex plural rules (like Polish or Arabic)?

**Answer:** Use `%n` notation and provide plural context in tr().

**Explanation:** Different languages have different plural rules:

- **English**: 2 forms (1 file, 2 files)
- **Polish**: 3 forms (1 plik, 2 pliki, 5 plików)
- **Arabic**: 6 forms (0, 1, 2, 3-10, 11-99, 100+)

**Correct approach:**

```cpp
int count = getFileCount();
QString msg = tr("%n file(s) found", "", count);
// Second parameter is disambiguation context (empty here)
// Third parameter is the count for plural selection
```

**In translation file (.ts):**

```xml
<message numerus="yes">
    <source>%n file(s) found</source>
    <translation>
        <numerusform>%n archivo encontrado</numerusform>
        <numerusform>%n archivos encontrados</numerusform>
    </translation>
</message>
```

Qt automatically selects the correct form based on count and language plural rules. The translator provides all necessary forms in Qt Linguist, and Qt handles the logic.

**Don't do this:**

```cpp
// Wrong: English-only plural logic
QString msg = (count == 1) ? "1 file" : QString("%1 files").arg(count);
// Fails in Polish, Arabic, etc.
```

## 4. What Qt tools are used in the translation workflow, and in what order?

**Answer:** lupdate → Qt Linguist → lrelease, then load .qm files at runtime.

**Explanation:** The complete translation workflow:

**1. lupdate - Extract translatable strings**

```bash
lupdate myapp.pro -ts translations/myapp_es.ts translations/myapp_fr.ts
```

Scans source code for `tr()` calls and generates/updates `.ts` XML files with all translatable strings.

**2. Qt Linguist - Translate**

```bash
linguist translations/myapp_es.ts
```

GUI tool where translators provide translations. Shows source string, context, and translation field. Can mark as "finished" and add comments.

**3. lrelease - Compile to binary**

```bash
lrelease translations/myapp_es.ts -qm translations/myapp_es.qm
```

Converts human-readable `.ts` to compact binary `.qm` files for runtime use.

**4. Runtime - Load translations**

```cpp
QTranslator translator;
if (translator.load("myapp_es.qm", ":/translations")) {
    app.installTranslator(&translator);
}
```

**Workflow summary:**
- **Developer**: Write code with `tr()`, run lupdate
- **Translator**: Use Linguist to translate .ts files
- **Developer**: Run lrelease, deploy .qm files
- **User**: App loads appropriate .qm at runtime

## 5. What accessibility property should you set on a custom icon-only button so screen readers can describe it?

**Answer:** Set `accessibleName` and optionally `accessibleDescription`.

**Explanation:** Icon-only buttons have no visible text, so screen readers need explicit labels:

```cpp
QPushButton *saveBtn = new QPushButton();
saveBtn->setIcon(QIcon(":/icons/save.png"));

// Required: What the button does
saveBtn->setAccessibleName(tr("Save Document"));

// Optional: More detailed description
saveBtn->setAccessibleDescription(tr("Saves the current document to disk"));

// Also set tooltip for visual users
saveBtn->setToolTip(tr("Save"));
```

**How screen readers use this:**

When a blind user tabs to the button, NVDA/JAWS/VoiceOver announces: "Save Document button. Saves the current document to disk."

**Without accessibleName:**

Screen reader says: "Button" (not helpful!)

**Additional accessibility properties:**

```cpp
// For custom widgets
widget->setAccessibleRole(QAccessible::Button);
widget->setAccessibleState(QAccessible::State::Focusable);

// For complex widgets, implement QAccessibleInterface
class MyWidget : public QWidget, public QAccessibleWidget {
    // Provide detailed accessibility tree
};
```

**Best practices:**
- ✓ All icon-only buttons must have accessibleName
- ✓ Tooltips for sighted users, accessibleName for screen readers
- ✓ Use descriptive names ("Save Document", not "Save")
- ✓ Test with actual screen reader (NVDA on Windows, VoiceOver on macOS)

## 6. How does Qt automatically handle right-to-left (RTL) languages like Arabic or Hebrew?

**Answer:** Qt mirrors layouts automatically when layoutDirection is RightToLeft.

**Explanation:** Qt provides automatic RTL support:

**Method 1: Auto-detect from translation**

```cpp
QTranslator translator;
if (translator.load("myapp_ar.qm")) { // Arabic translation
    app.installTranslator(&translator);
    // Qt auto-detects RTL language and sets layout direction
}
```

Translation files can specify RTL via language code.

**Method 2: Explicit setting**

```cpp
app.setLayoutDirection(Qt::RightToLeft);
```

**What gets mirrored:**

- **Layouts**: QHBoxLayout, QVBoxLayout reverse order
- **Text alignment**: Default switches to right-aligned
- **Icons**: Some icons (arrows) mirror automatically
- **Scrollbars**: Appear on left side
- **Sliders**: Progress right-to-left
- **Tab order**: Reverses

**Example:**

```cpp
// LTR (English):
// [Label] [TextEdit] [Button]

// RTL (Arabic):
// [Button] [TextEdit] [Label]
// Same QHBoxLayout code, automatic mirroring
```

**For images that shouldn't mirror:**

```cpp
label->setPixmap(pixmap);
label->setLayoutDirection(Qt::LeftToRight); // Force LTR for logo
```

**Testing RTL without translations:**

```cpp
// Force RTL mode for testing
QApplication app(argc, argv);
app.setLayoutDirection(Qt::RightToLeft);
```

This is why using Qt layouts (not absolute positioning) is important - they automatically adapt to RTL.

## 7. What's the correct way to format a currency value according to the user's locale?

**Answer:** Use `QLocale::toCurrencyString()` with the user's locale.

**Explanation:**

**Correct approach:**

```cpp
QLocale locale; // User's default locale
double price = 1234.56;

QString formatted = locale.toCurrencyString(price);
// US: "$1,234.56"
// Germany: "1.234,56 €"
// France: "1 234,56 €"
// Japan: "¥1,235" (no decimals)
```

**Specific locale:**

```cpp
QLocale usLocale(QLocale::English, QLocale::UnitedStates);
QLocale frLocale(QLocale::French, QLocale::France);

QString us = usLocale.toCurrencyString(1234.56);  // "$1,234.56"
QString fr = frLocale.toCurrencyString(1234.56);  // "1 234,56 €"
```

**With custom precision:**

```cpp
QString precise = locale.toCurrencyString(1234.567, "$", 3);
// "$1,234.567" (3 decimal places)
```

**Don't do this:**

```cpp
// Wrong: Assumes US format
QString bad = QString("$%1").arg(price, 0, 'f', 2);
// Always shows "$1234.56" regardless of user's locale
```

**Other locale-aware formatting:**

```cpp
// Numbers
locale.toString(123456.789);  // "123,456.789" (US) or "123.456,789" (DE)

// Dates
locale.toString(QDate::currentDate());  // "11/21/2025" (US) or "21.11.2025" (EU)

// Times
locale.toString(QTime::currentTime());  // "2:30 PM" (US) or "14:30" (EU)

// Percentages
locale.toString(0.156, 'f', 1) + "%";  // "15.6%" with locale's decimal separator
```

Always use QLocale for formatting to respect user's regional settings.

## 8. Why is keyboard navigation important for accessibility, and how do you ensure proper tab order in Qt?

**Answer:** Enables users with motor disabilities to navigate; use `setTabOrder()` for logical flow.

**Explanation:**

**Why keyboard navigation matters:**

- **Motor disabilities**: Users who can't use a mouse
- **Power users**: Keyboard shortcuts are faster
- **Screen reader users**: Navigate by tab, often can't use mouse
- **Legal requirement**: WCAG 2.1 Level A requires keyboard access

**Ensuring proper tab order:**

```cpp
// Create widgets
QLineEdit *nameEdit = new QLineEdit();
QLineEdit *emailEdit = new QLineEdit();
QLineEdit *phoneEdit = new QLineEdit();
QPushButton *submitBtn = new QPushButton(tr("Submit"));

// Set logical tab order
setTabOrder(nameEdit, emailEdit);
setTabOrder(emailEdit, phoneEdit);
setTabOrder(phoneEdit, submitBtn);

// Now Tab key follows: name → email → phone → submit
```

**Without setTabOrder**, Qt uses order of creation, which may not match visual layout.

**Focus policy:**

```cpp
// Control what can receive focus
button->setFocusPolicy(Qt::StrongFocus);    // Tab and click
label->setFocusPolicy(Qt::NoFocus);         // Never receives focus
lineEdit->setFocusPolicy(Qt::ClickFocus);   // Only by clicking
```

**Keyboard shortcuts:**

```cpp
// Standard shortcuts
QAction *saveAction = new QAction(tr("&Save"), this);
saveAction->setShortcut(QKeySequence::Save);  // Ctrl+S (Cmd+S on Mac)

// Custom shortcuts
QAction *customAction = new QAction(tr("&Custom"), this);
customAction->setShortcut(Qt::CTRL | Qt::Key_K);

// Mnemonic (Alt+S)
QPushButton *saveBtn = new QPushButton(tr("&Save"));
// & before letter creates Alt+S shortcut
```

**Testing keyboard navigation:**

1. Launch app, unplug mouse
2. Press Tab to navigate
3. Verify every interactive element reachable
4. Check tab order is logical
5. Test shortcuts work
6. Ensure Escape closes dialogs, Enter submits forms

**Best practices:**
- ✓ Set explicit tab order for forms
- ✓ Provide keyboard shortcuts for common actions
- ✓ Show focus indicators (Qt does this by default)
- ✓ Test with keyboard only
- ✗ Never require mouse for any functionality

## 9. If you change languages at runtime using QTranslator, what must you do to update all UI elements?

**Answer:** Send `LanguageChange` event via `QEvent::LanguageChange` or call `retranslateUi()`.

**Explanation:**

**Method 1: QEvent::LanguageChange (automatic)**

```cpp
void changeLanguage(const QString &langCode) {
    // Remove old translator
    app.removeTranslator(translator);
    
    // Load new translator
    if (translator.load("myapp_" + langCode + ".qm")) {
        app.installTranslator(translator);
    }
    
    // Send LanguageChange event to all widgets
    QEvent event(QEvent::LanguageChange);
    QCoreApplication::sendEvent(this, &event);
}

// In widget class, override:
void changeEvent(QEvent *event) override {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void retranslateUi() {
    saveButton->setText(tr("Save"));
    cancelButton->setText(tr("Cancel"));
    setWindowTitle(tr("My Application"));
}
```

**Method 2: Designer-generated code (recommended)**

If using Qt Designer, it generates `retranslateUi()` automatically:

```cpp
// In ui_mywidget.h (auto-generated)
void retranslateUi(QWidget *MyWidget) {
    MyWidget->setWindowTitle(QApplication::translate("MyWidget", "My Application"));
    saveButton->setText(QApplication::translate("MyWidget", "Save"));
    // ... all translatable strings
}

// In your code
void changeLanguage(const QString &langCode) {
    static QTranslator translator;
    app.removeTranslator(&translator);
    
    if (translator.load("myapp_" + langCode + ".qm")) {
        app.installTranslator(&translator);
        ui->retranslateUi(this);  // Update all strings
    }
}
```

**Why this is needed:**

Simply installing a translator doesn't update existing widgets - their `setText()` was called with the old language. You must re-call `setText()` with the new `tr()` results.

**Complete example:**

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow() {
        setupUi();
    }
    
private slots:
    void switchToSpanish() {
        switchLanguage("es");
    }
    
    void switchToFrench() {
        switchLanguage("fr");
    }
    
private:
    void switchLanguage(const QString &lang) {
        qApp->removeTranslator(&m_translator);
        
        if (m_translator.load(":/translations/myapp_" + lang + ".qm")) {
            qApp->installTranslator(&m_translator);
            retranslateUi();
        }
    }
    
    void retranslateUi() {
        setWindowTitle(tr("Application"));
        m_saveBtn->setText(tr("Save"));
        m_openBtn->setText(tr("Open"));
        m_statusBar->showMessage(tr("Ready"));
    }
    
    QTranslator m_translator;
    QPushButton *m_saveBtn;
    QPushButton *m_openBtn;
    QStatusBar *m_statusBar;
};
```

Without calling `retranslateUi()`, widgets keep their old text despite translator being loaded.

## 10. What's the difference between accessibility and internationalization, and why are both important?

**Answer:** Accessibility serves users with disabilities; i18n serves users in different languages/regions. Both expand audience.

**Explanation:**

**Accessibility (a11y):**

- **Target**: Users with visual, auditory, motor, or cognitive disabilities
- **Features**: 
  - Screen reader support (accessibleName, accessibleDescription)
  - Keyboard navigation (no mouse required)
  - High contrast themes
  - Adjustable font sizes
  - Clear focus indicators
  - Consistent navigation

- **Example users**:
  - Blind users using NVDA/JAWS/VoiceOver
  - Motor-impaired users navigating via keyboard only
  - Color-blind users needing non-color cues
  - Low-vision users with screen magnification

**Internationalization (i18n):**

- **Target**: Users speaking different languages, in different regions
- **Features**:
  - Translatable strings (tr())
  - Locale-aware formatting (dates, numbers, currency)
  - RTL language support
  - Character encoding (Unicode)
  - Cultural considerations (colors, icons)

- **Example users**:
  - Spanish speaker in Mexico
  - Arabic speaker in Saudi Arabia (RTL)
  - German user expecting European date format
  - Japanese user expecting Yen currency

**Why both matter:**

**Accessibility** is often a **legal requirement**:
- US: Section 508, ADA
- EU: EN 301 549
- Web: WCAG 2.1 Level AA

**Internationalization** expands **market reach**:
- English speakers: ~20% of internet users
- Supporting Chinese, Spanish, Arabic, Hindi: Reaches billions more
- Regional apps must support local languages/customs

**They're complementary:**

- Accessible app in English only: Excludes non-English speakers
- Multi-language app without accessibility: Excludes disabled users
- Both together: Reaches maximum audience

**Qt makes both easy:**

```cpp
// Accessibility
btn->setAccessibleName(tr("Save"));  // Translatable accessible name!

// i18n
QString msg = tr("File saved successfully");  // Works for all users

// Both together
QLocale locale;
QString date = locale.toString(QDate::currentDate());
label->setText(tr("Today is %1").arg(date));
label->setAccessibleName(tr("Current date"));
```

By implementing both, you create an inclusive application that serves the widest possible audience, regardless of language, location, or ability.
