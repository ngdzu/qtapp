#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTableView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>

// Progress bar delegate
class ProgressDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        int progress = index.data(Qt::DisplayRole).toInt();

        painter->save();

        // Draw selection
        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, option.palette.highlight());
        }

        // Draw progress bar
        QRect progressRect = option.rect.adjusted(5, 8, -5, -8);
        painter->setPen(Qt::black);
        painter->drawRect(progressRect);

        int fillWidth = progressRect.width() * progress / 100;
        QRect fillRect = progressRect.adjusted(1, 1, -progressRect.width() + fillWidth - 1, -1);
        painter->fillRect(fillRect, QColor(100, 200, 100));

        // Draw percentage text
        painter->setPen(Qt::black);
        painter->drawText(option.rect, Qt::AlignCenter, QString("%1%").arg(progress));

        painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        QSlider *slider = new QSlider(Qt::Horizontal, parent);
        slider->setRange(0, 100);
        return slider;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        QSlider *slider = static_cast<QSlider *>(editor);
        slider->setValue(index.data(Qt::EditRole).toInt());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        QSlider *slider = static_cast<QSlider *>(editor);
        model->setData(index, slider->value(), Qt::EditRole);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(40);
        return size;
    }
};

// Priority delegate with combo box
class PriorityDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        QComboBox *combo = new QComboBox(parent);
        combo->addItems({"Low", "Medium", "High"});
        return combo;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        QComboBox *combo = static_cast<QComboBox *>(editor);
        combo->setCurrentText(index.data(Qt::EditRole).toString());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        QComboBox *combo = static_cast<QComboBox *>(editor);
        model->setData(index, combo->currentText(), Qt::EditRole);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Lesson 10: Item Delegates");
    window.resize(700, 400);

    QVBoxLayout *layout = new QVBoxLayout(&window);

    QLabel *title = new QLabel("Custom Delegates Demo");
    QFont font = title->font();
    font.setPointSize(14);
    font.setBold(true);
    title->setFont(font);
    layout->addWidget(title);

    // Model with sample data
    QStandardItemModel *model = new QStandardItemModel(5, 3, &window);
    model->setHorizontalHeaderLabels({"Task", "Priority", "Progress"});

    QStringList tasks = {"Design UI", "Write Code", "Test App", "Documentation", "Deploy"};
    QStringList priorities = {"High", "Medium", "Low", "Medium", "Low"};
    QList<int> progress = {75, 50, 25, 10, 0};

    for (int row = 0; row < 5; ++row)
    {
        model->setItem(row, 0, new QStandardItem(tasks[row]));
        model->setItem(row, 1, new QStandardItem(priorities[row]));
        model->setItem(row, 2, new QStandardItem(QString::number(progress[row])));
    }

    // Table view
    QTableView *table = new QTableView;
    table->setModel(model);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);

    // Set custom delegates
    table->setItemDelegateForColumn(1, new PriorityDelegate(table));
    table->setItemDelegateForColumn(2, new ProgressDelegate(table));

    layout->addWidget(table);

    QLabel *instructions = new QLabel(
        "• Double-click Priority to see ComboBox editor\n"
        "• Double-click Progress to adjust with slider\n"
        "• Progress shows visual bar rendering");
    layout->addWidget(instructions);

    window.show();

    qDebug() << "Delegates demo started";

    return app.exec();
}
