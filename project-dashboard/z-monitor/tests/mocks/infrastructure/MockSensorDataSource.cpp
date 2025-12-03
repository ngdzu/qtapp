/**
 * @file MockSensorDataSource.cpp
 * @brief Mock implementation of ISensorDataSource for testing.
 *
 * Provides a test double for sensor data sources with:
 * - Call tracking and verification
 * - Configurable return values
 * - Signal emission verification
 * - No external hardware dependencies
 */

#include "MockSensorDataSource.h"

namespace zmon
{

    // Constructor - initialize tracking state
    MockSensorDataSource::MockSensorDataSource(QObject *parent)
        : ISensorDataSource(parent), m_active(false), m_shouldFailStart(false), m_startCallCount(0), m_stopCallCount(0), m_samplingRate(1000.0) // Default 1kHz
    {
    }

    MockSensorDataSource::~MockSensorDataSource() = default;

    Result<void> MockSensorDataSource::start()
    {
        m_startCallCount++;

        if (m_shouldFailStart)
        {
            return Result<void>::error(Error::create(
                ErrorCode::Unavailable,
                "Mock configured to fail start"));
        }

        m_active = true;
        emit started();
        return Result<void>::ok();
    }

    void MockSensorDataSource::stop()
    {
        m_stopCallCount++;
        m_active = false;
        emit stopped();
    }

    bool MockSensorDataSource::isActive() const
    {
        return m_active;
    }

    DataSourceInfo MockSensorDataSource::getInfo() const
    {
        return DataSourceInfo{
            "Mock Sensor",
            "MOCK-V1",
            "1.0.0",
            {},
            false // Not a real hardware device
        };
    }

    double MockSensorDataSource::getSamplingRate() const
    {
        return m_samplingRate;
    }

    // Mock-specific behavior configuration methods

    void MockSensorDataSource::setShouldFailStart(bool shouldFail)
    {
        m_shouldFailStart = shouldFail;
    }

    void MockSensorDataSource::setSamplingRate(double rate)
    {
        m_samplingRate = rate;
    }

    void MockSensorDataSource::reset()
    {
        m_active = false;
        m_shouldFailStart = false;
        m_startCallCount = 0;
        m_stopCallCount = 0;
        m_samplingRate = 1000.0;
    }

    // Verification methods

    int MockSensorDataSource::startCallCount() const
    {
        return m_startCallCount;
    }

    int MockSensorDataSource::stopCallCount() const
    {
        return m_stopCallCount;
    }

    bool MockSensorDataSource::wasStartCalled() const
    {
        return m_startCallCount > 0;
    }

    bool MockSensorDataSource::wasStopCalled() const
    {
        return m_stopCallCount > 0;
    }

} // namespace zmon
