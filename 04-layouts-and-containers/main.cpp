#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Main window
    QWidget window;
    window.setWindowTitle("Qt Layouts Demo");
    window.resize(400, 400);

    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Section 1: Horizontal Layout Demo
    QGroupBox *hBoxGroup = new QGroupBox("Horizontal Layout (QHBoxLayout)");
    QHBoxLayout *hBoxLayout = new QHBoxLayout();
    hBoxLayout->addWidget(new QPushButton("Left"));
    hBoxLayout->addWidget(new QPushButton("Center"));
    hBoxLayout->addWidget(new QPushButton("Right"));
    hBoxGroup->setLayout(hBoxLayout);
    mainLayout->addWidget(hBoxGroup);

    // Section 2: Vertical Layout Demo
    QGroupBox *vBoxGroup = new QGroupBox("Vertical Layout (QVBoxLayout)");
    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    vBoxLayout->addWidget(new QLabel("Name:"));
    vBoxLayout->addWidget(new QLineEdit());
    vBoxLayout->addWidget(new QLabel("Email:"));
    vBoxLayout->addWidget(new QLineEdit());
    vBoxLayout->addWidget(new QPushButton("Submit"));
    vBoxGroup->setLayout(vBoxLayout);
    mainLayout->addWidget(vBoxGroup);

    // Section 3: Grid Layout Demo
    QGroupBox *gridGroup = new QGroupBox("Grid Layout (QGridLayout)");
    QGridLayout *gridLayout = new QGridLayout();

    // Create a 3x3 grid of number buttons (like a calculator keypad)
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            int number = row * 3 + col + 1;
            QPushButton *button = new QPushButton(QString::number(number));
            gridLayout->addWidget(button, row, col);
        }
    }

    // Add a button spanning all columns in the last row
    QPushButton *zeroButton = new QPushButton("0");
    gridLayout->addWidget(zeroButton, 3, 0, 1, 3); // row, col, rowSpan, colSpan

    gridGroup->setLayout(gridLayout);
    mainLayout->addWidget(gridGroup);

    window.show();
    return app.exec();
}
