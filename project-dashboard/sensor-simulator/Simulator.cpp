#include "Simulator.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include <QtGlobal>
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
        if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
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
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
        qDebug() << "SensorSimulator: client connected";
}

void Simulator::onTextMessageReceived(const QString &message)
{
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
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
        if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
            qDebug() << "SensorSimulator: client disconnected";
    }
}

void Simulator::sendTelemetry()
{
    // Simulate realistic vital signs fluctuation
    // Heart Rate: Random walk between 60-100
    int deltaHr = QRandomGenerator::global()->bounded(3) - 1; // -1, 0, +1
    m_hr += deltaHr;
    if (m_hr < 60)
        m_hr = 60;
    if (m_hr > 100)
        m_hr = 100;

    // SpO2: Mostly stable 95-100, occasional drop
    if (QRandomGenerator::global()->bounded(10) == 0)
    {
        int deltaSpo2 = QRandomGenerator::global()->bounded(3) - 1;
        m_spo2 += deltaSpo2;
        if (m_spo2 < 95)
            m_spo2 = 95;
        if (m_spo2 > 100)
            m_spo2 = 100;
    }

    // Resp Rate: 12-20
    if (QRandomGenerator::global()->bounded(20) == 0)
    {
        int deltaRr = QRandomGenerator::global()->bounded(3) - 1;
        m_rr += deltaRr;
        if (m_rr < 12)
            m_rr = 12;
        if (m_rr > 20)
            m_rr = 20;
    }

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

    // Also print to stdout so container logs show the changing vitals for debugging
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
        qDebug() << "Telemetry: hr=" << m_hr << "spo2=" << m_spo2 << "rr=" << m_rr << "(clients=" << m_clients.size() << ")";

    // Notify QML/UI directly so vitals display can update in real time
    emit vitalsUpdated(m_hr, m_spo2, m_rr);

    for (QWebSocket *client : qAsConst(m_clients))
    {
        client->sendTextMessage(text);
    }

    // Emit a debug/info log for UI visibility (also used by QML)
    emit logEmitted("Debug", QString("Sent telemetry: hr=%1 spo2=%2 rr=%3").arg(m_hr).arg(m_spo2).arg(m_rr));
}

void Simulator::triggerCritical()
{
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
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
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
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
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
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
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
        qDebug() << "Simulator: quitApp() invoked from QML";
    // Ask Qt to quit normally
    QCoreApplication::quit();
    // Fallback: if the Qt event loop doesn't exit (containerized environment
    // or unexpected blocking), force process termination so the container can stop.
    std::exit(0);
}

void Simulator::requestQuit()
{
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty())
        qDebug() << "Simulator: requestQuit() invoked from QML - emitting quitRequested()";
    emit quitRequested();
    // Ensure the application quit is invoked on the main (GUI) thread.
    // Use a queued invocation in case this method is called from another thread.
    if (QCoreApplication::instance())
    {
        QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
    }
}
