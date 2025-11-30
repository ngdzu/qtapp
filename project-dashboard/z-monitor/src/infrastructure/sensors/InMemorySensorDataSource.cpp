/**
 * @file InMemorySensorDataSource.cpp
 * @brief In-memory sensor data source implementation.
 *
 * This file implements the InMemorySensorDataSource class which generates
 * realistic simulated sensor data in-memory.
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/sensors/InMemorySensorDataSource.h"
#include <QDateTime>
#include <QDebug>
#include <cmath>
#include <algorithm>

namespace zmon {

InMemorySensorDataSource::InMemorySensorDataSource(unsigned int seed, QObject* parent)
    : ISensorDataSource(parent)
    , m_active(false)
    , m_vitalsTimer(new QTimer(this))
    , m_waveformTimer(new QTimer(this))
    , m_startTimeMs(0)
    , m_rng(seed == 0 ? std::random_device{}() : seed)
    , m_uniformDist(0.0, 1.0)
    , m_normalDist(0.0, 1.0)
    , m_baseHeartRate(72.0 + m_uniformDist(m_rng) * 20.0)  // 72-92 BPM
    , m_baseSpO2(97.0 + m_uniformDist(m_rng) * 3.0)       // 97-100%
    , m_baseRespirationRate(16.0 + m_uniformDist(m_rng) * 4.0)  // 16-20 rpm
    , m_baseSystolicBP(120.0 + m_uniformDist(m_rng) * 10.0)  // 120-130 mmHg
    , m_baseDiastolicBP(75.0 + m_uniformDist(m_rng) * 10.0)  // 75-85 mmHg
    , m_baseTemperature(36.8 + m_uniformDist(m_rng) * 0.7)  // 36.8-37.5°C
    , m_ecgPhase(0.0)
    , m_plethPhase(0.0)
    , m_respirationPhase(0.0)
    , m_baselineWander(0.0)
{
    // Configure vitals timer (1 Hz = 1000ms interval)
    m_vitalsTimer->setInterval(1000);
    m_vitalsTimer->setSingleShot(false);
    connect(m_vitalsTimer, &QTimer::timeout, this, &InMemorySensorDataSource::generateVitals);
    
    // Configure waveform timer (250 Hz = 4ms interval)
    m_waveformTimer->setInterval(4);
    m_waveformTimer->setSingleShot(false);
    connect(m_waveformTimer, &QTimer::timeout, this, &InMemorySensorDataSource::generateWaveformSamples);
}

InMemorySensorDataSource::~InMemorySensorDataSource()
{
    stop();
}

Result<void> InMemorySensorDataSource::start()
{
    if (m_active) {
        return Result<void>::ok();
    }
    
    m_startTimeMs = QDateTime::currentMSecsSinceEpoch();
    m_elapsedTimer.start();
    
    // Initialize waveform phases
    m_ecgPhase = 0.0;
    m_plethPhase = 0.0;
    m_respirationPhase = 0.0;
    m_baselineWander = 0.0;
    
    // Start timers
    m_vitalsTimer->start();
    m_waveformTimer->start();
    
    m_active = true;
    
    // Emit connection status
    emit connectionStatusChanged(true, "ECG");
    emit connectionStatusChanged(true, "SpO2");
    emit connectionStatusChanged(true, "RESP");
    
    // Emit started signal
    emit started();
    
    qInfo() << "InMemorySensorDataSource: Started - generating simulated data";
    
    return Result<void>::ok();
}

void InMemorySensorDataSource::stop()
{
    if (!m_active) {
        return;
    }
    
    m_vitalsTimer->stop();
    m_waveformTimer->stop();
    
    m_active = false;
    
    // Emit connection status
    emit connectionStatusChanged(false, "ECG");
    emit connectionStatusChanged(false, "SpO2");
    emit connectionStatusChanged(false, "RESP");
    
    // Emit stopped signal
    emit stopped();
    
    qInfo() << "InMemorySensorDataSource: Stopped";
}

DataSourceInfo InMemorySensorDataSource::getInfo() const
{
    DataSourceInfo info;
    info.name = "In-Memory Simulator";
    info.type = "SIMULATOR";
    info.version = "1.0.0";
    info.capabilities = QStringList() << "HR" << "SPO2" << "RR" << "NIBP" << "TEMP" << "ECG" << "PLETH" << "RESP";
    info.supportsWaveforms = true;
    return info;
}

void InMemorySensorDataSource::generateVitals()
{
    if (!m_active) {
        return;
    }
    
    int64_t timestampMs = QDateTime::currentMSecsSinceEpoch();
    
    // Add small random variation to base values (realistic drift)
    double hrVariation = m_normalDist(m_rng) * 2.0;  // ±2 BPM variation
    double hr = std::max(60.0, std::min(100.0, m_baseHeartRate + hrVariation));
    
    double spo2Variation = m_normalDist(m_rng) * 0.5;  // ±0.5% variation
    double spo2 = std::max(95.0, std::min(100.0, m_baseSpO2 + spo2Variation));
    
    double rrVariation = m_normalDist(m_rng) * 1.0;  // ±1 rpm variation
    double rr = std::max(12.0, std::min(20.0, m_baseRespirationRate + rrVariation));
    
    double sysVariation = m_normalDist(m_rng) * 3.0;  // ±3 mmHg variation
    double systolic = std::max(110.0, std::min(130.0, m_baseSystolicBP + sysVariation));
    
    double diaVariation = m_normalDist(m_rng) * 2.0;  // ±2 mmHg variation
    double diastolic = std::max(70.0, std::min(85.0, m_baseDiastolicBP + diaVariation));
    
    double tempVariation = m_normalDist(m_rng) * 0.1;  // ±0.1°C variation
    double temp = std::max(36.5, std::min(37.5, m_baseTemperature + tempVariation));
    
    // Emit vital signs (using standard vital type names)
    emit vitalSignsReceived(VitalRecord("HR", hr, timestampMs, 100, "", "IN_MEMORY_SIM"));
    emit vitalSignsReceived(VitalRecord("SPO2", spo2, timestampMs, 100, "", "IN_MEMORY_SIM"));
    emit vitalSignsReceived(VitalRecord("RR", rr, timestampMs, 100, "", "IN_MEMORY_SIM"));
    emit vitalSignsReceived(VitalRecord("NIBP_SYS", systolic, timestampMs, 100, "", "IN_MEMORY_SIM"));
    emit vitalSignsReceived(VitalRecord("NIBP_DIA", diastolic, timestampMs, 100, "", "IN_MEMORY_SIM"));
    emit vitalSignsReceived(VitalRecord("TEMP", temp, timestampMs, 100, "", "IN_MEMORY_SIM"));
}

void InMemorySensorDataSource::generateWaveformSamples()
{
    if (!m_active) {
        return;
    }
    
    int64_t timestampMs = QDateTime::currentMSecsSinceEpoch();
    double elapsedSeconds = m_elapsedTimer.elapsed() / 1000.0;
    
    // Generate ECG sample (Lead II)
    double ecgValue = generateECGSample(timestampMs);
    emit waveformSampleReceived(WaveformSample::ECGLeadII(ecgValue, timestampMs, 250.0));
    
    // Generate SpO2 pleth sample
    double plethValue = generatePlethSample(timestampMs);
    emit waveformSampleReceived(WaveformSample::PLETH(plethValue, timestampMs, 250.0));
    
    // Generate respiration sample (at 25 Hz, so emit every 10th call)
    static int respCounter = 0;
    respCounter++;
    if (respCounter >= 10) {  // 250 Hz / 10 = 25 Hz
        respCounter = 0;
        double respValue = generateRespirationSample(timestampMs);
        emit waveformSampleReceived(WaveformSample("RESP", respValue, timestampMs, 25.0));
    }
}

double InMemorySensorDataSource::generateECGSample(int64_t timeMs)
{
    // Convert time to seconds for phase calculation
    double timeSeconds = (timeMs - m_startTimeMs) / 1000.0;
    
    // Get current heart rate
    double hr = getCurrentHeartRate();
    double heartRateHz = hr / 60.0;  // Convert BPM to Hz
    
    // Update ECG phase (for QRS complex generation)
    m_ecgPhase += heartRateHz * (4.0 / 250.0);  // 4ms interval at 250 Hz
    if (m_ecgPhase >= 1.0) {
        m_ecgPhase -= 1.0;
    }
    
    // Generate baseline wander (slow drift, ~0.5 Hz)
    m_baselineWander += 0.002 * M_PI;  // ~0.5 Hz at 250 Hz sample rate
    if (m_baselineWander > 2.0 * M_PI) {
        m_baselineWander -= 2.0 * M_PI;
    }
    double baseline = 0.1 * std::sin(m_baselineWander);
    
    // Generate QRS complex (simplified model)
    double qrsValue = 0.0;
    double qrsPhase = m_ecgPhase * 2.0 * M_PI;
    
    // QRS complex: sharp positive spike
    if (qrsPhase < 0.1) {  // Q wave (small negative)
        qrsValue = -0.1 * std::sin(qrsPhase * 10.0 * M_PI);
    } else if (qrsPhase < 0.3) {  // R wave (large positive)
        qrsValue = 1.0 * std::sin((qrsPhase - 0.1) * 5.0 * M_PI);
    } else if (qrsPhase < 0.4) {  // S wave (small negative)
        qrsValue = -0.2 * std::sin((qrsPhase - 0.3) * 10.0 * M_PI);
    }
    
    // T wave (after QRS)
    if (qrsPhase > 0.4 && qrsPhase < 0.7) {
        double tPhase = (qrsPhase - 0.4) / 0.3;
        qrsValue += 0.3 * std::sin(tPhase * M_PI);
    }
    
    // Add noise (60 Hz line noise + random noise)
    double noise = m_normalDist(m_rng) * 0.05;  // Random noise
    double lineNoise = 0.02 * std::sin(timeSeconds * 2.0 * M_PI * 60.0);  // 60 Hz line noise
    
    // Combine: QRS + baseline wander + noise
    return qrsValue + baseline + noise + lineNoise;
}

double InMemorySensorDataSource::generatePlethSample(int64_t timeMs)
{
    // Convert time to seconds
    double timeSeconds = (timeMs - m_startTimeMs) / 1000.0;
    
    // Get current heart rate
    double hr = getCurrentHeartRate();
    double heartRateHz = hr / 60.0;  // Convert BPM to Hz
    
    // Update pleth phase
    m_plethPhase += heartRateHz * (4.0 / 250.0);  // 4ms interval at 250 Hz
    if (m_plethPhase >= 1.0) {
        m_plethPhase -= 1.0;
    }
    
    // Generate pleth waveform (pulse-like, synchronized with heart rate)
    double phase = m_plethPhase * 2.0 * M_PI;
    
    // Pleth waveform: rapid rise, slower fall (pulse shape)
    double plethValue = 0.0;
    if (phase < M_PI / 2.0) {  // Rapid rise (systole)
        plethValue = 0.3 + 0.7 * std::sin(phase * 2.0);
    } else {  // Slower fall (diastole)
        double fallPhase = (phase - M_PI / 2.0) / (3.0 * M_PI / 2.0);
        plethValue = 1.0 - 0.7 * fallPhase;
    }
    
    // Add noise
    double noise = m_normalDist(m_rng) * 0.02;
    
    return plethValue + noise;
}

double InMemorySensorDataSource::generateRespirationSample(int64_t timeMs)
{
    // Convert time to seconds
    double timeSeconds = (timeMs - m_startTimeMs) / 1000.0;
    
    // Get current respiration rate
    double rr = getCurrentRespirationRate();
    double respirationRateHz = rr / 60.0;  // Convert rpm to Hz
    
    // Update respiration phase
    m_respirationPhase += respirationRateHz * (40.0 / 250.0);  // 25 Hz = every 40ms at 250 Hz base
    if (m_respirationPhase >= 2.0 * M_PI) {
        m_respirationPhase -= 2.0 * M_PI;
    }
    
    // Generate respiration waveform (sinusoidal)
    double respValue = 0.5 + 0.5 * std::sin(m_respirationPhase);
    
    // Add noise
    double noise = m_normalDist(m_rng) * 0.05;
    
    return respValue + noise;
}

double InMemorySensorDataSource::getCurrentHeartRate() const
{
    // Return base heart rate (variation handled in generateVitals)
    return m_baseHeartRate;
}

double InMemorySensorDataSource::getCurrentRespirationRate() const
{
    // Return base respiration rate (variation handled in generateVitals)
    return m_baseRespirationRate;
}

} // namespace zmon

