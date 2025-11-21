# Lesson 24: Testing and Automation

## Learning Goals

By the end of this lesson, you will:
- Understand Qt Test framework for unit and integration testing
- Learn how to write test cases using `QTest` macros
- Know how to simulate user interactions with `QTest::mouseClick()` and `QTest::keyClick()`
- Understand test data-driven testing with `_data()` functions
- Learn benchmarking with `QBENCHMARK`

## Introduction to Qt Test

Qt Test is a powerful framework for unit testing and GUI testing built into Qt. It provides macros for assertions, data-driven tests, benchmarking, and GUI event simulation. Unlike external frameworks like Google Test, Qt Test integrates seamlessly with Qt's meta-object system and signals/slots.

The framework is lightweight, header-only (just `#include <QTest>`), and produces detailed XML output compatible with CI/CD systems like Jenkins and GitHub Actions.

### Basic Test Structure

A Qt test case is a class inheriting from `QObject` with test functions as private slots:

```cpp
class MyTest : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() { /* runs once before all tests */ }
    void init() { /* runs before each test */ }
    void testAddition() { QVERIFY(2 + 2 == 4); }
    void cleanup() { /* runs after each test */ }
    void cleanupTestCase() { /* runs once after all tests */ }
};

QTEST_MAIN(MyTest)
```

The `QTEST_MAIN` macro generates a `main()` function that runs all test slots.

### Assertion Macros

Qt Test provides several assertion macros:

```cpp
QVERIFY(condition);              // Verify condition is true
QCOMPARE(actual, expected);      // Compare values with ==
QVERIFY2(condition, "message");  // Verify with custom message
QFAIL("message");                // Force test failure
QSKIP("reason");                 // Skip this test
```

`QCOMPARE` is preferred over `QVERIFY` because it prints both actual and expected values on failure.

### Data-Driven Testing

Test data can be supplied using a `_data()` companion function:

```cpp
void testAddition_data() {
    QTest::addColumn<int>("a");
    QTest::addColumn<int>("b");
    QTest::addColumn<int>("result");
    
    QTest::newRow("positive") << 2 << 3 << 5;
    QTest::newRow("negative") << -1 << -2 << -3;
    QTest::newRow("zero") << 0 << 5 << 5;
}

void testAddition() {
    QFETCH(int, a);
    QFETCH(int, b);
    QFETCH(int, result);
    QCOMPARE(a + b, result);
}
```

This runs `testAddition()` three times with different data rows.

## GUI Testing

Qt Test can simulate user interactions:

```cpp
QPushButton button("Click Me");
QTest::mouseClick(&button, Qt::LeftButton);
QVERIFY(button.isDown() == false);

QLineEdit lineEdit;
QTest::keyClicks(&lineEdit, "Hello");
QCOMPARE(lineEdit.text(), QString("Hello"));
```

For signal testing:

```cpp
QSignalSpy spy(&button, &QPushButton::clicked);
button.click();
QCOMPARE(spy.count(), 1);
```

## Example Walkthrough

Our demo application shows a testing dashboard that demonstrates:

1. **Assertion Examples** - Shows QVERIFY, QCOMPARE, and other macros
2. **Data-Driven Tests** - Demonstrates parameterized testing
3. **GUI Testing** - Simulates button clicks and input
4. **Signal Spy** - Monitors signal emissions
5. **Benchmarking** - Measures performance with QBENCHMARK

The app provides an interactive way to understand testing concepts without requiring a separate test executable. In real projects, tests are separate executables run via `ctest` or `make test`.

## Expected Output

The application displays a window with multiple tabs:
- **Assertions** tab showing test macro examples
- **GUI Test** tab with interactive widgets that can be tested
- **Signals** tab demonstrating QSignalSpy
- Output area showing test results in real-time

Click "Run All Tests" to see assertions in action, or use individual tabs to explore specific testing features.

## Try It

**Exercise 1**: Create a simple Calculator class with `add()`, `subtract()`, `multiply()`, `divide()` methods. Write a test class with data-driven tests for each operation.

**Exercise 2**: Build a QLineEdit validator test. Test that email addresses are validated correctly using QSignalSpy to monitor the `textChanged` signal.

**Exercise 3**: Write a benchmark comparing `QString::toUpper()` vs manual character conversion. Use `QBENCHMARK` to measure performance.

## Key Takeaways

- Qt Test provides a complete testing framework integrated with Qt
- Test classes inherit `QObject` with test functions as private slots
- Use `QCOMPARE` instead of `QVERIFY` for better failure messages
- Data-driven tests reduce code duplication and improve test coverage
- `QTest::mouseClick()` and `QTest::keyClicks()` simulate user interactions
- `QSignalSpy` monitors signal emissions for signal/slot testing
- `QBENCHMARK` enables performance regression testing
- Tests should be fast, isolated, and deterministic
