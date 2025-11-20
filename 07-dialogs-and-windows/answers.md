# Lesson 7 Quiz Answers: Dialogs and Windows

## 1. What is the key difference between modal and modeless dialogs? Provide a use case where each would be appropriate.

**Answer:** Modal dialogs block interaction with other windows until closed; modeless dialogs allow simultaneous interaction with multiple windows.

**Explanation:** Modal dialogs use `exec()` which blocks program execution until the dialog closes, preventing users from interacting with parent windows. This is appropriate for critical operations requiring immediate user decision (e.g., "Save changes before closing?"). Modeless dialogs use `show()` and don't block, allowing users to work with both the dialog and other windows simultaneously. This suits tools like Find/Replace panels where users want to search while editing, or floating toolbars that remain accessible while working on the main window.

**Modal use case:** Confirmation before deleting important data
**Modeless use case:** Find panel in a text editor that stays open while editing

## 2. What is wrong with this code, and how would you fix it?

```cpp
void MainWindow::showSettings() {
    QDialog dialog;
    dialog.setWindowTitle("Settings");
    dialog.show();
}
```

**Answer:** The dialog is created on the stack and will be destroyed when the function exits, causing it to disappear immediately.

**Explanation:** For modeless dialogs (using `show()`), you must allocate the dialog on the heap or as a member variable so it persists after the function returns. Additionally, the dialog has no parent, which can cause memory leaks and improper window relationship.

**Corrected code:**
```cpp
void MainWindow::showSettings() {
    QDialog *dialog = new QDialog(this);  // Heap allocation with parent
    dialog->setWindowTitle("Settings");
    dialog->setAttribute(Qt::WA_DeleteOnClose);  // Auto-cleanup
    dialog->show();
}
```

For modal dialogs, stack allocation is fine because `exec()` blocks:
```cpp
void MainWindow::showSettings() {
    QDialog dialog(this);  // Stack allocation OK for modal
    dialog.setWindowTitle("Settings");
    dialog.exec();  // Blocks until closed
}
```

## 3. You want to ask the user a yes/no question and take different actions based on their choice. Which dialog type should you use, and how do you check the user's response?

**Answer:** Use `QMessageBox::question()` and check the returned `StandardButton` value.

**Explanation:** `QMessageBox::question()` creates a modal dialog with customizable buttons and returns which button the user clicked. This is perfect for yes/no decisions.

**Code example:**
```cpp
QMessageBox::StandardButton reply = QMessageBox::question(
    this,
    "Confirm Delete",
    "Are you sure you want to delete this file?",
    QMessageBox::Yes | QMessageBox::No
);

if (reply == QMessageBox::Yes) {
    deleteFile();
    qDebug() << "File deleted";
} else {
    qDebug() << "Deletion cancelled";
}
```

You can also use multiple button combinations like `Yes | No | Cancel` for three-way decisions, and check for each possibility with if-else or switch statements.

## 4. Given this code, what happens when the user clicks the Cancel button in the file dialog?

```cpp
QString fileName = QFileDialog::getOpenFileName(
    this, "Open", QDir::homePath(), "*.txt"
);
QFile file(fileName);
file.open(QIODevice::ReadOnly);
```

**Answer:** `getOpenFileName()` returns an empty string, causing `QFile` to attempt opening a file with an empty path, which fails.

**Explanation:** When users cancel file dialogs, Qt returns an empty string. The code doesn't check for this, so it tries to open a file with an empty name. While `open()` will fail safely, the code doesn't handle the error, potentially causing issues downstream.

**Corrected code:**
```cpp
QString fileName = QFileDialog::getOpenFileName(
    this, "Open", QDir::homePath(), "Text Files (*.txt)"
);

if (!fileName.isEmpty()) {  // Check for cancellation
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        // Successfully opened
        // ... read file
    } else {
        QMessageBox::warning(this, "Error", "Could not open file");
    }
} else {
    qDebug() << "User cancelled file selection";
}
```

Always check dialog return values before using them.

