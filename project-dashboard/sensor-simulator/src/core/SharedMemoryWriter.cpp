/**
 * @file SharedMemoryWriter.cpp
 * @brief Implementation of shared memory ring buffer writer.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "SharedMemoryWriter.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstddef>

namespace SensorSimulator {

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

uint32_t SharedMemoryWriter::calculateCrc32(const uint8_t* data, size_t size) {
    init_crc32_table();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; ++i) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}

SharedMemoryWriter::SharedMemoryWriter(void* memory, size_t size, uint32_t frameSize, uint32_t frameCount)
    : m_memory(memory)
    , m_size(size)
    , m_header(nullptr)
    , m_frameSize(frameSize)
    , m_frameCount(frameCount)
    , m_sequenceNumber(0)
    , m_framesWritten(0)
{
    if (m_memory && m_size >= sizeof(RingBufferHeader)) {
        m_header = reinterpret_cast<RingBufferHeader*>(m_memory);
    }
}

SharedMemoryWriter::~SharedMemoryWriter() = default;

bool SharedMemoryWriter::initialize() {
    if (!isValid()) {
        return false;
    }
    
    // Initialize header
    m_header->magic = RingBufferHeader::MAGIC;
    m_header->version = RingBufferHeader::VERSION;
    m_header->reserved = 0;
    m_header->frameSize = m_frameSize;
    m_header->frameCount = m_frameCount;
    m_header->writeIndex.store(0, std::memory_order_release);
    m_header->readIndex = 0;
    m_header->heartbeatTimestamp.store(0, std::memory_order_release);
    
    // Calculate CRC32 of header (excluding crc32 field)
    const uint8_t* data = reinterpret_cast<const uint8_t*>(m_header);
    size_t size = offsetof(RingBufferHeader, crc32);
    m_header->crc32 = calculateCrc32(data, size);
    
    // Initialize all frame slots to Invalid
    for (uint32_t i = 0; i < m_frameCount; ++i) {
        SensorFrame* frame = getFrameAt(i);
        if (frame) {
            frame->type = static_cast<uint8_t>(SensorFrame::FrameType::Invalid);
            std::memset(frame, 0, m_frameSize);
        }
    }
    
    return true;
}

bool SharedMemoryWriter::writeVitalsFrame(uint64_t timestamp, int hr, int spo2, int rr, const std::string& jsonData) {
    std::string json = jsonData.empty() ? generateVitalsJson(hr, spo2, rr) : jsonData;
    return writeFrame(SensorFrame::FrameType::Vitals, timestamp, json);
}

bool SharedMemoryWriter::writeWaveformFrame(uint64_t timestamp, const std::string& channel, int sampleRate,
                                            int64_t startTimestamp, const std::vector<int>& values,
                                            const std::string& jsonData) {
    std::string json = jsonData.empty() ? generateWaveformJson(channel, sampleRate, startTimestamp, values) : jsonData;
    return writeFrame(SensorFrame::FrameType::Waveform, timestamp, json);
}

bool SharedMemoryWriter::writeHeartbeat(uint64_t timestamp) {
    if (!isValid()) {
        return false;
    }
    
    // Update heartbeat timestamp in header (atomic)
    m_header->heartbeatTimestamp.store(timestamp, std::memory_order_release);
    
    // Optionally write a heartbeat frame (for explicit heartbeat frames)
    // For now, we just update the header timestamp
    return true;
}

bool SharedMemoryWriter::writeFrame(SensorFrame::FrameType frameType, uint64_t timestamp, const std::string& jsonData) {
    if (!isValid()) {
        return false;
    }
    
    // Get next write index
    uint64_t writeIndex = m_header->writeIndex.load(std::memory_order_acquire);
    uint64_t nextIndex = (writeIndex + 1) % m_frameCount;
    
    // Get frame slot
    SensorFrame* frame = getFrameAt(writeIndex);
    if (!frame) {
        return false;
    }
    
    // Calculate required frame size
    size_t jsonSize = jsonData.size();
    size_t requiredFrameSize = sizeof(SensorFrame) + jsonSize;
    
    // Check if frame fits in slot
    if (requiredFrameSize > m_frameSize) {
        return false;  // Frame too large
    }
    
    // Clear frame slot
    std::memset(frame, 0, m_frameSize);
    
    // Fill frame header
    frame->type = static_cast<uint8_t>(frameType);
    frame->timestamp = timestamp;
    frame->sequenceNumber = m_sequenceNumber++;
    frame->dataSize = static_cast<uint32_t>(jsonSize);
    
    // Copy JSON data
    if (jsonSize > 0) {
        std::memcpy(reinterpret_cast<uint8_t*>(frame) + sizeof(SensorFrame), jsonData.data(), jsonSize);
    }
    
    // Calculate CRC32 of frame (excluding crc32 field)
    const uint8_t* frameData = reinterpret_cast<const uint8_t*>(frame);
    size_t frameDataSize = offsetof(SensorFrame, crc32);
    frame->crc32 = calculateCrc32(frameData, frameDataSize);
    
    // Update write index (atomic, release semantics for reader visibility)
    m_header->writeIndex.store(nextIndex, std::memory_order_release);
    
    // Update heartbeat timestamp
    m_header->heartbeatTimestamp.store(timestamp, std::memory_order_release);
    
    m_framesWritten++;
    
    return true;
}

SensorFrame* SharedMemoryWriter::getFrameAt(uint64_t index) {
    if (!m_header || !m_memory) {
        return nullptr;
    }
    
    uint64_t frameIndex = index % m_frameCount;
    size_t offset = sizeof(RingBufferHeader) + (frameIndex * m_frameSize);
    
    if (offset + sizeof(SensorFrame) > m_size) {
        return nullptr;
    }
    
    return reinterpret_cast<SensorFrame*>(
        reinterpret_cast<uint8_t*>(m_memory) + offset);
}

std::string SharedMemoryWriter::generateVitalsJson(int hr, int spo2, int rr) {
    std::ostringstream oss;
    oss << "{\"hr\":" << hr << ",\"spo2\":" << spo2 << ",\"rr\":" << rr << "}";
    return oss.str();
}

std::string SharedMemoryWriter::generateWaveformJson(const std::string& channel, int sampleRate,
                                                     int64_t startTimestamp, const std::vector<int>& values) {
    std::ostringstream oss;
    oss << "{\"channel\":\"" << channel << "\",\"sample_rate\":" << sampleRate
        << ",\"start_timestamp_ms\":" << startTimestamp << ",\"values\":[";
    
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) oss << ",";
        oss << values[i];
    }
    
    oss << "]}";
    return oss.str();
}

} // namespace SensorSimulator

