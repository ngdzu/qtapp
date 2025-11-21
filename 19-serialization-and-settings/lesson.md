# Lesson 19: Serialization and Settings

## Learning Goals
- Parse and generate JSON with Qt
- Store application settings with QSettings
- Serialize custom data structures
- Handle configuration files
- Implement persistent user preferences

## Introduction

Qt provides robust tools for data persistence: `QJsonDocument` for JSON serialization and `QSettings` for storing application preferences. JSON is ideal for data exchange and configuration files, while QSettings automatically handles platform-specific storage (Registry on Windows, plist on macOS, config files on Linux).

These tools eliminate the need for manual file parsing and provide type-safe access to stored data.

## Key Concepts

### JSON Parsing

Read JSON data:

```cpp
QByteArray jsonData = "{"name":"Alice","age":30}";
QJsonDocument doc = QJsonDocument::fromJson(jsonData);
QJsonObject obj = doc.object();

QString name = obj["name"].toString();
int age = obj["age"].toInt();
```

### JSON Generation

Create JSON data:

```cpp
QJsonObject person;
person["name"] = "Bob";
person["age"] = 25;
person["active"] = true;

QJsonDocument doc(person);
QByteArray json = doc.toJson(QJsonDocument::Indented);
```

### JSON Arrays

Work with arrays:

```cpp
QJsonArray users;
QJsonObject user1;
user1["name"] = "Alice";
users.append(user1);

QJsonObject root;
root["users"] = users;
```

### QSettings Basics

Store and retrieve settings:

```cpp
// Write settings
QSettings settings("MyCompany", "MyApp");
settings.setValue("username", "alice");
settings.setValue("windowSize", QSize(800, 600));

// Read settings
QString username = settings.value("username").toString();
QSize size = settings.value("windowSize", QSize(640, 480)).toSize();
```

### Settings with Groups

Organize settings:

```cpp
settings.beginGroup("Network");
settings.setValue("timeout", 30);
settings.setValue("retries", 3);
settings.endGroup();

settings.beginGroup("UI");
settings.setValue("theme", "dark");
settings.endGroup();
```

### Default Values

Provide fallbacks:

```cpp
// Second argument is the default value
int timeout = settings.value("timeout", 60).toInt();
bool enabled = settings.value("feature/enabled", false).toBool();
```

## Example Walkthrough

Our example demonstrates:

1. **JSON Generation** - Creating user data as JSON
2. **JSON Parsing** - Reading and displaying JSON
3. **QSettings** - Saving/loading application preferences
4. **Persistence** - Settings survive app restarts

The application shows a form where you can enter data, save it as JSON, and persist preferences using QSettings.

## Expected Output

A window with:
- Text fields for entering user data
- "Save as JSON" button that generates JSON in the text area
- "Load JSON" button that parses and displays JSON
- Settings that persist between application runs
- Pretty-printed JSON output

## Try It

1. Build and run the application
2. Enter name and age, click "Generate JSON"
3. See the formatted JSON in the text area
4. Click "Save Settings" to persist preferences
5. Close and reopen - settings are restored!
6. Edit the JSON and click "Parse JSON" to see it loaded

## Key Takeaways

- `QJsonDocument` handles JSON parsing and generation
- `QJsonObject` and `QJsonArray` represent JSON structures
- `QSettings` provides platform-native preferences storage
- Always provide default values when reading settings
- JSON is human-readable and perfect for config files
- QSettings automatically syncs to disk
- Use `toJson(QJsonDocument::Indented)` for readable output
- Settings are stored per-application and per-company name
