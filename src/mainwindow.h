#pragma once

#include <QMainWindow>
class QTcpServer;
class QTcpSocket;
class QKeyEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void saveFile();

private:
    void createMenus();
    void createActions();

    QMenu *fileMenu;
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
};
