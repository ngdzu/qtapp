# Lesson 10: Item Delegates

## Learning Goals

- Understand the role of delegates in Model/View
- Learn to subclass QStyledItemDelegate
- Implement custom cell editors
- Create custom rendering with paint()
- Handle editor creation, data setting, and geometry
- Use delegates for complex cell widgets

## Introduction

Delegates are the third component of Qt's Model/View architecture. While models store data and views display it, delegates control how individual items are rendered and edited. Custom delegates enable rich cell content, specialized editors, and interactive widgets within table and list views.

## QStyledItemDelegate

The modern delegate class to subclass:

```cpp
class CustomDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    // Custom painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        // Draw custom content
    }

    // Custom editor
    QWidget* createEditor(QWidget *parent, 
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override
    {
        return new QComboBox(parent);
    }

    // Set editor data from model
    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        QComboBox *combo = static_cast<QComboBox*>(editor);
        combo->setCurrentText(index.data(Qt::EditRole).toString());
    }

    // Save editor data to model
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override
    {
        QComboBox *combo = static_cast<QComboBox*>(editor);
        model->setData(index, combo->currentText(), Qt::EditRole);
    }
};
```

## Custom Painting

Override `paint()` for custom rendering:

```cpp
void paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const override
{
    painter->save();

    // Draw selection/focus state
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    // Draw custom content
    int progress = index.data(Qt::UserRole).toInt();
    QRect progressRect = option.rect.adjusted(5, 5, -5, -5);
    
    painter->drawRect(progressRect);
    painter->fillRect(progressRect.adjusted(0, 0, 
        -progressRect.width() * (100 - progress) / 100, 0),
        Qt::blue);

    painter->restore();
}
```

## Custom Editors

Create specialized editors for different data types:

```cpp
QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const override
{
    int column = index.column();
    
    if (column == 1) {  // Priority column
        QComboBox *combo = new QComboBox(parent);
        combo->addItems({"Low", "Medium", "High"});
        return combo;
    }
    else if (column == 2) {  // Progress column
        QSlider *slider = new QSlider(Qt::Horizontal, parent);
        slider->setRange(0, 100);
        return slider;
    }
    
    return QStyledItemDelegate::createEditor(parent, option, index);
}
```

## Editor Geometry

Control editor size and position:

```cpp
void updateEditorGeometry(QWidget *editor, 
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override
{
    editor->setGeometry(option.rect);
}
```

## Size Hints

Provide custom item sizes:

```cpp
QSize sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(qMax(size.height(), 40));  // Min height
    return size;
}
```

## Example Walkthrough

Our example creates:
- Progress bar delegate for visual progress indicators
- ComboBox delegate for priority selection
- Star rating delegate with custom painting
- Each delegate demonstrates different aspects of customization

## Expected Output

A table with custom cell rendering:
- Column 1: Text with custom background
- Column 2: ComboBox editor for priorities
- Column 3: Visual progress bars
- Column 4: Star rating display

## Try It

1. Double-click priority cells to see ComboBox editor
2. Edit progress values to see bar update
3. Observe custom painting for different data types
4. Add a date picker delegate
5. Implement a color picker delegate

## Key Takeaways

- Delegates control item rendering and editing
- Override paint() for custom visual appearance
- createEditor() provides specialized input widgets
- setEditorData() and setModelData() transfer data
- Delegates are reusable across different views
- One delegate per column or per data type
- Custom painting enables rich cell content
