/**
 * @file MockSensorDataSource.h
 * @brief Mock implementation of ISensorDataSource for tests.
 */
#pragma once

#include "infrastructure/interfaces/ISensorDataSource.h"

namespace zmon
{

    class MockSensorDataSource : public ISensorDataSource
    {
        Q_OBJECT
    public:
        explicit MockSensorDataSource(QObject *parent = nullptr) : ISensorDataSource(parent), m_active(false) {}
        ~MockSensorDataSource() override = default;

        Result<void> start() override
        {
            m_active = true;
            emit started();
            return Result<void>::ok();
        }
        void stop() override
        {
            m_active = false;
            emit stopped();
        }
        bool isActive() const override { return m_active; }
        DataSourceInfo getInfo() const override { return DataSourceInfo{"Mock Sensor", "MOCK", "1.0.0", {}, false}; }
        double getSamplingRate() const override { return 1.0; }

    private:
        bool m_active;
    };

} // namespace zmon
