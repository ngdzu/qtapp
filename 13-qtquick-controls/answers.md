# Lesson 13 Quiz Answers

1. **What is the difference between Qt Quick Controls and Qt Widgets?**

Qt Quick Controls are QML-based, declarative, and optimized for touch/modern UIs. Qt Widgets are C++-based, imperative, and designed for traditional desktop applications.

Qt Quick Controls use a scene graph for rendering (GPU-accelerated), while Widgets use raster painting. Controls are better for mobile and embedded, Widgets for complex desktop tools.

2. **Which import statement is required to use Button, TextField, and ComboBox in QML?**

`import QtQuick.Controls`

This import provides access to all Qt Quick Controls 2 components. You also need `import QtQuick` for basic QML types.

3. **What does this code do?**

It creates a TextField that only accepts numeric input.

The RegularExpressionValidator with `/[0-9]+/` allows only digits (0-9). Any letters or special characters will be rejected when the user types.

4. **How do you handle a button click in QML?**

Use the `onClicked` signal handler:
```qml
Button {
    text: "Click Me"
    onClicked: {
        console.log("Button was clicked")
    }
}
```

5. **What property binding updates `resultLabel.text` when `nameField.text` changes?**

Direct property binding:
```qml
Label {
    id: resultLabel
    text: "Hello, " + nameField.text
}
```
The binding automatically updates whenever `nameField.text` changes.

6. **Why use `ApplicationWindow` instead of `Window` for Qt Quick Controls apps?**

`ApplicationWindow` provides built-in support for menubar, toolbar, header, and footer.

It's optimized for Controls 2 styling and layout, ensuring consistent behavior across platforms. `Window` is more basic.

7. **What's the purpose of `Layout.fillWidth: true` in a ColumnLayout?**

It makes the item expand horizontally to fill available space in the layout.

Without it, the item uses its implicit width. This is essential for creating responsive UIs that adapt to window size.

8. **How do you access the currently selected text in a ComboBox?**

Use `comboBox.currentText`:
```qml
ComboBox {
    id: myCombo
    model: ["Option 1", "Option 2"]
}
// Access: myCombo.currentText
```

9. **What happens if you don't set `visible: true` on ApplicationWindow?**

The window won't appear on screen.

Unlike Qt Widgets (where `show()` makes windows visible), QML windows require `visible: true` to be displayed.

10. **Name three advantages of Qt Quick Controls 2 over Controls 1.**

(1) Better performance - uses Qt Quick scene graph efficiently
(2) Material Design and Universal styles built-in
(3) Smaller footprint and faster startup times
