#pragma once

#include <chrono>

namespace zmon
{

    struct RetryPolicyConfig
    {
        int maxAttempts{3};
        std::chrono::milliseconds initialDelay{std::chrono::milliseconds(500)};
        double backoffMultiplier{2.0};
    };

    class RetryPolicy
    {
    public:
        explicit RetryPolicy(RetryPolicyConfig cfg = {}) : m_cfg(cfg) {}
        int maxAttempts() const { return m_cfg.maxAttempts; }
        std::chrono::milliseconds delayForAttempt(int attempt) const
        {
            double d = m_cfg.initialDelay.count();
            for (int i = 1; i < attempt; ++i)
                d *= m_cfg.backoffMultiplier;
            return std::chrono::milliseconds(static_cast<int>(d));
        }

    private:
        RetryPolicyConfig m_cfg;
    };

} // namespace zmon
