/**
 * @file WaveformSample.h
 * @brief Value object representing a single waveform sample.
 *
 * This file contains the WaveformSample value object which represents a single
 * waveform sample (ECG, plethysmogram, etc.) with channel, value, and timestamp.
 * Value objects are immutable and defined by their attributes.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace zmon
{
    /**
     * @class WaveformSample
     * @brief Immutable value object representing a single waveform sample.
     *
     * This value object encapsulates a waveform sample with its channel identifier,
     * value, and timestamp. It is immutable and can be safely shared across threads.
     *
     * @note This is a value object - it has no identity and is defined by its attributes.
     * @note All members are const to enforce immutability.
     */
    struct WaveformSample
    {
        /**
         * @brief Waveform channel identifier.
         *
         * Examples: "ECG_LEAD_II", "PLETH", "ECG_LEAD_I".
         */
        const std::string channel;

        /**
         * @brief Sample value.
         *
         * The numeric value of the waveform sample (typically signed integer or float).
         */
        const double value;

        /**
         * @brief Timestamp when the sample was captured.
         *
         * Unix timestamp in milliseconds (epoch milliseconds).
         */
        const int64_t timestampMs;

        /**
         * @brief Sample rate in Hz.
         *
         * The sample rate at which this sample was captured (e.g., 250 Hz for ECG).
         */
        const double sampleRateHz;

        /**
         * @brief Default constructor.
         *
         * Creates an empty waveform sample with default values.
         */
        WaveformSample()
            : channel(""), value(0.0), timestampMs(0), sampleRateHz(0.0)
        {
        }

        /**
         * @brief Constructor with all parameters.
         *
         * @param ch Channel identifier (e.g., "ECG_LEAD_II", "PLETH")
         * @param val Sample value
         * @param ts Timestamp in milliseconds (epoch milliseconds)
         * @param rate Sample rate in Hz (e.g., 250.0 for ECG)
         */
        WaveformSample(const std::string &ch, double val, int64_t ts, double rate)
            : channel(ch), value(val), timestampMs(ts), sampleRateHz(rate)
        {
        }

        /**
         * @brief Common channel name constants to avoid hardcoded strings.
         *
         * Use these constants when constructing samples or composing channel
         * identifiers. These are constexpr `std::string_view` to avoid static
         * initialization order issues and to keep them usable in compile-time
         * contexts where appropriate.
         */
        static constexpr std::string_view CHANNEL_ECG_LEAD_II = "ECG_LEAD_II";
        static constexpr std::string_view CHANNEL_PLETH = "PLETH";

        /**
         * @brief Create a WaveformSample from explicit parameters.
         *
         * This static factory mirrors the main constructor but provides a
         * clearer, named entry point for code that prefers factory-style
         * construction (e.g. `WaveformSample::from(...)`).
         */
        static WaveformSample from(const std::string &ch, double val, int64_t ts, double rate)
        {
            return WaveformSample(ch, val, ts, rate);
        }

        /**
         * @brief Factory for a common ECG lead II channel sample.
         *
         * Convenience factory that sets the channel name to `ECG_LEAD_II`.
         */
        static WaveformSample ECGLeadII(double val, int64_t ts, double rate)
        {
            return WaveformSample(std::string(CHANNEL_ECG_LEAD_II), val, ts, rate);
        }

        /**
         * @brief Factory for a plethysmogram (PLETH) channel sample.
         */
        static WaveformSample PLETH(double val, int64_t ts, double rate)
        {
            return WaveformSample(std::string(CHANNEL_PLETH), val, ts, rate);
        }

        /**
         * @brief Copy constructor.
         *
         * @param other Source waveform sample
         */
        WaveformSample(const WaveformSample &other) = default;

        /**
         * @brief Assignment operator (deleted - value objects are immutable).
         */
        WaveformSample &operator=(const WaveformSample &) = delete;

        /**
         * @brief Equality comparison.
         *
         * Two waveform samples are equal if all their attributes match.
         *
         * @param other Other waveform sample to compare
         * @return true if all attributes are equal, false otherwise
         */
        bool operator==(const WaveformSample &other) const
        {
            return channel == other.channel &&
                   value == other.value &&
                   timestampMs == other.timestampMs &&
                   sampleRateHz == other.sampleRateHz;
        }

        /**
         * @brief Inequality comparison.
         *
         * @param other Other waveform sample to compare
         * @return true if any attribute differs, false otherwise
         */
        bool operator!=(const WaveformSample &other) const
        {
            return !(*this == other);
        }
    };

} // namespace zmon