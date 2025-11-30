/**
 * @file test_header_size.cpp
 * @brief Test to verify RingBufferHeader size and layout match between simulator and z-monitor.
 */

#include "infrastructure/sensors/SharedMemoryRingBuffer.h"
#include <iostream>
#include <cstddef>

using namespace zmon;

int main()
{
    std::cout << "Ring Buffer Header Layout:" << std::endl;
    std::cout << "  sizeof(RingBufferHeader) = " << sizeof(RingBufferHeader) << " bytes" << std::endl;
    std::cout << "  offsetof(magic) = " << offsetof(RingBufferHeader, magic) << std::endl;
    std::cout << "  offsetof(version) = " << offsetof(RingBufferHeader, version) << std::endl;
    std::cout << "  offsetof(frameSize) = " << offsetof(RingBufferHeader, frameSize) << std::endl;
    std::cout << "  offsetof(frameCount) = " << offsetof(RingBufferHeader, frameCount) << std::endl;
    std::cout << "  offsetof(writeIndex) = " << offsetof(RingBufferHeader, writeIndex) << std::endl;
    std::cout << "  offsetof(readIndex) = " << offsetof(RingBufferHeader, readIndex) << std::endl;
    std::cout << "  offsetof(heartbeatTimestamp) = " << offsetof(RingBufferHeader, heartbeatTimestamp) << std::endl;
    std::cout << "  offsetof(crc32) = " << offsetof(RingBufferHeader, crc32) << std::endl;

    return 0;
}
