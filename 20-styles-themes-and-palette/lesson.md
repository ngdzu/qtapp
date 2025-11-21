# Lesson 20: Styles, Themes, and Palette

## Learning Goals
- Understand Qt's styling system
- Use QPalette to customize colors
- Apply Qt Style Sheets (QSS)
- Work with QStyle and built-in styles
- Create custom application themes
- Switch between light and dark modes

## Introduction

Qt provides multiple ways to customize application appearance: **QPalette** for color schemes, **Qt Style Sheets (QSS)** for CSS-like styling, and **QStyle** for platform-native looks. These tools allow you to create beautiful, consistent UIs that match your brand or user preferences.

QPalette controls widget colors at a system level, QSS provides fine-grained control similar to web CSS, and QStyle determines the overall look (Windows, Fusion, macOS native, etc.).

## Key Concepts

### QPalette Basics

Change application colors:

```cpp
QPalette palette;
palette.setColor(QPalette::Window, QColor(53, 53, 53));
palette.setColor(QPalette::WindowText, Qt::white);
palette.setColor(QPalette::Base, QColor(25, 25, 25));
palette.setColor(QPalette::Button, QColor(53, 53, 53));
palette.setColor(QPalette::ButtonText, Qt::white);

QApplication::setPalette(palette);
```

### Color Roles

QPalette has different roles for different widget parts:
- **Window** - Background of widgets
- **WindowText** - Foreground text
- **Base** - Background of text entry widgets
- **Text** - Text in entry widgets
- **Button** - Button background
- **ButtonText** - Button text
- **Highlight** - Selected item background
- **HighlightedText** - Selected item text

### Qt Style Sheets (QSS)

CSS-like styling:

```cpp
QPushButton *button = new QPushButton("Click Me");
button->setStyleSheet(R"(
    QPushButton {
        background-color: #4CAF50;
        color: white;
        border: none;
        padding: 10px 20px;
        border-radius: 5px;
    }
    QPushButton:hover {
        background-color: #45a049;
    }
    QPushButton:pressed {
        background-color: #3d8b40;
    }
)");
```

### Application-Wide Styles

Apply styles to entire application:

```cpp
qApp->setStyleSheet(R"(
    QWidget {
        font-family: "Segoe UI";
        font-size: 14px;
    }
    QPushButton {
        background-color: #2196F3;
        color: white;
        padding: 8px 16px;
        border-radius: 4px;
    }
)");
```

### QStyle and Built-in Styles

Switch between platform styles:

```cpp
// Available styles: "Windows", "Fusion", "macOS", etc.
QApplication::setStyle("Fusion");

// Get available styles
QStringList styles = QStyleFactory::keys();
```

### Dark Theme Example

Complete dark theme using QPalette:

```cpp
QPalette darkPalette;
darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
darkPalette.setColor(QPalette::WindowText, Qt::white);
darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
darkPalette.setColor(QPalette::ToolTipText, Qt::white);
darkPalette.setColor(QPalette::Text, Qt::white);
darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
darkPalette.setColor(QPalette::ButtonText, Qt::white);
darkPalette.setColor(QPalette::BrightText, Qt::red);
darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
darkPalette.setColor(QPalette::HighlightedText, Qt::black);

QApplication::setPalette(darkPalette);
QApplication::setStyle("Fusion");
```

## Example Walkthrough

Our example creates a theme switcher demonstrating:

1. **Light Theme** - Clean, bright interface
2. **Dark Theme** - Modern dark mode
3. **Custom QSS** - CSS-like button styling
4. **Dynamic Switching** - Toggle themes at runtime

The application shows how different styling approaches work together.

## Expected Output

A window with:
- Theme selector buttons (Light/Dark/Custom)
- Sample widgets showing the current theme
- Styled buttons demonstrating QSS
- Real-time theme switching

## Try It

1. Build and run the application
2. Click "Light Theme" to see bright colors
3. Click "Dark Theme" for dark mode
4. Click "Custom QSS" to see CSS-like styling
5. Notice how all widgets update instantly
6. Observe the different color roles in action

## Key Takeaways

- **QPalette** controls system-wide colors and color roles
- **QSS** provides CSS-like styling with selectors and pseudo-states
- **QStyle** determines the platform look (Windows, Fusion, etc.)
- Combine QPalette and QStyle for complete dark themes
- QSS supports hover, pressed, disabled, and other states
- Use `R"()"` raw strings for multi-line QSS
- Application-wide styles with `qApp->setStyleSheet()`
- Fusion style works best for custom color schemes
- Always test themes with all widget types
