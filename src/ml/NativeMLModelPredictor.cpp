#include "epfd/ml/NativeMLModelPredictor.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>

namespace epfd {

NativeMLModelPredictor::NativeMLModelPredictor(std::string model_json_path)
    : model_path_(std::move(model_json_path)) {
    // Initialize with default trained weights from Python Pipeline
    means_ = {
        123.81, 11.79, 3.05, 0.28, 0.40, 2.56, 358.38, 1.18, 0.13,
        1.76, 0.06, 128.39, 39.01, 6.68, 0.09, 0.59, 0.10, 7.47
    };
    scales_ = {
        589.82, 4.91, 1.99, 0.45, 0.93, 2.18, 3209.87, 1.23, 0.34,
        1.03, 0.25, 780.77, 242.25, 11.27, 0.29, 0.49, 0.51, 10.17
    };
    weights_ = {
        0.0841, -0.0558, 0.0213, 0.0277, 0.4256, 0.4242, 0.0871, 0.5791, 0.2621,
        0.3851, 0.2091, 0.4754, 0.4635, 0.6018, 0.1934, -0.1299, 0.4278, 0.6756
    };
    intercept_ = -8.779;
    optimal_threshold_ = 0.50;
    is_loaded_ = true;

    // Try loading exact file if exists
    loadModelFromJson(model_path_);
}

bool NativeMLModelPredictor::loadModelFromJson(const std::string& json_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ifstream ifs(json_path);
    if (!ifs.is_open()) {
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());

    auto extractArray = [](const std::string& text, const std::string& key) -> std::vector<double> {
        std::vector<double> result;
        auto pos = text.find("\"" + key + "\"");
        if (pos == std::string::npos) return result;
        auto start = text.find('[', pos);
        auto end = text.find(']', start);
        if (start == std::string::npos || end == std::string::npos) return result;

        std::string arr_str = text.substr(start + 1, end - start - 1);
        std::stringstream ss(arr_str);
        std::string item;
        while (std::getline(ss, item, ',')) {
            try {
                item.erase(0, item.find_first_not_of(" \t\n\r"));
                item.erase(item.find_last_not_of(" \t\n\r") + 1);
                if (!item.empty()) {
                    result.push_back(std::stod(item));
                }
            } catch (...) {}
        }
        return result;
    };

    auto m = extractArray(content, "scaler_mean");
    auto s = extractArray(content, "scaler_scale");
    auto w = extractArray(content, "coefficients");

    if (m.size() == 18 && s.size() == 18 && w.size() == 18) {
        means_ = std::move(m);
        scales_ = std::move(s);
        weights_ = std::move(w);

        auto pos_int = content.find("\"intercept\":");
        if (pos_int != std::string::npos) {
            auto start_int = pos_int + 12;
            auto end_int = content.find_first_of(",\r\n}", start_int);
            if (end_int != std::string::npos) {
                try {
                    intercept_ = std::stod(content.substr(start_int, end_int - start_int));
                } catch (...) {}
            }
        }

        is_loaded_ = true;
        return true;
    }
    return false;
}

PredictionResult NativeMLModelPredictor::predict(const TransactionFeatures& features) {
    return predict(features.toVector());
}

PredictionResult NativeMLModelPredictor::predict(const std::vector<double>& features) {
    auto start_time = std::chrono::high_resolution_clock::now();

    double z = intercept_;
    size_t dim = std::min({features.size(), means_.size(), scales_.size(), weights_.size()});

    for (size_t i = 0; i < dim; ++i) {
        double scale = (scales_[i] > 1e-6) ? scales_[i] : 1.0;
        double x_scaled = (features[i] - means_[i]) / scale;
        z += weights_[i] * x_scaled;
    }

    double prob = computeSigmoid(z);

    auto end_time = std::chrono::high_resolution_clock::now();
    double latency_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    PredictionResult res;
    res.probability = prob;
    res.fraud_probability = prob;
    res.is_success = true;
    res.latency_ms = latency_ms;

    return res;
}

PredictionResult NativeMLModelPredictor::predict(const dsa::Vector<double>& features) {
    std::vector<double> std_f;
    std_f.reserve(features.size());
    for (size_t i = 0; i < features.size(); ++i) {
        std_f.push_back(features[i]);
    }
    return predict(std_f);
}

} // namespace epfd
