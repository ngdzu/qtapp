#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Create main window
    QWidget window;
    window.resize(300, 200);
    window.setWindowTitle("Qt Widgets Basics");

    // Create a button (child of window)
    QPushButton *button = new QPushButton("Click Me", &window);
    button->setGeometry(100, 50, 100, 30);

    // Create a label (child of window)
    QLabel *label = new QLabel("Clicks: 0", &window);
    label->setGeometry(50, 100, 200, 30);
    label->setAlignment(Qt::AlignCenter);

    // Track click count
    int clickCount = 0;

    // Connect button click to update label
    QObject::connect(button, &QPushButton::clicked, [&]()
                     {
        clickCount++;
        label->setText(QString("Clicks: %1").arg(clickCount)); });

    // Show window and start event loop
    window.show();
    return app.exec();
}
