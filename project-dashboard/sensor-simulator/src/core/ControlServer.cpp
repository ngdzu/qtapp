/**
 * @file ControlServer.cpp
 * @brief Implementation of Unix domain socket server for memfd descriptor passing.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "ControlServer.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QLoggingCategory>

namespace SensorSimulator {

ControlServer::ControlServer(const QString& socketPath, QObject* parent)
    : QObject(parent)
    , m_socketPath(socketPath)
    , m_serverFd(-1)
    , m_serverNotifier(nullptr)
    , m_listening(false)
    , m_memfdFd(-1)
    , m_ringBufferSize(0)
{
}

ControlServer::~ControlServer() {
    stop();
}

bool ControlServer::start() {
    if (m_listening) {
        return true;
    }
    
    // Remove old socket file if it exists
    QFile::remove(m_socketPath);
    
    // Create Unix domain socket
    m_serverFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_serverFd < 0) {
        qCritical() << "ControlServer: Failed to create socket:" << strerror(errno);
        emit errorOccurred(QString("Failed to create socket: %1").arg(strerror(errno)));
        return false;
    }
    
    // Set socket to non-blocking
    int flags = ::fcntl(m_serverFd, F_GETFL, 0);
    ::fcntl(m_serverFd, F_SETFL, flags | O_NONBLOCK);
    
    // Bind socket to path
    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, m_socketPath.toLocal8Bit().constData(), sizeof(addr.sun_path) - 1);
    
    if (::bind(m_serverFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        qCritical() << "ControlServer: Failed to bind socket:" << strerror(errno);
        ::close(m_serverFd);
        m_serverFd = -1;
        emit errorOccurred(QString("Failed to bind socket: %1").arg(strerror(errno)));
        return false;
    }
    
    // Listen for connections
    if (::listen(m_serverFd, 5) < 0) {
        qCritical() << "ControlServer: Failed to listen:" << strerror(errno);
        ::close(m_serverFd);
        m_serverFd = -1;
        QFile::remove(m_socketPath);
        emit errorOccurred(QString("Failed to listen: %1").arg(strerror(errno)));
        return false;
    }
    
    // Create socket notifier for accepting connections
    m_serverNotifier = new QSocketNotifier(m_serverFd, QSocketNotifier::Read, this);
    connect(m_serverNotifier, &QSocketNotifier::activated, this, &ControlServer::onNewConnection);
    
    m_listening = true;
    qInfo() << "ControlServer: Listening on" << m_socketPath;
    
    return true;
}

void ControlServer::stop() {
    if (!m_listening) {
        return;
    }
    
    // Close all client connections
    for (int clientFd : m_clients) {
        ::close(clientFd);
    }
    m_clients.clear();
    
    // Delete client notifiers
    for (QSocketNotifier* notifier : m_clientNotifiers) {
        notifier->deleteLater();
    }
    m_clientNotifiers.clear();
    
    // Close server socket
    if (m_serverFd >= 0) {
        if (m_serverNotifier) {
            m_serverNotifier->setEnabled(false);
            m_serverNotifier->deleteLater();
            m_serverNotifier = nullptr;
        }
        ::close(m_serverFd);
        m_serverFd = -1;
    }
    
    // Remove socket file
    QFile::remove(m_socketPath);
    
    m_listening = false;
    qInfo() << "ControlServer: Stopped";
}

void ControlServer::setMemfdInfo(int memfdFd, size_t ringBufferSize) {
    m_memfdFd = memfdFd;
    m_ringBufferSize = ringBufferSize;
}

void ControlServer::onNewConnection() {
    if (m_serverFd < 0) {
        return;
    }
    
    // Accept new connection
    struct sockaddr_un addr;
    socklen_t addrLen = sizeof(addr);
    int clientFd = ::accept(m_serverFd, reinterpret_cast<struct sockaddr*>(&addr), &addrLen);
    
    if (clientFd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            qWarning() << "ControlServer: Failed to accept connection:" << strerror(errno);
            emit errorOccurred(QString("Failed to accept connection: %1").arg(strerror(errno)));
        }
        return;
    }
    
    // Set client socket to non-blocking
    int flags = ::fcntl(clientFd, F_GETFL, 0);
    ::fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
    
    qInfo() << "ControlServer: Client connected, fd:" << clientFd;
    
    // Send memfd file descriptor
    if (m_memfdFd >= 0) {
        if (sendFileDescriptor(clientFd)) {
            m_clients.push_back(clientFd);
            
            // Create socket notifier for this client (for cleanup on disconnect)
            QSocketNotifier* notifier = new QSocketNotifier(clientFd, QSocketNotifier::Read, this);
            connect(notifier, &QSocketNotifier::activated, this, [this, clientFd]() {
                onClientDataAvailable(clientFd);
            });
            m_clientNotifiers.push_back(notifier);
            
            emit clientConnected(clientFd);
        } else {
            qWarning() << "ControlServer: Failed to send memfd to client";
            ::close(clientFd);
        }
    } else {
        qWarning() << "ControlServer: No memfd set, closing client connection";
        ::close(clientFd);
    }
}

void ControlServer::onClientDataAvailable(int clientFd) {
    // Read any data from client (for now, we just acknowledge connection)
    char buffer[256];
    ssize_t n = ::recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    
    if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            qWarning() << "ControlServer: Error reading from client:" << strerror(errno);
            onClientClosed(clientFd);
        }
    } else if (n == 0) {
        // Client closed connection
        onClientClosed(clientFd);
    }
    // Otherwise, data received (can be ignored for now)
}

void ControlServer::onClientClosed(int clientFd) {
    qInfo() << "ControlServer: Client disconnected, fd:" << clientFd;
    removeClient(clientFd);
    emit clientDisconnected(clientFd);
}

bool ControlServer::sendFileDescriptor(int clientFd) {
    if (m_memfdFd < 0) {
        return false;
    }
    
    // Prepare ControlMessage structure (matches Z-Monitor's ControlMessage)
    // Note: Z-Monitor receives ControlMessage first via recv(), then FD via recvmsg()
    // So we send them together in one sendmsg() - the ControlMessage in the data,
    // and the FD in ancillary data. Z-Monitor's recv() will get the ControlMessage,
    // and recvmsg() will get the FD from the same message.
    struct ControlMessage {
        uint8_t type;            // 0x01 = Handshake (MessageType::Handshake)
        uint8_t reserved[3];
        uint32_t memfdFd;        // Not used (FD is in ancillary data), but included for structure compatibility
        uint64_t ringBufferSize;
        char socketPath[108];
    } message;
    
    std::memset(&message, 0, sizeof(message));
    message.type = 0x01;  // Handshake (MessageType::Handshake)
    message.memfdFd = 0;  // Not used, FD is in ancillary data
    message.ringBufferSize = m_ringBufferSize;
    std::strncpy(message.socketPath, m_socketPath.toLocal8Bit().constData(), sizeof(message.socketPath) - 1);
    
    // Send ControlMessage and memfd file descriptor together via SCM_RIGHTS
    // Z-Monitor will receive ControlMessage via recv() and FD via recvmsg() from the same message
    struct msghdr msg = {0};
    struct iovec iov[1];
    char control_buf[CMSG_SPACE(sizeof(int))];
    
    // Prepare message data (ControlMessage structure)
    iov[0].iov_base = &message;
    iov[0].iov_len = sizeof(message);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    // Prepare ancillary data for file descriptor
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);
    
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    std::memcpy(CMSG_DATA(cmsg), &m_memfdFd, sizeof(int));
    
    // Update msg_controllen to actual size
    msg.msg_controllen = cmsg->cmsg_len;
    
    // Send message (ControlMessage + FD in one sendmsg call)
    // Note: Z-Monitor's recv() will get the ControlMessage, and recvmsg() will get the FD
    // from the same message (ancillary data persists across recv calls on the same message)
    ssize_t sent = ::sendmsg(clientFd, &msg, 0);
    if (sent < 0) {
        qCritical() << "ControlServer: Failed to send file descriptor:" << strerror(errno);
        return false;
    }
    
    qInfo() << "ControlServer: Sent memfd (fd:" << m_memfdFd << ", size:" << m_ringBufferSize << ") to client";
    return true;
}

void ControlServer::removeClient(int clientFd) {
    // Remove from clients list
    auto it = std::find(m_clients.begin(), m_clients.end(), clientFd);
    if (it != m_clients.end()) {
        m_clients.erase(it);
    }
    
    // Remove and delete notifier
    for (auto it = m_clientNotifiers.begin(); it != m_clientNotifiers.end(); ++it) {
        if ((*it)->socket() == clientFd) {
            (*it)->setEnabled(false);
            (*it)->deleteLater();
            m_clientNotifiers.erase(it);
            break;
        }
    }
    
    // Close file descriptor
    ::close(clientFd);
}

} // namespace SensorSimulator

