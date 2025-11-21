#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListView>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QInputDialog>
#include <QHeaderView>
#include <QDebug>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setWindowTitle("Lesson 8: Model/View Architecture");
        resize(800, 600);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Title
        QLabel *title = new QLabel("Model/View Architecture Demo");
        QFont titleFont = title->font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        title->setFont(titleFont);
        mainLayout->addWidget(title);

        // Horizontal layout for two sections
        QHBoxLayout *sectionsLayout = new QHBoxLayout;

        // Left section: List View with QStringListModel
        QGroupBox *listGroup = new QGroupBox("Task List (QStringListModel + QListView)");
        QVBoxLayout *listLayout = new QVBoxLayout(listGroup);

        m_taskModel = new QStringListModel(this);
        QStringList initialTasks = {
            "Learn Qt Model/View",
            "Build a todo app",
            "Master QTableView",
            "Explore custom models"};
        m_taskModel->setStringList(initialTasks);

        m_listView = new QListView;
        m_listView->setModel(m_taskModel);
        m_listView->setEditTriggers(QAbstractItemView::DoubleClicked);
        listLayout->addWidget(m_listView);

        m_listSelectionLabel = new QLabel("Selected: None");
        listLayout->addWidget(m_listSelectionLabel);

        QHBoxLayout *listButtons = new QHBoxLayout;
        QPushButton *addTaskBtn = new QPushButton("Add Task");
        QPushButton *removeTaskBtn = new QPushButton("Remove Task");
        listButtons->addWidget(addTaskBtn);
        listButtons->addWidget(removeTaskBtn);
        listLayout->addLayout(listButtons);

        sectionsLayout->addWidget(listGroup);

        // Right section: Table View with QStandardItemModel
        QGroupBox *tableGroup = new QGroupBox("Contacts (QStandardItemModel + QTableView)");
        QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);

        m_contactModel = new QStandardItemModel(0, 3, this);
        m_contactModel->setHorizontalHeaderLabels({"Name", "Email", "Phone"});

        // Add sample contacts
        addContact("Alice Johnson", "alice@example.com", "555-0101");
        addContact("Bob Smith", "bob@example.com", "555-0102");
        addContact("Carol White", "carol@example.com", "555-0103");

        m_tableView = new QTableView;
        m_tableView->setModel(m_contactModel);
        m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_tableView->setAlternatingRowColors(true);
        m_tableView->setSortingEnabled(true);
        m_tableView->horizontalHeader()->setStretchLastSection(true);
        tableLayout->addWidget(m_tableView);

        m_tableSelectionLabel = new QLabel("Selected: None");
        tableLayout->addWidget(m_tableSelectionLabel);

        QHBoxLayout *tableButtons = new QHBoxLayout;
        QPushButton *addContactBtn = new QPushButton("Add Contact");
        QPushButton *removeContactBtn = new QPushButton("Remove Contact");
        tableButtons->addWidget(addContactBtn);
        tableButtons->addWidget(removeContactBtn);
        tableLayout->addLayout(tableButtons);

        sectionsLayout->addWidget(tableGroup);

        mainLayout->addLayout(sectionsLayout);

        // Instructions
        QLabel *instructions = new QLabel(
            "• Double-click tasks to edit\n"
            "• Click table headers to sort\n"
            "• Select rows to see details");
        instructions->setWordWrap(true);
        mainLayout->addWidget(instructions);

        // Connect signals
        connect(addTaskBtn, &QPushButton::clicked, this, &MainWindow::addTask);
        connect(removeTaskBtn, &QPushButton::clicked, this, &MainWindow::removeTask);
        connect(addContactBtn, &QPushButton::clicked, this, &MainWindow::addContactDialog);
        connect(removeContactBtn, &QPushButton::clicked, this, &MainWindow::removeContact);

        // Selection changes
        connect(m_listView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &MainWindow::onListSelectionChanged);
        connect(m_tableView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &MainWindow::onTableSelectionChanged);

        qDebug() << "Model/View demo started";
    }

private slots:
    void addTask()
    {
        bool ok;
        QString task = QInputDialog::getText(
            this,
            "Add Task",
            "Task description:",
            QLineEdit::Normal,
            "",
            &ok);

        if (ok && !task.isEmpty())
        {
            int row = m_taskModel->rowCount();
            m_taskModel->insertRow(row);
            m_taskModel->setData(m_taskModel->index(row), task);
            qDebug() << "Added task:" << task;
        }
    }

    void removeTask()
    {
        QModelIndex current = m_listView->currentIndex();
        if (current.isValid())
        {
            QString task = current.data(Qt::DisplayRole).toString();
            m_taskModel->removeRow(current.row());
            qDebug() << "Removed task:" << task;
        }
    }

    void addContactDialog()
    {
        bool ok;
        QString name = QInputDialog::getText(this, "Add Contact", "Name:", QLineEdit::Normal, "", &ok);
        if (!ok || name.isEmpty())
            return;

        QString email = QInputDialog::getText(this, "Add Contact", "Email:", QLineEdit::Normal, "", &ok);
        if (!ok || email.isEmpty())
            return;

        QString phone = QInputDialog::getText(this, "Add Contact", "Phone:", QLineEdit::Normal, "", &ok);
        if (!ok || phone.isEmpty())
            return;

        addContact(name, email, phone);
        qDebug() << "Added contact:" << name;
    }

    void removeContact()
    {
        QModelIndex current = m_tableView->currentIndex();
        if (current.isValid())
        {
            QString name = m_contactModel->item(current.row(), 0)->text();
            m_contactModel->removeRow(current.row());
            qDebug() << "Removed contact:" << name;
        }
    }

    void onListSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
    {
        Q_UNUSED(previous);
        if (current.isValid())
        {
            QString task = current.data(Qt::DisplayRole).toString();
            m_listSelectionLabel->setText(QString("Selected: %1").arg(task));
        }
        else
        {
            m_listSelectionLabel->setText("Selected: None");
        }
    }

    void onTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
    {
        Q_UNUSED(previous);
        if (current.isValid())
        {
            int row = current.row();
            QString name = m_contactModel->item(row, 0)->text();
            QString email = m_contactModel->item(row, 1)->text();
            QString phone = m_contactModel->item(row, 2)->text();
            m_tableSelectionLabel->setText(
                QString("Selected: %1 (%2, %3)").arg(name).arg(email).arg(phone));
        }
        else
        {
            m_tableSelectionLabel->setText("Selected: None");
        }
    }

private:
    void addContact(const QString &name, const QString &email, const QString &phone)
    {
        int row = m_contactModel->rowCount();
        m_contactModel->insertRow(row);
        m_contactModel->setItem(row, 0, new QStandardItem(name));
        m_contactModel->setItem(row, 1, new QStandardItem(email));
        m_contactModel->setItem(row, 2, new QStandardItem(phone));
    }

    QStringListModel *m_taskModel;
    QStandardItemModel *m_contactModel;
    QListView *m_listView;
    QTableView *m_tableView;
    QLabel *m_listSelectionLabel;
    QLabel *m_tableSelectionLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
