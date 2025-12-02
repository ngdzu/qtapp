/**
 * @file ISensorDataSource.h
 * @brief Interface for sensor data retrieval.
 */
#pragma once

#include "domain/common/Result.h"
#include "domain/monitoring/VitalRecord.h"
#include "domain/monitoring/WaveformSample.h"
#include <string>
#include <vector>

namespace zmon
{

    class ISensorDataSource
    {
    public:
        virtual ~ISensorDataSource() = default;

        /**
         * @brief Read current vitals for a device.
         * @param deviceId Device identifier
         * @return Result<std::vector<VitalRecord>> - Current vitals
         */
        virtual Result<std::vector<VitalRecord>> readVitals(const std::string &deviceId) = 0;

        /**
         * @brief Read waveform samples.
         * @param deviceId Device identifier
         * @param channel Waveform channel name
         * @return Result<std::vector<WaveformSample>> - Waveform samples
         */
        virtual Result<std::vector<WaveformSample>> readWaveform(const std::string &deviceId, const std::string &channel) = 0;
    };

} // namespace zmon
