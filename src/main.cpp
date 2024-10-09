#include <QApplication>
#include <QShortcut>
#include "ChatOverlay.h"
#include "mainwindow.h"
#include "customapplication.h"

int main(int argc, char *argv[])
{
    CustomApplication app(argc, argv);
    MainWindow mainwindow;
    mainwindow.show();
    return app.exec();
}
