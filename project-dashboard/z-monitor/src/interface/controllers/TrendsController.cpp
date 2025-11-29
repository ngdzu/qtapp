/**
 * @file TrendsController.cpp
 * @brief Implementation of TrendsController.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#include "TrendsController.h"

namespace zmon
{
    TrendsController::TrendsController(QObject *parent) : QObject(parent)
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

    void TrendsController::loadTrendData()
    {
        // TODO: Load trend data from repository
        emit trendDataChanged();
    }

    void TrendsController::refreshData()
    {
        loadTrendData();
    }
} // namespace zmon
