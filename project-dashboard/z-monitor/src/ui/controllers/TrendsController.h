/**
 * @file TrendsController.h
 * @brief QML controller for trend data visualization UI.
 *
 * @author Z Monitor Team
 * @date 2025-11-29
 */

#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <memory>

namespace zmon
{
    class IVitalsRepository;
}

namespace zmon
{
    /**
     * @class TrendsController
     * @brief QML controller for trend data visualization.
     *
     * @thread Main/UI Thread
     * @ingroup Interface
     */
    class TrendsController : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QVariantList trendData READ trendData NOTIFY trendDataChanged)
        Q_PROPERTY(QDateTime startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged)
        Q_PROPERTY(QDateTime endTime READ endTime WRITE setEndTime NOTIFY endTimeChanged)
        Q_PROPERTY(QString selectedMetric READ selectedMetric WRITE setSelectedMetric NOTIFY selectedMetricChanged)

    public:
        explicit TrendsController(zmon::IVitalsRepository *vitalsRepo, QObject *parent = nullptr);
        ~TrendsController() override = default;

        QVariantList trendData() const { return m_trendData; }
        QDateTime startTime() const { return m_startTime; }
        void setStartTime(const QDateTime &time);
        QDateTime endTime() const { return m_endTime; }
        void setEndTime(const QDateTime &time);
        QString selectedMetric() const { return m_selectedMetric; }
        void setSelectedMetric(const QString &metric);

        Q_INVOKABLE void loadTrendData();
        Q_INVOKABLE void refreshData();

    signals:
        void trendDataChanged();
        void startTimeChanged();
        void endTimeChanged();
        void selectedMetricChanged();

    private:
        QVariantList m_trendData;
        QDateTime m_startTime;
        QDateTime m_endTime;
        QString m_selectedMetric{"heart_rate"};
        zmon::IVitalsRepository *m_vitalsRepo{nullptr};
        QString m_patientMrn{}; // optional filter; empty = all

        int decimationFactor(int64_t startMs, int64_t endMs) const;
        QVariantList toPoints(const std::vector<struct VitalRecord> &records) const;
    };
} // namespace zmon
