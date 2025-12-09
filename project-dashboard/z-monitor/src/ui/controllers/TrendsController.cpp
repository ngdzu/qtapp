/**
 * @file TrendsController.cpp
 * @brief Implementation of TrendsController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "TrendsController.h"
#include "domain/repositories/IVitalsRepository.h"
#include "domain/monitoring/VitalRecord.h"

namespace zmon
{
    TrendsController::TrendsController(IVitalsRepository *vitalsRepo, QObject *parent)
        : QObject(parent), m_vitalsRepo(vitalsRepo)
    {
        m_startTime = QDateTime::currentDateTime().addSecs(-3600); // Last hour
        m_endTime = QDateTime::currentDateTime();
    }

    void TrendsController::setStartTime(const QDateTime &time)
    {
        if (m_startTime != time)
        {
            m_startTime = time;
            emit startTimeChanged();
        }
    }

    void TrendsController::setEndTime(const QDateTime &time)
    {
        if (m_endTime != time)
        {
            m_endTime = time;
            emit endTimeChanged();
        }
    }

    void TrendsController::setSelectedMetric(const QString &metric)
    {
        if (m_selectedMetric != metric)
        {
            m_selectedMetric = metric;
            emit selectedMetricChanged();
        }
    }

    int TrendsController::decimationFactor(int64_t startMs, int64_t endMs) const
    {
        auto durationMs = endMs - startMs;
        // Basic heuristic: ~1 point per 2 seconds for long ranges
        if (durationMs <= 60 * 60 * 1000)
        {             // <= 1 hour
            return 1; // 1 Hz (assuming records roughly per-second)
        }
        else if (durationMs <= 6 * 60 * 60 * 1000)
        {             // <= 6 hours
            return 5; // ~0.2 Hz
        }
        else if (durationMs <= 24 * 60 * 60 * 1000)
        {              // <= 24 hours
            return 60; // ~0.016 Hz (~1/min)
        }
        return 300; // ~1 per 5 minutes for very long ranges
    }

    QVariantList TrendsController::toPoints(const std::vector<VitalRecord> &records) const
    {
        QVariantList points;
        points.reserve(static_cast<int>(records.size()));
        for (const auto &r : records)
        {
            QVariantMap p;
            p.insert("timestamp", static_cast<qlonglong>(r.timestampMs));
            p.insert("value", r.value);
            points.push_back(p);
        }
        return points;
    }

    void TrendsController::loadTrendData()
    {
        if (!m_vitalsRepo)
        {
            m_trendData.clear();
            emit trendDataChanged();
            return;
        }

        const auto startMs = m_startTime.toMSecsSinceEpoch();
        const auto endMs = m_endTime.toMSecsSinceEpoch();

        // Map selectedMetric to VitalRecord::vitalType codes
        std::string type;
        if (m_selectedMetric == "heart_rate")
            type = "HR";
        else if (m_selectedMetric == "spo2")
            type = "SPO2";
        else if (m_selectedMetric == "resp_rate")
            type = "RR";
        else if (m_selectedMetric == "temperature")
            type = "TEMP";
        else
            type = "HR";

        auto all = m_vitalsRepo->getRange(m_patientMrn.toStdString(), startMs, endMs);
        // Filter by vitalType
        std::vector<VitalRecord> filtered;
        filtered.reserve(all.size());
        for (const auto &r : all)
        {
            if (r.vitalType == type)
                filtered.push_back(r);
        }

        // Decimate
        const int factor = decimationFactor(startMs, endMs);
        if (factor > 1)
        {
            std::vector<VitalRecord> decimated;
            decimated.reserve(filtered.size() / factor + 1);
            for (size_t i = 0; i < filtered.size(); i += static_cast<size_t>(factor))
            {
                decimated.push_back(filtered[i]);
            }
            m_trendData = toPoints(decimated);
        }
        else
        {
            m_trendData = toPoints(filtered);
        }

        emit trendDataChanged();
    }

    void TrendsController::refreshData()
    {
        loadTrendData();
    }
} // namespace zmon
