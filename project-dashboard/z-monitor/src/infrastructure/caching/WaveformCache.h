/**
 * @file WaveformCache.h
 * @brief Circular buffer for waveform sample storage.
 *
 * Provides thread-safe circular buffer for 30 seconds of waveform data (~0.1 MB).
 * Display-only cache, not persisted to database.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include "domain/monitoring/WaveformSample.h"
#include <QReadWriteLock>
#include <deque>
#include <vector>
#include <QString>
#include <cstddef>

namespace zmon
{
    /**
     * @class WaveformCache
     * @brief Thread-safe circular buffer for waveform samples.
     *
     * Stores waveform samples in circular buffer with 30-second capacity.
     * Channels: ECG, Pleth, Respiration
     * Sample rate: 250 Hz per channel
     * Total capacity: 30 sec × 250 Hz × 3 channels = 22,500 samples (~0.1 MB)
     *
     * Display-only - not persisted to database.
     *
     * @thread Thread-safe (uses QReadWriteLock)
     * @ingroup Infrastructure
     */
    class WaveformCache
    {
    public:
        /**
         * @brief Constructor.
         *
         * @param capacity Maximum number of samples (default: 22,500)
         */
        explicit WaveformCache(size_t capacity = 22500);

        /**
         * @brief Append waveform sample to cache.
         *
         * Thread-safe. Overwrites oldest sample in circular buffer.
         *
         * @param sample Waveform sample to append
         */
        void append(const WaveformSample &sample);

        /**
         * @brief Get last N seconds of samples.
         *
         * Thread-safe read operation.
         *
         * @param seconds Number of seconds to retrieve
         * @return Vector of samples from last N seconds
         */
        std::vector<WaveformSample> getLastSeconds(int seconds) const;

        /**
         * @brief Get samples for specific channel.
         *
         * Thread-safe read operation.
         *
         * @param channel Channel name (e.g., "ecg", "pleth")
         * @param seconds Number of seconds to retrieve
         * @return Vector of channel samples from last N seconds
         */
        std::vector<WaveformSample> getChannelSamples(const QString &channel, int seconds) const;

        /**
         * @brief Clear all cached samples.
         *
         * Thread-safe write operation.
         */
        void clear();

        /**
         * @brief Get current cache size.
         *
         * Thread-safe read operation.
         *
         * @return Number of samples in cache
         */
        size_t size() const;

    private:
        mutable QReadWriteLock m_lock;
        std::deque<WaveformSample> m_samples; // Deque for efficient front/back operations
        size_t m_capacity;
    };

} // namespace zmon
