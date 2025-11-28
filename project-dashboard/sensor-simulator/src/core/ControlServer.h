/**
 * @file ControlServer.h
 * @brief Unix domain socket server for memfd file descriptor exchange.
 *
 * This class manages a Unix domain socket server that accepts connections
 * from Z-Monitor and sends the memfd file descriptor via SCM_RIGHTS ancillary
 * data. The socket is ONLY used for the initial handshake - all data transfer
 * happens through shared memory.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QSocketNotifier>
#include <memory>
#include <vector>

namespace SensorSimulator {

/**
 * @class ControlServer
 * @brief Unix domain socket server for memfd descriptor passing.
 *
 * This server listens on a Unix domain socket and sends the memfd file
 * descriptor to connected clients using SCM_RIGHTS. Multiple clients can
 * connect and receive the same descriptor (for multiple Z-Monitor instances).
 *
 * **Architecture:**
 * - **Control Channel (Socket):** Used ONLY for initial handshake to exchange
 *   the memfd file descriptor. This is a one-time operation per client connection.
 * - **Data Channel (Shared Memory):** All actual sensor data (60 Hz vitals,
 *   250 Hz waveforms) is transferred through the shared memory ring buffer
 *   for zero-copy, low-latency (< 16ms) performance.
 *
 * @note Thread-safe: Can be used from any thread
 * @note Uses QSocketNotifier for async socket I/O
 */
class ControlServer : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param socketPath Path to Unix domain socket (default: "/tmp/z-monitor-sensor.sock")
     * @param parent Parent QObject
     */
    explicit ControlServer(const QString& socketPath = "/tmp/z-monitor-sensor.sock",
                          QObject* parent = nullptr);
    
    /**
     * @brief Destructor.
     *
     * Closes socket and removes socket file.
     */
    ~ControlServer() override;
    
    /**
     * @brief Start listening for connections.
     *
     * Creates Unix domain socket and starts listening for client connections.
     *
     * @return true if server started successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop listening and close server.
     *
     * Closes all connections and removes socket file.
     */
    void stop();
    
    /**
     * @brief Set memfd file descriptor to send to clients.
     *
     * @param memfdFd File descriptor for memfd
     * @param ringBufferSize Size of ring buffer in bytes
     */
    void setMemfdInfo(int memfdFd, size_t ringBufferSize);
    
    /**
     * @brief Check if server is listening.
     *
     * @return true if server is listening, false otherwise
     */
    bool isListening() const {
        return m_listening;
    }
    
    /**
     * @brief Get number of connected clients.
     *
     * @return Number of connected clients
     */
    size_t getClientCount() const {
        return m_clients.size();
    }

signals:
    /**
     * @brief Emitted when a new client connects.
     *
     * @param clientFd File descriptor of new client
     */
    void clientConnected(int clientFd);
    
    /**
     * @brief Emitted when a client disconnects.
     *
     * @param clientFd File descriptor of disconnected client
     */
    void clientDisconnected(int clientFd);
    
    /**
     * @brief Emitted when error occurs.
     *
     * @param error Error message
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief Handle new connection on server socket.
     */
    void onNewConnection();
    
    /**
     * @brief Handle client socket data available.
     *
     * @param clientFd File descriptor of client
     */
    void onClientDataAvailable(int clientFd);
    
    /**
     * @brief Handle client socket closed.
     *
     * @param clientFd File descriptor of client
     */
    void onClientClosed(int clientFd);

private:
    QString m_socketPath;        ///< Path to Unix domain socket
    int m_serverFd;              ///< Server socket file descriptor
    QSocketNotifier* m_serverNotifier; ///< Socket notifier for server socket
    std::vector<int> m_clients;  ///< Connected client file descriptors
    std::vector<QSocketNotifier*> m_clientNotifiers; ///< Socket notifiers for clients
    bool m_listening;            ///< Server listening state
    int m_memfdFd;               ///< Memfd file descriptor to send
    size_t m_ringBufferSize;     ///< Ring buffer size to send
    
    /**
     * @brief Send memfd file descriptor to client.
     *
     * Uses SCM_RIGHTS to send file descriptor in ancillary data.
     *
     * @param clientFd Client file descriptor
     * @return true if file descriptor sent successfully, false otherwise
     */
    bool sendFileDescriptor(int clientFd);
    
    /**
     * @brief Remove client from tracking.
     *
     * @param clientFd Client file descriptor
     */
    void removeClient(int clientFd);
};

} // namespace SensorSimulator

