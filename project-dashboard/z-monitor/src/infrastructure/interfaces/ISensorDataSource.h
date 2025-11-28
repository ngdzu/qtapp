/**
 * @file ISensorDataSource.h
 * @brief Interface for vital signs data acquisition.
 *
 * This file defines the ISensorDataSource interface which abstracts the source
 * of vital signs data, enabling multiple implementations (simulator, real hardware,
 * mock, replay).
 *
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QDateTime>

#include "domain/monitoring/VitalRecord.h"
#include "domain/monitoring/WaveformSample.h"

namespace zmon {

/**
 * @struct DataSourceInfo
 * @brief Metadata about data source.
 */
struct DataSourceInfo {
    QString name;                  ///< Name (e.g., "Device Simulator", "Philips Monitor")
    QString type;                  ///< Type: "SIMULATOR", "HARDWARE", "MOCK", "REPLAY"
    QString version;               ///< Version (e.g., "1.0.0")
    QStringList capabilities;      ///< Capabilities (e.g., ["HR", "SPO2", "ECG", "NIBP"])
    bool supportsWaveforms;        ///< true if provides waveform data
};

/**
 * @enum ErrorCode
 * @brief Sensor error codes.
 */
enum class ErrorCode {
    None,                  ///< No error
    SensorDisconnected,    ///< Sensor physically disconnected
    SignalTooNoisy,        ///< Signal quality too poor to measure
    CalibrationFailed,     ///< Sensor calibration failed
    HardwareFailure,       ///< Hardware malfunction
    CommunicationError,    ///< Communication with sensor failed
    UnknownError          ///< Unexpected error
};

/**
 * @struct SensorError
 * @brief Sensor error information.
 */
struct SensorError {
    ErrorCode code;         ///< Error code
    QString message;        ///< Human-readable error message
    QString sensorType;     ///< Affected sensor (e.g., "ECG", "SpO2")
    QDateTime timestamp;    ///< When error occurred
    bool recoverable;       ///< true if error is recoverable (retry possible)
};

/**
 * @class ISensorDataSource
 * @brief Interface for vital signs data acquisition.
 *
 * This interface abstracts the source of vital signs data, enabling
 * multiple implementations (simulator, real hardware, mock, replay).
 *
 * @note All data is emitted via signals (Qt event-driven model)
 * @note Runs on Real-Time Processing Thread (high priority)
 * @see VitalRecord, WaveformSample, MonitoringService
 * @ingroup Infrastructure
 */
class ISensorDataSource : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ISensorDataSource() = default;

    /**
     * @brief Start data acquisition.
     *
     * Begins acquiring vital signs data and emitting signals.
     *
     * @return true if started successfully, false otherwise
     *
     * @note Non-blocking (data emitted via signals)
     * @note Call stop() before destroying object
     */
    virtual bool start() = 0;

    /**
     * @brief Stop data acquisition.
     *
     * Stops acquiring data and stops emitting signals.
     *
     * @note Graceful shutdown (flushes pending data)
     */
    virtual void stop() = 0;

    /**
     * @brief Check if data source is currently active.
     *
     * @return true if actively acquiring data
     */
    virtual bool isActive() const = 0;

    /**
     * @brief Get data source metadata (name, type, version).
     *
     * @return DataSourceInfo Metadata about this data source
     */
    virtual DataSourceInfo getInfo() const = 0;

    /**
     * @brief Get current sampling rate (Hz).
     *
     * @return Sampling rate in Hz (e.g., 1.0 for 1 Hz vitals, 500.0 for ECG)
     */
    virtual double getSamplingRate() const = 0;

signals:
    /**
     * @brief Emitted when new vital signs data available.
     *
     * @param vital Vital signs record (HR, SpO2, RR, etc.)
     *
     * @note Emitted at 1 Hz (once per second) for vitals
     * @note CRITICAL PATH: < 50ms from sensor reading to this signal
     */
    void vitalSignsReceived(const VitalRecord& vital);

    /**
     * @brief Emitted when waveform sample available.
     *
     * @param waveform Waveform sample (ECG, SpO2 pleth, etc.)
     *
     * @note Emitted at high frequency (125-500 Hz depending on waveform type)
     * @note Only for display (not persisted to database)
     */
    void waveformSampleReceived(const WaveformSample& waveform);

    /**
     * @brief Emitted when sensor connection status changes.
     *
     * @param connected true if sensors connected, false if disconnected
     * @param sensorType Type of sensor (e.g., "ECG", "SpO2", "NIBP")
     */
    void connectionStatusChanged(bool connected, const QString& sensorType);

    /**
     * @brief Emitted when sensor error occurs.
     *
     * @param error Error details
     */
    void sensorError(const SensorError& error);

    /**
     * @brief Emitted when data source starts.
     */
    void started();

    /**
     * @brief Emitted when data source stops.
     */
    void stopped();
};

} // namespace zmon