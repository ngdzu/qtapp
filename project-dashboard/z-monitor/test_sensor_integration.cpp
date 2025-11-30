/**
 * @file test_sensor_integration.cpp
 * @brief Integration test for SharedMemorySensorDataSource with simulator.
 *
 * This test connects to a running sensor simulator via Unix domain socket,
 * receives shared memory file descriptor, reads sensor frames, and measures latency.
 *
 * Usage: ./test_sensor_integration
 * Requires: Sensor simulator running at /tmp/z-monitor-sensor.sock
 */

#include "infrastructure/sensors/SharedMemorySensorDataSource.h"
#include "infrastructure/interfaces/ISensorDataSource.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/monitoring/WaveformSample.h"
#include <QCoreApplication>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <chrono>

using namespace zmon;

class SensorIntegrationTest : public QObject
{
    Q_OBJECT

public:
    SensorIntegrationTest(QObject *parent = nullptr)
        : QObject(parent), m_dataSource(std::make_unique<SharedMemorySensorDataSource>("/tmp/z-monitor-sensor.sock", this)), m_vitalCount(0), m_waveformCount(0), m_startTime(std::chrono::steady_clock::now())
    {
        // Connect signals
        connect(m_dataSource.get(), &SharedMemorySensorDataSource::started,
                this, &SensorIntegrationTest::onStarted);
        connect(m_dataSource.get(), &SharedMemorySensorDataSource::stopped,
                this, &SensorIntegrationTest::onStopped);
        connect(m_dataSource.get(), &SharedMemorySensorDataSource::vitalSignsReceived,
                this, &SensorIntegrationTest::onVitalReceived);
        connect(m_dataSource.get(), &SharedMemorySensorDataSource::waveformSampleReceived,
                this, &SensorIntegrationTest::onWaveformReceived);
        connect(m_dataSource.get(), &SharedMemorySensorDataSource::sensorError,
                this, &SensorIntegrationTest::onSensorError);

        // Stop after 5 seconds
        QTimer::singleShot(5000, this, &SensorIntegrationTest::stop);
    }

    void start()
    {
        qInfo() << "========================================";
        qInfo() << "SharedMemorySensorDataSource Integration Test";
        qInfo() << "========================================";
        qInfo() << "Starting sensor data source...";

        auto result = m_dataSource->start();
        if (!result.isOk())
        {
            qCritical() << "Failed to start sensor data source:" << result.error().message.c_str();
            QCoreApplication::exit(1);
        }
    }

    void stop()
    {
        qInfo() << "Stopping test...";
        m_dataSource->stop();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - m_startTime)
                            .count();

        qInfo() << "========================================";
        qInfo() << "Test Results:";
        qInfo() << "  Duration:" << duration << "ms";
        qInfo() << "  Vitals received:" << m_vitalCount;
        qInfo() << "  Waveforms received:" << m_waveformCount;
        qInfo() << "========================================";

        QCoreApplication::quit();
    }

private slots:
    void onStarted()
    {
        qInfo() << "✓ Sensor data source started - reading from shared memory";
        m_startTime = std::chrono::steady_clock::now();
    }

    void onStopped()
    {
        qInfo() << "✓ Sensor data source stopped";
    }

    void onVitalReceived(const VitalRecord &vital)
    {
        m_vitalCount++;
        if (m_vitalCount <= 3)
        {
            qInfo() << "  Vital:" << QString::fromStdString(vital.vitalType)
                    << "=" << vital.value
                    << "quality=" << vital.signalQuality
                    << "timestamp=" << vital.timestampMs;
        }
    }

    void onWaveformReceived(const WaveformSample &sample)
    {
        m_waveformCount++;
        if (m_waveformCount <= 3)
        {
            qInfo() << "  Waveform:" << QString::fromStdString(sample.channel)
                    << "=" << sample.value
                    << "timestamp=" << sample.timestampMs;
        }
    }

    void onSensorError(const SensorError &error)
    {
        qWarning() << "Sensor error:" << error.message
                   << "device=" << error.sensorType
                   << "critical=" << !error.recoverable;
    }

private:
    std::unique_ptr<SharedMemorySensorDataSource> m_dataSource;
    int m_vitalCount;
    int m_waveformCount;
    std::chrono::steady_clock::time_point m_startTime;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    SensorIntegrationTest test;
    QTimer::singleShot(0, &test, &SensorIntegrationTest::start);

    return app.exec();
}

#include "test_sensor_integration.moc"
