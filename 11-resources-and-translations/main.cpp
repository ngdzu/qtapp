#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Lesson 11: Resources");

    QVBoxLayout *layout = new QVBoxLayout(&window);

    QLabel *title = new QLabel("Qt Resource System Demo");
    layout->addWidget(title);

    // Load image from resources
    QLabel *imageLabel = new QLabel;
    QPixmap pixmap(":/images/qt-logo.png");
    if (!pixmap.isNull())
    {
        imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
    }
    else
    {
        imageLabel->setText("Image not found");
    }
    layout->addWidget(imageLabel);

    // Load stylesheet from resources
    QFile styleFile(":/styles/app.qss");
    if (styleFile.open(QFile::ReadOnly))
    {
        QString styleSheet = styleFile.readAll();
        window.setStyleSheet(styleSheet);
    }

    window.show();
    return app.exec();
}
