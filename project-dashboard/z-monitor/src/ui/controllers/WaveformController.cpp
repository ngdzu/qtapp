/**
 * @file WaveformController.cpp
 * @brief Implementation of WaveformController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "WaveformController.h"
#include "infrastructure/caching/WaveformCache.h"
#include <QVariantMap>
#include <algorithm> // for std::max, std::min

namespace zmon
{
    WaveformController::WaveformController(WaveformCache *waveformCache, QObject *parent)
        : QObject(parent),
          m_waveformCache(waveformCache),
          m_updateTimer(new QTimer(this))
    {
        // Set up 60 FPS timer (16ms interval)
        m_updateTimer->setInterval(16);
        connect(m_updateTimer, &QTimer::timeout, this, &WaveformController::updateWaveformData);
    }

    void WaveformController::setUpdateRate(int rate)
    {
        if (m_updateRate != rate)
        {
            m_updateRate = rate;
            emit updateRateChanged();

            // Update timer interval (milliseconds = 1000 / Hz)
            if (m_updateTimer)
            {
                m_updateTimer->setInterval(1000 / rate);
            }
        }
    }

    void WaveformController::setEcgGain(double gain)
    {
        if (m_ecgGain != gain)
        {
            m_ecgGain = gain;
            emit ecgGainChanged();
        }
    }

    void WaveformController::setPlethGain(double gain)
    {
        if (m_plethGain != gain)
        {
            m_plethGain = gain;
            emit plethGainChanged();
        }
    }

    void WaveformController::setSweepSpeed(double speed)
    {
        if (m_sweepSpeed != speed)
        {
            m_sweepSpeed = speed;
            emit sweepSpeedChanged();
        }
    }

    void WaveformController::startWaveforms()
    {
        if (m_updateTimer)
        {
            m_updateTimer->start();
        }
    }

    void WaveformController::stopWaveforms()
    {
        if (m_updateTimer)
        {
            m_updateTimer->stop();
        }

        m_ecgData.clear();
        m_plethData.clear();
        m_respData.clear();
        emit ecgDataChanged();
        emit plethDataChanged();
        emit respDataChanged();
    }

    void WaveformController::updateWaveformData()
    {
        if (!m_waveformCache)
        {
            return;
        }

        // Display window: 10 seconds for continuous waveform scrolling
        // Target points: 600 samples (60 FPS × 10 sec)
        // Raw sample rate: 250 Hz × 10 sec = 2,500 samples per channel
        // Decimation ratio: ~4:1 to achieve 60 FPS display rate
        const int displaySeconds = 10;
        const int targetPoints = 600; // 60 FPS × 10 sec display window

        // Get ECG samples from cache (raw 250 Hz)
        // Channel name must match WaveformSample::CHANNEL_ECG_LEAD_II = "ECG_LEAD_II"
        auto ecgSamples = m_waveformCache->getChannelSamples("ECG_LEAD_II", displaySeconds);
        m_ecgData.clear();

        if (!ecgSamples.empty())
        {
            // Calculate decimation step to achieve target point count
            // Use min-max decimation to preserve PQRST complex morphology
            size_t decimationStep = std::max(size_t(1), ecgSamples.size() / targetPoints);

            for (size_t i = 0; i < ecgSamples.size(); i += decimationStep)
            {
                // For better waveform morphology preservation, take min-max in decimation window
                size_t windowEnd = std::min(i + decimationStep, ecgSamples.size());

                // Find min and max in decimation window to preserve peaks/troughs
                auto minIt = ecgSamples.begin() + i;
                auto maxIt = ecgSamples.begin() + i;
                double minVal = ecgSamples[i].value;
                double maxVal = ecgSamples[i].value;
                int64_t avgTime = ecgSamples[i].timestampMs;

                for (size_t j = i; j < windowEnd; ++j)
                {
                    if (ecgSamples[j].value < minVal)
                    {
                        minVal = ecgSamples[j].value;
                        minIt = ecgSamples.begin() + j;
                    }
                    if (ecgSamples[j].value > maxVal)
                    {
                        maxVal = ecgSamples[j].value;
                        maxIt = ecgSamples.begin() + j;
                    }
                }

                // Add both min and max points to preserve morphology
                // Add min first if it appears before max in time
                if ((*minIt).timestampMs < (*maxIt).timestampMs)
                {
                    QVariantMap minPoint;
                    minPoint["time"] = (*minIt).timestampMs;
                    minPoint["value"] = minVal * m_ecgGain; // Apply gain
                    m_ecgData.append(minPoint);

                    QVariantMap maxPoint;
                    maxPoint["time"] = (*maxIt).timestampMs;
                    maxPoint["value"] = maxVal * m_ecgGain;
                    m_ecgData.append(maxPoint);
                }
                else
                {
                    QVariantMap maxPoint;
                    maxPoint["time"] = (*maxIt).timestampMs;
                    maxPoint["value"] = maxVal * m_ecgGain;
                    m_ecgData.append(maxPoint);

                    QVariantMap minPoint;
                    minPoint["time"] = (*minIt).timestampMs;
                    minPoint["value"] = minVal * m_ecgGain;
                    m_ecgData.append(minPoint);
                }
            }
        }
        emit ecgDataChanged();

        // Get Pleth samples from cache (raw 250 Hz)
        // Channel name must match WaveformSample::CHANNEL_PLETH = "PLETH"
        auto plethSamples = m_waveformCache->getChannelSamples("PLETH", displaySeconds);
        m_plethData.clear();

        if (!plethSamples.empty())
        {
            // Calculate decimation step
            size_t decimationStep = std::max(size_t(1), plethSamples.size() / targetPoints);

            for (size_t i = 0; i < plethSamples.size(); i += decimationStep)
            {
                // For pleth waveform, min-max decimation preserves pulse morphology
                size_t windowEnd = std::min(i + decimationStep, plethSamples.size());

                auto minIt = plethSamples.begin() + i;
                auto maxIt = plethSamples.begin() + i;
                double minVal = plethSamples[i].value;
                double maxVal = plethSamples[i].value;

                for (size_t j = i; j < windowEnd; ++j)
                {
                    if (plethSamples[j].value < minVal)
                    {
                        minVal = plethSamples[j].value;
                        minIt = plethSamples.begin() + j;
                    }
                    if (plethSamples[j].value > maxVal)
                    {
                        maxVal = plethSamples[j].value;
                        maxIt = plethSamples.begin() + j;
                    }
                }

                // Add min and max points in chronological order
                if ((*minIt).timestampMs < (*maxIt).timestampMs)
                {
                    QVariantMap minPoint;
                    minPoint["time"] = (*minIt).timestampMs;
                    minPoint["value"] = minVal * m_plethGain;
                    m_plethData.append(minPoint);

                    QVariantMap maxPoint;
                    maxPoint["time"] = (*maxIt).timestampMs;
                    maxPoint["value"] = maxVal * m_plethGain;
                    m_plethData.append(maxPoint);
                }
                else
                {
                    QVariantMap maxPoint;
                    maxPoint["time"] = (*maxIt).timestampMs;
                    maxPoint["value"] = maxVal * m_plethGain;
                    m_plethData.append(maxPoint);

                    QVariantMap minPoint;
                    minPoint["time"] = (*minIt).timestampMs;
                    minPoint["value"] = minVal * m_plethGain;
                    m_plethData.append(minPoint);
                }
            }
        }
        emit plethDataChanged();

        // Get Respiration samples from cache (raw 25 Hz)
        auto respSamples = m_waveformCache->getChannelSamples("RESP", displaySeconds);
        m_respData.clear();

        if (!respSamples.empty())
        {
            // Calculate decimation step (25 Hz is lower than 250 Hz, so less decimation needed)
            size_t decimationStep = std::max(size_t(1), respSamples.size() / targetPoints);

            for (size_t i = 0; i < respSamples.size(); i += decimationStep)
            {
                size_t windowEnd = std::min(i + decimationStep, respSamples.size());

                auto minIt = respSamples.begin() + i;
                auto maxIt = respSamples.begin() + i;
                double minVal = respSamples[i].value;
                double maxVal = respSamples[i].value;

                for (size_t j = i; j < windowEnd; ++j)
                {
                    if (respSamples[j].value < minVal)
                    {
                        minVal = respSamples[j].value;
                        minIt = respSamples.begin() + j;
                    }
                    if (respSamples[j].value > maxVal)
                    {
                        maxVal = respSamples[j].value;
                        maxIt = respSamples.begin() + j;
                    }
                }

                // Add min and max points in chronological order
                if ((*minIt).timestampMs < (*maxIt).timestampMs)
                {
                    QVariantMap minPoint;
                    minPoint["time"] = (*minIt).timestampMs;
                    minPoint["value"] = minVal;
                    m_respData.append(minPoint);

                    QVariantMap maxPoint;
                    maxPoint["time"] = (*maxIt).timestampMs;
                    maxPoint["value"] = maxVal;
                    m_respData.append(maxPoint);
                }
                else
                {
                    QVariantMap maxPoint;
                    maxPoint["time"] = (*maxIt).timestampMs;
                    maxPoint["value"] = maxVal;
                    m_respData.append(maxPoint);

                    QVariantMap minPoint;
                    minPoint["time"] = (*minIt).timestampMs;
                    minPoint["value"] = minVal;
                    m_respData.append(minPoint);
                }
            }
        }
        emit respDataChanged();
    }
} // namespace zmon
