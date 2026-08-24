#ifndef EPFD_ML_ML_MODEL_REGISTRY_HPP
#define EPFD_ML_ML_MODEL_REGISTRY_HPP

#include "epfd/ml/IModelPredictor.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>
#include <vector>

namespace epfd {

/**
 * @brief Dynamic Model Registry supporting zero-downtime hot-swapping and model version tracking.
 */
class MLModelRegistry {
public:
    MLModelRegistry() = default;

    bool registerModel(const std::string& version, std::shared_ptr<IModelPredictor> model);
    bool setActiveVersion(const std::string& version);
    std::shared_ptr<IModelPredictor> getActiveModel() const;
    std::shared_ptr<IModelPredictor> getModel(const std::string& version) const;

    std::vector<std::string> getRegisteredVersions() const;
    std::string getActiveVersion() const;
    size_t count() const;
    void clear();

private:
    mutable std::mutex mutex_;
    std::string active_version_;
    std::unordered_map<std::string, std::shared_ptr<IModelPredictor>> models_;
};

} // namespace epfd

#endif // EPFD_ML_ML_MODEL_REGISTRY_HPP
