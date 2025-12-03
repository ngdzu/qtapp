/**
 * @file TelemetryService.h
 * @brief Batching and upload service for telemetry data.
 *
 * TelemetryService batches vitals and alarm events and uploads them
 * to the central server every configured interval (default 10 minutes).
 * Uses gzip compression and HTTPS (TLS 1.3) via ITelemetryServer adapter.
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QString>
#include <QByteArray>

#include "ITelemetryServer.h"
#include "../../infrastructure/network/RetryPolicy.h"
#include "../../infrastructure/network/CircuitBreaker.h"

namespace zmon
{

    /**
     * @brief TelemetryService batches and uploads telemetry payloads.
     */
    class TelemetryService : public QObject
    {
        Q_OBJECT
    public:
        explicit TelemetryService(ITelemetryServer *server, QObject *parent = nullptr);

        /** @brief Start periodic batching timer. */
        void start();
        /** @brief Stop periodic batching timer. */
        void stop();

        /** @brief Enqueue a vitals JSON record. */
        void enqueueVital(const QByteArray &payload);
        /** @brief Enqueue an alarm JSON record. */
        void enqueueAlarm(const QByteArray &payload);

        /** @brief Set batch interval in milliseconds (default 10 minutes). */
        void setBatchIntervalMs(int intervalMs);
        /** @brief Configure retry policy. */
        void setRetryPolicy(const zmon::RetryPolicy &policy);
        /** @brief Configure circuit breaker. */
        void setCircuitBreaker(const zmon::CircuitBreaker &breaker);
        /** @brief Force immediate flush of current batch. */
        void flushNow();

    signals:
        /** @brief Emitted when a compressed batch is ready (pre-upload). */
        void batchReady(const QByteArray &compressedBatch);
        /** @brief Emitted after a successful upload. */
        void uploadSucceeded();
        /** @brief Emitted when an upload fails. */
        void uploadFailed(const QString &reason);

    private slots:
        void onBatchTimer();

    private:
        void flushBatch();
        QByteArray compressGzip(const QByteArray &input) const;

        ITelemetryServer *m_server{nullptr};
        QTimer m_timer;
        int m_batchIntervalMs{10 * 60 * 1000}; // default 10 minutes
        QByteArray m_batchBuffer;
        RetryPolicy m_retryPolicy{};
        CircuitBreaker m_circuitBreaker{};
    };

} // namespace zmon
