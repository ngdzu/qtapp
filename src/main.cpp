// src/main.cpp
#include <QApplication>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv); // QApplication for GUI applications

    MainWindow window;
    window.show();

    return app.exec();
}
