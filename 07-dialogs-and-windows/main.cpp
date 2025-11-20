#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QColorDialog>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QDir>
#include <QDebug>

// Custom dialog for demonstrating dialog creation
class CustomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Custom Login Dialog");
        setModal(true);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // Username field
        layout->addWidget(new QLabel("Username:"));
        m_usernameEdit = new QLineEdit(this);
        m_usernameEdit->setPlaceholderText("Enter username");
        layout->addWidget(m_usernameEdit);

        // Password field
        layout->addWidget(new QLabel("Password:"));
        m_passwordEdit = new QLineEdit(this);
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_passwordEdit->setPlaceholderText("Enter password");
        layout->addWidget(m_passwordEdit);

        // Button box with OK and Cancel
        QDialogButtonBox *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            this);
        layout->addWidget(buttonBox);

        // Connect button box signals to dialog slots
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        // Connect text changes to validation
        connect(m_usernameEdit, &QLineEdit::textChanged,
                this, &CustomDialog::validateInput);
        connect(m_passwordEdit, &QLineEdit::textChanged,
                this, &CustomDialog::validateInput);

        // Store button reference for validation
        m_okButton = buttonBox->button(QDialogButtonBox::Ok);
        validateInput();
    }

    QString username() const { return m_usernameEdit->text(); }
    QString password() const { return m_passwordEdit->text(); }

private slots:
    void validateInput()
    {
        // Enable OK button only when both fields have text
        bool valid = !m_usernameEdit->text().isEmpty() &&
                     !m_passwordEdit->text().isEmpty();
        m_okButton->setEnabled(valid);
    }

private:
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_okButton;
};

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setWindowTitle("Lesson 7: Dialogs and Windows");
        resize(600, 500);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Instructions
        QLabel *instructions = new QLabel(
            "Click the buttons below to explore different Qt dialog types.\n"
            "Results will be displayed in the text area.");
        instructions->setWordWrap(true);
        mainLayout->addWidget(instructions);

        // Output text area
        m_output = new QTextEdit(this);
        m_output->setReadOnly(true);
        mainLayout->addWidget(m_output);

        // Button layouts
        QHBoxLayout *row1 = new QHBoxLayout();
        QHBoxLayout *row2 = new QHBoxLayout();
        QHBoxLayout *row3 = new QHBoxLayout();

        // File dialog buttons
        QPushButton *openFileBtn = new QPushButton("Open File", this);
        QPushButton *saveFileBtn = new QPushButton("Save File", this);
        row1->addWidget(openFileBtn);
        row1->addWidget(saveFileBtn);

        // Message box buttons
        QPushButton *infoBtn = new QPushButton("Info Message", this);
        QPushButton *warningBtn = new QPushButton("Warning", this);
        QPushButton *questionBtn = new QPushButton("Question", this);
        row2->addWidget(infoBtn);
        row2->addWidget(warningBtn);
        row2->addWidget(questionBtn);

        // Input and color dialog buttons
        QPushButton *textInputBtn = new QPushButton("Text Input", this);
        QPushButton *numberInputBtn = new QPushButton("Number Input", this);
        QPushButton *colorBtn = new QPushButton("Color Picker", this);
        QPushButton *customBtn = new QPushButton("Custom Dialog", this);
        row3->addWidget(textInputBtn);
        row3->addWidget(numberInputBtn);
        row3->addWidget(colorBtn);
        row3->addWidget(customBtn);

        mainLayout->addLayout(row1);
        mainLayout->addLayout(row2);
        mainLayout->addLayout(row3);

        // Connect signals
        connect(openFileBtn, &QPushButton::clicked, this, &MainWindow::openFile);
        connect(saveFileBtn, &QPushButton::clicked, this, &MainWindow::saveFile);
        connect(infoBtn, &QPushButton::clicked, this, &MainWindow::showInfo);
        connect(warningBtn, &QPushButton::clicked, this, &MainWindow::showWarning);
        connect(questionBtn, &QPushButton::clicked, this, &MainWindow::showQuestion);
        connect(textInputBtn, &QPushButton::clicked, this, &MainWindow::getTextInput);
        connect(numberInputBtn, &QPushButton::clicked, this, &MainWindow::getNumberInput);
        connect(colorBtn, &QPushButton::clicked, this, &MainWindow::pickColor);
        connect(customBtn, &QPushButton::clicked, this, &MainWindow::showCustomDialog);

        log("Application started. Try the dialog buttons!");
    }

private slots:
    void openFile()
    {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "Open File",
            QDir::homePath(),
            "Text Files (*.txt);;All Files (*)");

        if (!fileName.isEmpty())
        {
            log(QString("Selected file to open: %1").arg(fileName));
        }
        else
        {
            log("File open dialog cancelled");
        }
    }

    void saveFile()
    {
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Save File",
            QDir::homePath() + "/untitled.txt",
            "Text Files (*.txt);;All Files (*)");

        if (!fileName.isEmpty())
        {
            log(QString("Selected file to save: %1").arg(fileName));
        }
        else
        {
            log("File save dialog cancelled");
        }
    }

    void showInfo()
    {
        QMessageBox::information(
            this,
            "Information",
            "This is an information message.\n\n"
            "Use this for general notifications to the user.");
        log("Showed information message");
    }

    void showWarning()
    {
        QMessageBox::warning(
            this,
            "Warning",
            "This is a warning message.\n\n"
            "Use this to alert users about potential issues.");
        log("Showed warning message");
    }

    void showQuestion()
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Confirmation",
            "Do you want to proceed with this action?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            log("User clicked Yes in question dialog");
        }
        else
        {
            log("User clicked No in question dialog");
        }
    }

    void getTextInput()
    {
        bool ok;
        QString text = QInputDialog::getText(
            this,
            "Text Input",
            "Enter your name:",
            QLineEdit::Normal,
            "",
            &ok);

        if (ok && !text.isEmpty())
        {
            log(QString("User entered: %1").arg(text));
        }
        else
        {
            log("Text input dialog cancelled");
        }
    }

    void getNumberInput()
    {
        bool ok;
        int number = QInputDialog::getInt(
            this,
            "Number Input",
            "Enter your age:",
            25,  // default value
            0,   // minimum
            120, // maximum
            1,   // step
            &ok);

        if (ok)
        {
            log(QString("User entered age: %1").arg(number));
        }
        else
        {
            log("Number input dialog cancelled");
        }
    }

    void pickColor()
    {
        QColor color = QColorDialog::getColor(
            m_currentColor,
            this,
            "Select Background Color");

        if (color.isValid())
        {
            m_currentColor = color;
            setStyleSheet(QString("background-color: %1").arg(color.name()));
            log(QString("Background color changed to: %1").arg(color.name()));
        }
        else
        {
            log("Color dialog cancelled");
        }
    }

    void showCustomDialog()
    {
        CustomDialog dialog(this);

        if (dialog.exec() == QDialog::Accepted)
        {
            log(QString("Login accepted - Username: %1, Password: %2")
                    .arg(dialog.username())
                    .arg(QString("*").repeated(dialog.password().length())));
        }
        else
        {
            log("Custom dialog cancelled");
        }
    }

private:
    void log(const QString &message)
    {
        m_output->append(QString("→ %1").arg(message));
    }

    QTextEdit *m_output;
    QColor m_currentColor = Qt::white;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
