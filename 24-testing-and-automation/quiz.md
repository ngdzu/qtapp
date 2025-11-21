# Lesson 24 Quiz: Testing and Automation

1. What is the difference between `QVERIFY` and `QCOMPARE`, and when should you use each?

2. What is the purpose of the `_data()` function in data-driven testing?

3. Consider this test code. What's wrong with it?
```cpp
void MyTest::testWidget() {
    QPushButton *btn = new QPushButton("Test");
    QVERIFY(btn->text() == "Test");
}
```

4. How do you test that a signal was emitted exactly twice?

5. What do `init()` and `initTestCase()` do differently in Qt Test?

6. Write a data-driven test for a `divide(int a, int b)` function that should handle division by zero.

7. Why might this benchmark give inconsistent results?
```cpp
void testPerformance() {
    QBENCHMARK {
        QString result;
        for (int i = 0; i < qrand() % 1000; i++)
            result += "x";
    }
}
```

8. How would you simulate typing "Hello" into a QLineEdit and verify the text?

9. What's the advantage of Qt Test's XML output format?

10. In a real project, where should test files be located and how are they built?
