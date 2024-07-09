#include "mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>  
#include <QMessageBox>  
#include <QTcpServer> 
#include <QTcpSocket>

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent), tcpServer(new QTcpServer(this)) 
{
    createActions();
    createMenus();

    QToolBar *toolBar = addToolBar("Main Toolbar");
    toolBar->addAction(newAction);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout();

    QLabel *label = new QLabel("Welcome to the Main Window");
    mainLayout->addWidget(label);

    QLineEdit *textField = new QLineEdit();
    mainLayout->addWidget(textField);

    QPushButton *button = new QPushButton("Click Me");
    mainLayout->addWidget(button);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::handleNewConnection);
}


void MainWindow::startServer() {
    const int port = 5151;

    if (tcpServer->listen(QHostAddress::Any, port)) {
        QMessageBox::information(this, tr("Server Started"), tr("Server started on port %1").arg(port));
    } else {
        QMessageBox::critical(this, tr("Server Error"), tcpServer->errorString());
    }
}

void MainWindow::handleNewConnection() {
    clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::handleRequest);
}

void MainWindow::handleRequest() {
    QByteArray requestData = clientSocket->readAll();
    qDebug() << "Request received:" << requestData;

    QByteArray responseData;
    responseData.append("HTTP/1.1 200 OK\r\n");
    responseData.append("Content-Type: text/html\r\n");
    responseData.append("\r\n");
    responseData.append("<html><body><h1>Hello World</h1></body></html>");

    clientSocket->write(responseData);
    clientSocket->flush();
    clientSocket->disconnectFromHost();
}

MainWindow::~MainWindow() {
    // Destructor implementation
}

void MainWindow::newFile() {
    // Implement the New action
    QMessageBox::information(this, tr("New File"), tr("New file created."));
}

void MainWindow::openFile() {
    // Implement the Open action
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*)"));
    if (!fileName.isEmpty()) {
        // Process opening the file
        QMessageBox::information(this, tr("Open File"), tr("File opened: ") + fileName);
    }
}

void MainWindow::saveFile() {
    // Implement the Save action
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("All Files (*)"));
    if (!fileName.isEmpty()) {
        // Process saving the file
        QMessageBox::information(this, tr("Save File"), tr("File saved: ") + fileName);
    }
}

void MainWindow::createActions() {
    newAction = new QAction(tr("&New"), this);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    openAction = new QAction(tr("&Open"), this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    saveAction = new QAction(tr("&Save"), this);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    startServerAction = new QAction(tr("&Start Server"), this);
    connect(startServerAction, &QAction::triggered, this, &MainWindow::startServer);
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(startServerAction); // Add the start server action to the menu
}

