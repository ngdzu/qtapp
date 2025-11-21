# Lesson 8 Quiz: Model/View Architecture

1. What are the main benefits of using Qt's Model/View architecture compared to convenience widgets like QListWidget or QTableWidget?

2. What happens in this code, and why is it problematic?
```cpp
QModelIndex index = model->index(0, 0);
model->removeRow(5);
QString data = index.data(Qt::DisplayRole).toString();
```

3. You have a QStringListModel with 100 items and want to display it in both a QListView and a QComboBox. How would you set this up, and what's the advantage?

4. What is the purpose of data roles like Qt::DisplayRole and Qt::EditRole, and when would you use different roles for the same model index?

5. Given this code, what will the table view display?
```cpp
QStandardItemModel *model = new QStandardItemModel(2, 2);
model->setItem(0, 0, new QStandardItem("A"));
model->setItem(1, 1, new QStandardItem("B"));
QTableView *view = new QTableView;
view->setModel(model);
```

6. How do you make a QTableView column stretch to fill available space, and why might you want to do this?

7. You want to detect when the user selects a different row in a QTableView. Which signal should you connect to, and what object emits it?

8. Spot the issue in this code:
```cpp
void MyWidget::setupView() {
    QStringListModel *model = new QStringListModel;
    model->setStringList({"A", "B", "C"});
    
    QListView *view = new QListView(this);
    view->setModel(model);
}
```

9. What's the difference between `setSelectionBehavior(QAbstractItemView::SelectRows)` and `setSelectionMode(QAbstractItemView::SingleSelection)`?

10. Reflection: You're building a music library application with artists, albums, and songs. Which view type would you use for each data level, and why? How would you connect them so selecting an artist shows their albums?
