# Lesson 18: SQL and Models

## Learning Goals
- Use QSqlDatabase for database connections
- Execute SQL queries with QSqlQuery
- Display data with QSqlTableModel
- Implement CRUD operations
- Handle database errors

## Introduction

Qt SQL provides database integration with a unified API supporting SQLite, MySQL, PostgreSQL, and others. QSqlTableModel bridges SQL tables with Qt's view classes, enabling automatic UI updates when data changes.

For lightweight applications, SQLite is ideal - it's embedded, requires no server, and stores everything in a single file.

## Key Concepts

### Database Connection

```cpp
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName(":memory:"); // Or file path
if (!db.open()) {
    qDebug() << "Database error:" << db.lastError();
}
```

### Executing Queries

```cpp
QSqlQuery query;
query.exec("CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)");
query.exec("INSERT INTO users (name) VALUES ('Alice')");

query.exec("SELECT * FROM users");
while (query.next()) {
    qDebug() << query.value("name").toString();
}
```

### QSqlTableModel

```cpp
QSqlTableModel *model = new QSqlTableModel;
model->setTable("users");
model->select();

QTableView *view = new QTableView;
view->setModel(model);
```

## Expected Output

Application displaying a database table with add/remove functionality.

## Key Takeaways

- QSqlDatabase manages connections
- QSqlQuery executes SQL statements
- QSqlTableModel provides automatic view synchronization
- Always check for errors after database operations
- SQLite is perfect for local storage
