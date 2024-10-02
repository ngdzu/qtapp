#include "mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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
}

MainWindow::~MainWindow()
{
    // Destructor implementation
}

void MainWindow::newFile()
{
    // Implement the New action
    QMessageBox::information(this, tr("New File"), tr("New file created."));
}

void MainWindow::openFile()
{
    // Implement the Open action
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*)"));
    if (!fileName.isEmpty())
    {
        // Process opening the file
        QMessageBox::information(this, tr("Open File"), tr("File opened: ") + fileName);
    }
}

void MainWindow::saveFile()
{
    // Implement the Save action
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("All Files (*)"));
    if (!fileName.isEmpty())
    {
        // Process saving the file
        QMessageBox::information(this, tr("Save File"), tr("File saved: ") + fileName);
    }
}

void MainWindow::createActions()
{
    newAction = new QAction(tr("&New"), this);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    openAction = new QAction(tr("&Open"), this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    saveAction = new QAction(tr("&Save"), this);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
}
