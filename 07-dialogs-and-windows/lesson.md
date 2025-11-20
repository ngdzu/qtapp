# Lesson 7: Dialogs and Windows

## Learning Goals

- Master Qt's standard dialog classes for common tasks
- Understand modal vs. modeless dialog behavior
- Learn to use QFileDialog, QMessageBox, QInputDialog, and QColorDialog
- Create custom dialogs by subclassing QDialog
- Handle dialog results and return values
- Implement parent-child relationships for proper dialog ownership

## Introduction

Dialogs are essential UI elements for user interaction in Qt applications. Qt provides a rich set of standard dialogs for common tasks like file selection, displaying messages, gathering input, and choosing colors. Understanding how to use these dialogs effectively, along with creating custom dialogs when needed, is fundamental to building professional desktop applications.

Dialogs can be modal (blocking the parent window) or modeless (allowing interaction with other windows), each serving different use cases.

## Modal vs. Modeless Dialogs

**Modal dialogs** block interaction with other windows until closed:
```cpp
QMessageBox msgBox;
msgBox.setText("This is modal - you must close it first");
msgBox.exec();  // Blocks until dialog closes
qDebug() << "Dialog closed";
```

**Modeless dialogs** allow interaction with other windows:
```cpp
QDialog *dialog = new QDialog(parent);
dialog->setWindowTitle("Modeless Dialog");
dialog->show();  // Returns immediately
// User can interact with both dialog and parent window
```

Modal dialogs use `exec()`, which blocks until the dialog closes and returns a result code (Accepted or Rejected). Modeless dialogs use `show()` and require signals/slots for communication.

## QMessageBox - Standard Messages

`QMessageBox` displays information, warnings, errors, and questions:

```cpp
// Information message
QMessageBox::information(this, "Success", "File saved successfully!");

// Warning with custom buttons
QMessageBox::StandardButton reply = QMessageBox::question(
    this, "Confirm", "Delete this file?",
    QMessageBox::Yes | QMessageBox::No
);
if (reply == QMessageBox::Yes) {
    // Delete the file
}

// Critical error
QMessageBox::critical(this, "Error", "Failed to open database!");
```

Message boxes support standard buttons (OK, Cancel, Yes, No, etc.) and custom button text. They're perfect for simple user notifications and confirmations.

## QFileDialog - File Selection

`QFileDialog` provides file and directory selection:

```cpp
// Open single file
QString fileName = QFileDialog::getOpenFileName(
    this,
    "Open File",
    QDir::homePath(),
    "Text Files (*.txt);;All Files (*)"
);

// Save file with suggested name
QString saveFile = QFileDialog::getSaveFileName(
    this,
    "Save File",
    "untitled.txt",
    "Text Files (*.txt)"
);

// Select directory
QString dir = QFileDialog::getExistingDirectory(
    this,
    "Select Directory",
    QDir::homePath()
);
```

File dialogs support file filters, multiple selection, native vs. Qt dialogs, and starting directories.

## QInputDialog - Simple Input

`QInputDialog` gathers single input values:

```cpp
// Text input
QString name = QInputDialog::getText(
    this, "Input", "Enter your name:"
);

// Integer input with range
int age = QInputDialog::getInt(
    this, "Input", "Enter age:", 25, 0, 120
);

// Choice from list
QStringList items = {"Red", "Green", "Blue"};
QString color = QInputDialog::getItem(
    this, "Select", "Choose color:", items
);
```

Input dialogs are ideal for quick, single-value inputs without creating custom forms.

## QColorDialog - Color Selection

`QColorDialog` provides a color picker:

```cpp
QColor color = QColorDialog::getColor(
    Qt::white,
    this,
    "Select Background Color"
);

if (color.isValid()) {
    widget->setStyleSheet(
        QString("background-color: %1").arg(color.name())
    );
}
```

The color dialog includes RGB, HSV, and custom color selection with preview.

## Custom Dialogs

For complex interactions, create custom dialogs by subclassing `QDialog`:

```cpp
class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    LoginDialog(QWidget *parent = nullptr) : QDialog(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        
        usernameEdit = new QLineEdit(this);
        passwordEdit = new QLineEdit(this);
        passwordEdit->setEchoMode(QLineEdit::Password);
        
        layout->addWidget(new QLabel("Username:"));
        layout->addWidget(usernameEdit);
        layout->addWidget(new QLabel("Password:"));
        layout->addWidget(passwordEdit);
        
        QDialogButtonBox *buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel
        );
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttons);
    }
    
    QString username() const { return usernameEdit->text(); }
    QString password() const { return passwordEdit->text(); }

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
};

// Usage:
LoginDialog dialog(this);
if (dialog.exec() == QDialog::Accepted) {
    QString user = dialog.username();
    QString pass = dialog.password();
}
```

Custom dialogs provide full control over layout, validation, and behavior.

## Dialog Best Practices

1. **Parent ownership**: Always pass a parent widget to prevent memory leaks
2. **Result handling**: Check return values (`exec()` result, `isValid()`, etc.)
3. **User experience**: Use appropriate dialog types (modal for critical actions)
4. **Button boxes**: Use `QDialogButtonBox` for consistent button layout
5. **Validation**: Disable OK button until input is valid

## Example Walkthrough

Our example creates a main window with buttons to demonstrate:
- QFileDialog for opening and saving files
- QMessageBox for information, warnings, and questions
- QInputDialog for text and integer input
- QColorDialog for background color selection
- Custom dialog for structured data input

Each dialog displays its result in the main window, showing how to handle return values and user choices.

## Expected Output

A window with buttons for each dialog type. Clicking buttons opens:
- **Open File**: Native file browser showing text files
- **Save File**: Save dialog with suggested filename
- **Message Box**: Information/warning/question dialogs
- **Input Dialog**: Text and number input prompts
- **Color Dialog**: Color picker that changes window background
- **Custom Dialog**: Login form with username/password fields

Results are displayed in a text area in the main window.

## Try It

1. Build and run the application
2. Test each dialog type and observe its behavior
3. Try canceling dialogs and see how the app handles it
4. Experiment with file filters in QFileDialog
5. Create a custom dialog for a settings form with multiple fields
6. Implement form validation in a custom dialog (disable OK until valid)
7. Create a modeless dialog that updates the main window in real-time

## Key Takeaways

- Qt provides standard dialogs for common tasks (files, messages, input, colors)
- Modal dialogs (`exec()`) block until closed; modeless (`show()`) don't block
- QMessageBox supports multiple message types and custom buttons
- QFileDialog handles file/directory selection with filters
- QInputDialog is perfect for simple, single-value inputs
- Custom dialogs (subclass QDialog) provide full control for complex interactions
- Always check dialog return values and handle cancellation
- Use QDialogButtonBox for standard accept/reject buttons
- Parent-child relationships prevent memory leaks and provide proper ownership
