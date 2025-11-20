# Quiz: Layouts and Containers

## Question 1
What is the main advantage of using Qt layouts over manual widget positioning with `setGeometry()`?

## Question 2
Which layout would you use to arrange three buttons side-by-side horizontally?

## Question 3
What does the following code create?

```cpp
QGridLayout *layout = new QGridLayout();
layout->addWidget(new QLabel("Username:"), 0, 0);
layout->addWidget(new QLineEdit(), 0, 1);
layout->addWidget(new QLabel("Password:"), 1, 0);
layout->addWidget(new QLineEdit(), 1, 1);
```

## Question 4
Spot the bug in this code:

```cpp
QWidget *window = new QWidget();
QVBoxLayout *layout = new QVBoxLayout();
layout->addWidget(new QPushButton("Button 1"));
layout->addWidget(new QPushButton("Button 2"));
window->show();
```

## Question 5
Fill in the missing line to create a button that spans 2 columns in a grid layout:

```cpp
QGridLayout *layout = new QGridLayout();
QPushButton *wideButton = new QPushButton("Wide Button");
// Missing line here - should place button at row 0, column 0, spanning 1 row and 2 columns
```

## Question 6
What is the purpose of size policies in Qt layouts?

## Question 7
If you want to add empty space that expands to push widgets apart in a layout, what would you use?

## Question 8
What happens when you call `layout->setSpacing(20)` on a QHBoxLayout?

## Question 9
Why would you use `QGroupBox` when working with layouts?

## Question 10
How would you use layouts to create a typical login form with username/password fields and a submit button? Describe which layouts you'd use and how you'd organize them.
