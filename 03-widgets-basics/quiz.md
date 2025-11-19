# Quiz: Qt Widgets Basics

## Question 1
What is the purpose of the QWidget class in Qt?

## Question 2
What happens if you create a QWidget without specifying a parent widget?

## Question 3
Why is the event loop important in Qt GUI applications?

## Question 4
What is the output of the following code when you click the button twice?

```cpp
QLabel *label = new QLabel("Count: 0");
QPushButton *button = new QPushButton("Click");
int count = 0;
QObject::connect(button, &QPushButton::clicked, [&]() {
    count++;
    label->setText(QString("Count: %1").arg(count));
});
```

## Question 5
Spot the bug in this code:

```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget window;
    window.setWindowTitle("My App");
    window.show();
    // Missing something here
}
```

## Question 6
Fill in the missing line to connect a button click to show "Hello!" in a label:

```cpp
QPushButton *button = new QPushButton("Say Hello");
QLabel *label = new QLabel("");
// Missing line here
```

## Question 7
What is the difference between QPushButton and QLabel in terms of user interaction?

## Question 8
What does `setGeometry(50, 100, 200, 30)` do to a widget?

## Question 9
Why do widgets created with `new` and a parent pointer not require manual `delete`?

## Question 10
How would you use QPushButton and QLabel in a simple calculator application? Describe one possible use case.
