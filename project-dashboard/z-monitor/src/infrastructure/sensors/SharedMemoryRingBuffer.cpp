/**
 * @file SharedMemoryRingBuffer.cpp
 * @brief Implementation of shared memory ring buffer reader.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/sensors/SharedMemoryRingBuffer.h"
#include <algorithm>
#include <cstring>
#include <chrono>
#include <cstddef>

namespace ZMonitor {
namespace Infrastructure {
namespace Sensors {

// Simple CRC32 implementation (polynomial 0xEDB88320)
static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

static void init_crc32_table() {
    if (crc32_table_initialized) return;
    
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        }
        crc32_table[i] = crc;
    }
    crc32_table_initialized = true;
}

uint32_t SharedMemoryRingBuffer::calculateCrc32(const uint8_t* data, size_t size) {
    init_crc32_table();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; ++i) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}


SharedMemoryRingBuffer::SharedMemoryRingBuffer(void* memory, size_t size)
    : m_memory(memory)
    , m_size(size)
    , m_header(nullptr)
    , m_readIndex(0)
{
    if (m_memory && m_size >= sizeof(RingBufferHeader)) {
        m_header = reinterpret_cast<RingBufferHeader*>(m_memory);
        // Initialize read index from header write index (start from current position)
        m_readIndex = m_header->writeIndex.load(std::memory_order_acquire);
    }
}

SharedMemoryRingBuffer::~SharedMemoryRingBuffer() = default;

bool SharedMemoryRingBuffer::validateHeader() const {
    if (!m_header) {
        return false;
    }
    
    if (!m_header->isValid()) {
        return false;
    }
    
    // Validate CRC32
    uint32_t calculatedCrc = m_header->calculateCrc32();
    if (calculatedCrc != m_header->crc32) {
        return false;
    }
    
    // Validate frame size and count
    if (m_header->frameSize == 0 || m_header->frameCount == 0) {
        return false;
    }
    
    // Validate buffer size
    size_t requiredSize = sizeof(RingBufferHeader) + 
                         (m_header->frameSize * m_header->frameCount);
    if (m_size < requiredSize) {
        return false;
    }
    
    return true;
}

const SensorFrame* SharedMemoryRingBuffer::getFrameAt(uint64_t index) const {
    if (!m_header || !m_memory) {
        return nullptr;
    }
    
    uint64_t frameIndex = index % m_header->frameCount;
    size_t offset = sizeof(RingBufferHeader) + (frameIndex * m_header->frameSize);
    
    if (offset + sizeof(SensorFrame) > m_size) {
        return nullptr;
    }
    
    return reinterpret_cast<const SensorFrame*>(
        reinterpret_cast<const uint8_t*>(m_memory) + offset);
}

bool SharedMemoryRingBuffer::readNextFrame(const SensorFrame*& frame) {
    if (!isValid()) {
        return false;
    }
    
    // Get current write index (atomic read)
    uint64_t writeIndex = m_header->writeIndex.load(std::memory_order_acquire);
    
    // Check if there are new frames
    if (m_readIndex >= writeIndex) {
        return false;  // No new frames
    }
    
    // Check for overrun (reader too far behind)
    uint64_t framesBehind = writeIndex - m_readIndex;
    if (framesBehind > m_header->frameCount) {
        // Overrun detected - reset to latest frame
        m_readIndex = writeIndex;
        return false;
    }
    
    // Read frame at current read index
    frame = getFrameAt(m_readIndex);
    if (!frame) {
        return false;
    }
    
    // Validate frame
    if (frame->type == static_cast<uint8_t>(SensorFrame::FrameType::Invalid)) {
        return false;
    }
    
    // Validate frame size
    if (frame->totalSize() > m_header->frameSize) {
        return false;
    }
    
    // Validate CRC32
    if (!frame->validateCrc32()) {
        return false;
    }
    
    // Advance read index
    m_readIndex++;
    
    return true;
}

bool SharedMemoryRingBuffer::isWriterStalled(uint64_t thresholdMs) const {
    if (!m_header) {
        return true;
    }
    
    uint64_t lastHeartbeat = m_header->heartbeatTimestamp.load(std::memory_order_acquire);
    if (lastHeartbeat == 0) {
        return true;  // No heartbeat yet
    }
    
    // Get current time (milliseconds since epoch)
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    uint64_t elapsed = static_cast<uint64_t>(now) - lastHeartbeat;
    return elapsed > thresholdMs;
}

uint64_t SharedMemoryRingBuffer::getLastHeartbeat() const {
    if (!m_header) {
        return 0;
    }
    return m_header->heartbeatTimestamp.load(std::memory_order_acquire);
}

uint64_t SharedMemoryRingBuffer::getWriteIndex() const {
    if (!m_header) {
        return 0;
    }
    return m_header->writeIndex.load(std::memory_order_acquire);
}

} // namespace Sensors
} // namespace Infrastructure
} // namespace ZMonitor

