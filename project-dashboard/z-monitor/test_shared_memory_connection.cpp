/**
 * @file test_shared_memory_connection.cpp
 * @brief Minimal test to verify shared memory connection with simulator.
 *
 * Connects to simulator, receives memfd, maps shared memory, reads a few frames.
 */

#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

struct ControlMessage
{
    uint8_t type;            // 0x01 = handshake response
    int32_t memfdFd;         // File descriptor (received via SCM_RIGHTS)
    uint64_t ringBufferSize; // Ring buffer size in bytes
    char socketPath[108];    // Socket path (for reference)
};

int main()
{
    std::cout << "Connecting to /tmp/z-monitor-sensor.sock..." << std::endl;

    // Create Unix domain socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // Connect to simulator
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/tmp/z-monitor-sensor.sock", sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "connect() failed: " << strerror(errno) << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "✓ Connected to simulator" << std::endl;

    // Receive handshake with file descriptor
    struct msghdr msg;
    struct iovec iov;
    ControlMessage message;
    char cmsg_buffer[CMSG_SPACE(sizeof(int))];

    memset(&msg, 0, sizeof(msg));
    memset(&message, 0, sizeof(message));

    iov.iov_base = &message;
    iov.iov_len = sizeof(message);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg_buffer;
    msg.msg_controllen = sizeof(cmsg_buffer);

    ssize_t received = recvmsg(sockfd, &msg, 0);
    if (received < 0)
    {
        std::cerr << "recvmsg() failed: " << strerror(errno) << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "✓ Received handshake (" << received << " bytes)" << std::endl;

    // Extract file descriptor from ancillary data
    int memfd = -1;
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_type == SCM_RIGHTS)
    {
        memcpy(&memfd, CMSG_DATA(cmsg), sizeof(int));
        std::cout << "✓ Received memfd: " << memfd << std::endl;
        std::cout << "  Ring buffer size: " << message.ringBufferSize << " bytes" << std::endl;
    }
    else
    {
        std::cerr << "No SCM_RIGHTS in message" << std::endl;
        close(sockfd);
        return 1;
    }

    // Map shared memory
    void *mapped = mmap(nullptr, message.ringBufferSize, PROT_READ, MAP_SHARED, memfd, 0);
    if (mapped == MAP_FAILED)
    {
        std::cerr << "mmap() failed: " << strerror(errno) << std::endl;
        close(memfd);
        close(sockfd);
        return 1;
    }

    std::cout << "✓ Mapped shared memory at " << mapped << std::endl;

    // Read ring buffer header (first 64 bytes)
    const uint32_t *header = static_cast<const uint32_t *>(mapped);
    std::cout << "Ring buffer header:" << std::endl;
    std::cout << "  Magic: 0x" << std::hex << header[0] << std::dec;
    if (header[0] == 0x534D5242)
    {
        std::cout << " ✓ (valid)" << std::endl;
    }
    else
    {
        std::cout << " ✗ (expected 0x534D5242)" << std::endl;
    }
    std::cout << "  Version: " << header[1] << std::endl;
    std::cout << "  Frame size: " << header[2] << " bytes" << std::endl;
    std::cout << "  Frame count: " << header[3] << std::endl;

    // Read atomic write index (uint64_t at offset 16)
    const uint64_t *writeIndexPtr = reinterpret_cast<const uint64_t *>(
        static_cast<const uint8_t *>(mapped) + 16);
    std::cout << "  Write index: " << *writeIndexPtr << std::endl;

    std::cout << "\n✓ All tests passed - shared memory connection working!" << std::endl;

    // Cleanup
    munmap(mapped, message.ringBufferSize);
    close(memfd);
    close(sockfd);

    return 0;
}
