#ifndef EPFD_ML_MOCK_MODEL_PREDICTOR_HPP
#define EPFD_ML_MOCK_MODEL_PREDICTOR_HPP

#include "epfd/ml/IModelPredictor.hpp"
#include <chrono>

namespace epfd {

class MockModelPredictor : public IModelPredictor {
public:
    MockModelPredictor(std::string name = "MockXGBoostModel", 
                       std::string version = "1.0.0", 
                       size_t feature_count = 10,
                       bool ready = true,
                       double fixed_probability = 0.15)
        : name_(std::move(name)),
          version_(std::move(version)),
          feature_count_(feature_count),
          ready_(ready),
          fixed_probability_(fixed_probability) {}

    const std::string& getModelName() const noexcept override { return name_; }
    const std::string& getModelVersion() const noexcept override { return version_; }
    bool isReady() const noexcept override { return ready_; }
    size_t getExpectedFeatureCount() const noexcept override { return feature_count_; }

    void setReady(bool ready) noexcept { ready_ = ready; }
    void setFixedProbability(double prob) noexcept { fixed_probability_ = prob; }
    void setShouldSimulateError(bool simulate) noexcept { simulate_error_ = simulate; }

    PredictionResult predict(const std::vector<double>& features) override {
        auto start = std::chrono::high_resolution_clock::now();
        PredictionResult res;

        if (!ready_ || simulate_error_) {
            res.is_success = false;
            res.probability = 0.0;
            res.error_message = "MockModelPredictor: Model runtime unavailable or error simulated";
            return res;
        }

        if (features.size() != feature_count_) {
            res.is_success = false;
            res.probability = 0.0;
            res.error_message = "MockModelPredictor: Feature dimension mismatch";
            return res;
        }

        res.is_success = true;
        res.probability = fixed_probability_;
        res.fraud_probability = fixed_probability_;
        auto end = std::chrono::high_resolution_clock::now();
        res.latency_ms = std::chrono::duration<double, std::milli>(end - start).count();
        return res;
    }

private:
    std::string name_;
    std::string version_;
    size_t feature_count_{10};
    bool ready_{true};
    double fixed_probability_{0.15};
    bool simulate_error_{false};
};

} // namespace epfd

#endif // EPFD_ML_MOCK_MODEL_PREDICTOR_HPP
