# Lesson 9 Quiz: Custom Models

1. What are the minimum required methods you must implement when subclassing QAbstractTableModel?

2. Why is it critical to call beginInsertRows() and endInsertRows() when adding data to a custom model?

3. What's wrong with this code?
```cpp
void MyModel::addItem(const QString &item) {
    m_data.append(item);
    emit dataChanged(index(0, 0), index(m_data.size()-1, 0));
}
```

4. What is the purpose of the Qt::ItemFlags returned by the flags() method, and how does Qt::ItemIsEditable affect user interaction?

5. When implementing setData(), why must you emit dataChanged() and what parameters should you pass?

6. What's the difference between Qt::DisplayRole and Qt::EditRole, and when would you return different values for each?

7. Spot the bug:
```cpp
bool MyModel::removeRows(int row, int count, const QModelIndex &parent) {
    m_data.remove(row, count);
    beginRemoveRows(parent, row, row + count - 1);
    endRemoveRows();
    return true;
}
```

8. How would you implement background color coding in a custom model so high-priority items appear red?

9. You have a custom model wrapping a QVector<Person> where Person is a struct. How would you handle the case where the underlying vector is modified externally?

10. Reflection: You're building an email client with a custom model for the inbox. What data would you store in your model's internal structure, and which Qt::ItemDataRoles would you implement beyond DisplayRole?
