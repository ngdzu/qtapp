/**
 * @file MockSensorDataSource.h
 * @brief Mock implementation of ISensorDataSource for testing.
 *
 * Provides a configurable test double with:
 * - Call count tracking
 * - Configurable failure scenarios
 * - Verification methods
 */
#pragma once

#include "infrastructure/interfaces/ISensorDataSource.h"

namespace zmon
{

    /**
     * @class MockSensorDataSource
     * @brief Test double for ISensorDataSource with tracking and configuration.
     *
     * This mock allows tests to:
     * - Configure behavior (e.g., force start() to fail)
     * - Track method calls (e.g., verify start() was called)
     * - Control sampling rate
     * - Reset state between tests
     */
    class MockSensorDataSource : public ISensorDataSource
    {
        Q_OBJECT
    public:
        explicit MockSensorDataSource(QObject *parent = nullptr);
        ~MockSensorDataSource() override;

        // ISensorDataSource interface
        Result<void> start() override;
        void stop() override;
        bool isActive() const override;
        DataSourceInfo getInfo() const override;
        double getSamplingRate() const override;

        // Mock control methods

        /**
         * @brief Configure whether start() should fail
         * @param shouldFail If true, start() returns error
         */
        void setShouldFailStart(bool shouldFail);

        /**
         * @brief Set the sampling rate returned by getSamplingRate()
         * @param rate Sampling rate in Hz
         */
        void setSamplingRate(double rate);

        /**
         * @brief Reset all mock state (call counts, active state, config)
         */
        void reset();

        // Verification methods

        /**
         * @brief Get number of times start() was called
         * @return Call count
         */
        int startCallCount() const;

        /**
         * @brief Get number of times stop() was called
         * @return Call count
         */
        int stopCallCount() const;

        /**
         * @brief Check if start() was called at least once
         * @return true if start() called, false otherwise
         */
        bool wasStartCalled() const;

        /**
         * @brief Check if stop() was called at least once
         * @return true if stop() called, false otherwise
         */
        bool wasStopCalled() const;

    private:
        bool m_active;
        bool m_shouldFailStart;
        int m_startCallCount;
        int m_stopCallCount;
        double m_samplingRate;
    };

} // namespace zmon
