#pragma once

#include <QMainWindow>
class QTcpServer;
class QTcpSocket;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void startServer();
    void handleNewConnection();  // Slot to handle new connections
    void handleRequest();

private:
    void createMenus();
    void createActions();

    QMenu *fileMenu;
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *startServerAction;
    QTcpServer *tcpServer;
    QTcpSocket *clientSocket;
};
