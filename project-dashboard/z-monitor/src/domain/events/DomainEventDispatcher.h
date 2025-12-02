/**
 * @file DomainEventDispatcher.h
 * @brief Dispatcher for domain events supporting synchronous and asynchronous handlers.
 */

#pragma once

#include "domain/events/IDomainEvent.h"
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>

namespace zmon
{
    namespace DomainEvents
    {

        /**
         * @class DomainEventDispatcher
         * @brief Central event bus for domain events (sync + async handlers).
         *
         * Thread safety: Registration and dispatch are thread-safe. Asynchronous
         * handlers are executed on an internal worker thread.
         */
        class DomainEventDispatcher
        {
        public:
            DomainEventDispatcher();
            ~DomainEventDispatcher();

            DomainEventDispatcher(const DomainEventDispatcher &) = delete;
            DomainEventDispatcher &operator=(const DomainEventDispatcher &) = delete;

            /**
             * @brief Register synchronous handler for event type T.
             */
            template <typename T>
            void registerSync(std::function<void(const T &)> handler)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                auto wrapper = [h = std::move(handler)](const IDomainEvent &e)
                {
                    h(static_cast<const T &>(e));
                };
                syncHandlers_[std::type_index(typeid(T))].push_back(std::move(wrapper));
            }

            /**
             * @brief Register asynchronous handler for event type T.
             */
            template <typename T>
            void registerAsync(std::function<void(const T &)> handler)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                auto wrapper = [h = std::move(handler)](const IDomainEvent &e)
                {
                    h(static_cast<const T &>(e));
                };
                asyncHandlers_[std::type_index(typeid(T))].push_back(std::move(wrapper));
            }

            /**
             * @brief Dispatch an event to all registered handlers.
             * Synchronous handlers run inline; asynchronous handlers are queued.
             */
            void dispatch(const IDomainEvent &event);

            /**
             * @brief Gracefully stop asynchronous worker (drains queue first).
             */
            void shutdown();

        private:
            using Handler = std::function<void(const IDomainEvent &)>;

            std::unordered_map<std::type_index, std::vector<Handler>> syncHandlers_;
            std::unordered_map<std::type_index, std::vector<Handler>> asyncHandlers_;

            std::mutex mutex_;

            struct Task
            {
                Handler handler;
                std::unique_ptr<IDomainEvent> eventPtr;
            };
            std::queue<Task> queue_;
            std::thread worker_;
            std::condition_variable cv_;
            std::atomic<bool> stop_{false};

            void workerLoop();
        };

    }
} // namespace zmon::DomainEvents
