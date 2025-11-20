# Lesson 7 Quiz: Dialogs and Windows

1. What is the key difference between modal and modeless dialogs? Provide a use case where each would be appropriate.

2. What is wrong with this code, and how would you fix it?
```cpp
void MainWindow::showSettings() {
    QDialog dialog;
    dialog.setWindowTitle("Settings");
    dialog.show();
}
```

3. You want to ask the user a yes/no question and take different actions based on their choice. Which dialog type should you use, and how do you check the user's response?

4. Given this code, what happens when the user clicks the Cancel button in the file dialog?
```cpp
QString fileName = QFileDialog::getOpenFileName(
    this, "Open", QDir::homePath(), "*.txt"
);
QFile file(fileName);
file.open(QIODevice::ReadOnly);
```

5. What is the purpose of `QDialogButtonBox`, and what advantage does it provide over manually creating OK/Cancel buttons?

6. You create a custom dialog but forget to connect the button box signals. What happens when the user clicks OK or Cancel?

7. In a custom dialog, you want to disable the OK button until the user fills in all required fields. How would you implement this validation pattern?

8. Spot the potential issue in this custom dialog:
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

9. What does `QColorDialog::getColor()` return if the user cancels the dialog, and how should you check for this case?

10. Reflection: You're building a text editor application. List three different dialog types you would use and explain when each would appear (e.g., on which user action).
