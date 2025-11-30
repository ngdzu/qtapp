/**
 * @file WaveformCache.cpp
 * @brief Implementation of WaveformCache.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "infrastructure/caching/WaveformCache.h"
#include <QWriteLocker>
#include <QReadLocker>
#include <algorithm>

namespace zmon
{

    WaveformCache::WaveformCache(size_t capacity)
        : m_capacity(capacity)
    {
        // std::deque will grow dynamically
    }

    void WaveformCache::append(const WaveformSample &sample)
    {
        QWriteLocker lock(&m_lock);

        // Add to back
        m_samples.push_back(sample);

        // Remove from front if over capacity
        while (m_samples.size() > m_capacity)
        {
            m_samples.pop_front();
        }
    }

    std::vector<WaveformSample> WaveformCache::getLastSeconds(int seconds) const
    {
        QReadLocker lock(&m_lock);

        // Calculate number of samples (250 Hz sample rate)
        size_t sampleCount = std::min(static_cast<size_t>(seconds * 250), m_samples.size());
        std::vector<WaveformSample> result;
        result.reserve(sampleCount);

        // Get last N samples from deque
        auto startIt = m_samples.end() - sampleCount;
        for (auto it = startIt; it != m_samples.end(); ++it)
        {
            result.push_back(*it);
        }
        return result;
    }

    std::vector<WaveformSample> WaveformCache::getChannelSamples(const QString &channel, int seconds) const
    {
        QReadLocker lock(&m_lock);

        // Calculate number of samples to check
        size_t sampleCount = std::min(static_cast<size_t>(seconds * 250), m_samples.size());
        std::vector<WaveformSample> result;

        // Get last N samples and filter by channel
        auto startIt = m_samples.end() - sampleCount;
        for (auto it = startIt; it != m_samples.end(); ++it)
        {
            if (QString::fromStdString(it->channel) == channel)
            {
                result.push_back(*it);
            }
        }
        return result;
    }

    void WaveformCache::clear()
    {
        QWriteLocker lock(&m_lock);
        m_samples.clear();
    }

    size_t WaveformCache::size() const
    {
        QReadLocker lock(&m_lock);
        return m_samples.size();
    }

} // namespace zmon
