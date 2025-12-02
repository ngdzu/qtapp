#pragma once

#include <QByteArray>
#include <QString>

namespace zmon
{

    class ITelemetryServer
    {
    public:
        virtual ~ITelemetryServer() = default;
        virtual bool upload(const QByteArray &compressedBatch, QString &errorOut) = 0;
    };

} // namespace zmon
