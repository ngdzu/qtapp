/**
 * @file SharedMemoryRingBuffer.h
 * @brief Shared memory ring buffer layout and reader utilities.
 *
 * This file defines the structure and utilities for reading from a shared-memory
 * ring buffer used for low-latency sensor data transmission. The ring buffer
 * is created by the sensor simulator using memfd and shared via Unix domain socket.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <atomic>
#include <string>

namespace zmon {

/**
 * @struct RingBufferHeader
 * @brief Header structure for the shared memory ring buffer.
 *
 * The header is located at the start of the shared memory region and contains
 * metadata about the ring buffer layout and version information.
 */
struct RingBufferHeader {
    static constexpr uint32_t MAGIC = 0x534D5242;  // "SMRB" (Shared Memory Ring Buffer)
    static constexpr uint16_t VERSION = 1;
    
    uint32_t magic;              ///< Magic number (MAGIC)
    uint16_t version;            ///< Version number (VERSION)
    uint16_t reserved;           ///< Reserved for future use
    uint32_t frameSize;          ///< Size of each frame in bytes
    uint32_t frameCount;         ///< Total number of frames in ring buffer
    uint64_t writeIndex;         ///< Current write index (atomic, updated by writer)
    uint64_t readIndex;          ///< Current read index (atomic, updated by reader)
    uint64_t heartbeatTimestamp; ///< Last heartbeat timestamp (ms since epoch, atomic)
    uint32_t crc32;              ///< CRC32 of header (excluding this field)
    
    /**
     * @brief Validate header magic and version.
     *
     * @return true if header is valid, false otherwise
     */
    bool isValid() const {
        return magic == MAGIC && version == VERSION;
    }
    
    /**
     * @brief Calculate CRC32 of header (excluding crc32 field).
     *
     * @return CRC32 value
     */
    uint32_t calculateCrc32() const {
        // Calculate CRC32 of header excluding crc32 field
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        size_t size = offsetof(RingBufferHeader, crc32);
        return SharedMemoryRingBuffer::calculateCrc32(data, size);
    }
};

/**
 * @struct SensorFrame
 * @brief Frame structure for sensor data in ring buffer.
 *
 * Each frame contains either vital signs data or waveform data.
 */
struct SensorFrame {
    enum class FrameType : uint8_t {
        Vitals = 0x01,
        Waveform = 0x02,
        Heartbeat = 0x03,
        Invalid = 0xFF
    };
    
    uint8_t type;                ///< Frame type (FrameType enum)
    uint8_t reserved[3];        ///< Reserved for alignment
    uint64_t timestamp;          ///< Timestamp in milliseconds since epoch
    uint32_t sequenceNumber;     ///< Sequence number (monotonically increasing)
    uint32_t dataSize;           ///< Size of data payload in bytes
    uint32_t crc32;              ///< CRC32 of frame (excluding this field)
    // Data payload follows (variable length)
    
    /**
     * @brief Get pointer to data payload.
     *
     * @return Pointer to data payload
     */
    const uint8_t* data() const {
        return reinterpret_cast<const uint8_t*>(this) + sizeof(SensorFrame);
    }
    
    /**
     * @brief Get total frame size (header + data).
     *
     * @return Total frame size in bytes
     */
    uint32_t totalSize() const {
        return sizeof(SensorFrame) + dataSize;
    }
    
    /**
     * @brief Validate frame CRC32.
     *
     * @return true if CRC32 is valid, false otherwise
     */
    bool validateCrc32() const {
        return crc32 == calculateCrc32();
    }
    
    /**
     * @brief Calculate CRC32 of frame (excluding crc32 field).
     *
     * @return CRC32 value
     */
    uint32_t calculateCrc32() const {
        // Calculate CRC32 of frame excluding crc32 field
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        size_t size = offsetof(SensorFrame, crc32);
        return SharedMemoryRingBuffer::calculateCrc32(data, size);
    }
};

/**
 * @class SharedMemoryRingBuffer
 * @brief Reader for shared memory ring buffer.
 *
 * This class provides utilities for reading frames from a shared memory
 * ring buffer. It handles frame validation, CRC checking, and overrun detection.
 *
 * @note Thread-safe for single reader, multiple writers
 * @note Zero heap allocations on hot path
 */
class SharedMemoryRingBuffer {
public:
    /**
     * @brief Constructor.
     *
     * @param memory Pointer to mapped shared memory region
     * @param size Size of shared memory region in bytes
     */
    SharedMemoryRingBuffer(void* memory, size_t size);
    
    /**
     * @brief Destructor.
     */
    ~SharedMemoryRingBuffer();
    
    /**
     * @brief Validate ring buffer header.
     *
     * Checks magic number, version, and CRC32.
     *
     * @return true if header is valid, false otherwise
     */
    bool validateHeader() const;
    
    /**
     * @brief Read next frame from ring buffer.
     *
     * Reads the next available frame, validates it, and returns pointer to frame.
     * Handles ring buffer wrapping and overrun detection.
     *
     * @param frame Output parameter for frame pointer
     * @return true if frame was read successfully, false if no frame available or error
     */
    bool readNextFrame(const SensorFrame*& frame);
    
    /**
     * @brief Check if writer is stalled (no heartbeat for threshold).
     *
     * @param thresholdMs Heartbeat threshold in milliseconds (default: 250ms)
     * @return true if writer appears stalled, false otherwise
     */
    bool isWriterStalled(uint64_t thresholdMs = 250) const;
    
    /**
     * @brief Get last heartbeat timestamp.
     *
     * @return Last heartbeat timestamp in milliseconds since epoch
     */
    uint64_t getLastHeartbeat() const;
    
    /**
     * @brief Get current read index.
     *
     * @return Current read index
     */
    uint64_t getReadIndex() const {
        return m_readIndex;
    }
    
    /**
     * @brief Get current write index.
     *
     * @return Current write index (from header)
     */
    uint64_t getWriteIndex() const;
    
    /**
     * @brief Reset read index (for resync after overrun).
     */
    void resetReadIndex() {
        m_readIndex = getWriteIndex();
    }
    
    /**
     * @brief Check if ring buffer is valid.
     *
     * @return true if ring buffer is valid and ready, false otherwise
     */
    bool isValid() const {
        return m_memory != nullptr && m_size > 0 && validateHeader();
    }

private:
    void* m_memory;              ///< Pointer to mapped shared memory
    size_t m_size;               ///< Size of shared memory region
    RingBufferHeader* m_header;  ///< Pointer to header (at start of memory)
    uint64_t m_readIndex;        ///< Current read index (local to reader)
    
    /**
     * @brief Get pointer to frame at given index.
     *
     * @param index Frame index
     * @return Pointer to frame, or nullptr if invalid
     */
    const SensorFrame* getFrameAt(uint64_t index) const;
    
    /**
     * @brief Calculate CRC32 of data.
     *
     * @param data Pointer to data
     * @param size Size of data in bytes
     * @return CRC32 value
     */
    static uint32_t calculateCrc32(const uint8_t* data, size_t size);
};

} // namespace zmon