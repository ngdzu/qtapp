#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QBrush>
#include <QPen>
#include <QPolygonF>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Create scene
    QGraphicsScene *scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, 400, 300);

    // Add a blue rectangle
    QGraphicsRectItem *rect = scene->addRect(50, 50, 100, 80, 
                                              QPen(Qt::black), 
                                              QBrush(Qt::blue));
    rect->setFlag(QGraphicsItem::ItemIsMovable);
    rect->setFlag(QGraphicsItem::ItemIsSelectable);

    // Add a red circle
    QGraphicsEllipseItem *ellipse = scene->addEllipse(200, 50, 80, 80,
                                                       QPen(Qt::darkRed, 2),
                                                       QBrush(Qt::red));
    ellipse->setFlag(QGraphicsItem::ItemIsMovable);
    ellipse->setFlag(QGraphicsItem::ItemIsSelectable);

    // Add a green triangle
    QPolygonF triangle;
    triangle << QPointF(100, 200) << QPointF(150, 250) << QPointF(50, 250);
    QGraphicsPolygonItem *poly = scene->addPolygon(triangle,
                                                    QPen(Qt::darkGreen, 2),
                                                    QBrush(Qt::green));
    poly->setFlag(QGraphicsItem::ItemIsMovable);
    poly->setFlag(QGraphicsItem::ItemIsSelectable);

    // Create view
    QGraphicsView *view = new QGraphicsView(scene);
    view->setWindowTitle("Lesson 14: Graphics View Framework");
    view->setRenderHint(QPainter::Antialiasing);
    view->resize(450, 350);
    view->show();

    return app.exec();
}
