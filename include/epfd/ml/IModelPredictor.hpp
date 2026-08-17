#ifndef EPFD_ML_I_MODEL_PREDICTOR_HPP
#define EPFD_ML_I_MODEL_PREDICTOR_HPP

#include <string>
#include <vector>
#include <optional>
#include <stdexcept>

namespace epfd {

enum class MLFailurePolicy {
    FAIL_OPEN,   // On ML failure, assume 0.0 fraud probability (allow rule engine to decide)
    FAIL_CLOSED, // On ML failure, assume 1.0 fraud probability / trigger high risk
    FALLBACK_SCORE // Return configurable default fallback score
};

struct PredictionResult {
    double probability{0.0};
    bool is_success{true};
    std::string error_message;
    double latency_ms{0.0};
};

/**
 * @brief Interface for Machine Learning Inference Engine (DIP & ISP).
 * Isolates C++ core from runtime technologies (ONNX, C-API, Mock).
 */
class IModelPredictor {
public:
    virtual ~IModelPredictor() = default;

    virtual const std::string& getModelName() const noexcept = 0;
    virtual const std::string& getModelVersion() const noexcept = 0;
    virtual bool isReady() const noexcept = 0;
    virtual size_t getExpectedFeatureCount() const noexcept = 0;

    /**
     * @brief Predicts probability of fraud [0.0, 1.0] given a vector of engineered features.
     */
    virtual PredictionResult predict(const std::vector<double>& features) = 0;
};

} // namespace epfd

#endif // EPFD_ML_I_MODEL_PREDICTOR_HPP
