#ifndef EPFD_ML_NATIVE_ML_MODEL_PREDICTOR_HPP
#define EPFD_ML_NATIVE_ML_MODEL_PREDICTOR_HPP

#include "epfd/ml/IModelPredictor.hpp"
#include "epfd/features/TransactionFeatures.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <cmath>

namespace epfd {

/**
 * @brief Native C++ ML Inference Predictor executing models exported from Python ML Pipeline.
 * Performs sub-millisecond real-time inference with zero external library overhead.
 */
class NativeMLModelPredictor : public IModelPredictor {
public:
    explicit NativeMLModelPredictor(std::string model_json_path = "models/fraud_ml_model.json");

    const std::string& getModelName() const noexcept override { return model_name_; }
    const std::string& getModelVersion() const noexcept override { return model_version_; }
    bool isReady() const noexcept override { return is_loaded_; }
    size_t getExpectedFeatureCount() const noexcept override { return 18; }

    PredictionResult predict(const std::vector<double>& features) override;
    PredictionResult predict(const dsa::Vector<double>& features) override;
    PredictionResult predict(const TransactionFeatures& features);

    bool loadModelFromJson(const std::string& json_path);
    bool isLoaded() const noexcept { return is_loaded_; }
    double getThreshold() const noexcept { return optimal_threshold_; }

private:
    double computeSigmoid(double z) const noexcept {
        return 1.0 / (1.0 + std::exp(-z));
    }

    std::string model_name_{"LogisticRegression"};
    std::string model_version_{"v1.0.0-PythonExported"};
    std::string model_path_;
    std::vector<double> means_;
    std::vector<double> scales_;
    std::vector<double> weights_;
    double intercept_{-8.779};
    double optimal_threshold_{0.50};
    bool is_loaded_{false};
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_ML_NATIVE_ML_MODEL_PREDICTOR_HPP
