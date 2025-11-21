#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QTabWidget>
#include <QElapsedTimer>

class SignalCounter : public QObject {
    Q_OBJECT
public:
    SignalCounter(QObject *parent = nullptr) : QObject(parent), count(0) {}
    int getCount() const { return count; }
    
public slots:
    void onSignal() { count++; }
    void onTextChanged(const QString &text) { 
        count++; 
        lastText = text;
    }
    QString getLastText() const { return lastText; }
    
private:
    int count;
    QString lastText;
};

class TestDemoWidget : public QWidget {
    Q_OBJECT
public:
    TestDemoWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
    }

private slots:
    void runAssertionTests() {
        output->clear();
        output->append("<b>Running Assertion Tests...</b><br>");
        
        output->append("<b>1. QVERIFY Test:</b>");
        bool condition = (2 + 2 == 4);
        if (condition) {
            output->append("  ✓ QVERIFY(2 + 2 == 4) - PASS");
        }
        
        output->append("<br><b>2. QCOMPARE Test:</b>");
        int actual = 10;
        int expected = 10;
        if (actual == expected) {
            output->append(QString("  ✓ QCOMPARE(%1, %2) - PASS").arg(actual).arg(expected));
        }
        
        output->append("<br><b>All assertion tests completed!</b>");
    }
    
    void runDataDrivenTests() {
        output->clear();
        output->append("<b>Running Data-Driven Tests...</b><br>");
        
        struct TestData {
            int a, b, expected;
            QString name;
        };
        
        QList<TestData> testData = {
            {2, 3, 5, "positive numbers"},
            {-1, -2, -3, "negative numbers"}
        };
        
        for (const auto &data : testData) {
            int result = data.a + data.b;
            if (result == data.expected) {
                output->append(QString("  ✓ Row '%1': %2 + %3 = %4 - PASS")
                    .arg(data.name).arg(data.a).arg(data.b).arg(result));
            }
        }
    }
    
    void runGUITests() {
        output->clear();
        output->append("<b>Running GUI Tests...</b><br>");
        testButton->click();
        output->append("  ✓ Button click simulation - PASS");
    }
    
    void runSignalTests() {
        output->clear();
        output->append("<b>Running Signal Tests...</b><br>");
        SignalCounter counter;
        connect(testButton, &QPushButton::clicked, &counter, &SignalCounter::onSignal);
        testButton->click();
        testButton->click();
        if (counter.getCount() == 2) {
            output->append("  ✓ Signal emitted 2 times - PASS");
        }
    }

private:
    void setupUI() {
        setWindowTitle("Lesson 24: Testing and Automation");
        resize(700, 500);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QLabel *title = new QLabel("<h2>Qt Test Framework Demo</h2>");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);
        
        QTabWidget *tabs = new QTabWidget();
        
        QWidget *assertTab = new QWidget();
        QVBoxLayout *assertLayout = new QVBoxLayout(assertTab);
        QPushButton *runAssert = new QPushButton("Run Assertion Tests");
        connect(runAssert, &QPushButton::clicked, this, &TestDemoWidget::runAssertionTests);
        assertLayout->addWidget(runAssert);
        tabs->addTab(assertTab, "Assertions");
        
        QWidget *dataTab = new QWidget();
        QVBoxLayout *dataLayout = new QVBoxLayout(dataTab);
        QPushButton *runData = new QPushButton("Run Data-Driven Tests");
        connect(runData, &QPushButton::clicked, this, &TestDemoWidget::runDataDrivenTests);
        dataLayout->addWidget(runData);
        tabs->addTab(dataTab, "Data-Driven");
        
        QWidget *signalTab = new QWidget();
        QVBoxLayout *signalLayout = new QVBoxLayout(signalTab);
        QPushButton *runSignal = new QPushButton("Run Signal Tests");
        connect(runSignal, &QPushButton::clicked, this, &TestDemoWidget::runSignalTests);
        signalLayout->addWidget(runSignal);
        tabs->addTab(signalTab, "Signals");
        
        mainLayout->addWidget(tabs);
        
        output = new QTextEdit();
        output->setReadOnly(true);
        mainLayout->addWidget(output);
        
        testButton = new QPushButton("Test Button");
    }
    
    QTextEdit *output;
    QPushButton *testButton;
    QLineEdit *testLineEdit;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestDemoWidget demo;
    demo.show();
    return app.exec();
}

#include "main.moc"
