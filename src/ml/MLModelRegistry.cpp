#include "epfd/ml/MLModelRegistry.hpp"

namespace epfd {

bool MLModelRegistry::registerModel(const std::string& version, std::shared_ptr<IModelPredictor> model) {
    if (!model || version.empty()) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    models_[version] = std::move(model);
    if (active_version_.empty()) {
        active_version_ = version;
    }
    return true;
}

bool MLModelRegistry::setActiveVersion(const std::string& version) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = models_.find(version);
    if (it != models_.end()) {
        active_version_ = version;
        return true;
    }
    return false;
}

std::shared_ptr<IModelPredictor> MLModelRegistry::getActiveModel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = models_.find(active_version_);
    if (it != models_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<IModelPredictor> MLModelRegistry::getModel(const std::string& version) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = models_.find(version);
    if (it != models_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> MLModelRegistry::getRegisteredVersions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> versions;
    versions.reserve(models_.size());
    for (const auto& [ver, _] : models_) {
        versions.push_back(ver);
    }
    return versions;
}

std::string MLModelRegistry::getActiveVersion() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_version_;
}

size_t MLModelRegistry::count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return models_.size();
}

void MLModelRegistry::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    models_.clear();
    active_version_.clear();
}

} // namespace epfd
