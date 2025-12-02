#pragma once

#include <chrono>

namespace zmon
{

    class CircuitBreaker
    {
    public:
        explicit CircuitBreaker(int failureThreshold = 5,
                                std::chrono::milliseconds resetAfter = std::chrono::seconds(30))
            : m_failureThreshold(failureThreshold), m_resetAfter(resetAfter) {}

        void recordSuccess() { m_failures = 0; }
        void recordFailure() { ++m_failures; }
        bool isOpen() const { return m_failures >= m_failureThreshold; }

    private:
        int m_failures{0};
        int m_failureThreshold{5};
        std::chrono::milliseconds m_resetAfter{std::chrono::seconds(30)};
    };

} // namespace zmon
