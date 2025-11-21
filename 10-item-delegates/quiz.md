# Lesson 10 Quiz: Item Delegates

1. What is the role of a delegate in Qt's Model/View architecture?

2. Which method would you override to create a custom ComboBox editor for a specific column?

3. What's the purpose of setEditorData() and setModelData() in a delegate?

4. How do you make all cells in a specific column use a custom delegate?

5. In the paint() method, how do you detect if a cell is currently selected?

6. What's wrong with this delegate code?
```cpp
void MyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const {
    QLineEdit *edit = (QLineEdit*)editor;  // C-style cast
    model->setData(index, edit->text());
}
```

7. How would you create a delegate that displays a date picker when editing?

8. What does updateEditorGeometry() do, and when might you need to override it?

9. Why is QStyledItemDelegate preferred over QItemDelegate for new code?

10. Reflection: You're building a spreadsheet application. What custom delegates would you implement for different cell types (formula, date, currency)?
