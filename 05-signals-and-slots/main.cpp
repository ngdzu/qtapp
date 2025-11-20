#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

// Custom QObject class with signals and slots
class Counter : public QObject
{
    Q_OBJECT

public:
    explicit Counter(QObject *parent = nullptr) : QObject(parent), m_value(0) {}

    int value() const { return m_value; }

signals:
    void valueChanged(int newValue);
    void thresholdReached(int threshold);

public slots:
    void increment()
    {
        m_value++;
        qDebug() << "Counter incremented to:" << m_value;
        emit valueChanged(m_value);

        // Emit threshold signal at specific values
        if (m_value == 5 || m_value == 10)
        {
            emit thresholdReached(m_value);
        }
    }

    void reset()
    {
        m_value = 0;
        qDebug() << "Counter reset";
        emit valueChanged(m_value);
    }

private:
    int m_value;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Signals and Slots Demo");
    window.resize(350, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Create counter object
    Counter counter;

    // Display label for counter value
    QLabel *valueLabel = new QLabel("Counter: 0");
    valueLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(valueLabel);

    // Status label for messages
    QLabel *statusLabel = new QLabel("Click increment to start");
    mainLayout->addWidget(statusLabel);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *incButton = new QPushButton("Increment");
    QPushButton *resetButton = new QPushButton("Reset");
    buttonLayout->addWidget(incButton);
    buttonLayout->addWidget(resetButton);
    mainLayout->addWidget(new QWidget); // Spacer
    mainLayout->addLayout(buttonLayout);

    // Connection 1: Button to counter slot (modern syntax)
    QObject::connect(incButton, &QPushButton::clicked, &counter, &Counter::increment);
    QObject::connect(resetButton, &QPushButton::clicked, &counter, &Counter::reset);

    // Connection 2: Counter signal to lambda updating label
    QObject::connect(&counter, &Counter::valueChanged, [valueLabel](int newValue)
                     {
        valueLabel->setText(QString("Counter: %1").arg(newValue));
        qDebug() << "Lambda: Updated label to" << newValue; });

    // Connection 3: Counter signal to another lambda for status updates
    QObject::connect(&counter, &Counter::valueChanged, [statusLabel](int newValue)
                     {
        if (newValue == 0) {
            statusLabel->setText("Counter reset to zero");
        } else {
            statusLabel->setText(QString("Incremented %1 time%2")
                                    .arg(newValue)
                                    .arg(newValue == 1 ? "" : "s"));
        } });

    // Connection 4: Threshold signal to lambda
    QObject::connect(&counter, &Counter::thresholdReached, [statusLabel](int threshold)
                     {
        statusLabel->setText(QString("🎉 Milestone! Reached %1!").arg(threshold));
        statusLabel->setStyleSheet("color: green; font-weight: bold;");
        qDebug() << "Threshold reached:" << threshold; });

    // Connection 5: Demonstrate queued connection
    // In same thread, this behaves like direct, but shows the syntax
    QObject::connect(&counter, &Counter::valueChanged, &counter, [](int value)
                     { qDebug() << "Queued connection handler, value:" << value; }, Qt::QueuedConnection);

    window.show();
    return app.exec();
}

#include "main.moc" // Required for Q_OBJECT in .cpp file
