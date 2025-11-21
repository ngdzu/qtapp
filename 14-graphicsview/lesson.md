# Lesson 14: Graphics View Framework

## Learning Goals
- Understand QGraphicsScene and QGraphicsView architecture
- Create and manipulate graphical items
- Implement drag-and-drop and transformations
- Handle user interaction with graphics items

## Introduction

The Graphics View framework provides a surface for managing and interacting with large numbers of custom 2D graphical items. It uses a scene-view architecture where `QGraphicsScene` holds the items, and `QGraphicsView` displays them with support for zooming, rotating, and scrolling.

This architecture is ideal for applications like diagram editors, games, or any scenario requiring interactive 2D graphics.

## Key Concepts

### Scene-View Architecture

- **QGraphicsScene**: Container for graphics items
- **QGraphicsView**: Viewport that renders the scene
- **QGraphicsItem**: Base class for items in the scene

Multiple views can display the same scene simultaneously.

### Creating a Basic Scene

```cpp
QGraphicsScene *scene = new QGraphicsScene();
QGraphicsView *view = new QGraphicsView(scene);

// Add items
scene->addRect(0, 0, 100, 100, QPen(Qt::black), QBrush(Qt::blue));
scene->addEllipse(50, 50, 80, 80, QPen(Qt::red));
```

### Item Flags

Items can have flags that control behavior:

```cpp
item->setFlag(QGraphicsItem::ItemIsMovable);
item->setFlag(QGraphicsItem::ItemIsSelectable);
```

## Example Walkthrough

Our example creates:
1. A scene with multiple shapes (rectangles, ellipses, polygons)
2. Movable and selectable items
3. Different colored shapes with pens and brushes
4. Mouse interaction demonstration

The main window shows how to set up the scene, add items, and configure the view.

## Expected Output

A window displaying:
- A blue rectangle
- A red circle
- A green triangle
- All items are draggable and selectable
- Click and drag items to move them
- Click on items to select them (they'll highlight)

## Try It

1. Build and run the application
2. Click and drag shapes around
3. Select different shapes by clicking
4. Try selecting multiple shapes with Ctrl+click
5. Experiment with moving shapes to different positions

## Key Takeaways

- Graphics View provides efficient 2D graphics management
- Scene-view separation enables multiple views of same data
- Items support transformations, selection, and interaction
- Ideal for diagram editors, games, and visualization tools
- Hardware acceleration available through OpenGL backend
