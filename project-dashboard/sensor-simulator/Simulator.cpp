#include "Simulator.h"
#include "src/core/SharedMemoryWriter.h"
#include "src/core/ControlServer.h"
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
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>

// memfd_create support (Linux-specific)
#ifdef __linux__
#include <sys/syscall.h>
#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001U
#endif
#ifndef MFD_ALLOW_SEALING
#define MFD_ALLOW_SEALING 0x0002U
#endif
#ifndef SYS_memfd_create
#define SYS_memfd_create 319
#endif
static int memfd_create(const char *name, unsigned int flags) {
    return syscall(SYS_memfd_create, name, flags);
}
#else
// Fallback for non-Linux systems (use shm_open instead)
#include <sys/stat.h>
#include <sys/shm.h>
#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001U
#endif
#ifndef MFD_ALLOW_SEALING
#define MFD_ALLOW_SEALING 0x0002U
#endif
static int memfd_create(const char *name, unsigned int flags) {
    // Use shm_open as fallback
    int fd = shm_open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
    if (fd >= 0) {
        // Set close-on-exec if requested
        if (flags & MFD_CLOEXEC) {
            fcntl(fd, F_SETFD, FD_CLOEXEC);
        }
    }
    return fd;
}
#endif

Simulator::Simulator(QObject *parent)
    : QObject(parent)
{
    // Initialize shared memory transport (primary)
    if (initializeSharedMemory()) {
        // Setup vitals timer (60 Hz = 16.67 ms)
        connect(&m_vitalsTimer, &QTimer::timeout, this, &Simulator::sendVitals);
        m_vitalsTimer.setInterval(VITALS_INTERVAL_MS);
        m_vitalsTimer.setTimerType(Qt::PreciseTimer);
        m_vitalsTimer.start();
        
        // Setup waveform timer (250 Hz = 4 ms)
        connect(&m_waveformTimer, &QTimer::timeout, this, &Simulator::sendWaveform);
        m_waveformTimer.setInterval(WAVEFORM_INTERVAL_MS);
        m_waveformTimer.setTimerType(Qt::PreciseTimer);
        m_waveformTimer.start();
        
        // Setup heartbeat timer (update every frame)
        connect(&m_heartbeatTimer, &QTimer::timeout, this, [this]() {
            if (m_sharedMemoryWriter) {
                uint64_t timestamp = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
                m_sharedMemoryWriter->writeHeartbeat(timestamp);
            }
        });
        m_heartbeatTimer.setInterval(10); // Update every 10ms
        m_heartbeatTimer.start();
        
        qInfo() << "Simulator: Shared memory transport initialized (60 Hz vitals, 250 Hz waveforms)";
    } else {
        qWarning() << "Simulator: Failed to initialize shared memory, falling back to WebSocket only";
    }
    
    // Legacy WebSocket timer (for fallback/compatibility)
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
    cleanupSharedMemory();
    
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

void Simulator::sendVitals()
{
    // Update vitals using random walk
    m_hr = qRound(randomWalk(m_hr, 50, 160, 2));
    m_spo2 = qRound(randomWalk(m_spo2, 85, 100, 0.5));
    m_rr = qRound(randomWalk(m_rr, 8, 30, 0.5));
    
    // Write to shared memory ring buffer
    if (m_sharedMemoryWriter) {
        uint64_t timestamp = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
        if (!m_sharedMemoryWriter->writeVitalsFrame(timestamp, m_hr, m_spo2, m_rr)) {
            qWarning() << "Simulator: Failed to write vitals frame to shared memory";
        }
    }
    
    // Notify QML/UI
    emit vitalsUpdated(m_hr, m_spo2, m_rr);
    
    if (!qgetenv("SIMULATOR_DEBUG").isEmpty()) {
        qDebug() << "Simulator: Vitals written (hr=" << m_hr << " spo2=" << m_spo2 << " rr=" << m_rr << ")";
    }
}

void Simulator::sendWaveform()
{
    // Generate ECG waveform samples (10 samples per frame at 250 Hz)
    QVariantList waveformSamples = generateECGChunk(WAVEFORM_SAMPLES_PER_FRAME, m_ecgPhase, m_hr);
    
    // Convert to vector of ints
    std::vector<int> values;
    for (const QVariant &sample : waveformSamples) {
        values.push_back(sample.toInt());
    }
    
    // Write to shared memory ring buffer
    if (m_sharedMemoryWriter) {
        uint64_t timestamp = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
        int64_t startTimestamp = static_cast<int64_t>(QDateTime::currentMSecsSinceEpoch());
        
        if (!m_sharedMemoryWriter->writeWaveformFrame(timestamp, "ecg", SAMPLE_RATE,
                                                       startTimestamp, values)) {
            qWarning() << "Simulator: Failed to write waveform frame to shared memory";
        }
    }
    
    // Notify QML/UI for visualization
    emit waveformUpdated(waveformSamples);
}

bool Simulator::initializeSharedMemory() {
    // Calculate ring buffer size
    size_t headerSize = sizeof(SensorSimulator::RingBufferHeader);
    size_t framesSize = FRAME_SIZE * FRAME_COUNT;
    m_mappedSize = headerSize + framesSize;
    
    // Create memfd
    m_memfdFd = memfd_create("zmonitor-sim-ring", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (m_memfdFd < 0) {
        qCritical() << "Simulator: Failed to create memfd:" << strerror(errno);
        return false;
    }
    
    // Set size
    if (ftruncate(m_memfdFd, m_mappedSize) < 0) {
        qCritical() << "Simulator: Failed to set memfd size:" << strerror(errno);
        ::close(m_memfdFd);
        m_memfdFd = -1;
        return false;
    }
    
    // Map shared memory
    m_mappedMemory = mmap(nullptr, m_mappedSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_memfdFd, 0);
    if (m_mappedMemory == MAP_FAILED) {
        qCritical() << "Simulator: Failed to mmap memfd:" << strerror(errno);
        ::close(m_memfdFd);
        m_memfdFd = -1;
        return false;
    }
    
    // Initialize SharedMemoryWriter
    m_sharedMemoryWriter = std::make_unique<SensorSimulator::SharedMemoryWriter>(
        m_mappedMemory, m_mappedSize, FRAME_SIZE, FRAME_COUNT);
    
    if (!m_sharedMemoryWriter->initialize()) {
        qCritical() << "Simulator: Failed to initialize shared memory writer";
        cleanupSharedMemory();
        return false;
    }
    
    // Start control server (socket path matches Z-Monitor default: /tmp/z-monitor-sensor.sock)
    // Note: Documentation mentions unix://run/zmonitor-sim.sock but code uses /tmp/z-monitor-sensor.sock
    m_controlServer = std::make_unique<SensorSimulator::ControlServer>("/tmp/z-monitor-sensor.sock", this);
    m_controlServer->setMemfdInfo(m_memfdFd, m_mappedSize);
    
    if (!m_controlServer->start()) {
        qCritical() << "Simulator: Failed to start control server";
        cleanupSharedMemory();
        return false;
    }
    
    qInfo() << "Simulator: Shared memory initialized (size:" << m_mappedSize 
            << " bytes, frames:" << FRAME_COUNT << ", frame size:" << FRAME_SIZE << " bytes)";
    
    return true;
}

void Simulator::cleanupSharedMemory() {
    // Stop control server
    if (m_controlServer) {
        m_controlServer->stop();
        m_controlServer.reset();
    }
    
    // Cleanup writer
    m_sharedMemoryWriter.reset();
    
    // Unmap memory
    if (m_mappedMemory && m_mappedMemory != MAP_FAILED) {
        munmap(m_mappedMemory, m_mappedSize);
        m_mappedMemory = nullptr;
    }
    
    // Close memfd
    if (m_memfdFd >= 0) {
        ::close(m_memfdFd);
        m_memfdFd = -1;
    }
    
    m_mappedSize = 0;
}
