#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QSlider>
#include <QPalette>
#include <QStyleFactory>

void applyLightTheme(QWidget *window) {
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::Highlight, QColor(76, 163, 224));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);
    
    QApplication::setPalette(lightPalette);
    QApplication::setStyle("Fusion");
    window->setStyleSheet("");
}

void applyDarkTheme(QWidget *window) {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    QApplication::setPalette(darkPalette);
    QApplication::setStyle("Fusion");
    window->setStyleSheet("");
}

void applyCustomQSS(QWidget *window) {
    window->setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: "Segoe UI", Arial, sans-serif;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
        QPushButton#themeButton {
            background-color: #2196F3;
        }
        QPushButton#themeButton:hover {
            background-color: #0b7dda;
        }
        QLineEdit, QTextEdit {
            background-color: #3d3d3d;
            color: white;
            border: 2px solid #555555;
            border-radius: 4px;
            padding: 5px;
        }
        QLineEdit:focus, QTextEdit:focus {
            border: 2px solid #4CAF50;
        }
        QCheckBox {
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 3px;
            border: 2px solid #555555;
        }
        QCheckBox::indicator:checked {
            background-color: #4CAF50;
            border-color: #4CAF50;
        }
        QSlider::groove:horizontal {
            background: #555555;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #4CAF50;
            width: 16px;
            margin: -5px 0;
            border-radius: 8px;
        }
    )");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Lesson 20: Styles, Themes, and Palette");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Title
    QLabel *titleLabel = new QLabel("Qt Styling Demo");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Theme buttons
    QHBoxLayout *themeLayout = new QHBoxLayout();
    
    QPushButton *lightBtn = new QPushButton("Light Theme");
    lightBtn->setObjectName("themeButton");
    lightBtn->setMinimumHeight(40);
    
    QPushButton *darkBtn = new QPushButton("Dark Theme");
    darkBtn->setObjectName("themeButton");
    darkBtn->setMinimumHeight(40);
    
    QPushButton *customBtn = new QPushButton("Custom QSS");
    customBtn->setObjectName("themeButton");
    customBtn->setMinimumHeight(40);
    
    themeLayout->addWidget(lightBtn);
    themeLayout->addWidget(darkBtn);
    themeLayout->addWidget(customBtn);
    mainLayout->addLayout(themeLayout);

    // Sample widgets
    QLabel *infoLabel = new QLabel("Sample Widgets:");
    infoLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(infoLabel);

    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setPlaceholderText("Enter text here...");
    mainLayout->addWidget(lineEdit);

    QTextEdit *textEdit = new QTextEdit();
    textEdit->setPlaceholderText("Multi-line text area...");
    textEdit->setMaximumHeight(100);
    mainLayout->addWidget(textEdit);

    QCheckBox *checkbox1 = new QCheckBox("Enable feature A");
    checkbox1->setChecked(true);
    mainLayout->addWidget(checkbox1);

    QCheckBox *checkbox2 = new QCheckBox("Enable feature B");
    mainLayout->addWidget(checkbox2);

    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(50);
    mainLayout->addWidget(slider);

    QLabel *sliderLabel = new QLabel("Slider value: 50");
    mainLayout->addWidget(sliderLabel);

    // Connect slider
    QObject::connect(slider, &QSlider::valueChanged, [sliderLabel](int value) {
        sliderLabel->setText(QString("Slider value: %1").arg(value));
    });

    // Status label
    QLabel *statusLabel = new QLabel("Current theme: Light");
    statusLabel->setStyleSheet("color: #666; margin-top: 10px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // Connect theme buttons
    QObject::connect(lightBtn, &QPushButton::clicked, [&window, statusLabel]() {
        applyLightTheme(&window);
        statusLabel->setText("Current theme: Light");
    });

    QObject::connect(darkBtn, &QPushButton::clicked, [&window, statusLabel]() {
        applyDarkTheme(&window);
        statusLabel->setText("Current theme: Dark");
    });

    QObject::connect(customBtn, &QPushButton::clicked, [&window, statusLabel]() {
        applyCustomQSS(&window);
        statusLabel->setText("Current theme: Custom QSS");
    });

    // Start with light theme
    applyLightTheme(&window);

    window.resize(500, 500);
    window.show();

    return app.exec();
}
