/**
 * @file VitalsCache.h
 * @brief In-memory cache for vital signs data.
 *
 * Provides thread-safe caching of vital signs with 3-day capacity (~39 MB).
 * Used for UI display and batch persistence to database.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include "domain/monitoring/VitalRecord.h"
#include <QReadWriteLock>
#include <deque>
#include <vector>
#include <cstddef>

namespace zmon
{
    /**
     * @class VitalsCache
     * @brief Thread-safe in-memory cache for vital signs.
     *
     * Stores vital signs in memory with 3-day capacity (~259,200 records at 60 Hz).
     * Provides range queries and persistence tracking.
     *
     * Memory estimate: ~39 MB (150 bytes per record × 259,200 records)
     *
     * @thread Thread-safe (uses QReadWriteLock)
     * @ingroup Infrastructure
     */
    class VitalsCache
    {
    public:
        /**
         * @brief Constructor.
         *
         * @param maxCapacity Maximum number of vitals to store (default: 259,200 = 3 days at 60 Hz)
         */
        explicit VitalsCache(size_t maxCapacity = 259200);

        /**
         * @brief Append vital record to cache.
         *
         * Thread-safe. If at capacity, removes oldest 10% before appending.
         *
         * @param vital Vital record to append
         */
        void append(const VitalRecord &vital);

        /**
         * @brief Get vitals in time range.
         *
         * Thread-safe read operation.
         *
         * @param start Start timestamp
         * @param end End timestamp
         * @return Vector of vitals in range (moved)
         */
        std::vector<VitalRecord> getRange(int64_t startMs, int64_t endMs) const;

        /**
         * @brief Get vitals not yet persisted to database.
         *
         * Thread-safe read operation.
         *
         * @return Vector of unpersisted vitals (moved)
         */
        std::vector<VitalRecord> getUnpersistedVitals() const;

        /**
         * @brief Mark vitals as persisted up to timestamp.
         *
         * Thread-safe write operation.
         *
         * @param upToTimestampMs Timestamp up to which vitals are persisted
         */
        void markAsPersisted(int64_t upToTimestampMs);

        /**
         * @brief Get current cache size.
         *
         * Thread-safe read operation.
         *
         * @return Number of vitals in cache
         */
        size_t size() const;

        /**
         * @brief Clear all cached vitals.
         *
         * Thread-safe write operation.
         */
        void clear();

        /**
         * @brief Get latest vital of specific type.
         *
         * Thread-safe read operation.
         *
         * @param vitalType Vital type (e.g., "HR", "SPO2")
         * @param found Output parameter set to true if found, false otherwise
         * @return Latest vital of type (copy), or default VitalRecord if not found
         */
        VitalRecord getLatest(const std::string &vitalType, bool &found) const;

    private:
        mutable QReadWriteLock m_lock;
        std::deque<VitalRecord> m_vitals; // deque supports efficient pop_front with move-only types
        int64_t m_lastPersistedTimestampMs;
        size_t m_maxCapacity;
    };

} // namespace zmon
