/**
 * @file VitalsCache.cpp
 * @brief Implementation of VitalsCache.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "infrastructure/caching/VitalsCache.h"
#include <QWriteLocker>
#include <QReadLocker>
#include <algorithm>

namespace zmon
{

    VitalsCache::VitalsCache(size_t maxCapacity)
        : m_lastPersistedTimestampMs(0), m_maxCapacity(maxCapacity)
    {
        // std::deque doesn't need pre-allocation - it manages chunks efficiently
    }

    void VitalsCache::append(const VitalRecord &vital)
    {
        QWriteLocker lock(&m_lock);

        // Evict old records if at capacity
        if (m_vitals.size() >= m_maxCapacity)
        {
            // Remove oldest 10% to avoid frequent evictions
            size_t removeCount = m_maxCapacity / 10;
            for (size_t i = 0; i < removeCount; ++i)
            {
                m_vitals.pop_front(); // Efficient O(1) removal from front
            }
        }

        // Move construct into deque
        m_vitals.push_back(std::move(vital));
    }

    std::vector<VitalRecord> VitalsCache::getRange(int64_t startMs, int64_t endMs) const
    {
        QReadLocker lock(&m_lock);

        std::vector<VitalRecord> result;
        for (const auto &vital : m_vitals)
        {
            if (vital.timestampMs >= startMs && vital.timestampMs <= endMs)
            {
                // Copy construct (const members prevent assignment but allow copy construction)
                result.push_back(vital);
            }
        }
        return result;
    }

    std::vector<VitalRecord> VitalsCache::getUnpersistedVitals() const
    {
        QReadLocker lock(&m_lock);

        std::vector<VitalRecord> result;
        for (const auto &vital : m_vitals)
        {
            if (vital.timestampMs > m_lastPersistedTimestampMs)
            {
                result.push_back(vital);
            }
        }
        return result;
    }

    void VitalsCache::markAsPersisted(int64_t upToTimestampMs)
    {
        QWriteLocker lock(&m_lock);
        m_lastPersistedTimestampMs = upToTimestampMs;
    }

    size_t VitalsCache::size() const
    {
        QReadLocker lock(&m_lock);
        return static_cast<size_t>(m_vitals.size());
    }

    void VitalsCache::clear()
    {
        QWriteLocker lock(&m_lock);
        m_vitals.clear();
        m_lastPersistedTimestampMs = 0;
    }

    VitalRecord VitalsCache::getLatest(const std::string &vitalType, bool &found) const
    {
        QReadLocker lock(&m_lock);

        // Search backwards for latest matching type
        for (int i = m_vitals.size() - 1; i >= 0; --i)
        {
            if (m_vitals[i].vitalType == vitalType)
            {
                found = true;
                return m_vitals[i];
            }
        }

        // Return default (zero) record if not found
        found = false;
        return VitalRecord{"", 0.0, 0, 0};
    }

} // namespace zmon
