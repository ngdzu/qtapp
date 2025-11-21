# Lesson 12: Qt Quick/QML Introduction

## Learning Goals
- Understand QML declarative syntax
- Create basic Qt Quick applications
- Integrate QML with C++ backend
- Use QtQuick Controls
- Handle mouse and keyboard input in QML

## Introduction

Qt Quick is Qt's declarative UI framework using QML (Qt Modeling Language). Unlike Widgets, QML describes what the UI should look like, not how to create it. Perfect for modern, animated, touch-friendly interfaces.

## Basic QML Syntax

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "QML App"

    Rectangle {
        anchors.fill: parent
        color: "lightblue"

        Text {
            anchors.centerIn: parent
            text: "Hello QML!"
            font.pixelSize: 24
        }
    }
}
```

## Property Bindings

```qml
Rectangle {
    id: rect
    width: 200
    height: width  // Automatically updates
    color: mouseArea.containsMouse ? "red" : "blue"

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }
}
```

## Signals and Handlers

```qml
Button {
    text: "Click Me"
    onClicked: {
        console.log("Button clicked")
        rect.color = "green"
    }
}
```

## C++ Integration

```cpp
// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
```

## Key Takeaways
- QML is declarative, not imperative
- Property bindings update automatically
- Integrates with C++ for backend logic
- Great for modern UIs and animations
- Scene graph rendering for performance
