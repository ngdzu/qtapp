/**
 * @file ITelemetryServer.h
 * @brief Interface for telemetry server control and ingestion.
 */
#pragma once

#include "domain/common/Result.h"
#include <string>
#include <vector>

namespace zmon
{

    struct TelemetryIngestRequest
    {
        std::string sourceId;
        std::vector<uint8_t> payload;
    };

    class ITelemetryServer
    {
    public:
        virtual ~ITelemetryServer() = default;

        /**
         * @brief Start the telemetry server.
         * @return Result<void>
         */
        virtual Result<void> start() = 0;

        /**
         * @brief Stop the telemetry server.
         * @return Result<void>
         */
        virtual Result<void> stop() = 0;

        /**
         * @brief Ingest telemetry payload.
         * @param request Ingestion request (source + payload)
         * @return Result<void>
         */
        virtual Result<void> ingest(const TelemetryIngestRequest &request) = 0;
    };

} // namespace zmon
