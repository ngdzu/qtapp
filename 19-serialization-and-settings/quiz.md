# Lesson 19 Quiz: Serialization and Settings

1. What is the difference between QJsonDocument and QSettings?

2. How do you parse a JSON string in Qt?

3. What does this code do?
```cpp
QJsonObject obj;
obj["name"] = "Alice";
obj["age"] = 30;
QJsonDocument doc(obj);
QByteArray json = doc.toJson(QJsonDocument::Indented);
```

4. How do you read a setting with a default value?

5. What happens to QSettings data when you close the application?

6. How do you organize settings into groups?

7. Where does QSettings store data on macOS vs Windows?

8. How do you create a JSON array?

9. What's the difference between `toJson()` and `toJson(QJsonDocument::Indented)`?

10. How do you check if JSON parsing succeeded?
