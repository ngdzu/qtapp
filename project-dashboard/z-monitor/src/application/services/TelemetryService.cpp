#include "TelemetryService.h"
#include "ITelemetryServer.h"
#include <QDateTime>
#include <QThread>
#include <QtCore/qbytearray.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qbuffer.h>

namespace zmon
{

    TelemetryService::TelemetryService(ITelemetryServer *server, QObject *parent)
        : QObject(parent), m_server(server)
    {
        QObject::connect(&m_timer, &QTimer::timeout, this, &TelemetryService::onBatchTimer);
    }

    void TelemetryService::start()
    {
        m_timer.start(m_batchIntervalMs);
    }

    void TelemetryService::stop()
    {
        m_timer.stop();
    }

    void TelemetryService::setBatchIntervalMs(int intervalMs)
    {
        m_batchIntervalMs = intervalMs;
        if (m_timer.isActive())
        {
            m_timer.start(m_batchIntervalMs);
        }
    }

    void TelemetryService::setRetryPolicy(const zmon::RetryPolicy &policy)
    {
        m_retryPolicy = policy;
    }

    void TelemetryService::setCircuitBreaker(const zmon::CircuitBreaker &breaker)
    {
        m_circuitBreaker = breaker;
    }

    void TelemetryService::enqueueVital(const QByteArray &payload)
    {
        m_batchBuffer.append(payload);
        m_batchBuffer.append('\n');
    }

    void TelemetryService::enqueueAlarm(const QByteArray &payload)
    {
        m_batchBuffer.append(payload);
        m_batchBuffer.append('\n');
    }

    void TelemetryService::onBatchTimer()
    {
        flushBatch();
    }

    void TelemetryService::flushNow()
    {
        flushBatch();
    }

    void TelemetryService::flushBatch()
    {
        if (m_batchBuffer.isEmpty())
            return;

        if (m_circuitBreaker.isOpen())
        {
            emit uploadFailed(QStringLiteral("circuit breaker open"));
            return;
        }

        QByteArray compressed = compressGzip(m_batchBuffer);
        emit batchReady(compressed);

        QString err;
        bool ok = false;
        int attempt = 1;
        const int maxAttempts = m_retryPolicy.maxAttempts();
        while (attempt <= maxAttempts)
        {
            ok = (m_server && m_server->upload(compressed, err));
            if (ok)
                break;
            auto delay = m_retryPolicy.delayForAttempt(attempt);
            QThread::msleep(static_cast<unsigned long>(delay.count()));
            ++attempt;
        }

        if (ok)
        {
            emit uploadSucceeded();
            m_circuitBreaker.recordSuccess();
            m_batchBuffer.clear();
        }
        else
        {
            m_circuitBreaker.recordFailure();
            emit uploadFailed(err.isEmpty() ? QStringLiteral("upload failed") : err);
        }
    }

    QByteArray TelemetryService::compressGzip(const QByteArray &input) const
    {
        // Note: qCompress produces zlib-compressed data; acceptable for tests.
        // Production gzip format will be handled in HttpTelemetryServerAdapter.
        return qCompress(input, 6);
    }

} // namespace zmon
