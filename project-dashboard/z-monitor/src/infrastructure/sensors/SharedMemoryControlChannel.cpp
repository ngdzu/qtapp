/**
 * @file SharedMemoryControlChannel.cpp
 * @brief Implementation of Unix domain socket control channel.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/sensors/SharedMemoryControlChannel.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <QDebug>
#include <QOverload>

namespace zmon
{

    SharedMemoryControlChannel::SharedMemoryControlChannel(const QString &socketPath, QObject *parent)
        : QObject(parent), m_socketPath(socketPath), m_socketFd(-1), m_notifier(nullptr), m_connected(false), m_memfdFd(-1), m_ringBufferSize(0)
    {
    }

    SharedMemoryControlChannel::~SharedMemoryControlChannel()
    {
        disconnect();
    }

    bool SharedMemoryControlChannel::connect()
    {
        if (m_connected)
        {
            return true;
        }

        // Create Unix domain socket
        m_socketFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (m_socketFd < 0)
        {
            qWarning() << "SharedMemoryControlChannel: Failed to create socket:" << strerror(errno);
            emit errorOccurred(QString("Failed to create socket: %1").arg(strerror(errno)));
            return false;
        }

        // Set non-blocking
        int flags = ::fcntl(m_socketFd, F_GETFL, 0);
        ::fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK);

        // Connect to socket
        struct sockaddr_un addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        ::strncpy(addr.sun_path, m_socketPath.toLocal8Bit().constData(), sizeof(addr.sun_path) - 1);

        if (::connect(m_socketFd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
        {
            qWarning() << "SharedMemoryControlChannel: Failed to connect:" << strerror(errno);
            ::close(m_socketFd);
            m_socketFd = -1;
            emit errorOccurred(QString("Failed to connect: %1").arg(strerror(errno)));
            return false;
        }

        // Create socket notifier for async I/O
        m_notifier = new QSocketNotifier(m_socketFd, QSocketNotifier::Read, this);
        // Qt6: QSocketNotifier::activated signal
        QObject::connect(m_notifier, &QSocketNotifier::activated, this, &SharedMemoryControlChannel::onSocketDataAvailable);
        m_notifier->setEnabled(true);

        m_connected = true;

        // Wait for handshake message
        // The handshake will be processed in onSocketDataAvailable()

        return true;
    }

    void SharedMemoryControlChannel::disconnect()
    {
        if (m_notifier)
        {
            m_notifier->setEnabled(false);
            m_notifier->deleteLater();
            m_notifier = nullptr;
        }

        if (m_socketFd >= 0)
        {
            ::close(m_socketFd);
            m_socketFd = -1;
        }

        m_connected = false;
        m_memfdFd = -1;
        m_ringBufferSize = 0;
    }

    void SharedMemoryControlChannel::onSocketDataAvailable()
    {
        if (!m_connected || m_socketFd < 0)
        {
            return;
        }

        // **CRITICAL:** Use recvmsg() to receive BOTH control message AND file descriptor in ONE call
        // Using recv() first would consume the data but lose the SCM_RIGHTS ancillary data

        struct msghdr msg;
        struct iovec iov;
        ControlMessage message;
        char cmsg_buffer[CMSG_SPACE(sizeof(int))];

        ::memset(&msg, 0, sizeof(msg));
        ::memset(&iov, 0, sizeof(iov));
        ::memset(&message, 0, sizeof(message));

        iov.iov_base = &message;
        iov.iov_len = sizeof(message);

        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsg_buffer;
        msg.msg_controllen = sizeof(cmsg_buffer);

        ssize_t received = ::recvmsg(m_socketFd, &msg, 0);

        if (received < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                qWarning() << "SharedMemoryControlChannel: Receive error:" << strerror(errno);
                emit errorOccurred(QString("Receive error: %1").arg(strerror(errno)));
                disconnect();
                emit connectionLost();
            }
            return;
        }

        if (received == 0)
        {
            // Connection closed
            disconnect();
            emit connectionLost();
            return;
        }

        if (received < static_cast<ssize_t>(sizeof(ControlMessage)))
        {
            qWarning() << "SharedMemoryControlChannel: Incomplete message received";
            return;
        }

        // Extract file descriptor from ancillary data
        int fd = -1;
        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS)
        {
            ::memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
        }
        else
        {
            qWarning() << "SharedMemoryControlChannel: No file descriptor received in SCM_RIGHTS";
            return;
        }

        // Process handshake message
        if (message.type == ControlMessage::MessageType::Handshake)
        {
            m_memfdFd = fd;
            m_ringBufferSize = message.ringBufferSize;
            qInfo() << "SharedMemoryControlChannel: Handshake completed, fd=" << m_memfdFd << "size=" << m_ringBufferSize;
            emit handshakeCompleted(m_memfdFd, m_ringBufferSize);
        }
        else if (message.type == ControlMessage::MessageType::Shutdown)
        {
            disconnect();
            emit connectionLost();
        }
    }

    bool SharedMemoryControlChannel::receiveFileDescriptor(int &fd)
    {
        // This method is now deprecated - kept for compatibility but not used
        // The file descriptor is received in onSocketDataAvailable() via recvmsg()
        struct msghdr msg;
        struct iovec iov;
        char buffer[1];
        char cmsg_buffer[CMSG_SPACE(sizeof(int))];

        ::memset(&msg, 0, sizeof(msg));
        ::memset(&iov, 0, sizeof(iov));

        iov.iov_base = buffer;
        iov.iov_len = sizeof(buffer);

        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsg_buffer;
        msg.msg_controllen = sizeof(cmsg_buffer);

        ssize_t received = ::recvmsg(m_socketFd, &msg, 0);
        if (received < 0)
        {
            return false;
        }

        // Extract file descriptor from ancillary data
        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS)
        {
            ::memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
            return true;
        }

        return false;
    }

    bool SharedMemoryControlChannel::parseControlMessage(const uint8_t *data, size_t size, ControlMessage &message)
    {
        if (size < sizeof(ControlMessage))
        {
            return false;
        }
        ::memcpy(&message, data, sizeof(ControlMessage));
        return true;
    }

} // namespace zmon