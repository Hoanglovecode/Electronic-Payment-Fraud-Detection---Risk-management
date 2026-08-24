#include "epfd/feedback/LabelStore.hpp"
#include <fstream>

namespace epfd {

bool LabelStore::storeLabel(const GroundTruthRecord& record) {
    if (record.transaction_id.empty()) return false;
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = records_.find(record.transaction_id);
    if (it != records_.end()) {
        if (it->second.label == GroundTruthLabel::FRAUD) fraud_count_--;
        else if (it->second.label == GroundTruthLabel::LEGITIMATE) legit_count_--;
    }

    records_[record.transaction_id] = record;
    if (record.label == GroundTruthLabel::FRAUD) {
        fraud_count_++;
    } else if (record.label == GroundTruthLabel::LEGITIMATE) {
        legit_count_++;
    }
    return true;
}

std::optional<GroundTruthRecord> LabelStore::getLabel(const std::string& tx_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = records_.find(tx_id);
    if (it != records_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<GroundTruthRecord> LabelStore::getAllLabels() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<GroundTruthRecord> result;
    result.reserve(records_.size());
    for (const auto& [_, rec] : records_) {
        result.push_back(rec);
    }
    return result;
}

bool LabelStore::exportLabeledDatasetCsv(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream ofs(filepath);
    if (!ofs.is_open()) return false;

    // Header matching 18 features + label
    auto names = TransactionFeatures::getFeatureNames();
    for (size_t i = 0; i < names.size(); ++i) {
        ofs << names[i] << ",";
    }
    ofs << "is_fraud\n";

    for (const auto& [_, rec] : records_) {
        auto vec = rec.features.toVector();
        for (size_t i = 0; i < vec.size(); ++i) {
            ofs << vec[i] << ",";
        }
        int label_val = (rec.label == GroundTruthLabel::FRAUD) ? 1 : 0;
        ofs << label_val << "\n";
    }

    return true;
}

size_t LabelStore::getFraudCount() const noexcept {
    return fraud_count_;
}

size_t LabelStore::getLegitCount() const noexcept {
    return legit_count_;
}

size_t LabelStore::size() const noexcept {
    return records_.size();
}

void LabelStore::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
    fraud_count_ = 0;
    legit_count_ = 0;
}

} // namespace epfd
