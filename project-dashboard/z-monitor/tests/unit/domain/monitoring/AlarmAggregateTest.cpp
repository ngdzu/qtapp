#include <gtest/gtest.h>
#include "domain/monitoring/AlarmAggregate.h"
#include "domain/monitoring/AlarmSnapshot.h"
#include "domain/monitoring/AlarmThreshold.h"

using namespace zmon;

namespace
{

    TEST(AlarmAggregateTest, RaiseCreatesActiveAlarm)
    {
        AlarmAggregate agg;
        auto snapshot = agg.raise("HR_HIGH", AlarmPriority::HIGH, 150.0, 120.0, "MRN123", "DEV001");

        ASSERT_FALSE(snapshot.alarmId.empty());
        EXPECT_EQ(snapshot.alarmType, std::string("HR_HIGH"));
        EXPECT_EQ(snapshot.priority, AlarmPriority::HIGH);
        EXPECT_EQ(snapshot.status, AlarmStatus::Active);
        EXPECT_EQ(snapshot.patientMrn, std::string("MRN123"));
        EXPECT_EQ(snapshot.deviceId, std::string("DEV001"));

        auto actives = agg.getActiveAlarms();
        ASSERT_EQ(actives.size(), 1u);
        EXPECT_EQ(actives[0].alarmId, snapshot.alarmId);
    }

    TEST(AlarmAggregateTest, AcknowledgeUpdatesStatusAndHistory)
    {
        AlarmAggregate agg;
        auto raised = agg.raise("SPO2_LOW", AlarmPriority::MEDIUM, 85.0, 90.0, "MRN999", "DEV002");
        ASSERT_FALSE(raised.alarmId.empty());

        bool ok = agg.acknowledge(raised.alarmId, "userA");
        EXPECT_TRUE(ok);

        auto actives = agg.getActiveAlarms();
        ASSERT_EQ(actives.size(), 1u);
        EXPECT_EQ(actives[0].status, AlarmStatus::Acknowledged);
        EXPECT_EQ(actives[0].acknowledgedBy, std::string("userA"));
        EXPECT_GT(actives[0].acknowledgedAtMs, 0);
    }

    TEST(AlarmAggregateTest, SilenceChangesStatus)
    {
        AlarmAggregate agg;
        auto raised = agg.raise("RR_HIGH", AlarmPriority::LOW, 30.0, 25.0, "MRN777", "DEV003");
        ASSERT_FALSE(raised.alarmId.empty());

        bool ok = agg.silence(raised.alarmId, 300000 /*5 min*/);
        EXPECT_TRUE(ok);

        auto actives = agg.getActiveAlarms();
        ASSERT_EQ(actives.size(), 1u);
        EXPECT_EQ(actives[0].status, AlarmStatus::Silenced);
    }

    TEST(AlarmAggregateTest, EscalateRaisesPriority)
    {
        AlarmAggregate agg;
        auto raised = agg.raise("TEMP_HIGH", AlarmPriority::LOW, 39.0, 38.0, "MRN555", "DEV004");
        ASSERT_FALSE(raised.alarmId.empty());

        bool ok = agg.escalate(raised.alarmId);
        EXPECT_TRUE(ok);

        auto actives = agg.getActiveAlarms();
        ASSERT_EQ(actives.size(), 1u);
        EXPECT_EQ(actives[0].priority, AlarmPriority::MEDIUM);
    }

    TEST(AlarmAggregateTest, ResolveRemovesFromActive)
    {
        AlarmAggregate agg;
        auto raised = agg.raise("HR_HIGH", AlarmPriority::HIGH, 150.0, 120.0, "MRN123", "DEV001");
        ASSERT_FALSE(raised.alarmId.empty());

        bool ok = agg.resolve(raised.alarmId);
        EXPECT_TRUE(ok);

        auto actives = agg.getActiveAlarms();
        EXPECT_TRUE(actives.empty());

        auto history = agg.getHistory(0, std::numeric_limits<int64_t>::max());
        ASSERT_FALSE(history.empty());
        EXPECT_EQ(history.back().status, AlarmStatus::Resolved);
    }

    TEST(AlarmAggregateTest, DuplicateSuppressionWindow)
    {
        AlarmAggregate agg;
        auto first = agg.raise("HR_HIGH", AlarmPriority::HIGH, 150.0, 120.0, "MRN123", "DEV001");
        ASSERT_FALSE(first.alarmId.empty());

        // Immediately raising same type for same patient should be suppressed
        auto second = agg.raise("HR_HIGH", AlarmPriority::HIGH, 151.0, 120.0, "MRN123", "DEV001");
        EXPECT_TRUE(second.alarmId.empty());

        auto actives = agg.getActiveAlarms();
        ASSERT_EQ(actives.size(), 1u);
    }

    TEST(AlarmAggregateTest, HistoryRangeFiltering)
    {
        AlarmAggregate agg;
        // First alarm raised and resolved
        auto a1 = agg.raise("BP_HIGH", AlarmPriority::MEDIUM, 190.0, 120.0, "MRN42", "DEV-A");
        ASSERT_FALSE(a1.alarmId.empty());
        ASSERT_TRUE(agg.resolve(a1.alarmId));

        // Second alarm later
        auto a2 = agg.raise("BP_HIGH", AlarmPriority::MEDIUM, 185.0, 120.0, "MRN42", "DEV-A");
        ASSERT_FALSE(a2.alarmId.empty());
        ASSERT_TRUE(agg.resolve(a2.alarmId));

        // Query entire history
        auto historyAll = agg.getHistory(0, std::numeric_limits<int64_t>::max());
        ASSERT_GE(historyAll.size(), 2u); // at least one raise + one resolve recorded

        // Narrow range: expect at least one entry belonging to second alarm near the end
        auto historyTail = agg.getHistory(historyAll.back().timestampMs - 100000, std::numeric_limits<int64_t>::max());
        ASSERT_FALSE(historyTail.empty());
    }

    TEST(AlarmAggregateTest, EscalateMediumToHigh)
    {
        AlarmAggregate agg;
        auto raised = agg.raise("TEMP_HIGH", AlarmPriority::MEDIUM, 40.0, 36.5, "MRN7", "DEV-T");
        ASSERT_FALSE(raised.alarmId.empty());
        // escalate twice to reach HIGH
        ASSERT_TRUE(agg.escalate(raised.alarmId)); // MEDIUM -> HIGH or MEDIUM -> next
        auto actives = agg.getActiveAlarms();
        ASSERT_EQ(actives.size(), 1u);
        EXPECT_EQ(actives[0].priority, AlarmPriority::HIGH);
    }

    TEST(AlarmAggregateTest, AcknowledgeIdempotent)
    {
        AlarmAggregate agg;
        auto raised = agg.raise("O2_LOW", AlarmPriority::HIGH, 80.0, 95.0, "MRN8", "DEV-O2");
        ASSERT_FALSE(raised.alarmId.empty());

        ASSERT_TRUE(agg.acknowledge(raised.alarmId, std::string("nurseA")));
        // Re-acknowledge by same user; domain may treat as no-op and return false
        ASSERT_FALSE(agg.acknowledge(raised.alarmId, std::string("nurseA")));

        auto actives = agg.getActiveAlarms();
        ASSERT_EQ(actives.size(), 1u);
        EXPECT_EQ(actives[0].status, AlarmStatus::Acknowledged);
        EXPECT_EQ(actives[0].acknowledgedBy, std::string("nurseA"));

        auto history = agg.getHistory(0, std::numeric_limits<int64_t>::max());
        ASSERT_GE(history.size(), 1u); // at least one event recorded
    }

} // namespace
