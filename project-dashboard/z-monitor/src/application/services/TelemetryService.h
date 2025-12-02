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

    class TelemetryService : public QObject
    {
        Q_OBJECT
    public:
        explicit TelemetryService(ITelemetryServer *server,
                                  QObject *parent = nullptr);

        void start();
        void stop();

        void enqueueVital(const QByteArray &payload);
        void enqueueAlarm(const QByteArray &payload);

        void setBatchIntervalMs(int intervalMs);
        void setRetryPolicy(const zmon::RetryPolicy &policy);
        void setCircuitBreaker(const zmon::CircuitBreaker &breaker);
        void flushNow();

    signals:
        void batchReady(const QByteArray &compressedBatch);
        void uploadSucceeded();
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
