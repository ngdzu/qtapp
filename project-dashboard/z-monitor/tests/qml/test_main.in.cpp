#include <QtQuickTest>
#include <QApplication>
#include <QLibraryInfo>
#include <QByteArray>

// QtCharts requires QApplication (not just QGuiApplication) because it uses QGraphicsScene
// Ensure QML can locate Qt's QML modules (e.g., QtCharts) by setting QML2_IMPORT_PATH
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // Add Qt's built-in QML import path so QML modules like QtCharts are found
    const QString qtQmlPath = QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
    QByteArray current = qgetenv("QML2_IMPORT_PATH");
    QByteArray updated;
    if (!current.isEmpty())
    {
        updated = current + ":" + qtQmlPath.toUtf8();
    }
    else
    {
        updated = qtQmlPath.toUtf8();
    }
    qputenv("QML2_IMPORT_PATH", updated);

    QTEST_SET_MAIN_SOURCE_PATH
    return quick_test_main(argc, argv, "@TEST_NAME@", "@SOURCE_DIR@");
}
