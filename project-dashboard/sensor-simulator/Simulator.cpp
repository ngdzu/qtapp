#include "Simulator.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QMetaObject>
#include <cstdlib>

Simulator::Simulator(QObject *parent)
    : QObject(parent)
{
    connect(&m_telemetryTimer, &QTimer::timeout, this, &Simulator::sendTelemetry);
    // Send telemetry several times per second (e.g. 5Hz => 200ms)
    m_telemetryTimer.start(200);

    connect(&m_demoTimer, &QTimer::timeout, this, [this]()
            {
        ++m_demoStep;
        if (m_demoStep == 1) {
            triggerCritical();
        } else if (m_demoStep == 2) {
            triggerNotification("Alarm acknowledged, switching to warning");
        } else if (m_demoStep == 3) {
            triggerWarning();
        } else {
            m_demoTimer.stop();
            m_demoStep = 0;
        } });
}

Simulator::~Simulator()
{
    if (m_server)
    {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
}

void Simulator::startServer(quint16 port)
{
    if (m_server)
        return;

    m_server = new QWebSocketServer(QStringLiteral("SensorSimulator"), QWebSocketServer::NonSecureMode, this);
    if (m_server->listen(QHostAddress::Any, port))
    {
        qDebug() << "SensorSimulator: listening on port" << port;
        connect(m_server, &QWebSocketServer::newConnection, this, &Simulator::onNewConnection);
    }
    else
    {
        qWarning() << "SensorSimulator: failed to listen on port" << port;
    }
}

void Simulator::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();
    m_clients.insert(socket);
    connect(socket, &QWebSocket::textMessageReceived, this, &Simulator::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected, this, &Simulator::onClientDisconnected);
    qDebug() << "SensorSimulator: client connected";
}

void Simulator::onTextMessageReceived(const QString &message)
{
    qDebug() << "SensorSimulator: received message:" << message;
    // For now, ignore control messages from clients
}

void Simulator::onClientDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket)
    {
        m_clients.remove(socket);
        socket->deleteLater();
        qDebug() << "SensorSimulator: client disconnected";
    }
}

void Simulator::sendTelemetry()
{
    QJsonObject packet;
    packet["type"] = "vitals";
    packet["timestamp_ms"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    packet["hr"] = m_hr;
    packet["spo2"] = m_spo2;
    packet["rr"] = m_rr;

    // simple waveform chunk (small sample set)
    QJsonObject waveform;
    waveform["channel"] = "ecg";
    waveform["sample_rate"] = 250;
    waveform["start_timestamp_ms"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    QJsonArray values;
    for (int i = 0; i < 40; ++i)
    {
        double v = 1000.0 * sin((m_telemetryTimer.interval() + i) * 0.1) + (static_cast<int>(QRandomGenerator::global()->bounded(50)) - 25);
        values.append(static_cast<int>(v));
    }
    waveform["values"] = values;
    packet["waveform"] = waveform;

    QJsonDocument doc(packet);
    QString text = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    for (QWebSocket *client : qAsConst(m_clients))
    {
        client->sendTextMessage(text);
    }

    // Emit a debug/info log for UI visibility
    emit logEmitted("Debug", QString("Sent telemetry: hr=%1 spo2=%2 rr=%3").arg(m_hr).arg(m_spo2).arg(m_rr));
}

void Simulator::triggerCritical()
{
    qDebug() << "Simulator: triggerCritical";
    emit alarmTriggered("Critical");
    emit logEmitted("Critical", "Critical alarm triggered");

    QJsonObject msg;
    msg["type"] = "alarm";
    msg["level"] = "critical";
    msg["timestamp_ms"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    QJsonDocument d(msg);
    QString t = QString::fromUtf8(d.toJson(QJsonDocument::Compact));
    for (QWebSocket *client : qAsConst(m_clients))
        client->sendTextMessage(t);
}

void Simulator::triggerWarning()
{
    qDebug() << "Simulator: triggerWarning";
    emit alarmTriggered("Warning");
    emit logEmitted("Warning", "Warning alarm triggered");
    QJsonObject msg;
    msg["type"] = "alarm";
    msg["level"] = "warning";
    msg["timestamp_ms"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    QJsonDocument d(msg);
    QString t = QString::fromUtf8(d.toJson(QJsonDocument::Compact));
    for (QWebSocket *client : qAsConst(m_clients))
        client->sendTextMessage(t);
}

void Simulator::triggerNotification(const QString &text)
{
    qDebug() << "Simulator: triggerNotification" << text;
    emit notification(text);
    QJsonObject msg;
    msg["type"] = "notification";
    msg["text"] = text;
    msg["timestamp_ms"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    QJsonDocument d(msg);
    QString t = QString::fromUtf8(d.toJson(QJsonDocument::Compact));
    for (QWebSocket *client : qAsConst(m_clients))
        client->sendTextMessage(t);

    emit logEmitted("Info", text);
}

void Simulator::playDemo()
{
    if (!m_demoTimer.isActive())
    {
        m_demoStep = 0;
        m_demoTimer.start(1500);
    }
}

void Simulator::quitApp()
{
    qDebug() << "Simulator: quitApp() invoked from QML";
    // Ask Qt to quit normally
    QCoreApplication::quit();
    // Fallback: if the Qt event loop doesn't exit (containerized environment
    // or unexpected blocking), force process termination so the container can stop.
    std::exit(0);
}

void Simulator::requestQuit()
{
    qDebug() << "Simulator: requestQuit() invoked from QML - emitting quitRequested()";
    emit quitRequested();
    // Ensure the application quit is invoked on the main (GUI) thread.
    // Use a queued invocation in case this method is called from another thread.
    if (QCoreApplication::instance())
    {
        QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
    }
}
