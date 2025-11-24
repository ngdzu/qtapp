#pragma once

#include <QObject>
#include <QTimer>
#include <QSet>
#include <QWebSocketServer>
#include <QWebSocket>

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

private:
    QWebSocketServer *m_server = nullptr;
    QSet<QWebSocket *> m_clients;
    QTimer m_telemetryTimer;
    QTimer m_demoTimer;

    int m_hr = 72;
    int m_spo2 = 98;
    int m_rr = 16;
    int m_demoStep = 0;
    
    // ECG waveform generation state
    double m_ecgPhase = 0.0;
    static constexpr int SAMPLE_RATE = 250; // Hz
    static constexpr int PACKET_INTERVAL_MS = 200; // 5Hz telemetry
    static constexpr int SAMPLES_PER_PACKET = (SAMPLE_RATE * PACKET_INTERVAL_MS) / 1000; // 50 samples per packet
};
