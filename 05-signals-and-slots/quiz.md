# Quiz: Signals and Slots

## Question 1
What is the purpose of Qt's signals and slots mechanism?

## Question 2
Why must a class inherit from QObject and include the Q_OBJECT macro to use signals and slots?

## Question 3
What is the main advantage of the modern connection syntax over the old SIGNAL/SLOT macro syntax?

## Question 4
What will happen when this button is clicked?

```cpp
QPushButton *button = new QPushButton("Click");
int count = 0;
QObject::connect(button, &QPushButton::clicked, [&count]() {
    count++;
    qDebug() << count;
});
```

## Question 5
Spot the bug in this code:

```cpp
class MyClass : public QObject
{
public:
    explicit MyClass(QObject *parent = nullptr);
signals:
    void dataReady(int value);
public slots:
    void processData(int value);
};
```

## Question 6
Fill in the missing line to connect a slider's value change to a label:

```cpp
QSlider *slider = new QSlider();
QLabel *label = new QLabel("0");
// Missing line: connect slider valueChanged signal to update label text
```

## Question 7
What is the difference between Qt::DirectConnection and Qt::QueuedConnection?

## Question 8
Can one signal be connected to multiple slots? If yes, in what order are the slots called?

## Question 9
What does the `#include "main.moc"` line do when you have Q_OBJECT in a .cpp file?

## Question 10
How would you use signals and slots to implement a simple notification system where multiple UI elements need to update when data changes? Describe the design pattern you'd use.
