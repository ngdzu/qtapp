
#include <QApplication>

class CustomApplication : public QApplication
{
    using QApplication::QApplication;

protected:
    bool notify(QObject *receiver, QEvent *event) override;
};