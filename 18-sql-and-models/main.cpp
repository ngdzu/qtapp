#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QHeaderView>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QAbstractItemView>

bool createDatabase()
{
    // Create in-memory SQLite database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");

    if (!db.open())
    {
        qDebug() << "Error: Could not open database";
        qDebug() << db.lastError().text();
        return false;
    }

    // Create employees table
    QSqlQuery query;
    QString createTable = R"(
        CREATE TABLE employees (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            department TEXT,
            salary INTEGER
        )
    )";

    if (!query.exec(createTable))
    {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }

    // Insert sample data
    QStringList sampleData = {
        "INSERT INTO employees (name, department, salary) VALUES ('Alice Johnson', 'Engineering', 75000)",
        "INSERT INTO employees (name, department, salary) VALUES ('Bob Smith', 'Marketing', 65000)",
        "INSERT INTO employees (name, department, salary) VALUES ('Charlie Brown', 'Engineering', 80000)",
        "INSERT INTO employees (name, department, salary) VALUES ('Diana Prince', 'HR', 70000)",
        "INSERT INTO employees (name, department, salary) VALUES ('Eve Adams', 'Sales', 60000)"};

    for (const QString &sql : sampleData)
    {
        if (!query.exec(sql))
        {
            qDebug() << "Error inserting data:" << query.lastError().text();
            return false;
        }
    }

    qDebug() << "Database created successfully with sample data";
    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Create and populate database
    if (!createDatabase())
    {
        QMessageBox::critical(nullptr, "Database Error",
                              "Failed to create database. Check console for details.");
        return 1;
    }

    QWidget window;
    window.setWindowTitle("Lesson 18: SQL and Models");
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Title
    QLabel *titleLabel = new QLabel("Employee Database Manager");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Create model
    QSqlTableModel *model = new QSqlTableModel();
    model->setTable("employees");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    // Set friendly column headers
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Name");
    model->setHeaderData(2, Qt::Horizontal, "Department");
    model->setHeaderData(3, Qt::Horizontal, "Salary");

    // Create table view
    QTableView *tableView = new QTableView();
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    mainLayout->addWidget(tableView);

    // Status label
    QLabel *statusLabel = new QLabel("Database loaded with 5 employees. Double-click cells to edit.");
    statusLabel->setStyleSheet("color: #666; font-size: 11px; padding: 5px;");
    mainLayout->addWidget(statusLabel);

    // Add new employee section
    QLabel *addLabel = new QLabel("Add New Employee:");
    addLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(addLabel);

    QHBoxLayout *inputLayout = new QHBoxLayout();

    QLineEdit *nameInput = new QLineEdit();
    nameInput->setPlaceholderText("Name");
    inputLayout->addWidget(nameInput);

    QLineEdit *deptInput = new QLineEdit();
    deptInput->setPlaceholderText("Department");
    inputLayout->addWidget(deptInput);

    QLineEdit *salaryInput = new QLineEdit();
    salaryInput->setPlaceholderText("Salary");
    salaryInput->setMaximumWidth(100);
    inputLayout->addWidget(salaryInput);

    mainLayout->addLayout(inputLayout);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *addBtn = new QPushButton("Add Employee");
    QPushButton *deleteBtn = new QPushButton("Delete Selected");
    QPushButton *saveBtn = new QPushButton("Save Changes");
    QPushButton *revertBtn = new QPushButton("Revert");
    QPushButton *queryBtn = new QPushButton("Show High Earners");

    addBtn->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px;");
    deleteBtn->setStyleSheet("background-color: #f44336; color: white; padding: 8px;");
    saveBtn->setStyleSheet("background-color: #2196F3; color: white; padding: 8px;");
    revertBtn->setStyleSheet("background-color: #FF9800; color: white; padding: 8px;");
    queryBtn->setStyleSheet("background-color: #9C27B0; color: white; padding: 8px;");

    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(revertBtn);
    buttonLayout->addWidget(queryBtn);

    mainLayout->addLayout(buttonLayout);

    // Info label
    QLabel *infoLabel = new QLabel(
        "Qt SQL demonstrates:\n"
        "• QSqlDatabase - SQLite in-memory database\n"
        "• QSqlTableModel - Automatic view synchronization\n"
        "• QSqlQuery - Custom SQL queries\n"
        "• CRUD operations (Create, Read, Update, Delete)\n"
        "• Edit cells by double-clicking, then click 'Save Changes'");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #555; font-size: 11px; margin-top: 10px; padding: 10px; background: #e3f2fd; border-radius: 5px;");
    mainLayout->addWidget(infoLabel);

    // Add employee button handler
    QObject::connect(addBtn, &QPushButton::clicked, [&, model, nameInput, deptInput, salaryInput, statusLabel]()
                     {
        QString name = nameInput->text().trimmed();
        QString dept = deptInput->text().trimmed();
        QString salaryStr = salaryInput->text().trimmed();
        
        if (name.isEmpty()) {
            QMessageBox::warning(&window, "Input Error", "Please enter a name");
            return;
        }
        
        bool ok;
        int salary = salaryStr.toInt(&ok);
        if (!ok && !salaryStr.isEmpty()) {
            QMessageBox::warning(&window, "Input Error", "Salary must be a number");
            return;
        }
        
        // Insert new row
        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 1), name);
        model->setData(model->index(row, 2), dept);
        model->setData(model->index(row, 3), ok ? salary : QVariant());
        
        if (model->submitAll()) {
            statusLabel->setText(QString("Added employee: %1").arg(name));
            nameInput->clear();
            deptInput->clear();
            salaryInput->clear();
            model->select(); // Refresh to show auto-generated ID
        } else {
            statusLabel->setText("Error adding employee: " + model->lastError().text());
            model->revertAll();
        } });

    // Delete button handler
    QObject::connect(deleteBtn, &QPushButton::clicked, [&, model, tableView, statusLabel]()
                     {
        QModelIndexList selection = tableView->selectionModel()->selectedRows();
        if (selection.isEmpty()) {
            QMessageBox::information(&window, "No Selection", "Please select a row to delete");
            return;
        }
        
        int row = selection.first().row();
        QString name = model->data(model->index(row, 1)).toString();
        
        if (model->removeRow(row)) {
            if (model->submitAll()) {
                statusLabel->setText(QString("Deleted employee: %1").arg(name));
                model->select();
            } else {
                statusLabel->setText("Error deleting: " + model->lastError().text());
                model->revertAll();
            }
        } });

    // Save button handler
    QObject::connect(saveBtn, &QPushButton::clicked, [model, statusLabel]()
                     {
        if (model->submitAll()) {
            statusLabel->setText("Changes saved successfully!");
            statusLabel->setStyleSheet("color: green; font-weight: bold; font-size: 11px; padding: 5px;");
        } else {
            statusLabel->setText("Error saving: " + model->lastError().text());
            statusLabel->setStyleSheet("color: red; font-weight: bold; font-size: 11px; padding: 5px;");
        }
        model->select(); });

    // Revert button handler
    QObject::connect(revertBtn, &QPushButton::clicked, [model, statusLabel]()
                     {
        model->revertAll();
        statusLabel->setText("Changes reverted");
        statusLabel->setStyleSheet("color: orange; font-weight: bold; font-size: 11px; padding: 5px;"); });

    // Custom query button handler
    QObject::connect(queryBtn, &QPushButton::clicked, [&, statusLabel]()
                     {
        QSqlQuery query;
        query.prepare("SELECT name, department, salary FROM employees WHERE salary > :threshold ORDER BY salary DESC");
        query.bindValue(":threshold", 70000);
        
        if (!query.exec()) {
            QMessageBox::warning(&window, "Query Error", query.lastError().text());
            return;
        }
        
        QString result = "High Earners (>$70,000):\n\n";
        while (query.next()) {
            QString name = query.value(0).toString();
            QString dept = query.value(1).toString();
            int salary = query.value(2).toInt();
            result += QString("%1 (%2): $%3\n").arg(name).arg(dept).arg(salary);
        }
        
        QMessageBox::information(&window, "Query Results", result);
        statusLabel->setText("Custom query executed successfully");
        statusLabel->setStyleSheet("color: purple; font-weight: bold; font-size: 11px; padding: 5px;"); });

    window.resize(800, 600);
    window.show();

    return app.exec();
}
