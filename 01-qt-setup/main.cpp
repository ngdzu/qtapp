#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLabel label("Hello, Qt!");
    label.setWindowTitle("Lesson 1: Qt Setup");
    label.setAlignment(Qt::AlignCenter);
    label.resize(300, 100);
    label.show();

    return app.exec();
}
