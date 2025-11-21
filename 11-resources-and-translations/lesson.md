# Lesson 11: Resources and Qt Resource System

## Learning Goals
- Master Qt Resource System (.qrc files)
- Embed images, icons, and files in executables
- Access resources with :/ prefix
- Use resources in stylesheets and QML
- Understand compilation into binary

## Introduction

Qt's Resource System embeds files directly into your application's executable. Images, icons, QML files, and other assets become part of the binary, simplifying deployment and ensuring resources are always available.

## QRC Files

Define resources in .qrc XML files:

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/images">
        <file>logo.png</file>
        <file>icon.png</file>
    </qresource>
    <qresource prefix="/styles">
        <file>app.qss</file>
    </qresource>
</RCC>
```

## Accessing Resources

Use :/ prefix to access embedded resources:

```cpp
QPixmap logo(":/images/logo.png");
QIcon icon(":/images/icon.png");

// Load stylesheet
QFile file(":/styles/app.qss");
file.open(QFile::ReadOnly);
QString styleSheet = file.readAll();
app.setStyleSheet(styleSheet);
```

## CMake Integration

```cmake
set(CMAKE_AUTORCC ON)
qt_add_resources(RESOURCES resources.qrc)
add_executable(myapp main.cpp ${RESOURCES})
```

## Key Takeaways
- Resources are compiled into binary
- Access with :/ prefix
- Simplifies deployment
- Works in stylesheets and QML
- No runtime file dependencies
