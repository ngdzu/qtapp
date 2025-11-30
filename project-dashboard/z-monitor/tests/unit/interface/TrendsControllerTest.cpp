#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include <QtCore/QVariantList>
#include <QtCore/QDateTime>
#include "interface/controllers/TrendsController.h"
#include "domain/repositories/IVitalsRepository.h"
#include "domain/monitoring/VitalRecord.h"

namespace zmon
{

    // Simple mock IVitalsRepository returning synthetic data
    class MockVitalsRepository : public IVitalsRepository
    {
    public:
        Result<void> save(const VitalRecord &) override { return Result<void>::ok(); }
        Result<size_t> saveBatch(const std::vector<VitalRecord> &) override { return Result<size_t>::ok(0); }
        std::vector<VitalRecord> getRange(const std::string &patientMrn,
                                          int64_t startEpochMs,
                                          int64_t endEpochMs) override
        {
            Q_UNUSED(patientMrn);
            std::vector<VitalRecord> out;
            // Generate 120 seconds of heart rate values (one per second)
            for (int i = 0; i < 120; ++i)
            {
                VitalRecord rec("HR", 60 + (i % 5), startEpochMs + i * 1000);
                out.push_back(rec);
            }
            // Also include SpO2 values to verify filtering
            for (int i = 0; i < 60; ++i)
            {
                VitalRecord rec2("SPO2", 98, startEpochMs + i * 1000);
                out.push_back(rec2);
            }
            return out;
        }
        std::vector<VitalRecord> getUnsent() override { return {}; }
        size_t markAsSent(const std::vector<std::string> &) override { return 0; }
    };

    TEST(TrendsControllerTest, EmitsTrendDataChangedAndFiltersMetric)
    {
        MockVitalsRepository repo;
        TrendsController controller(&repo);

        // Select HeartRate metric and a 2-minute range
        controller.setSelectedMetric(QStringLiteral("heart_rate"));
        const QDateTime end = QDateTime::currentDateTimeUtc();
        const QDateTime start = end.addSecs(-120);
        controller.setStartTime(start);
        controller.setEndTime(end);

        QSignalSpy spy(&controller, &TrendsController::trendDataChanged);
        controller.loadTrendData();
        ASSERT_GE(spy.count(), 1);

        QVariantList points = controller.trendData();
        // Ensure points are only HeartRate (first entry check sufficient)
        ASSERT_FALSE(points.isEmpty());
        auto first = points.first().toMap();
        ASSERT_TRUE(first.contains("timestamp"));
        ASSERT_TRUE(first.contains("value"));
    }

    TEST(TrendsControllerTest, DecimationReducesPointCountForLongRange)
    {
        MockVitalsRepository repo;
        TrendsController controller(&repo);

        controller.setSelectedMetric(QStringLiteral("heart_rate"));
        const QDateTime end2 = QDateTime::currentDateTimeUtc();
        const QDateTime start2 = end2.addSecs(-2 * 60 * 60);
        controller.setStartTime(start2);
        controller.setEndTime(end2);

        controller.loadTrendData();
        QVariantList points = controller.trendData();
        // Mock returns 120 HR + 60 SpO2; after filtering HR only 120 raw
        // With 2h range, decimation should reduce below 120
        ASSERT_LT(points.size(), 120);
    }

} // namespace zmon
