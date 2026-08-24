#include "epfd/ml/MLResilienceManager.hpp"
#include <cmath>
#include <iostream>

namespace epfd {

ResilientModelPredictor::ResilientModelPredictor(std::shared_ptr<IModelPredictor> primary_predictor,
                                                 MLResilienceConfig config)
    : primary_(std::move(primary_predictor)),
      config_(config) {}

const std::string& ResilientModelPredictor::getModelName() const noexcept {
    static const std::string fallback_name = "ResilientProxy(NoUnderlyingModel)";
    if (primary_) {
        return primary_->getModelName();
    }
    return fallback_name;
}

const std::string& ResilientModelPredictor::getModelVersion() const noexcept {
    static const std::string fallback_version = "v0.0.0-fallback";
    if (primary_) {
        return primary_->getModelVersion();
    }
    return fallback_version;
}

bool ResilientModelPredictor::isReady() const noexcept {
    return primary_ != nullptr && primary_->isReady();
}

size_t ResilientModelPredictor::getExpectedFeatureCount() const noexcept {
    if (primary_) {
        return primary_->getExpectedFeatureCount();
    }
    return 18;
}

PredictionResult ResilientModelPredictor::handleFailure(const std::string& reason) {
    failure_count_++;

    double fallback_prob = 0.0;
    switch (config_.failure_policy) {
        case MLFailurePolicy::FAIL_OPEN:
            // Fail open: assume zero ML fraud score (allow rules to handle safely)
            fallback_prob = 0.0;
            break;
        case MLFailurePolicy::FAIL_CLOSED:
            // Fail closed: assume high risk / challenge 3DS
            fallback_prob = 1.0;
            break;
        case MLFailurePolicy::FALLBACK_SCORE:
        default:
            fallback_prob = config_.fallback_score / 100.0;
            break;
    }

    PredictionResult res;
    res.probability = fallback_prob;
    res.fraud_probability = fallback_prob;
    res.is_success = false;
    res.error_message = "ML Failure (" + reason + ") -> Applied " +
                        (config_.failure_policy == MLFailurePolicy::FAIL_OPEN ? "FAIL_OPEN" :
                         config_.failure_policy == MLFailurePolicy::FAIL_CLOSED ? "FAIL_CLOSED" : "FALLBACK_SCORE");
    res.latency_ms = 0.0;
    return res;
}

PredictionResult ResilientModelPredictor::predict(const std::vector<double>& features) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 1. Guard against uninitialized / unavailable model
    if (!primary_ || !primary_->isReady()) {
        return handleFailure("Model is unavailable or uninitialized");
    }

    // 2. Guard against feature dimension mismatch
    if (features.size() != primary_->getExpectedFeatureCount()) {
        return handleFailure("Feature dimension mismatch: expected " +
                             std::to_string(primary_->getExpectedFeatureCount()) +
                             ", received " + std::to_string(features.size()));
    }

    // 3. Guard against version mismatch
    if (!config_.expected_model_version.empty() &&
        primary_->getModelVersion() != config_.expected_model_version) {
        return handleFailure("Model version mismatch: expected " + config_.expected_model_version +
                             ", active is " + primary_->getModelVersion());
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // 4. Safe execution with exception isolation
    PredictionResult raw_res;
    try {
        raw_res = primary_->predict(features);
    } catch (const std::exception& ex) {
        return handleFailure("Exception during inference: " + std::string(ex.what()));
    } catch (...) {
        return handleFailure("Unknown fatal exception during inference");
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double latency = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    // 5. Guard against latency timeout SLA breach
    if (latency > config_.max_allowed_latency_ms) {
        return handleFailure("Latency SLA timeout breach: " + std::to_string(latency) +
                             "ms > limit " + std::to_string(config_.max_allowed_latency_ms) + "ms");
    }

    // 6. Guard against invalid probability (NaN, Inf, or out-of-bounds)
    if (std::isnan(raw_res.probability) || std::isinf(raw_res.probability) ||
        raw_res.probability < 0.0 || raw_res.probability > 1.0) {
        return handleFailure("Invalid model output: probability out of bounds [0, 1] or NaN/Inf");
    }

    success_count_++;
    raw_res.is_success = true;
    raw_res.latency_ms = latency;
    return raw_res;
}

PredictionResult ResilientModelPredictor::predict(const dsa::Vector<double>& features) {
    std::vector<double> std_f;
    std_f.reserve(features.size());
    for (size_t i = 0; i < features.size(); ++i) {
        std_f.push_back(features[i]);
    }
    return predict(std_f);
}

void ResilientModelPredictor::setPrimaryPredictor(std::shared_ptr<IModelPredictor> predictor) {
    std::lock_guard<std::mutex> lock(mutex_);
    primary_ = std::move(predictor);
}

void ResilientModelPredictor::setConfig(const MLResilienceConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
}

} // namespace epfd
