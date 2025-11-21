# Lesson 8: Model/View Architecture

## Learning Goals

- Understand Qt's Model/View architecture and separation of concerns
- Learn about QAbstractItemModel and its subclasses
- Master QListView, QTableView, and QTreeView widgets
- Work with built-in models: QStringListModel, QStandardItemModel
- Understand the role of indexes (QModelIndex) in accessing data
- Implement data display and interaction patterns

## Introduction

Qt's Model/View architecture separates data (model) from presentation (view), following the Model-View-Controller (MVC) pattern. This separation enables multiple views of the same data, easier testing, and flexible UI updates. Understanding this architecture is essential for working with lists, tables, and trees in Qt applications.

The architecture consists of three components:
- **Models** store data and provide interface for access
- **Views** display data and handle user interaction
- **Delegates** customize rendering and editing (covered in Lesson 10)

## The Model/View Pattern

Traditional widgets like QListWidget combine data and presentation. Model/View separates them:

```cpp
// Traditional approach (coupled)
QListWidget *list = new QListWidget;
list->addItem("Item 1");
list->addItem("Item 2");

// Model/View approach (separated)
QStringListModel *model = new QStringListModel;
model->setStringList({"Item 1", "Item 2"});
QListView *view = new QListView;
view->setModel(model);
```

Benefits of separation:
- Multiple views can share one model
- Models can be tested independently
- Data updates automatically refresh all views
- Views can be swapped without changing data logic

## QModelIndex - Accessing Data

`QModelIndex` represents a location in the model:

```cpp
QModelIndex index = model->index(row, column);
QVariant data = model->data(index, Qt::DisplayRole);
QString text = data.toString();
```

Model indexes are lightweight handles provided by the model. They become invalid when the model changes, so don't store them long-term.

## Built-in Models

Qt provides ready-to-use models for common cases:

**QStringListModel - Simple string lists:**
```cpp
QStringListModel *model = new QStringListModel;
QStringList list = {"Apple", "Banana", "Cherry"};
model->setStringList(list);

// Modify data
model->insertRows(0, 1);
model->setData(model->index(0), "Apricot");
```

**QStandardItemModel - General purpose hierarchical data:**
```cpp
QStandardItemModel *model = new QStandardItemModel(3, 2);
model->setHorizontalHeaderLabels({"Name", "Age"});
model->setItem(0, 0, new QStandardItem("Alice"));
model->setItem(0, 1, new QStandardItem("25"));
```

## View Widgets

Qt provides view widgets for different data structures:

**QListView - Linear lists:**
```cpp
QListView *listView = new QListView;
listView->setModel(stringListModel);
listView->setEditTriggers(QAbstractItemView::DoubleClicked);
```

**QTableView - Tabular data:**
```cpp
QTableView *tableView = new QTableView;
tableView->setModel(standardItemModel);
tableView->horizontalHeader()->setStretchLastSection(true);
```

**QTreeView - Hierarchical data:**
```cpp
QTreeView *treeView = new QTreeView;
treeView->setModel(standardItemModel);
treeView->expandAll();
```

## View Configuration

Views are highly configurable:

```cpp
// Selection behavior
view->setSelectionMode(QAbstractItemView::SingleSelection);
view->setSelectionBehavior(QAbstractItemView::SelectRows);

// Edit triggers
view->setEditTriggers(
    QAbstractItemView::DoubleClicked | 
    QAbstractItemView::EditKeyPressed
);

// Visual options
tableView->setAlternatingRowColors(true);
tableView->setSortingEnabled(true);
```

## Responding to Selection Changes

Views emit signals when selection changes:

```cpp
QItemSelectionModel *selectionModel = view->selectionModel();
connect(selectionModel, &QItemSelectionModel::currentChanged,
        [](const QModelIndex &current, const QModelIndex &previous) {
    QString text = current.data(Qt::DisplayRole).toString();
    qDebug() << "Selected:" << text;
});
```

## Data Roles

Models store multiple data types per item using roles:

```cpp
// Common roles
Qt::DisplayRole      // Text shown in view
Qt::EditRole         // Text for editing
Qt::DecorationRole   // Icon
Qt::ToolTipRole      // Tooltip text
Qt::BackgroundRole   // Background color
Qt::ForegroundRole   // Text color

// Usage
model->setData(index, "Display Text", Qt::DisplayRole);
model->setData(index, QColor(Qt::red), Qt::BackgroundRole);
```

## Example Walkthrough

Our example demonstrates:
- QStringListModel with QListView for a task list
- QStandardItemModel with QTableView for contacts
- Adding, removing, and modifying data
- Selection handling and displaying selected items
- Header configuration and visual customization

Each view connects to its model and provides interactive controls for data manipulation.

## Expected Output

A window with two sections:

**Task List (QListView + QStringListModel):**
- List of tasks displayed vertically
- Buttons to add, remove, and edit tasks
- Double-click to edit task names inline

**Contact Table (QTableView + QStandardItemModel):**
- Table with Name, Email, Phone columns
- Buttons to add and remove contacts
- Sortable columns by clicking headers
- Alternating row colors for readability

Selected items display their details below each view.

## Try It

1. Build and run the application
2. Add and remove tasks in the list view
3. Double-click a task to edit it inline
4. Add contacts to the table view
5. Click column headers to sort the table
6. Select different rows and observe selection handling
7. Modify the code to add a "Status" column to contacts
8. Implement filtering to show only tasks containing certain text

## Key Takeaways

- Model/View separates data storage from presentation
- QModelIndex provides access to data locations in models
- Built-in models (QStringListModel, QStandardItemModel) handle common cases
- QListView, QTableView, QTreeView display different data structures
- Views are highly configurable (selection, editing, appearance)
- Selection models track and emit changes in selection
- Data roles allow multiple data types per model item
- One model can feed multiple views simultaneously
- Model/View is more flexible than convenience widgets (QListWidget, etc.)
- This architecture scales better for large datasets and complex UIs
