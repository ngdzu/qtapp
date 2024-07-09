// src/main.cpp
#include <QApplication>
#include <QDebug>
#include "mainwindow.h"

void serializeData();
void deserializeData();

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);  // QApplication for GUI applications

    MainWindow window;
    window.show();

    serializeData();
    deserializeData();

    qDebug() << "Qt version:" << qVersion();

    return app.exec();
}
