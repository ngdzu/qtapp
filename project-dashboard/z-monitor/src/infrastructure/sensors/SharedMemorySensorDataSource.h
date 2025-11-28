/**
 * @file SharedMemorySensorDataSource.h
 * @brief Shared memory sensor data source implementation (memfd reader).
 *
 * This file defines the SharedMemorySensorDataSource class which implements
 * ISensorDataSource by reading from a shared memory ring buffer created by
 * the sensor simulator. This provides low-latency (< 16ms) sensor data
 * acquisition for development and testing.
 *
 * **Architecture:**
 * - Uses Unix domain socket (control channel) ONLY for initial handshake to
 *   receive the memfd file descriptor. This is a one-time operation.
 * - All actual data transfer (60 Hz vitals, 250 Hz waveforms) happens through
 *   the shared memory ring buffer for zero-copy, low-latency performance.
 * - The socket is NOT used for data transfer - it's only for setup/teardown.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <memory>

#include "infrastructure/interfaces/ISensorDataSource.h"
#include "infrastructure/sensors/SharedMemoryRingBuffer.h"
#include "infrastructure/sensors/SharedMemoryControlChannel.h"

namespace zmon {

/**
 * @class SharedMemorySensorDataSource
 * @brief Shared memory sensor data source implementation.
 *
 * This class implements ISensorDataSource by reading from a shared memory
 * ring buffer (memfd) created by the sensor simulator. It provides:
 * - Low-latency data acquisition (< 16ms requirement)
 * - Heartbeat/stall detection (250ms threshold)
 * - Ring buffer overrun handling
 * - Automatic reconnection on connection loss
 *
 * **Data Transfer Architecture:**
 * - **Control Channel (Socket):** Used ONLY during initial handshake to receive
 *   the memfd file descriptor. This is a one-time operation, not used for data.
 * - **Data Channel (Shared Memory):** All sensor data (vitals, waveforms) is
 *   read directly from the shared memory ring buffer with zero-copy, achieving
 *   < 16ms latency from simulator write to signal emission.
 *
 * @note Runs on Real-Time Processing Thread (high priority)
 * @note Zero heap allocations on hot path (except startup)
 * @note Socket is only used for handshake, not for data transfer
 * @see ISensorDataSource, SharedMemoryRingBuffer, SharedMemoryControlChannel
 * @ingroup Infrastructure
 */
class SharedMemorySensorDataSource : public ISensorDataSource {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param socketPath Path to Unix domain socket for control channel
     * @param parent Parent QObject
     */
    explicit SharedMemorySensorDataSource(const QString& socketPath = "/tmp/z-monitor-sensor.sock",
                                         QObject* parent = nullptr);
    
    /**
     * @brief Destructor.
     *
     * Stops data acquisition and unmaps shared memory.
     */
    ~SharedMemorySensorDataSource() override;
    
    // ISensorDataSource interface
    /**
     * @brief Start data acquisition.
     *
     * Connects to control channel, performs handshake, maps shared memory,
     * and starts reading frames.
     *
     * @return true if started successfully, false otherwise
     */
    bool start() override;
    
    /**
     * @brief Stop data acquisition.
     *
     * Stops reading frames, unmaps shared memory, and disconnects from
     * control channel.
     */
    void stop() override;
    
    /**
     * @brief Check if data source is currently active.
     *
     * @return true if actively acquiring data
     */
    bool isActive() const override {
        return m_active;
    }
    
    /**
     * @brief Get data source metadata.
     *
     * @return DataSourceInfo Metadata about this data source
     */
    Interfaces::DataSourceInfo getInfo() const override;
    
    /**
     * @brief Get current sampling rate.
     *
     * @return Sampling rate in Hz (60.0 for vitals, 250.0 for waveforms)
     */
    double getSamplingRate() const override {
        return 60.0;  // Vitals at 60 Hz, waveforms at 250 Hz
    }

private slots:
    /**
     * @brief Process frames from ring buffer.
     *
     * Called periodically by timer to read and process frames.
     */
    void processFrames();
    
    /**
     * @brief Handle handshake completion.
     *
     * @param memfdFd File descriptor for memfd
     * @param ringBufferSize Size of ring buffer in bytes
     */
    void onHandshakeCompleted(int memfdFd, size_t ringBufferSize);
    
    /**
     * @brief Handle connection loss.
     */
    void onConnectionLost();
    
    /**
     * @brief Handle control channel error.
     *
     * @param error Error message
     */
    void onControlChannelError(const QString& error);
    
    /**
     * @brief Check for writer stall.
     *
     * Called periodically to check if writer is stalled.
     */
    void checkWriterStall();

private:
    /**
     * @brief Map shared memory from file descriptor.
     *
     * @param fd File descriptor (memfd)
     * @param size Size of memory region
     * @return true if mapping succeeded, false otherwise
     */
    bool mapSharedMemory(int fd, size_t size);
    
    /**
     * @brief Unmap shared memory.
     */
    void unmapSharedMemory();
    
    /**
     * @brief Parse vitals frame and emit signal.
     *
     * @param frame Sensor frame containing vitals data
     */
    void parseVitalsFrame(const SharedMemoryRingBuffer::SensorFrame* frame);
    
    /**
     * @brief Parse waveform frame and emit signal.
     *
     * @param frame Sensor frame containing waveform data
     */
    void parseWaveformFrame(const SharedMemoryRingBuffer::SensorFrame* frame);
    
    /**
     * @brief Handle ring buffer overrun.
     *
     * Logs warning and resyncs to latest frame.
     */
    void handleOverrun();
    
    QString m_socketPath;                                    ///< Path to Unix domain socket
    std::unique_ptr<SharedMemoryControlChannel> m_controlChannel; ///< Control channel for handshake
    std::unique_ptr<SharedMemoryRingBuffer> m_ringBuffer;    ///< Ring buffer reader
    void* m_mappedMemory;                                    ///< Mapped shared memory (mmap)
    size_t m_mappedSize;                                     ///< Size of mapped memory
    int m_memfdFd;                                          ///< Memfd file descriptor
    bool m_active;                                          ///< Active state
    QTimer* m_processTimer;                                 ///< Timer for periodic frame processing
    QTimer* m_stallCheckTimer;                              ///< Timer for stall detection
    uint64_t m_lastFrameTimestamp;                          ///< Timestamp of last processed frame
    uint32_t m_overrunCount;                                ///< Count of overruns detected
};

} // namespace zmon