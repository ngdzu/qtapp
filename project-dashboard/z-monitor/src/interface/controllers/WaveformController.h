/**
 * @file WaveformController.h
 * @brief QML controller for ECG and plethysmograph waveform display.
 *
 * Provides high-frequency waveform data to QML UI layer for 60 FPS rendering.
 * Implements buffer management and decimation strategies per doc/41_WAVEFORM_DISPLAY_IMPLEMENTATION.md.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 * @ingroup interface
 */

#pragma once

#include <QObject>
#include <QVariantList>
#include <QTimer>

namespace zmon
{
    // Forward declarations
    class WaveformCache;
    /**
     * @class WaveformController
     * @brief Controller for ECG and plethysmograph waveform display.
     *
     * This controller manages high-frequency waveform data for QML rendering at 60 FPS.
     * It delegates to MonitoringService for raw waveform data and implements buffering
     * and decimation strategies for efficient rendering.
     *
     * @thread UI thread (all property access and methods)
     */
    class WaveformController : public QObject
    {
        Q_OBJECT

        /// ECG waveform data buffer (array of {time, value} objects)
        Q_PROPERTY(QVariantList ecgData READ ecgData NOTIFY ecgDataChanged)

        /// Plethysmograph waveform data buffer (array of {time, value} objects)
        Q_PROPERTY(QVariantList plethData READ plethData NOTIFY plethDataChanged)

        /// Waveform update rate in Hz (default: 60 FPS)
        Q_PROPERTY(int updateRate READ updateRate WRITE setUpdateRate NOTIFY updateRateChanged)

        /// ECG gain (mV/mm)
        Q_PROPERTY(double ecgGain READ ecgGain WRITE setEcgGain NOTIFY ecgGainChanged)

        /// Pleth gain (arbitrary units)
        Q_PROPERTY(double plethGain READ plethGain WRITE setPlethGain NOTIFY plethGainChanged)

        /// Waveform sweep speed (mm/s)
        Q_PROPERTY(double sweepSpeed READ sweepSpeed WRITE setSweepSpeed NOTIFY sweepSpeedChanged)

    public:
        /**
         * @brief Constructs WaveformController.
         * @param waveformCache Waveform cache for reading samples
         * @param parent Parent QObject
         */
        explicit WaveformController(WaveformCache *waveformCache = nullptr,
                                    QObject *parent = nullptr);

        /**
         * @brief Gets ECG waveform data.
         * @return List of {time, value} objects
         */
        QVariantList ecgData() const { return m_ecgData; }

        /**
         * @brief Gets pleth waveform data.
         * @return List of {time, value} objects
         */
        QVariantList plethData() const { return m_plethData; }

        /**
         * @brief Gets waveform update rate.
         * @return Update rate in Hz
         */
        int updateRate() const { return m_updateRate; }

        /**
         * @brief Sets waveform update rate.
         * @param rate Update rate in Hz (typically 60)
         */
        void setUpdateRate(int rate);

        /**
         * @brief Gets ECG gain.
         * @return Gain in mV/mm
         */
        double ecgGain() const { return m_ecgGain; }

        /**
         * @brief Sets ECG gain.
         * @param gain Gain in mV/mm
         */
        void setEcgGain(double gain);

        /**
         * @brief Gets pleth gain.
         * @return Gain in arbitrary units
         */
        double plethGain() const { return m_plethGain; }

        /**
         * @brief Sets pleth gain.
         * @param gain Gain in arbitrary units
         */
        void setPlethGain(double gain);

        /**
         * @brief Gets waveform sweep speed.
         * @return Sweep speed in mm/s
         */
        double sweepSpeed() const { return m_sweepSpeed; }

        /**
         * @brief Sets waveform sweep speed.
         * @param speed Sweep speed in mm/s
         */
        void setSweepSpeed(double speed);

        /**
         * @brief Starts waveform updates.
         *
         * Begins receiving waveform data from MonitoringService at configured update rate.
         */
        Q_INVOKABLE void startWaveforms();

        /**
         * @brief Stops waveform updates.
         *
         * Stops receiving waveform data and clears buffers.
         */
        Q_INVOKABLE void stopWaveforms();

    signals:
        void ecgDataChanged();
        void plethDataChanged();
        void updateRateChanged();
        void ecgGainChanged();
        void plethGainChanged();
        void sweepSpeedChanged();

    private slots:
        /**
         * @brief Update waveform data from cache (60 FPS timer).
         */
        void updateWaveformData();

    private:
        WaveformCache *m_waveformCache; ///< Waveform cache (not owned)
        QTimer *m_updateTimer;          ///< 60 FPS update timer
        QVariantList m_ecgData;
        QVariantList m_plethData;
        int m_updateRate{60};
        double m_ecgGain{10.0};
        double m_plethGain{1.0};
        double m_sweepSpeed{25.0};
    };
} // namespace zmon
