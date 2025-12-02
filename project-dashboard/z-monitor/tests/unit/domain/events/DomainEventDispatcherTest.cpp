#include <gtest/gtest.h>
#include "domain/events/DomainEventDispatcher.h"
#include "domain/events/IDomainEvent.h"
#include <atomic>
#include <chrono>
#include <thread>

using namespace zmon::DomainEvents;

// Test event implementing IDomainEvent
class TestEvent : public IDomainEvent
{
public:
    TestEvent(std::string aggId, int64_t ts) : aggId_(std::move(aggId)), ts_(ts) {}
    const std::string &aggregateId() const override { return aggId_; }
    int64_t occurredAtMs() const override { return ts_; }
    const char *eventType() const override { return "TestEvent"; }
    std::unique_ptr<IDomainEvent> clone() const override { return std::make_unique<TestEvent>(*this); }

private:
    std::string aggId_;
    int64_t ts_;
};

TEST(DomainEventDispatcherTest, SyncHandlerReceivesEvent)
{
    DomainEventDispatcher dispatcher;
    std::atomic<int> count{0};
    dispatcher.registerSync<TestEvent>([&](const TestEvent &e)
                                       {
        EXPECT_EQ(e.aggregateId(), "A1");
        ++count; });
    TestEvent evt("A1", 123456);
    dispatcher.dispatch(evt);
    EXPECT_EQ(count.load(), 1);
    dispatcher.shutdown();
}

TEST(DomainEventDispatcherTest, MultipleSyncHandlersAllFire)
{
    DomainEventDispatcher dispatcher;
    std::atomic<int> count{0};
    dispatcher.registerSync<TestEvent>([&](const TestEvent &)
                                       { ++count; });
    dispatcher.registerSync<TestEvent>([&](const TestEvent &)
                                       { ++count; });
    TestEvent evt("A2", 999);
    dispatcher.dispatch(evt);
    EXPECT_EQ(count.load(), 2);
    dispatcher.shutdown();
}

TEST(DomainEventDispatcherTest, AsyncHandlersExecuteEventually)
{
    DomainEventDispatcher dispatcher;
    std::atomic<int> count{0};
    dispatcher.registerAsync<TestEvent>([&](const TestEvent &)
                                        { ++count; });
    dispatcher.registerAsync<TestEvent>([&](const TestEvent &)
                                        { ++count; });
    TestEvent evt("A3", 555);
    dispatcher.dispatch(evt);
    // Wait up to 500ms for async handlers
    for (int i = 0; i < 50 && count.load() < 2; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(count.load(), 2);
    dispatcher.shutdown();
}

TEST(DomainEventDispatcherTest, MixedSyncAndAsyncHandlers)
{
    DomainEventDispatcher dispatcher;
    std::atomic<int> syncCount{0};
    std::atomic<int> asyncCount{0};
    dispatcher.registerSync<TestEvent>([&](const TestEvent &)
                                       { ++syncCount; });
    dispatcher.registerAsync<TestEvent>([&](const TestEvent &)
                                        { ++asyncCount; });
    TestEvent evt("A4", 777);
    dispatcher.dispatch(evt);
    EXPECT_EQ(syncCount.load(), 1);
    for (int i = 0; i < 50 && asyncCount.load() < 1; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(asyncCount.load(), 1);
    dispatcher.shutdown();
}

TEST(DomainEventDispatcherTest, ShutdownStopsWorkerGracefully)
{
    DomainEventDispatcher dispatcher;
    std::atomic<int> asyncCount{0};
    dispatcher.registerAsync<TestEvent>([&](const TestEvent &)
                                        { ++asyncCount; });
    TestEvent evt("A5", 1);
    dispatcher.dispatch(evt);
    // Allow brief time for async handler to run before shutdown
    for (int i = 0; i < 50 && asyncCount.load() < 1; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    dispatcher.shutdown();
    // After shutdown, dispatch should not queue async handlers
    TestEvent evt2("A5", 2);
    dispatcher.dispatch(evt2);       // async won't fire
    EXPECT_GE(asyncCount.load(), 1); // at least first dispatch
}
