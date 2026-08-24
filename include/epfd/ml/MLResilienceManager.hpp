#ifndef EPFD_ML_ML_RESILIENCE_MANAGER_HPP
#define EPFD_ML_ML_RESILIENCE_MANAGER_HPP

#include "epfd/ml/IModelPredictor.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <chrono>

namespace epfd {

struct MLResilienceConfig {
    MLFailurePolicy failure_policy{MLFailurePolicy::FALLBACK_SCORE};
    double fallback_score{30.0};              // Fallback risk score when ML fails
    double max_allowed_latency_ms{50.0};      // SLA timeout threshold
    std::string expected_model_version{""};   // Version mismatch check (empty = any)
    bool log_failures{true};
};

/**
 * @brief Resilient Model Predictor Proxy (Decorator Pattern & DIP).
 * Guarantees zero unhandled exceptions, enforces latency SLAs, bounds validation, and configurable Fail-Open / Fail-Closed policies.
 */
class ResilientModelPredictor : public IModelPredictor {
public:
    explicit ResilientModelPredictor(std::shared_ptr<IModelPredictor> primary_predictor = nullptr,
                                    MLResilienceConfig config = MLResilienceConfig{});

    const std::string& getModelName() const noexcept override;
    const std::string& getModelVersion() const noexcept override;
    bool isReady() const noexcept override;
    size_t getExpectedFeatureCount() const noexcept override;

    PredictionResult predict(const std::vector<double>& features) override;
    PredictionResult predict(const dsa::Vector<double>& features) override;

    void setPrimaryPredictor(std::shared_ptr<IModelPredictor> predictor);
    void setConfig(const MLResilienceConfig& config);
    const MLResilienceConfig& getConfig() const noexcept { return config_; }

    size_t getFailureCount() const noexcept { return failure_count_; }
    size_t getSuccessCount() const noexcept { return success_count_; }

private:
    PredictionResult handleFailure(const std::string& reason);

    std::shared_ptr<IModelPredictor> primary_;
    MLResilienceConfig config_;
    size_t success_count_{0};
    size_t failure_count_{0};
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_ML_ML_RESILIENCE_MANAGER_HPP