## 5. What is the purpose of `QDialogButtonBox`, and what advantage does it provide over manually creating OK/Cancel buttons?

**Answer:** `QDialogButtonBox` provides platform-native button ordering and standard button connections for dialogs.

**Explanation:** Different platforms have different conventions for button ordering (e.g., Windows uses OK/Cancel, macOS uses Cancel/OK). `QDialogButtonBox` automatically arranges buttons according to the current platform's style guidelines, ensuring native look and feel. It also provides standard buttons with pre-configured text, icons, and roles.

**Advantages:**
1. **Platform consistency**: Correct button order for each OS automatically
2. **Standard signals**: `accepted()` and `rejected()` signals for common patterns
3. **Built-in buttons**: Standard buttons (Ok, Cancel, Yes, No, Apply, etc.) with proper text and roles
4. **Less code**: Simpler than creating and positioning individual buttons

**Example:**
```cpp
QDialogButtonBox *buttonBox = new QDialogButtonBox(
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
    this
);
connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
```

This is much simpler and more maintainable than manual button creation, and it automatically adapts to platform conventions.

## 6. You create a custom dialog but forget to connect the button box signals. What happens when the user clicks OK or Cancel?

**Answer:** The buttons do nothing—the dialog remains open and unresponsive to button clicks.

**Explanation:** `QDialogButtonBox` emits `accepted()` and `rejected()` signals when standard buttons are clicked, but these signals must be connected to `QDialog::accept()` and `QDialog::reject()` slots to actually close the dialog. Without these connections, clicking buttons has no effect.

**Required connections:**
```cpp
QDialogButtonBox *buttonBox = new QDialogButtonBox(
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
    this
);

// These connections are essential:
connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
```

`accept()` sets the dialog result to `QDialog::Accepted` and closes it.
`reject()` sets the result to `QDialog::Rejected` and closes it.

These result codes are what you check when calling `exec()`:
```cpp
if (dialog.exec() == QDialog::Accepted) {
    // User clicked OK
}
```

## 7. In a custom dialog, you want to disable the OK button until the user fills in all required fields. How would you implement this validation pattern?

**Answer:** Connect field change signals to a validation slot that enables/disables the OK button based on input completeness.

**Explanation:** Store a reference to the OK button and connect all input widgets' change signals to a validation function that checks whether all required fields are filled, enabling the OK button only when valid.

**Implementation:**
```cpp
class FormDialog : public QDialog {
    Q_OBJECT
public:
    FormDialog(QWidget *parent = nullptr) : QDialog(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        
        nameEdit = new QLineEdit(this);
        emailEdit = new QLineEdit(this);
        layout->addWidget(new QLabel("Name:"));
        layout->addWidget(nameEdit);
        layout->addWidget(new QLabel("Email:"));
        layout->addWidget(emailEdit);
        
        QDialogButtonBox *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel
        );
        okButton = buttonBox->button(QDialogButtonBox::Ok);
        layout->addWidget(buttonBox);
        
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        
        // Connect input changes to validation
        connect(nameEdit, &QLineEdit::textChanged, this, &FormDialog::validate);
        connect(emailEdit, &QLineEdit::textChanged, this, &FormDialog::validate);
        
        validate();  // Initial validation
    }

private slots:
    void validate() {
        bool isValid = !nameEdit->text().isEmpty() &&
                       !emailEdit->text().isEmpty() &&
                       emailEdit->text().contains('@');
        okButton->setEnabled(isValid);
    }

private:
    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
    QPushButton *okButton;
};
```

This pattern ensures users cannot submit incomplete or invalid forms, providing immediate feedback and preventing errors.

## 8. Spot the potential issue in this custom dialog:

```cpp
class MyDialog : public QDialog {
public:
    MyDialog() : QDialog() {  // No parent
        QLineEdit *edit = new QLineEdit(this);
        // ... more setup
    }
    QString getValue() const { return edit->text(); }
private:
    QLineEdit *edit;  // Uninitialized in usage
};
```

**Answer:** The member variable `edit` is not initialized; a local variable shadows it in the constructor.

