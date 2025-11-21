# Lesson 9: Custom Models

## Learning Goals

- Understand how to subclass QAbstractListModel and QAbstractTableModel
- Implement required model interface methods (rowCount, data, setData)
- Learn about model change notification (beginInsertRows, endInsertRows, etc.)
- Handle editing with flags() and setData()
- Work with custom data structures as model backends
- Emit dataChanged signals properly

## Introduction

While Qt's built-in models handle many cases, custom models give you full control over data representation and behavior. By subclassing abstract model classes, you can wrap any C++ data structure (vectors, custom classes, databases) and present it through Qt's Model/View framework.

Custom models are essential when:
- Data comes from external sources (databases, files, networks)
- You need specialized sorting or filtering logic
- Built-in models don't match your data structure
- Performance optimization is critical for large datasets

## QAbstractItemModel Hierarchy

```
QAbstractItemModel (base for all models)
├── QAbstractListModel (1D data)
├── QAbstractTableModel (2D data)
└── QAbstractProxyModel (wraps other models)
```

Choose based on your data structure:
- **QAbstractListModel**: Lists (one column)
- **QAbstractTableModel**: Tables (rows and columns)
- **QAbstractItemModel**: Trees (hierarchical data)

## Required Methods

All custom models must implement:

```cpp
class MyListModel : public QAbstractListModel
{
public:
    // Required: return number of items
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;  // List has no children
        return m_data.size();
    }

    // Required: return data for display
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || index.row() >= m_data.size())
            return QVariant();
        
        if (role == Qt::DisplayRole)
            return m_data[index.row()];
        
        return QVariant();
    }

private:
    QStringList m_data;
};
```

For tables, add `columnCount()`:

```cpp
class MyTableModel : public QAbstractTableModel
{
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;
        return m_data.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;
        return 3;  // Three columns
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid()) return QVariant();
        if (role == Qt::DisplayRole) {
            return m_data[index.row()][index.column()];
        }
        return QVariant();
    }

private:
    QVector<QVector<QString>> m_data;
};
```

## Editing Support

Enable editing by implementing `setData()` and `flags()`:

```cpp
Qt::ItemFlags flags(const QModelIndex &index) const override
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool setData(const QModelIndex &index, const QVariant &value, int role) override
{
    if (index.isValid() && role == Qt::EditRole) {
        m_data[index.row()] = value.toString();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}
```

Key points:
- `flags()` controls what users can do (edit, select, drag, etc.)
- `setData()` modifies the underlying data
- Always emit `dataChanged()` after modifications
- Return `true` from `setData()` on success

## Inserting and Removing Data

Notify views before and after structural changes:

```cpp
bool insertRows(int row, int count, const QModelIndex &parent) override
{
    if (parent.isValid()) return false;
    
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
        m_data.insert(row, QString());
    endInsertRows();
    
    return true;
}

bool removeRows(int row, int count, const QModelIndex &parent) override
{
    if (parent.isValid()) return false;
    if (row + count > m_data.size()) return false;
    
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_data.remove(row, count);
    endRemoveRows();
    
    return true;
}
```

**Critical**: Always call `beginInsertRows()`/`endInsertRows()` and `beginRemoveRows()`/`endRemoveRows()`. Views crash without these notifications.

## Header Data

Provide column/row headers:

```cpp
QVariant headerData(int section, Qt::Orientation orientation, 
                    int role) const override
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "Name";
            case 1: return "Age";
            case 2: return "Email";
        }
    }
    return QVariant();
}
```

## Working with Custom Data

Models can wrap any C++ data:

```cpp
struct Person {
    QString name;
    int age;
    QString email;
};

class PersonModel : public QAbstractTableModel
{
public:
    void addPerson(const Person &person) {
        beginInsertRows(QModelIndex(), m_people.size(), m_people.size());
        m_people.append(person);
        endInsertRows();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();
        
        const Person &p = m_people[index.row()];
        switch (index.column()) {
            case 0: return p.name;
            case 1: return p.age;
            case 2: return p.email;
        }
        return QVariant();
    }

private:
    QVector<Person> m_people;
};
```

## Example Walkthrough

Our example creates a custom table model for a simple task manager:
- Task struct with title, priority, and completion status
- Custom model implementing all required methods
- Support for editing, inserting, and removing tasks
- Custom data roles for background colors based on priority
- Integration with QTableView for display and interaction

## Expected Output

A window with a table displaying tasks with columns: Title, Priority, Done.
- Different background colors for different priorities (high=red, medium=yellow, low=green)
- Inline editing by double-clicking cells
- Buttons to add and remove tasks
- Checkbox column for completion status

## Try It

1. Build and run the application
2. Add new tasks and observe model notifications
3. Edit task titles and priorities inline
4. Toggle completion status
5. Remove tasks and verify model updates
6. Add a "Due Date" column to the model
7. Implement sorting by priority
8. Add filtering to show only incomplete tasks

## Key Takeaways

- Custom models subclass QAbstractListModel or QAbstractTableModel
- Must implement: rowCount(), data(), and columnCount() (for tables)
- Enable editing with flags() and setData()
- Always emit dataChanged() when data changes
- Use begin/end functions for structural changes (insert/remove rows)
- Models can wrap any C++ data structure
- Views automatically update when models emit proper signals
- Custom models enable integration with databases, files, and APIs
- Performance-critical code benefits from custom models
- Model/View separation makes testing easier
