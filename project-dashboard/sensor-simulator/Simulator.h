#pragma once

#include <QObject>
#include <QTimer>
#include <QSet>
#ifdef WEBSOCKETS_ENABLED
#include <QWebSocketServer>
#include <QWebSocket>
#endif
#include <memory>

// Forward declarations
namespace zmon
{
    class SharedMemoryWriter;
}
namespace SensorSimulator
{
    class ControlServer;
}

class Simulator : public QObject
{
    Q_OBJECT
public:
    explicit Simulator(QObject *parent = nullptr);
    ~Simulator() override;

    Q_INVOKABLE void triggerCritical();
    Q_INVOKABLE void triggerWarning();
    Q_INVOKABLE void triggerNotification(const QString &text);
    Q_INVOKABLE void playDemo();
    Q_INVOKABLE void startServer(quint16 port = 9002);
    Q_INVOKABLE void quitApp();
    // Request the application to quit via a signal (safe main-thread quit)
    Q_INVOKABLE void requestQuit();

signals:
    void alarmTriggered(const QString &level);
    void notification(const QString &text);
    // Emitted when vitals are generated each telemetry tick
    void vitalsUpdated(int hr, int spo2, int rr);
    // Emitted when waveform data is generated (array of sample values)
    void waveformUpdated(const QVariantList &samples);
    // General structured log emitted for UI and tests: level is one of "Critical","Warning","Info","Debug"
    void logEmitted(const QString &level, const QString &text);
    // Emitted when UI requests the app to quit; connect to QCoreApplication::quit() in main()
    void quitRequested();

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onClientDisconnected();
    void sendTelemetry();
    void sendVitals();   // 60 Hz vitals (every 16.67 ms)
    void sendWaveform(); // 250 Hz waveforms (every 4 ms, batched)

private:
#ifdef WEBSOCKETS_ENABLED
    // WebSocket server (optional fallback)
    QWebSocketServer *m_server = nullptr;
    QSet<QWebSocket *> m_clients;
#endif

    // Shared memory transport (primary)
    std::unique_ptr<zmon::SharedMemoryWriter> m_sharedMemoryWriter;
    std::unique_ptr<SensorSimulator::ControlServer> m_controlServer;
    void *m_mappedMemory = nullptr;
    size_t m_mappedSize = 0;
    int m_memfdFd = -1;

    // Timers
    QTimer m_telemetryTimer; // Legacy WebSocket timer (200ms)
    QTimer m_vitalsTimer;    // 60 Hz vitals (16.67 ms)
    QTimer m_waveformTimer;  // 250 Hz waveforms (4 ms)
    QTimer m_heartbeatTimer; // Heartbeat updates (every frame)
    QTimer m_demoTimer;

    // Vital signs state
    int m_hr = 72;
    int m_spo2 = 98;
    int m_rr = 16;
    int m_demoStep = 0;

    // ECG waveform generation state
    double m_ecgPhase = 0.0;
    static constexpr int SAMPLE_RATE = 250;                                              // Hz
    static constexpr int PACKET_INTERVAL_MS = 200;                                       // 5Hz telemetry (legacy WebSocket)
    static constexpr int SAMPLES_PER_PACKET = (SAMPLE_RATE * PACKET_INTERVAL_MS) / 1000; // 50 samples per packet
    static constexpr int VITALS_RATE_HZ = 60;                                            // 60 Hz vitals
    static constexpr int VITALS_INTERVAL_MS = 1000 / VITALS_RATE_HZ;                     // ~16.67 ms
    static constexpr int WAVEFORM_RATE_HZ = 250;                                         // 250 Hz waveforms
    static constexpr int WAVEFORM_INTERVAL_MS = 1000 / WAVEFORM_RATE_HZ;                 // 4 ms
    static constexpr int WAVEFORM_SAMPLES_PER_FRAME = 10;                                // Batch 10 samples per frame

    // Ring buffer configuration
    static constexpr uint32_t FRAME_SIZE = 4096;  // 4KB per frame slot
    static constexpr uint32_t FRAME_COUNT = 2048; // 2048 frames (8MB total)

    /**
     * @brief Initialize shared memory ring buffer.
     *
     * Creates memfd, maps it, and initializes SharedMemoryWriter.
     *
     * @return true if initialization succeeded, false otherwise
     */
    bool initializeSharedMemory();

    /**
     * @brief Cleanup shared memory.
     */
    void cleanupSharedMemory();
};
