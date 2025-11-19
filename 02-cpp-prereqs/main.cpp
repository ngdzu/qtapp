#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <memory>
#include <vector>

class Resource
{
public:
    Resource(const QString &name) : m_name(name)
    {
        qDebug() << "Resource created:" << m_name;
    }
    ~Resource()
    {
        qDebug() << "Resource destroyed:" << m_name;
    }
    QString name() const { return m_name; }

private:
    QString m_name;
};

QString createMessage()
{
    QString msg = "This is a message that will be moved, not copied";
    qDebug() << "Message created, will be moved";
    return msg; // RVO or move, not copy
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "\n=== Demonstrating Modern C++ with Qt ===\n";

    // 1. RAII: Automatic cleanup via scope
    qDebug() << "1. RAII - Resources cleaned up automatically:";
    {
        Resource r1("ScopedResource");
        qDebug() << "  Inside scope, resource exists";
    } // r1 destroyed here automatically
    qDebug() << "  Outside scope, resource destroyed\n";

    // 2. Smart pointers for non-QObject data
    qDebug() << "2. Smart pointers:";
    auto data = std::make_unique<std::vector<int>>(5, 42);
    qDebug() << "  Created unique_ptr with vector, size:" << data->size();
    qDebug() << "  No manual delete needed\n";

    // 3. Lambda expressions with captures
    qDebug() << "3. Lambda expressions:";

    int clickCount = 0;
    auto incrementCounter = [&clickCount]()
    {
        clickCount++;
        qDebug() << "  Counter incremented! Count:" << clickCount;
    };

    // Call the lambda multiple times
    qDebug() << "  Calling lambda function...";
    incrementCounter();
    incrementCounter();
    incrementCounter();
    qDebug() << "";

    // 4. Move semantics
    qDebug() << "4. Move semantics:";
    QString msg = createMessage();
    qDebug() << "  Message received (moved):" << msg.left(30) << "...";

    QStringList list;
    list.append(std::move(msg)); // msg is now empty/moved-from
    qDebug() << "  After move, original msg is empty:" << (msg.isEmpty() ? "true" : "false");
    qDebug() << "  List contains:" << list.first().left(30) << "...\n";

    qDebug() << "=== All demonstrations complete ===";
    qDebug() << "Note: Automatic cleanup will happen when app exits\n";

    return 0;
}
