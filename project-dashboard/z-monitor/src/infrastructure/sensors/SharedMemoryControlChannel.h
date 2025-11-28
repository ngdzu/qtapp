/**
 * @file SharedMemoryControlChannel.h
 * @brief Unix domain socket control channel for shared memory handshake.
 *
 * This file defines the control channel used to exchange shared memory
 * file descriptor information between the sensor simulator (writer) and
 * the Z Monitor (reader). The control channel uses Unix domain sockets
 * for local IPC.
 *
 * **Important:** The Unix domain socket is ONLY used for the initial handshake
 * to exchange the memfd file descriptor. All actual data transfer (60 Hz vitals,
 * 250 Hz waveforms) happens through the shared memory ring buffer for zero-copy,
 * low-latency (< 16ms) performance. The socket is NOT used for data transfer.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QSocketNotifier>
#include <memory>

namespace ZMonitor {
namespace Infrastructure {
namespace Sensors {

/**
 * @struct ControlMessage
 * @brief Control message structure for Unix domain socket.
 */
struct ControlMessage {
    enum class MessageType : uint8_t {
        Handshake = 0x01,        ///< Initial handshake with shared memory info
        Heartbeat = 0x02,        ///< Periodic heartbeat
        Shutdown = 0x03,         ///< Shutdown notification
        Error = 0xFF             ///< Error notification
    };
    
    MessageType type;            ///< Message type
    uint8_t reserved[3];         ///< Reserved for alignment
    uint32_t memfdFd;            ///< File descriptor for memfd (in ancillary data)
    uint64_t ringBufferSize;     ///< Size of ring buffer in bytes
    char socketPath[108];         ///< Path to Unix domain socket (max 108 bytes for sockaddr_un)
};

/**
 * @class SharedMemoryControlChannel
 * @brief Unix domain socket control channel for shared memory handshake.
 *
 * This class manages the Unix domain socket connection to the sensor simulator
 * for exchanging shared memory file descriptor information. It handles the
 * handshake protocol and provides the memfd file descriptor for mapping.
 *
 * **Architecture:**
 * - **Control Channel (Socket):** Used ONLY for initial handshake to exchange
 *   the memfd file descriptor. This is a one-time operation during connection
 *   setup. The socket is NOT used for data transfer.
 * - **Data Channel (Shared Memory):** All actual sensor data (60 Hz vitals,
 *   250 Hz waveforms) is transferred through the shared memory ring buffer
 *   for zero-copy, low-latency (< 16ms) performance.
 *
 * This pattern (socket for setup, shared memory for data) is standard for
 * high-performance IPC where you need to pass file descriptors but want to
 * avoid socket I/O overhead for every frame.
 *
 * @note Thread-safe: Can be used from any thread
 * @note Uses QSocketNotifier for async socket I/O
 * @note Socket is only used during handshake, not for data transfer
 */
class SharedMemoryControlChannel : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param socketPath Path to Unix domain socket (default: "/tmp/z-monitor-sensor.sock")
     * @param parent Parent QObject
     */
    explicit SharedMemoryControlChannel(const QString& socketPath = "/tmp/z-monitor-sensor.sock",
                                       QObject* parent = nullptr);
    
    /**
     * @brief Destructor.
     */
    ~SharedMemoryControlChannel() override;
    
    /**
     * @brief Connect to control channel.
     *
     * Establishes connection to Unix domain socket and performs handshake.
     *
     * @return true if connection succeeded, false otherwise
     */
    bool connect();
    
    /**
     * @brief Disconnect from control channel.
     */
    void disconnect();
    
    /**
     * @brief Check if connected.
     *
     * @return true if connected, false otherwise
     */
    bool isConnected() const {
        return m_connected;
    }
    
    /**
     * @brief Get memfd file descriptor.
     *
     * Returns the file descriptor received during handshake.
     * The caller is responsible for mapping this fd to memory.
     *
     * @return File descriptor, or -1 if not available
     */
    int getMemfdFd() const {
        return m_memfdFd;
    }
    
    /**
     * @brief Get ring buffer size.
     *
     * Returns the ring buffer size received during handshake.
     *
     * @return Ring buffer size in bytes, or 0 if not available
     */
    size_t getRingBufferSize() const {
        return m_ringBufferSize;
    }

signals:
    /**
     * @brief Emitted when handshake completes successfully.
     *
     * @param memfdFd File descriptor for memfd
     * @param ringBufferSize Size of ring buffer in bytes
     */
    void handshakeCompleted(int memfdFd, size_t ringBufferSize);
    
    /**
     * @brief Emitted when connection is lost.
     */
    void connectionLost();
    
    /**
     * @brief Emitted when error occurs.
     *
     * @param error Error message
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief Handle socket data available.
     */
    void onSocketDataAvailable();

private:
    QString m_socketPath;        ///< Path to Unix domain socket
    int m_socketFd;              ///< Socket file descriptor
    QSocketNotifier* m_notifier; ///< Socket notifier for async I/O
    bool m_connected;            ///< Connection state
    int m_memfdFd;               ///< Memfd file descriptor (from handshake)
    size_t m_ringBufferSize;     ///< Ring buffer size (from handshake)
    
    /**
     * @brief Receive file descriptor via Unix domain socket.
     *
     * Uses SCM_RIGHTS to receive file descriptor in ancillary data.
     *
     * @param fd Output parameter for received file descriptor
     * @return true if file descriptor received, false otherwise
     */
    bool receiveFileDescriptor(int& fd);
    
    /**
     * @brief Parse control message.
     *
     * @param data Pointer to message data
     * @param size Size of message data
     * @param message Output parameter for parsed message
     * @return true if message parsed successfully, false otherwise
     */
    bool parseControlMessage(const uint8_t* data, size_t size, ControlMessage& message);
};

} // namespace Sensors
} // namespace Infrastructure
} // namespace ZMonitor

