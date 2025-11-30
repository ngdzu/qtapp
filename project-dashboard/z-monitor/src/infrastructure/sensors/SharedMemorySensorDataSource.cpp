/**
 * @file SharedMemorySensorDataSource.cpp
 * @brief Implementation of shared memory sensor data source.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/sensors/SharedMemorySensorDataSource.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/monitoring/WaveformSample.h"
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>

namespace zmon
{

    SharedMemorySensorDataSource::SharedMemorySensorDataSource(const QString &socketPath, QObject *parent)
        : ISensorDataSource(parent), m_socketPath(socketPath), m_controlChannel(std::make_unique<SharedMemoryControlChannel>(socketPath, this)), m_ringBuffer(nullptr), m_mappedMemory(nullptr), m_mappedSize(0), m_memfdFd(-1), m_active(false), m_processTimer(new QTimer(this)), m_stallCheckTimer(new QTimer(this)), m_lastFrameTimestamp(0), m_overrunCount(0)
    {
        // Connect control channel signals
        connect(m_controlChannel.get(), &SharedMemoryControlChannel::handshakeCompleted,
                this, &SharedMemorySensorDataSource::onHandshakeCompleted);
        connect(m_controlChannel.get(), &SharedMemoryControlChannel::connectionLost,
                this, &SharedMemorySensorDataSource::onConnectionLost);
        connect(m_controlChannel.get(), &SharedMemoryControlChannel::errorOccurred,
                this, &SharedMemorySensorDataSource::onControlChannelError);

        // Setup frame processing timer (high frequency for low latency)
        m_processTimer->setInterval(1); // 1ms interval for < 16ms latency
        m_processTimer->setSingleShot(false);
        connect(m_processTimer, &QTimer::timeout, this, &SharedMemorySensorDataSource::processFrames);

        // Setup stall check timer (250ms threshold)
        m_stallCheckTimer->setInterval(100); // Check every 100ms
        m_stallCheckTimer->setSingleShot(false);
        connect(m_stallCheckTimer, &QTimer::timeout, this, &SharedMemorySensorDataSource::checkWriterStall);
    }

    SharedMemorySensorDataSource::~SharedMemorySensorDataSource()
    {
        stop();
        unmapSharedMemory();
    }

    Result<void> SharedMemorySensorDataSource::start()
    {
        if (m_active)
        {
            return Result<void>::ok();
        }

        // Connect to control channel
        if (!m_controlChannel->connect())
        {
            qWarning() << "SharedMemorySensorDataSource: Failed to connect to control channel";
            return Result<void>::error(Error::create(
                ErrorCode::Internal,
                "Failed to connect to control channel",
                {{"socketPath", m_socketPath.toStdString()}}));
        }

        // Handshake will complete asynchronously via onHandshakeCompleted()
        // We'll start timers after handshake completes

        return Result<void>::ok();
    }

    void SharedMemorySensorDataSource::stop()
    {
        if (!m_active)
        {
            return;
        }

        m_active = false;
        m_processTimer->stop();
        m_stallCheckTimer->stop();

        m_controlChannel->disconnect();

        emit stopped();
    }

    DataSourceInfo SharedMemorySensorDataSource::getInfo() const
    {
        DataSourceInfo info;
        info.name = "Shared Memory Sensor Data Source";
        info.type = "SIMULATOR";
        info.version = "1.0.0";
        info.capabilities = QStringList() << "HR" << "SPO2" << "RR" << "ECG" << "PLETH";
        info.supportsWaveforms = true;
        return info;
    }

    void SharedMemorySensorDataSource::onHandshakeCompleted(int memfdFd, size_t ringBufferSize)
    {
        // Map shared memory using the file descriptor received via socket
        // Note: After this point, the socket is no longer used - all data
        // transfer happens through the mapped shared memory (zero-copy, < 16ms latency)
        if (!mapSharedMemory(memfdFd, ringBufferSize))
        {
            qCritical() << "SharedMemorySensorDataSource: Failed to map shared memory";
            emit sensorError({SensorErrorCode::CommunicationError,
                              "Failed to map shared memory",
                              "SharedMemory",
                              QDateTime::currentDateTimeUtc(),
                              true});
            return;
        }

        // Create ring buffer reader
        m_ringBuffer = std::make_unique<SharedMemoryRingBuffer>(m_mappedMemory, m_mappedSize);

        if (!m_ringBuffer->isValid())
        {
            qCritical() << "SharedMemorySensorDataSource: Invalid ring buffer";
            qCritical() << "  Validation details - calling validateHeader()...";
            if (!m_ringBuffer->validateHeader())
            {
                qCritical() << "  validateHeader() returned false";
            }
            emit sensorError({SensorErrorCode::CommunicationError,
                              "Invalid ring buffer",
                              "SharedMemory",
                              QDateTime::currentDateTimeUtc(),
                              true});
            unmapSharedMemory();
            return;
        }

        // At this point, the socket handshake is complete and we have the memfd mapped.
        // The socket is no longer needed - all data transfer happens through shared memory.
        // We could optionally disconnect the control channel here, but we keep it connected
        // to receive shutdown/error messages if needed.

        // Start processing frames from shared memory (no socket I/O)
        m_active = true;
        m_processTimer->start();
        m_stallCheckTimer->start();

        emit started();
        emit connectionStatusChanged(true, "SharedMemory");

        qInfo() << "SharedMemorySensorDataSource: Started successfully - data transfer via shared memory (no socket I/O)";
    }

    void SharedMemorySensorDataSource::onConnectionLost()
    {
        qWarning() << "SharedMemorySensorDataSource: Connection lost";

        m_active = false;
        m_processTimer->stop();
        m_stallCheckTimer->stop();

        unmapSharedMemory();

        emit connectionStatusChanged(false, "SharedMemory");
        emit sensorError({SensorErrorCode::SensorDisconnected,
                          "Connection to sensor simulator lost",
                          "SharedMemory",
                          QDateTime::currentDateTimeUtc(),
                          true});
    }

    void SharedMemorySensorDataSource::onControlChannelError(const QString &error)
    {
        qWarning() << "SharedMemorySensorDataSource: Control channel error:" << error;
        emit sensorError({SensorErrorCode::CommunicationError,
                          error,
                          "SharedMemory",
                          QDateTime::currentDateTimeUtc(),
                          true});
    }

    void SharedMemorySensorDataSource::processFrames()
    {
        if (!m_active || !m_ringBuffer)
        {
            return;
        }

        // Read frames (process multiple frames per call for efficiency)
        const SensorFrame *frame = nullptr;
        int framesProcessed = 0;
        const int MAX_FRAMES_PER_CALL = 10; // Limit to avoid blocking too long

        while (framesProcessed < MAX_FRAMES_PER_CALL && m_ringBuffer->readNextFrame(frame))
        {
            if (!frame)
            {
                break;
            }

            // Update last frame timestamp
            m_lastFrameTimestamp = frame->timestamp;

            // Parse frame based on type
            switch (static_cast<SensorFrame::FrameType>(frame->type))
            {
            case SensorFrame::FrameType::Vitals:
                parseVitalsFrame(frame);
                break;
            case SensorFrame::FrameType::Waveform:
                parseWaveformFrame(frame);
                break;
            case SensorFrame::FrameType::Heartbeat:
                // Heartbeat handled by checkWriterStall()
                break;
            default:
                qWarning() << "SharedMemorySensorDataSource: Unknown frame type:" << frame->type;
                break;
            }

            framesProcessed++;
        }
    }

    void SharedMemorySensorDataSource::checkWriterStall()
    {
        if (!m_active || !m_ringBuffer)
        {
            return;
        }

        if (m_ringBuffer->isWriterStalled(250))
        {
            qWarning() << "SharedMemorySensorDataSource: Writer stalled (no heartbeat for 250ms)";
            emit sensorError({SensorErrorCode::CommunicationError,
                              "Sensor writer stalled (no heartbeat)",
                              "SharedMemory",
                              QDateTime::currentDateTimeUtc(),
                              true});
        }
    }

    bool SharedMemorySensorDataSource::mapSharedMemory(int fd, size_t size)
    {
        if (m_mappedMemory)
        {
            unmapSharedMemory();
        }

        // Map shared memory
        m_mappedMemory = ::mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
        if (m_mappedMemory == MAP_FAILED)
        {
            qCritical() << "SharedMemorySensorDataSource: mmap failed:" << strerror(errno);
            m_mappedMemory = nullptr;
            return false;
        }

        m_mappedSize = size;
        m_memfdFd = fd;

        return true;
    }

    void SharedMemorySensorDataSource::unmapSharedMemory()
    {
        if (m_mappedMemory && m_mappedMemory != MAP_FAILED)
        {
            ::munmap(m_mappedMemory, m_mappedSize);
            m_mappedMemory = nullptr;
        }

        if (m_memfdFd >= 0)
        {
            ::close(m_memfdFd);
            m_memfdFd = -1;
        }

        m_mappedSize = 0;
        m_ringBuffer.reset();
    }

    void SharedMemorySensorDataSource::parseVitalsFrame(const SensorFrame *frame)
    {
        if (!frame || frame->dataSize == 0)
        {
            return;
        }

        // Parse JSON vitals data
        QByteArray data(reinterpret_cast<const char *>(frame->data()), frame->dataSize);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError)
        {
            qWarning() << "SharedMemorySensorDataSource: Failed to parse vitals JSON:" << error.errorString();
            return;
        }

        QJsonObject obj = doc.object();

        // Parse vital type from JSON or determine from frame context
        QString vitalTypeStr = obj.value("type").toString("VITALS");
        int64_t timestampMs = frame->timestamp;
        int signalQualityInt = obj.value("signal_quality").toInt(100);

        // Create VitalRecord using constructor (all members are const)
        // TODO: Determine proper vital type, for now emit individual vitals

        // Emit HR if present
        if (obj.contains("hr"))
        {
            VitalRecord hrVital("HR", obj.value("hr").toDouble(), timestampMs, signalQualityInt);
            emit vitalSignsReceived(hrVital);
        }

        // Emit SpO2 if present
        if (obj.contains("spo2"))
        {
            VitalRecord spo2Vital("SPO2", obj.value("spo2").toDouble(), timestampMs, signalQualityInt);
            emit vitalSignsReceived(spo2Vital);
        }

        // Emit RR if present
        if (obj.contains("rr"))
        {
            VitalRecord rrVital("RR", obj.value("rr").toDouble(), timestampMs, signalQualityInt);
            emit vitalSignsReceived(rrVital);
        }
    }

    void SharedMemorySensorDataSource::parseWaveformFrame(const SensorFrame *frame)
    {
        if (!frame || frame->dataSize == 0)
        {
            return;
        }

        // Parse JSON waveform data
        QByteArray data(reinterpret_cast<const char *>(frame->data()), frame->dataSize);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError)
        {
            qWarning() << "SharedMemorySensorDataSource: Failed to parse waveform JSON:" << error.errorString();
            return;
        }

        QJsonObject obj = doc.object();
        QJsonArray valuesArray = obj.value("values").toArray();

        // Emit waveform samples
        QString waveformType = obj.value("channel").toString();
        int sampleRateHz = obj.value("sample_rate").toInt(250);
        int64_t startTimestamp = obj.value("start_timestamp_ms").toVariant().toLongLong();

        for (int i = 0; i < valuesArray.size(); ++i)
        {
            int64_t timestampMs = startTimestamp + (i * 1000 / sampleRateHz);
            std::string channel = waveformType.toStdString();
            double value = valuesArray[i].toDouble();
            double sampleRate = static_cast<double>(sampleRateHz);

            WaveformSample sample(channel, value, timestampMs, sampleRate);
            emit waveformSampleReceived(sample);
        }
    }

    void SharedMemorySensorDataSource::handleOverrun()
    {
        m_overrunCount++;
        qWarning() << "SharedMemorySensorDataSource: Ring buffer overrun detected (count:" << m_overrunCount << ")";

        if (m_ringBuffer)
        {
            m_ringBuffer->resetReadIndex();
        }
    }

} // namespace zmon