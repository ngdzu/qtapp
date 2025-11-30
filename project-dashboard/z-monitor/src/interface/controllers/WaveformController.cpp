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
        emit ecgDataChanged();
        emit plethDataChanged();
    }

    void WaveformController::updateWaveformData()
    {
        if (!m_waveformCache)
        {
            return;
        }

        // Get last 6 seconds for ECG (250 Hz × 6 sec = 1,500 samples per channel)
        // Display sweeps at 25 mm/s across typical 6-second display width
        int displaySeconds = 6;

        // Get ECG samples
        auto ecgSamples = m_waveformCache->getChannelSamples("ecg", displaySeconds);
        m_ecgData.clear();
        for (const auto &sample : ecgSamples)
        {
            QVariantMap point;
            point["time"] = sample.timestampMs;
            point["value"] = sample.value * m_ecgGain; // Apply gain
            m_ecgData.append(point);
        }
        emit ecgDataChanged();

        // Get Pleth samples
        auto plethSamples = m_waveformCache->getChannelSamples("pleth", displaySeconds);
        m_plethData.clear();
        for (const auto &sample : plethSamples)
        {
            QVariantMap point;
            point["time"] = sample.timestampMs;
            point["value"] = sample.value * m_plethGain; // Apply gain
            m_plethData.append(point);
        }
        emit plethDataChanged();
    }
} // namespace zmon
