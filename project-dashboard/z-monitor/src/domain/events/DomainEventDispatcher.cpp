#include "domain/events/DomainEventDispatcher.h"
#include <chrono>

namespace zmon
{
    namespace DomainEvents
    {

        DomainEventDispatcher::DomainEventDispatcher()
        {
            worker_ = std::thread([this]
                                  { workerLoop(); });
        }

        DomainEventDispatcher::~DomainEventDispatcher()
        {
            shutdown();
        }

        void DomainEventDispatcher::shutdown()
        {
            bool expected = false;
            if (stop_.compare_exchange_strong(expected, true))
            {
                cv_.notify_all();
                if (worker_.joinable())
                {
                    worker_.join();
                }
            }
        }

        void DomainEventDispatcher::dispatch(const IDomainEvent &event)
        {
            // Copy handlers under lock
            std::vector<Handler> sync;
            std::vector<Handler> async;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                auto typeIdx = std::type_index(typeid(event));
                if (syncHandlers_.count(typeIdx))
                    sync = syncHandlers_[typeIdx];
                if (asyncHandlers_.count(typeIdx))
                    async = asyncHandlers_[typeIdx];
            }

            // Run synchronous handlers inline
            for (auto &h : sync)
            {
                h(event);
            }

            if (!async.empty())
            {
                std::lock_guard<std::mutex> lock(mutex_);
                for (auto &h : async)
                {
                    queue_.push(Task{h, event.clone()});
                }
                cv_.notify_one();
            }
        }

        void DomainEventDispatcher::workerLoop()
        {
            while (!stop_)
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]
                         { return stop_ || !queue_.empty(); });
                if (stop_ && queue_.empty())
                {
                    break;
                }
                Task task = std::move(queue_.front());
                queue_.pop();
                lock.unlock();
                // Execute handler
                task.handler(*task.eventPtr);
            }
        }

    }
} // namespace zmon::DomainEvents
