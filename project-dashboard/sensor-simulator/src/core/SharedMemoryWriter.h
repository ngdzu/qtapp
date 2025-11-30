/**
 * @file SharedMemoryWriter.h
 * @brief Shared memory ring buffer writer for sensor simulator.
 *
 * This class manages writing sensor data frames (vitals, waveforms) to a
 * shared memory ring buffer using memfd. It handles frame serialization,
 * CRC32 calculation, and atomic index updates for lock-free operation.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <string>
#include <memory>
#include <vector>

namespace zmon
{

    /**
     * @struct RingBufferHeader
     * @brief Header structure for the shared memory ring buffer.
     *
     * Matches the structure expected by Z-Monitor's SharedMemoryRingBuffer reader.
     */
    struct RingBufferHeader
    {
        static constexpr uint32_t MAGIC = 0x534D5242; // "SMRB" (Shared Memory Ring Buffer)
        static constexpr uint16_t VERSION = 1;

        uint32_t magic;                           ///< Magic number (MAGIC)
        uint16_t version;                         ///< Version number (VERSION)
        uint16_t reserved;                        ///< Reserved for future use
        uint32_t frameSize;                       ///< Size of each frame in bytes
        uint32_t frameCount;                      ///< Total number of frames in ring buffer
        std::atomic<uint64_t> writeIndex;         ///< Current write index (atomic, updated by writer)
        uint64_t readIndex;                       ///< Current read index (updated by readers, not used by writer)
        std::atomic<uint64_t> heartbeatTimestamp; ///< Last heartbeat timestamp (ms since epoch, atomic)
        uint32_t crc32;                           ///< CRC32 of header (excluding this field)
    };

    /**
     * @struct SensorFrame
     * @brief Frame structure for sensor data in ring buffer.
     *
     * Matches the structure expected by Z-Monitor's SharedMemoryRingBuffer reader.
     */
    struct SensorFrame
    {
        enum class FrameType : uint8_t
        {
            Vitals = 0x01,
            Waveform = 0x02,
            Heartbeat = 0x03,
            Invalid = 0xFF
        };

        uint8_t type;            ///< Frame type (FrameType enum)
        uint8_t reserved[3];     ///< Reserved for alignment
        uint64_t timestamp;      ///< Timestamp in milliseconds since epoch
        uint32_t sequenceNumber; ///< Sequence number (monotonically increasing)
        uint32_t dataSize;       ///< Size of data payload in bytes
        uint32_t crc32;          ///< CRC32 of frame (excluding this field)
        // Data payload follows (variable length, JSON string)
    };

    /**
     * @class SharedMemoryWriter
     * @brief Writer for shared memory ring buffer.
     *
     * This class manages writing sensor data frames to a shared memory ring buffer.
     * It handles frame serialization, CRC32 calculation, and atomic index updates.
     *
     * @note Thread-safe for single writer, multiple readers
     * @note Zero heap allocations on hot path (except frame data serialization)
     */
    class SharedMemoryWriter
    {
    public:
        /**
         * @brief Constructor.
         *
         * @param memory Pointer to mapped shared memory region
         * @param size Size of shared memory region in bytes
         * @param frameSize Size of each frame slot in bytes
         * @param frameCount Number of frame slots in ring buffer
         */
        SharedMemoryWriter(void *memory, size_t size, uint32_t frameSize, uint32_t frameCount);

        /**
         * @brief Destructor.
         */
        ~SharedMemoryWriter();

        /**
         * @brief Initialize ring buffer header.
         *
         * Sets up the header with magic, version, frame size/count, and calculates CRC32.
         *
         * @return true if initialization succeeded, false otherwise
         */
        bool initialize();

        /**
         * @brief Write vitals frame to ring buffer.
         *
         * Serializes vitals data as JSON and writes to next available frame slot.
         *
         * @param timestamp Timestamp in milliseconds since epoch
         * @param hr Heart rate (BPM)
         * @param spo2 SpO2 (%)
         * @param rr Respiration rate (RPM)
         * @param jsonData JSON string with vitals data (optional, will generate if empty)
         * @return true if frame written successfully, false otherwise
         */
        bool writeVitalsFrame(uint64_t timestamp, int hr, int spo2, int rr, const std::string &jsonData = "");

        /**
         * @brief Write waveform frame to ring buffer.
         *
         * Serializes waveform data as JSON and writes to next available frame slot.
         *
         * @param timestamp Timestamp in milliseconds since epoch
         * @param channel Waveform channel (e.g., "ecg", "pleth")
         * @param sampleRate Sample rate in Hz
         * @param startTimestamp Start timestamp for first sample
         * @param values Array of waveform sample values
         * @param jsonData JSON string with waveform data (optional, will generate if empty)
         * @return true if frame written successfully, false otherwise
         */
        bool writeWaveformFrame(uint64_t timestamp, const std::string &channel, int sampleRate,
                                int64_t startTimestamp, const std::vector<int> &values,
                                const std::string &jsonData = "");

        /**
         * @brief Write heartbeat frame to ring buffer.
         *
         * Updates heartbeat timestamp in header and optionally writes a heartbeat frame.
         *
         * @param timestamp Timestamp in milliseconds since epoch
         * @return true if heartbeat updated successfully, false otherwise
         */
        bool writeHeartbeat(uint64_t timestamp);

        /**
         * @brief Check if ring buffer is valid.
         *
         * @return true if ring buffer is valid and ready, false otherwise
         */
        bool isValid() const
        {
            return m_memory != nullptr && m_size > 0 && m_header != nullptr;
        }

        /**
         * @brief Get current write index.
         *
         * @return Current write index
         */
        uint64_t getWriteIndex() const
        {
            return m_header ? m_header->writeIndex.load(std::memory_order_acquire) : 0;
        }

        /**
         * @brief Get number of frames written.
         *
         * @return Total number of frames written since initialization
         */
        uint64_t getFramesWritten() const
        {
            return m_framesWritten;
        }

    private:
        void *m_memory;             ///< Pointer to mapped shared memory
        size_t m_size;              ///< Size of shared memory region
        RingBufferHeader *m_header; ///< Pointer to header (at start of memory)
        uint32_t m_frameSize;       ///< Size of each frame slot
        uint32_t m_frameCount;      ///< Number of frame slots
        uint32_t m_sequenceNumber;  ///< Sequence number (monotonically increasing)
        uint64_t m_framesWritten;   ///< Total frames written

        /**
         * @brief Get pointer to frame at given index.
         *
         * @param index Frame index
         * @return Pointer to frame slot, or nullptr if invalid
         */
        SensorFrame *getFrameAt(uint64_t index);

        /**
         * @brief Write frame to ring buffer.
         *
         * @param frameType Frame type
         * @param timestamp Timestamp in milliseconds since epoch
         * @param jsonData JSON data payload
         * @return true if frame written successfully, false otherwise
         */
        bool writeFrame(SensorFrame::FrameType frameType, uint64_t timestamp, const std::string &jsonData);

        /**
         * @brief Calculate CRC32 of data.
         *
         * @param data Pointer to data
         * @param size Size of data in bytes
         * @return CRC32 value
         */
        static uint32_t calculateCrc32(const uint8_t *data, size_t size);

        /**
         * @brief Generate JSON for vitals data.
         *
         * @param hr Heart rate
         * @param spo2 SpO2
         * @param rr Respiration rate
         * @return JSON string
         */
        static std::string generateVitalsJson(int hr, int spo2, int rr);

        /**
         * @brief Generate JSON for waveform data.
         *
         * @param channel Waveform channel
         * @param sampleRate Sample rate
         * @param startTimestamp Start timestamp
         * @param values Waveform sample values
         * @return JSON string
         */
        static std::string generateWaveformJson(const std::string &channel, int sampleRate,
                                                int64_t startTimestamp, const std::vector<int> &values);
    };

} // namespace zmon
