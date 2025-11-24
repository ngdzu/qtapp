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
#include <cmath>
#include <QVariant>

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

// Helper function for random walk (Brownian motion)
static double randomWalk(double value, double min, double max, double step)
{
    double change = (QRandomGenerator::global()->generateDouble() - 0.5) * step;
    double newValue = value + change;
    return qBound(min, newValue, max);
}

// Generate synthetic ECG data (PQRST complex approximation)
static QVariantList generateECGChunk(int samples, double &phase, int heartRate)
{
    QVariantList values;
    constexpr int SAMPLE_RATE = 250; // Hz
    const double beatPeriod = 60.0 / heartRate; // seconds per beat
    const double samplesPerBeat = SAMPLE_RATE * beatPeriod;
    
    for (int i = 0; i < samples; i++)
    {
        const double t = phase / samplesPerBeat; // 0 to 1 progress through beat
        
        // Synthetic ECG function (approximate PQRST complex)
        double y = 0;
        
        // Baseline noise
        y += (QRandomGenerator::global()->generateDouble() - 0.5) * 5;
        
        // P wave
        y += 10 * std::exp(-std::pow((t - 0.2) * 20, 2));
        
        // QRS Complex
        y -= 10 * std::exp(-std::pow((t - 0.45) * 50, 2)); // Q
        y += 100 * std::exp(-std::pow((t - 0.5) * 100, 2)); // R peak
        y -= 15 * std::exp(-std::pow((t - 0.55) * 50, 2)); // S
        
        // T wave
        y += 15 * std::exp(-std::pow((t - 0.8) * 15, 2));
        
        values.append(qRound(y));
        
        phase++;
        if (phase >= samplesPerBeat)
        {
            phase = 0;
        }
    }
    return values;
}

void Simulator::sendTelemetry()
{
    // Simulate realistic vital signs fluctuation using random walk
    m_hr = qRound(randomWalk(m_hr, 50, 160, 2));
    m_spo2 = qRound(randomWalk(m_spo2, 85, 100, 0.5));
    m_rr = qRound(randomWalk(m_rr, 8, 30, 0.5));

    // Generate ECG waveform chunk
    QVariantList waveformSamples = generateECGChunk(SAMPLES_PER_PACKET, m_ecgPhase, m_hr);

    QJsonObject packet;
    packet["type"] = "vitals";
    packet["timestamp_ms"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    packet["hr"] = m_hr;
    packet["spo2"] = m_spo2;
    packet["rr"] = m_rr;

    // Waveform chunk for WebSocket clients
    QJsonObject waveform;
    waveform["channel"] = "ecg";
    waveform["sample_rate"] = SAMPLE_RATE;
    waveform["start_timestamp_ms"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    QJsonArray values;
    for (const QVariant &sample : waveformSamples)
    {
        values.append(sample.toInt());
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
    
    // Emit waveform data for real-time visualization
    emit waveformUpdated(waveformSamples);

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