**Explanation:** In the constructor, `QLineEdit *edit` creates a new local variable that shadows the member variable. The member `edit` remains uninitialized (nullptr), so calling `getValue()` will crash when trying to access `edit->text()`.

**Corrected code:**
```cpp
class MyDialog : public QDialog {
public:
    MyDialog(QWidget *parent = nullptr) : QDialog(parent) {
        edit = new QLineEdit(this);  // Initialize member, not local
        // or: m_edit = new QLineEdit(this);  // Use m_ prefix to avoid confusion
        
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(edit);
        
        QDialogButtonBox *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel
        );
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttonBox);
    }
    
    QString getValue() const { return edit->text(); }

private:
    QLineEdit *edit = nullptr;  // Initialize to nullptr for safety
};
```

Additional improvements:
1. Accept a parent parameter for proper ownership
2. Don't redeclare `edit` in the constructor
3. Initialize pointer members to `nullptr`
4. Consider using `m_` prefix for members to avoid shadowing

## 9. What does `QColorDialog::getColor()` return if the user cancels the dialog, and how should you check for this case?

**Answer:** It returns an invalid `QColor` object, which you check using `isValid()`.

**Explanation:** When users cancel the color dialog, Qt returns a `QColor` object with `isValid()` returning `false`. You must check validity before using the color to avoid applying invalid color values.

**Correct usage:**
```cpp
QColor color = QColorDialog::getColor(
    Qt::white,        // Initial color
    this,             // Parent widget
    "Choose Color"    // Dialog title
);

if (color.isValid()) {
    // User selected a color
    widget->setStyleSheet(
        QString("background-color: %1").arg(color.name())
    );
    qDebug() << "Selected color:" << color.name();
} else {
    // User cancelled
    qDebug() << "Color selection cancelled";
}
```

This pattern applies to other dialogs too:
- `QFileDialog::getOpenFileName()` returns empty string when cancelled
- `QInputDialog::getText()` uses an `ok` boolean parameter
- `QDialog::exec()` returns `Accepted` or `Rejected`

Always validate dialog return values before using them.

## 10. Reflection: You're building a text editor application. List three different dialog types you would use and explain when each would appear (e.g., on which user action).

**Answer:** QFileDialog (File→Open/Save), QMessageBox (unsaved changes warning), QInputDialog (Go to Line feature).

**Explanation:** A text editor needs multiple dialog types for different interactions:

**1. QFileDialog - File operations**
- **When**: User selects File→Open or File→Save As
- **Why**: Standard file browser for selecting documents
```cpp
QString fileName = QFileDialog::getOpenFileName(
    this, "Open File", QDir::homePath(),
    "Text Files (*.txt);;All Files (*)"
);
```

**2. QMessageBox - Confirmations and warnings**
- **When**: User tries to close with unsaved changes
- **Why**: Prevent data loss with clear options
```cpp
QMessageBox::StandardButton reply = QMessageBox::question(
    this, "Unsaved Changes",
    "Save changes before closing?",
    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
);
```

**3. QInputDialog - Quick input**
- **When**: User selects Edit→Go to Line
- **Why**: Simple numeric input without complex form
```cpp
bool ok;
int line = QInputDialog::getInt(
    this, "Go to Line", "Line number:",
    1, 1, document()->lineCount(), 1, &ok
);
if (ok) moveCursorToLine(line);
```

**Additional dialogs for a complete editor:**

**4. Custom FindReplaceDialog - Modeless search**
- **When**: User presses Ctrl+F
- **Why**: Needs to stay open while editing
- Implements as modeless dialog with find next/previous buttons

**5. Custom PreferencesDialog - Settings**
- **When**: User selects Edit→Preferences
- **Why**: Multiple settings require custom form layout
- Tab widget with font, colors, behavior settings

**6. QMessageBox::critical() - Error handling**
- **When**: File save fails due to permissions or disk space
- **Why**: Clear error communication with icon and message

Each dialog type serves a specific purpose, with modal dialogs for critical decisions and modeless for ongoing interactions.
