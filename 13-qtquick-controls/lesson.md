# Lesson 13: Qt Quick Controls

## Learning Goals
- Understand Qt Quick Controls 2 component library
- Build interactive UIs with buttons, text fields, and combo boxes
- Master control properties and styling
- Handle user input in QML applications

## Introduction

Qt Quick Controls 2 provides a set of UI controls for building modern, fluid user interfaces. Unlike Qt Widgets, these controls are designed specifically for touch interfaces and modern design patterns, though they work equally well with mouse and keyboard input.

The Controls module includes buttons, text inputs, sliders, dialogs, and more—all styled consistently and optimized for performance.

## Key Concepts

### Control Basics

Qt Quick Controls inherit from `Control`, which provides:
- **Material Design** and **Universal** styling out of the box
- Responsive touch and mouse handling
- Automatic layout and sizing
- Theme integration

Common controls include:
- `Button` - Clickable button with text/icon
- `TextField` - Single-line text input
- `ComboBox` - Dropdown selection
- `CheckBox`, `RadioButton` - Selection controls
- `Slider`, `SpinBox` - Value input

### Importing Controls

```qml
import QtQuick
import QtQuick.Controls
```

### Button Example

```qml
Button {
    text: "Click Me"
    onClicked: console.log("Button clicked!")
}
```

### TextField with Validation

```qml
TextField {
    id: nameField
    placeholderText: "Enter your name"
    validator: RegularExpressionValidator {
        regularExpression: /[A-Za-z ]+/
    }
}
```

## Example Walkthrough

Our example creates a simple form with:
1. A `TextField` for user input
2. A `ComboBox` for selecting options
3. A `Button` to submit
4. A `Label` to display results

The QML file uses `ColumnLayout` for vertical arrangement and demonstrates property binding to update the result label when the button is clicked.

## Expected Output

You'll see a window with:
- A text field labeled "Name:"
- A combo box for selecting a color
- A submit button
- A result label that updates when you click submit

The interface demonstrates modern Material Design styling with smooth interactions.

## Try It

1. Build and run the application
2. Enter text in the TextField
3. Select different options from the ComboBox
4. Click the button and observe the result label update
5. Try typing invalid characters (numbers) in the name field—they'll be rejected by the validator

## Key Takeaways

- Qt Quick Controls 2 provides modern, touch-friendly UI components
- Controls use declarative syntax and property bindings
- Validators can enforce input constraints
- Signal handlers like `onClicked` respond to user interaction
- Material and Universal styles provide consistent theming
