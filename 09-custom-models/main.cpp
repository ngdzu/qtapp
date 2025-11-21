#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QAbstractTableModel>
#include <QInputDialog>
#include <QHeaderView>
#include <QLabel>
#include <QFont>
#include <QDebug>

// Custom data structure
struct Task
{
    QString title;
    QString priority; // "Low", "Medium", "High"
    bool done;
};

// Custom model
class TaskModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TaskModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent)
    {
        // Add sample data
        addTask("Learn custom models", "High", false);
        addTask("Practice Qt", "Medium", false);
        addTask("Build an app", "Low", false);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return m_tasks.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return 3; // Title, Priority, Done
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || index.row() >= m_tasks.size())
            return QVariant();

        const Task &task = m_tasks[index.row()];

        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            switch (index.column())
            {
            case 0:
                return task.title;
            case 1:
                return task.priority;
            case 2:
                return task.done ? "Yes" : "No";
            }
        }
        else if (role == Qt::BackgroundRole && index.column() == 1)
        {
            // Color code by priority
            if (task.priority == "High")
                return QColor(255, 200, 200); // Light red
            else if (task.priority == "Medium")
                return QColor(255, 255, 200); // Light yellow
            else
                return QColor(200, 255, 200); // Light green
        }
        else if (role == Qt::CheckStateRole && index.column() == 2)
        {
            return task.done ? Qt::Checked : Qt::Unchecked;
        }

        return QVariant();
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!index.isValid() || index.row() >= m_tasks.size())
            return false;

        Task &task = m_tasks[index.row()];

        if (role == Qt::EditRole)
        {
            switch (index.column())
            {
            case 0:
                task.title = value.toString();
                break;
            case 1:
                task.priority = value.toString();
                break;
            default:
                return false;
            }
            emit dataChanged(index, index, {role, Qt::DisplayRole});
            return true;
        }
        else if (role == Qt::CheckStateRole && index.column() == 2)
        {
            task.done = (value.toInt() == Qt::Checked);
            emit dataChanged(index, index, {role, Qt::DisplayRole});
            return true;
        }

        return false;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return Qt::NoItemFlags;

        Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

        if (index.column() == 2)
        {
            flags |= Qt::ItemIsUserCheckable;
        }
        else
        {
            flags |= Qt::ItemIsEditable;
        }

        return flags;
    }

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case 0:
                return "Task";
            case 1:
                return "Priority";
            case 2:
                return "Done";
            }
        }
        return QVariant();
    }

    bool insertRows(int row, int count, const QModelIndex &parent) override
    {
        if (parent.isValid())
            return false;

        beginInsertRows(QModelIndex(), row, row + count - 1);
        for (int i = 0; i < count; ++i)
        {
            m_tasks.insert(row, Task{"New Task", "Low", false});
        }
        endInsertRows();

        return true;
    }

    bool removeRows(int row, int count, const QModelIndex &parent) override
    {
        if (parent.isValid())
            return false;
        if (row + count > m_tasks.size())
            return false;

        beginRemoveRows(QModelIndex(), row, row + count - 1);
        m_tasks.remove(row, count);
        endRemoveRows();

        return true;
    }

    void addTask(const QString &title, const QString &priority, bool done)
    {
        int row = m_tasks.size();
        beginInsertRows(QModelIndex(), row, row);
        m_tasks.append(Task{title, priority, done});
        endInsertRows();
    }

private:
    QVector<Task> m_tasks;
};

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setWindowTitle("Lesson 9: Custom Models");
        resize(700, 500);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // Title
        QLabel *title = new QLabel("Custom Task Model Demo");
        QFont font = title->font();
        font.setPointSize(14);
        font.setBold(true);
        title->setFont(font);
        layout->addWidget(title);

        // Table view
        m_model = new TaskModel(this);
        m_tableView = new QTableView;
        m_tableView->setModel(m_model);
        m_tableView->horizontalHeader()->setStretchLastSection(true);
        m_tableView->setAlternatingRowColors(true);
        layout->addWidget(m_tableView);

        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        QPushButton *addBtn = new QPushButton("Add Task");
        QPushButton *removeBtn = new QPushButton("Remove Task");
        buttonLayout->addWidget(addBtn);
        buttonLayout->addWidget(removeBtn);
        buttonLayout->addStretch();
        layout->addLayout(buttonLayout);

        // Instructions
        QLabel *instructions = new QLabel(
            "• Double-click cells to edit\n"
            "• Priority column shows color coding\n"
            "• Click Done checkbox to toggle completion");
        layout->addWidget(instructions);

        // Connect signals
        connect(addBtn, &QPushButton::clicked, this, &MainWindow::addTask);
        connect(removeBtn, &QPushButton::clicked, this, &MainWindow::removeTask);

        qDebug() << "Custom model demo started";
    }

private slots:
    void addTask()
    {
        bool ok;
        QString title = QInputDialog::getText(
            this,
            "Add Task",
            "Task description:",
            QLineEdit::Normal,
            "",
            &ok);

        if (ok && !title.isEmpty())
        {
            QStringList priorities = {"Low", "Medium", "High"};
            QString priority = QInputDialog::getItem(
                this,
                "Priority",
                "Select priority:",
                priorities,
                0,
                false,
                &ok);

            if (ok)
            {
                m_model->addTask(title, priority, false);
                qDebug() << "Added task:" << title << priority;
            }
        }
    }

    void removeTask()
    {
        QModelIndex current = m_tableView->currentIndex();
        if (current.isValid())
        {
            QString title = m_model->data(m_model->index(current.row(), 0), Qt::DisplayRole).toString();
            m_model->removeRow(current.row());
            qDebug() << "Removed task:" << title;
        }
    }

private:
    TaskModel *m_model;
    QTableView *m_tableView;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
