# Lesson 20 Quiz Answers

1. **What is the difference between QPalette and Qt Style Sheets (QSS)?**

QPalette controls system-level colors using predefined roles. QSS provides CSS-like fine-grained control over individual widget properties.

QPalette is faster, respects platform conventions, and works with all Qt styles. QSS is more powerful but can override platform appearance. Use QPalette for themes, QSS for specific customizations.

2. **How do you apply a dark theme using QPalette?**

Set dark colors for all palette roles:
```cpp
QPalette darkPalette;
darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
darkPalette.setColor(QPalette::WindowText, Qt::white);
darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
darkPalette.setColor(QPalette::Text, Qt::white);
QApplication::setPalette(darkPalette);
QApplication::setStyle("Fusion");
```

3. **What does this QSS code do?**

Changes button background color when the mouse hovers over it.

The `:hover` pseudo-state selector applies styles only when the mouse is over the button. This creates visual feedback for user interaction.

4. **What are QPalette color roles? Name three.**

Color roles define colors for different widget parts:
- **Window** - Widget background
- **WindowText** - Text on windows
- **Button** - Button background
- **ButtonText** - Button text
- **Highlight** - Selected item background
- **Base** - Text entry widget background

5. **How do you set an application-wide style sheet?**

Use `setStyleSheet()` on QApplication:
```cpp
qApp->setStyleSheet(R"(
    QPushButton {
        background-color: blue;
        color: white;
    }
)");
```
This affects all widgets in the application.

6. **What is the "Fusion" style and when should you use it?**

Fusion is Qt's cross-platform style that looks consistent across all operating systems.

Use it when applying custom color schemes (especially dark themes) because it respects QPalette colors well. Native styles may ignore custom palettes.

7. **How do you change a widget's style sheet at runtime?**

Call `setStyleSheet()` with new CSS:
```cpp
button->setStyleSheet("background-color: red;");
```
The widget updates immediately - no restart needed.

8. **What's the advantage of using QPalette over QSS for theming?**

QPalette is more performant and maintains platform native behavior.

It affects all widgets automatically without specifying each type. QSS can be brittle and may not cover all widgets. QPalette also works with all QStyle implementations.

9. **How do you style pseudo-states in QSS (hover, pressed, etc.)?**

Use selectors with colons:
```cpp
QPushButton:hover { background: blue; }
QPushButton:pressed { background: darkblue; }
QPushButton:disabled { color: gray; }
QPushButton:checked { background: green; }
```

10. **What does `R"()"` mean in C++ when writing style sheets?**

Raw string literal - allows multi-line strings without escape characters.

`R"(text)"` preserves newlines, quotes, and backslashes literally. Perfect for CSS-like QSS that spans multiple lines. Without it, you'd need `\n` and `\"` everywhere.
