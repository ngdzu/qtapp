# Lesson 24 Quiz Answers: Testing and Automation

## 1. What is the difference between `QVERIFY` and `QCOMPARE`, and when should you use each?

**Answer:** `QCOMPARE` compares two values and prints both on failure; `QVERIFY` only checks a boolean condition.

**Explanation:** `QVERIFY(actual == expected)` only tells you the test failed. `QCOMPARE(actual, expected)` shows "Actual: 5, Expected: 3" making debugging easier. Always prefer `QCOMPARE` when comparing values. Use `QVERIFY` only for pure boolean conditions like `QVERIFY(file.exists())`.

## 2. What is the purpose of the `_data()` function in data-driven testing?

**Answer:** It provides multiple sets of test data to run the same test function with different inputs.

**Explanation:** A test function like `testAddition()` can have a companion `testAddition_data()` that uses `QTest::addColumn()` and `QTest::newRow()` to define test cases. The test function then uses `QFETCH()` to retrieve each data row. This eliminates code duplication when testing the same logic with different inputs, and each row is reported separately in test output.

## 3. Consider this test code. What's wrong with it?

```cpp
void MyTest::testWidget() {
    QPushButton *btn = new QPushButton("Test");
    QVERIFY(btn->text() == "Test");
}
```

**Answer:** Memory leak - the button is never deleted.

**Explanation:** The button is allocated with `new` but never deleted, causing a memory leak. In Qt Test, either use stack allocation (`QPushButton btn("Test");`) or use a parent (`QPushButton *btn = new QPushButton("Test", this);`) or explicitly delete (`delete btn;`) in cleanup. The test will still pass functionally, but repeated test runs will leak memory. Better approach:

```cpp
void MyTest::testWidget() {
    QPushButton btn("Test");  // Stack allocation - auto cleanup
    QVERIFY(btn.text() == "Test");
}
```

## 4. How do you test that a signal was emitted exactly twice?

**Answer:** Use `QSignalSpy` and check `spy.count()`.

**Explanation:** `QSignalSpy` monitors signal emissions. Example:

```cpp
QPushButton button;
QSignalSpy spy(&button, &QPushButton::clicked);
button.click();
button.click();
QCOMPARE(spy.count(), 2);
```

`QSignalSpy` inherits `QList<QList<QVariant>>`, so you can also inspect signal arguments with `spy.at(0)` or `spy.last()`.

## 5. What do `init()` and `initTestCase()` do differently in Qt Test?

**Answer:** `initTestCase()` runs once before all tests; `init()` runs before each test.

**Explanation:** `initTestCase()` is for expensive one-time setup (database connections, loading files). `init()` runs before every test function, useful for resetting state. Similarly, `cleanup()` runs after each test and `cleanupTestCase()` runs once at the end. Order: `initTestCase()` → `init()` → test → `cleanup()` → `init()` → test → ... → `cleanupTestCase()`.

## 6. Write a data-driven test for a `divide(int a, int b)` function that should handle division by zero.

**Answer:**

```cpp
void TestCalculator::testDivide_data() {
    QTest::addColumn<int>("a");
    QTest::addColumn<int>("b");
    QTest::addColumn<int>("expected");
    QTest::addColumn<bool>("shouldThrow");
    
    QTest::newRow("normal") << 10 << 2 << 5 << false;
    QTest::newRow("negative") << -10 << 2 << -5 << false;
    QTest::newRow("division by zero") << 10 << 0 << 0 << true;
}

void TestCalculator::testDivide() {
    QFETCH(int, a);
    QFETCH(int, b);
    QFETCH(int, expected);
    QFETCH(bool, shouldThrow);
    
    if (shouldThrow) {
        QVERIFY_EXCEPTION_THROWN(divide(a, b), std::runtime_error);
    } else {
        QCOMPARE(divide(a, b), expected);
    }
}
```

**Explanation:** This demonstrates testing both normal cases and error conditions. The `shouldThrow` column indicates whether an exception is expected, allowing one test function to handle both success and failure paths.

## 7. Why might this benchmark give inconsistent results?

```cpp
void testPerformance() {
    QBENCHMARK {
        QString result;
        for (int i = 0; i < qrand() % 1000; i++)
            result += "x";
    }
}
```

**Answer:** The loop count is random, making each iteration different.

**Explanation:** `qrand() % 1000` produces a different value each run, so you're measuring different amounts of work. Benchmarks must be deterministic - measure the same operation every time. Also, `qrand()` is deprecated; use `QRandomGenerator`. Fixed version:

```cpp
void testPerformance() {
    QBENCHMARK {
        QString result;
        for (int i = 0; i < 1000; i++)  // Fixed count
            result += "x";
    }
}
```

## 8. How would you simulate typing "Hello" into a QLineEdit and verify the text?

**Answer:**

```cpp
QLineEdit lineEdit;
QTest::keyClicks(&lineEdit, "Hello");
QCOMPARE(lineEdit.text(), QString("Hello"));
```

**Explanation:** `QTest::keyClicks()` simulates typing each character with proper key press/release events. This is more realistic than `lineEdit.setText()` because it triggers the same signals and events as actual user typing. You can also use `QTest::keyClick()` for single keys with modifiers: `QTest::keyClick(&lineEdit, Qt::Key_A, Qt::ControlModifier)` for Ctrl+A.

## 9. What's the advantage of Qt Test's XML output format?

**Answer:** CI/CD tools can parse XML to display test results and trends.

**Explanation:** Use `-o results.xml -xunitxml` to generate JUnit-compatible XML. Jenkins, GitHub Actions, and GitLab CI can parse this format to show test results in the UI, track pass/fail trends over time, and fail builds when tests fail. The XML includes test names, timing, failure messages, and stack traces. Plain text output is fine locally, but XML is essential for automated pipelines.

## 10. In a real project, where should test files be located and how are they built?

**Answer:** Tests go in a `tests/` directory and are built as separate executables.

**Explanation:** Typical structure:

```
project/
  src/
    myclass.cpp
  tests/
    test_myclass.cpp
  CMakeLists.txt
```

CMakeLists.txt:

```cmake
enable_testing()
add_executable(test_myclass tests/test_myclass.cpp)
target_link_libraries(test_myclass Qt6::Test mylib)
add_test(NAME test_myclass COMMAND test_myclass)
```

Run with `ctest` or `make test`. Each test is a standalone executable that returns 0 on success, non-zero on failure. Keep tests separate from production code to avoid shipping test dependencies.
