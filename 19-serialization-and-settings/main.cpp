#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set organization and app name for QSettings
    QCoreApplication::setOrganizationName("QtLearning");
    QCoreApplication::setApplicationName("Lesson19");

    QWidget window;
    window.setWindowTitle("Lesson 19: Serialization and Settings");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Title
    QLabel *titleLabel = new QLabel("JSON & Settings Demo");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Input form
    QFormLayout *formLayout = new QFormLayout();
    
    QLineEdit *nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Enter name");
    formLayout->addRow("Name:", nameEdit);
    
    QLineEdit *ageEdit = new QLineEdit();
    ageEdit->setPlaceholderText("Enter age");
    formLayout->addRow("Age:", ageEdit);
    
    QLineEdit *emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("Enter email");
    formLayout->addRow("Email:", emailEdit);
    
    mainLayout->addLayout(formLayout);

    // JSON display
    QTextEdit *jsonDisplay = new QTextEdit();
    jsonDisplay->setPlaceholderText("JSON output will appear here...");
    jsonDisplay->setMaximumHeight(200);
    mainLayout->addWidget(jsonDisplay);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *generateBtn = new QPushButton("Generate JSON");
    QPushButton *parseBtn = new QPushButton("Parse JSON");
    QPushButton *saveSettingsBtn = new QPushButton("Save Settings");
    QPushButton *loadSettingsBtn = new QPushButton("Load Settings");
    
    buttonLayout->addWidget(generateBtn);
    buttonLayout->addWidget(parseBtn);
    buttonLayout->addWidget(saveSettingsBtn);
    buttonLayout->addWidget(loadSettingsBtn);
    
    mainLayout->addLayout(buttonLayout);

    // Info label
    QLabel *infoLabel = new QLabel("Settings persist between app runs!");
    infoLabel->setStyleSheet("color: #666; font-size: 12px; margin-top: 10px;");
    infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);

    // Generate JSON button
    QObject::connect(generateBtn, &QPushButton::clicked, [&]() {
        QJsonObject person;
        person["name"] = nameEdit->text();
        person["age"] = ageEdit->text().toInt();
        person["email"] = emailEdit->text();
        person["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        // Create array with single user
        QJsonArray users;
        users.append(person);
        
        QJsonObject root;
        root["users"] = users;
        root["version"] = "1.0";
        
        QJsonDocument doc(root);
        jsonDisplay->setText(doc.toJson(QJsonDocument::Indented));
        
        infoLabel->setText("✓ JSON generated successfully!");
        infoLabel->setStyleSheet("color: green; font-size: 12px; margin-top: 10px;");
    });

    // Parse JSON button
    QObject::connect(parseBtn, &QPushButton::clicked, [&]() {
        QByteArray jsonData = jsonDisplay->toPlainText().toUtf8();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        
        if (doc.isNull()) {
            infoLabel->setText("✗ Invalid JSON!");
            infoLabel->setStyleSheet("color: red; font-size: 12px; margin-top: 10px;");
            return;
        }
        
        QJsonObject root = doc.object();
        QJsonArray users = root["users"].toArray();
        
        if (!users.isEmpty()) {
            QJsonObject person = users[0].toObject();
            nameEdit->setText(person["name"].toString());
            ageEdit->setText(QString::number(person["age"].toInt()));
            emailEdit->setText(person["email"].toString());
            
            infoLabel->setText("✓ JSON parsed successfully!");
            infoLabel->setStyleSheet("color: green; font-size: 12px; margin-top: 10px;");
        }
    });

    // Save Settings button
    QObject::connect(saveSettingsBtn, &QPushButton::clicked, [&]() {
        QSettings settings;
        
        settings.beginGroup("UserData");
        settings.setValue("name", nameEdit->text());
        settings.setValue("age", ageEdit->text());
        settings.setValue("email", emailEdit->text());
        settings.endGroup();
        
        settings.setValue("lastSaved", QDateTime::currentDateTime());
        
        infoLabel->setText("✓ Settings saved!");
        infoLabel->setStyleSheet("color: green; font-size: 12px; margin-top: 10px;");
    });

    // Load Settings button
    QObject::connect(loadSettingsBtn, &QPushButton::clicked, [&]() {
        QSettings settings;
        
        settings.beginGroup("UserData");
        nameEdit->setText(settings.value("name", "").toString());
        ageEdit->setText(settings.value("age", "").toString());
        emailEdit->setText(settings.value("email", "").toString());
        settings.endGroup();
        
        QDateTime lastSaved = settings.value("lastSaved").toDateTime();
        if (lastSaved.isValid()) {
            infoLabel->setText("✓ Settings loaded from " + lastSaved.toString());
            infoLabel->setStyleSheet("color: green; font-size: 12px; margin-top: 10px;");
        } else {
            infoLabel->setText("No saved settings found");
            infoLabel->setStyleSheet("color: #666; font-size: 12px; margin-top: 10px;");
        }
    });

    // Auto-load settings on startup
    QSettings settings;
    settings.beginGroup("UserData");
    nameEdit->setText(settings.value("name", "").toString());
    ageEdit->setText(settings.value("age", "").toString());
    emailEdit->setText(settings.value("email", "").toString());
    settings.endGroup();

    window.resize(500, 450);
    window.show();

    return app.exec();
}
