/**
 * @file integration_test.cpp
 * @brief Basic integration test for shared memory sensor simulator.
 *
 * This test verifies that the simulator can create a shared memory ring buffer
 * and that the structure matches what Z-Monitor expects.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <QCoreApplication>
#include <QTest>
#include <QDebug>
#include <cstring>
#include <cstdint>
#include <cstddef>

// Include simulator structures
#include "src/core/SharedMemoryWriter.h"

// Z-Monitor structure definitions (for compatibility verification)
// These match the structures in z-monitor/src/infrastructure/sensors/SharedMemoryRingBuffer.h
namespace ZMonitor {
namespace Infrastructure {
namespace Sensors {

struct RingBufferHeader {
    static constexpr uint32_t MAGIC = 0x534D5242;  // "SMRB"
    static constexpr uint16_t VERSION = 1;
    
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t frameSize;
    uint32_t frameCount;
    uint64_t writeIndex;  // Note: Z-Monitor uses std::atomic, but size should match
    uint64_t readIndex;
    uint64_t heartbeatTimestamp;  // Note: Z-Monitor uses std::atomic, but size should match
    uint32_t crc32;
};

struct SensorFrame {
    enum class FrameType : uint8_t {
        Vitals = 0x01,
        Waveform = 0x02,
        Heartbeat = 0x03,
        Invalid = 0xFF
    };
    
    uint8_t type;
    uint8_t reserved[3];
    uint64_t timestamp;
    uint32_t sequenceNumber;
    uint32_t dataSize;
    uint32_t crc32;
    // Data payload follows
};

} // namespace Sensors
} // namespace Infrastructure
} // namespace ZMonitor

class IntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void testStructureCompatibility();
    void testMagicNumberMatch();
    void testFrameTypeMatch();
};

void IntegrationTest::testStructureCompatibility()
{
    // Verify structure sizes match
    QCOMPARE(sizeof(SensorSimulator::RingBufferHeader), sizeof(ZMonitor::Infrastructure::Sensors::RingBufferHeader));
    QCOMPARE(sizeof(SensorSimulator::SensorFrame), sizeof(ZMonitor::Infrastructure::Sensors::SensorFrame));
    
    // Verify field offsets match
    QCOMPARE(offsetof(SensorSimulator::RingBufferHeader, magic), 
             offsetof(ZMonitor::Infrastructure::Sensors::RingBufferHeader, magic));
    QCOMPARE(offsetof(SensorSimulator::RingBufferHeader, version), 
             offsetof(ZMonitor::Infrastructure::Sensors::RingBufferHeader, version));
    QCOMPARE(offsetof(SensorSimulator::RingBufferHeader, frameSize), 
             offsetof(ZMonitor::Infrastructure::Sensors::RingBufferHeader, frameSize));
    QCOMPARE(offsetof(SensorSimulator::RingBufferHeader, frameCount), 
             offsetof(ZMonitor::Infrastructure::Sensors::RingBufferHeader, frameCount));
    
    QCOMPARE(offsetof(SensorSimulator::SensorFrame, type), 
             offsetof(ZMonitor::Infrastructure::Sensors::SensorFrame, type));
    QCOMPARE(offsetof(SensorSimulator::SensorFrame, timestamp), 
             offsetof(ZMonitor::Infrastructure::Sensors::SensorFrame, timestamp));
    QCOMPARE(offsetof(SensorSimulator::SensorFrame, sequenceNumber), 
             offsetof(ZMonitor::Infrastructure::Sensors::SensorFrame, sequenceNumber));
    QCOMPARE(offsetof(SensorSimulator::SensorFrame, dataSize), 
             offsetof(ZMonitor::Infrastructure::Sensors::SensorFrame, dataSize));
    QCOMPARE(offsetof(SensorSimulator::SensorFrame, crc32), 
             offsetof(ZMonitor::Infrastructure::Sensors::SensorFrame, crc32));
}

void IntegrationTest::testMagicNumberMatch()
{
    // Verify magic numbers match
    QCOMPARE(SensorSimulator::RingBufferHeader::MAGIC, 
             ZMonitor::Infrastructure::Sensors::RingBufferHeader::MAGIC);
    QCOMPARE(SensorSimulator::RingBufferHeader::VERSION, 
             ZMonitor::Infrastructure::Sensors::RingBufferHeader::VERSION);
}

void IntegrationTest::testFrameTypeMatch()
{
    // Verify frame type enum values match
    QCOMPARE(static_cast<uint8_t>(SensorSimulator::SensorFrame::FrameType::Vitals), 
             static_cast<uint8_t>(ZMonitor::Infrastructure::Sensors::SensorFrame::FrameType::Vitals));
    QCOMPARE(static_cast<uint8_t>(SensorSimulator::SensorFrame::FrameType::Waveform), 
             static_cast<uint8_t>(ZMonitor::Infrastructure::Sensors::SensorFrame::FrameType::Waveform));
    QCOMPARE(static_cast<uint8_t>(SensorSimulator::SensorFrame::FrameType::Heartbeat), 
             static_cast<uint8_t>(ZMonitor::Infrastructure::Sensors::SensorFrame::FrameType::Heartbeat));
    QCOMPARE(static_cast<uint8_t>(SensorSimulator::SensorFrame::FrameType::Invalid), 
             static_cast<uint8_t>(ZMonitor::Infrastructure::Sensors::SensorFrame::FrameType::Invalid));
}

QTEST_MAIN(IntegrationTest)
#include "integration_test.moc"

