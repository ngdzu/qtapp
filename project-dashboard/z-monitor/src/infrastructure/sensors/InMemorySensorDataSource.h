/**
 * @file InMemorySensorDataSource.h
 * @brief In-memory sensor data source implementation (simulated data generator).
 *
 * This file defines the InMemorySensorDataSource class which implements
 * ISensorDataSource by generating realistic simulated sensor data in-memory.
 * This allows Z Monitor to display data without requiring an external sensor
 * simulator process.
 *
 * **Data Generation:**
 * - Vitals: Generated at 1 Hz (once per second) with realistic values
 *   - Heart rate: 60-100 BPM (normal sinus rhythm)
 *   - SpO2: 95-100% (normal range)
 *   - Respiration rate: 12-20 rpm (normal range)
 *   - NIBP: Systolic 110-130 mmHg, Diastolic 70-85 mmHg
 *   - Temperature: 36.5-37.5°C (normal range)
 * - Waveforms: Generated at 250 Hz (4ms intervals)
 *   - ECG Lead II: Realistic QRS complexes with baseline wander and noise
 *   - SpO2 Pleth: Pulse waveform synchronized with heart rate
 *   - Respiration: Sinusoidal waveform at respiration rate
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <random>
#include <cmath>

#include "infrastructure/interfaces/ISensorDataSource.h"

namespace zmon {

/**
 * @class InMemorySensorDataSource
 * @brief In-memory sensor data source implementation.
 *
 * This class implements ISensorDataSource by generating realistic simulated
 * sensor data in-memory. It provides:
 * - Realistic vital signs generation (HR, SpO2, RR, NIBP, Temp)
 * - Realistic waveform generation (ECG, SpO2 pleth, Respiration)
 * - Configurable data rates (vitals at 1 Hz, waveforms at 250 Hz)
 * - Deterministic generation for testing (optional seed)
 *
 * @note Runs on Real-Time Processing Thread (high priority)
 * @note Uses QTimer for periodic data generation
 * @see ISensorDataSource, MonitoringService
 * @ingroup Infrastructure
 */
class InMemorySensorDataSource : public ISensorDataSource {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * @param seed Random seed for deterministic generation (0 = use current time)
     * @param parent Parent QObject
     */
    explicit InMemorySensorDataSource(unsigned int seed = 0, QObject* parent = nullptr);
    
    /**
     * @brief Destructor.
     *
     * Stops data generation if active.
     */
    ~InMemorySensorDataSource() override;
    
    // ISensorDataSource interface
    /**
     * @brief Start data generation.
     *
     * Starts timers for vitals and waveform generation.
     *
     * @return Result<void> - Success if started successfully, Error if failed
     */
    Result<void> start() override;
    
    /**
     * @brief Stop data generation.
     *
     * Stops all timers and stops emitting signals.
     */
    void stop() override;
    
    /**
     * @brief Check if data source is currently active.
     *
     * @return true if actively generating data
     */
    bool isActive() const override {
        return m_active;
    }
    
    /**
     * @brief Get data source metadata.
     *
     * @return DataSourceInfo Metadata about this data source
     */
    DataSourceInfo getInfo() const override;
    
    /**
     * @brief Get current sampling rate.
     *
     * @return Sampling rate in Hz (250.0 for waveforms, 1.0 for vitals)
     */
    double getSamplingRate() const override {
        return 250.0;  // Waveforms at 250 Hz
    }

private slots:
    /**
     * @brief Generate and emit vital signs.
     *
     * Called periodically (1 Hz) to generate and emit vital signs.
     */
    void generateVitals();
    
    /**
     * @brief Generate and emit waveform samples.
     *
     * Called periodically (250 Hz) to generate and emit waveform samples.
     */
    void generateWaveformSamples();

private:
    /**
     * @brief Generate ECG waveform sample.
     *
     * @param timeMs Current time in milliseconds
     * @return ECG sample value
     */
    double generateECGSample(int64_t timeMs);
    
    /**
     * @brief Generate SpO2 pleth waveform sample.
     *
     * @param timeMs Current time in milliseconds
     * @return Pleth sample value
     */
    double generatePlethSample(int64_t timeMs);
    
    /**
     * @brief Generate respiration waveform sample.
     *
     * @param timeMs Current time in milliseconds
     * @return Respiration sample value
     */
    double generateRespirationSample(int64_t timeMs);
    
    /**
     * @brief Get current heart rate (BPM).
     *
     * @return Current heart rate in BPM
     */
    double getCurrentHeartRate() const;
    
    /**
     * @brief Get current respiration rate (rpm).
     *
     * @return Current respiration rate in rpm
     */
    double getCurrentRespirationRate() const;
    
    bool m_active;                          ///< Active state
    QTimer* m_vitalsTimer;                 ///< Timer for vitals generation (1 Hz)
    QTimer* m_waveformTimer;                ///< Timer for waveform generation (250 Hz)
    QElapsedTimer m_elapsedTimer;           ///< Elapsed time since start
    int64_t m_startTimeMs;                  ///< Start time in milliseconds (epoch)
    
    // Random number generation
    std::mt19937 m_rng;                     ///< Mersenne Twister random number generator
    std::uniform_real_distribution<double> m_uniformDist;  ///< Uniform distribution [0, 1)
    std::normal_distribution<double> m_normalDist;         ///< Normal distribution (mean=0, std=1)
    
    // Vital signs state (for realistic variation)
    double m_baseHeartRate;                 ///< Base heart rate (BPM)
    double m_baseSpO2;                      ///< Base SpO2 (%)
    double m_baseRespirationRate;           ///< Base respiration rate (rpm)
    double m_baseSystolicBP;               ///< Base systolic blood pressure (mmHg)
    double m_baseDiastolicBP;               ///< Base diastolic blood pressure (mmHg)
    double m_baseTemperature;               ///< Base temperature (°C)
    
    // Waveform generation state
    double m_ecgPhase;                      ///< ECG waveform phase (for QRS generation)
    double m_plethPhase;                    ///< Pleth waveform phase
    double m_respirationPhase;              ///< Respiration waveform phase
    double m_baselineWander;                ///< ECG baseline wander value
};

} // namespace zmon

