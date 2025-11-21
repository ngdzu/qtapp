# Lesson 19 Quiz Answers

1. **What is the difference between QJsonDocument and QSettings?**

`QJsonDocument` is for JSON serialization - converting data to/from JSON format. `QSettings` is for storing application preferences persistently.

QJsonDocument works with JSON strings/files that you control. QSettings automatically manages platform-specific storage (Registry/plist/config files) and is designed for app preferences that persist across runs.

2. **How do you parse a JSON string in Qt?**

Use `QJsonDocument::fromJson()`:
```cpp
QByteArray jsonData = "{"name":"Alice"}";
QJsonDocument doc = QJsonDocument::fromJson(jsonData);
QJsonObject obj = doc.object();
QString name = obj["name"].toString();
```

3. **What does this code do?**

It creates a JSON object with name and age fields and converts it to a formatted JSON string.

The `Indented` argument produces pretty-printed JSON with newlines and indentation, making it human-readable. Without it, you'd get compact JSON: `{"name":"Alice","age":30}`

4. **How do you read a setting with a default value?**

Pass the default as the second argument to `value()`:
```cpp
QSettings settings;
int timeout = settings.value("timeout", 60).toInt();
QString theme = settings.value("theme", "light").toString();
```
If the setting doesn't exist, the default is returned.

5. **What happens to QSettings data when you close the application?**

It's automatically saved to disk and persists between application runs.

QSettings writes changes immediately (or on destruction). The data survives app restarts, reboots, and updates - it's permanent until explicitly deleted.

6. **How do you organize settings into groups?**

Use `beginGroup()` and `endGroup()`:
```cpp
settings.beginGroup("Network");
settings.setValue("timeout", 30);
settings.setValue("port", 8080);
settings.endGroup();
```
Groups create hierarchical organization like folders.

7. **Where does QSettings store data on macOS vs Windows?**

macOS: `~/Library/Preferences/com.MyCompany.MyApp.plist`
Windows: Registry at `HKEY_CURRENT_USER\Software\MyCompany\MyApp`
Linux: `~/.config/MyCompany/MyApp.conf`

QSettings handles this automatically based on organization/application names.

8. **How do you create a JSON array?**

Use `QJsonArray`:
```cpp
QJsonArray arr;
arr.append("item1");
arr.append(42);
arr.append(true);

QJsonObject obj;
obj["items"] = arr;
```

9. **What's the difference between `toJson()` and `toJson(QJsonDocument::Indented)`?**

`toJson()` produces compact JSON: `{"name":"Alice","age":30}`

`toJson(QJsonDocument::Indented)` produces formatted JSON:
```json
{
    "name": "Alice",
    "age": 30
}
```
Use Indented for config files and debugging.

10. **How do you check if JSON parsing succeeded?**

Check if the document is null:
```cpp
QJsonDocument doc = QJsonDocument::fromJson(jsonData);
if (doc.isNull()) {
    qDebug() << "Invalid JSON";
} else {
    // Parse succeeded
}
```
