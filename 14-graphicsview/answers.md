# Lesson 14 Quiz Answers

1. **What is the difference between QGraphicsScene and QGraphicsView?**

QGraphicsScene is the container that holds graphics items. QGraphicsView is the widget that displays the scene.

The scene manages items and their coordinates, while the view provides visualization with support for transformations (zoom, rotate, scroll). You can have multiple views showing the same scene.

2. **How do you make a QGraphicsItem movable and selectable?**

Use `setFlag()` with the appropriate flags:
```cpp
item->setFlag(QGraphicsItem::ItemIsMovable);
item->setFlag(QGraphicsItem::ItemIsSelectable);
```

3. **What does this code do?**

It creates a blue movable rectangle in the scene.

The `addRect()` creates a 100x50 rectangle at position (0,0), and `ItemIsMovable` allows users to drag it with the mouse.

4. **Can you have multiple QGraphicsView instances showing the same QGraphicsScene?**

Yes! This is a key feature of the scene-view architecture.

Multiple views can display the same scene with different zoom levels or transformations, useful for detail/overview windows.

5. **What method enables antialiasing in a QGraphicsView?**

```cpp
view->setRenderHint(QPainter::Antialiasing);
```

This smooths edges of graphics items for better visual quality.

6. **How do you add a custom shape to a QGraphicsScene?**

Subclass `QGraphicsItem` and override `boundingRect()` and `paint()`:
```cpp
class CustomItem : public QGraphicsItem {
    QRectF boundingRect() const override;
    void paint(QPainter *painter, ...) override;
};
```
Then add to scene: `scene->addItem(new CustomItem());`

7. **What is the benefit of using Graphics View over custom painting with QPainter?**

Graphics View provides built-in item management, collision detection, hit testing, and transformations.

It handles complex scenarios like thousands of items efficiently, with automatic optimization and caching. Custom QPainter painting requires manual management of all these features.

8. **How do you respond to mouse clicks on a QGraphicsItem?**

Override `mousePressEvent()`:
```cpp
void CustomItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    qDebug() << "Item clicked!";
    QGraphicsItem::mousePressEvent(event);
}
```

9. **What does `scene->setSceneRect()` do?**

It defines the logical coordinate space of the scene.

This determines the scrollable area and helps the view manage its viewport. Items can exist outside this rect, but it affects scrollbar behavior.

10. **Name three types of predefined QGraphicsItem subclasses.**

(1) QGraphicsRectItem - rectangles
(2) QGraphicsEllipseItem - circles and ellipses  
(3) QGraphicsTextItem - editable text
Others include QGraphicsPixmapItem, QGraphicsPolygonItem, QGraphicsLineItem.
