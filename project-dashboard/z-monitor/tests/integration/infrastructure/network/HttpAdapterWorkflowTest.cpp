#include <gtest/gtest.h>
#include <QtCore/QCoreApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QUrl>

#include "src/infrastructure/network/HttpTelemetryServerAdapter.h"

class SimpleHttpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit SimpleHttpServer(QObject *parent = nullptr) : QTcpServer(parent) {}

    void setResponses(const QList<QByteArray> &responses) { m_responses = responses; }

protected:
    void incomingConnection(qintptr handle) override
    {
        QTcpSocket *socket = new QTcpSocket(this);
        socket->setSocketDescriptor(handle);
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]()
                {
            socket->readAll();
            if (m_index < m_responses.size()) {
                socket->write(m_responses[m_index++]);
            } else {
                socket->write("HTTP/1.1 200 OK\r\nContent-Length:0\r\n\r\n");
            }
            socket->flush();
            socket->disconnectFromHost(); });
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }

private:
    QList<QByteArray> m_responses;
    int m_index{0};
};

TEST(HttpAdapterWorkflowTest, ReturnsFalseOn500ThenTrueOn200)
{
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    SimpleHttpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));

    QList<QByteArray> responses;
    responses << QByteArray("HTTP/1.1 500 Internal Server Error\r\nContent-Length:0\r\n\r\n");
    responses << QByteArray("HTTP/1.1 200 OK\r\nContent-Length:0\r\n\r\n");
    server.setResponses(responses);

    QUrl endpoint(QString("http://127.0.0.1:%1/telemetry").arg(server.serverPort()));
    HttpTelemetryServerAdapter adapter(endpoint);
    adapter.setTimeoutMs(1000);

    QByteArray payload("compressed-batch");
    QString err;

    bool ok1 = adapter.upload(payload, err);
    EXPECT_FALSE(ok1);

    QString err2;
    bool ok2 = adapter.upload(payload, err2);
    EXPECT_TRUE(ok2);
}

#include "HttpAdapterWorkflowTest.moc"
