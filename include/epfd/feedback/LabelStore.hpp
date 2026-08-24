#ifndef EPFD_FEEDBACK_LABEL_STORE_HPP
#define EPFD_FEEDBACK_LABEL_STORE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <chrono>
#include "epfd/common/Types.hpp"
#include "epfd/features/TransactionFeatures.hpp"

namespace epfd {

enum class LabelSource {
    ANALYST_INVESTIGATION,
    CHARGEBACK_CONFIRMED,
    CUSTOMER_DISPUTE,
    AUTHENTICATION_FAILED,
    AUTO_CLEARED
};

struct GroundTruthRecord {
    std::string transaction_id;
    GroundTruthLabel label{GroundTruthLabel::UNKNOWN};
    LabelSource source{LabelSource::ANALYST_INVESTIGATION};
    double confidence{1.0}; // [0.0, 1.0]
    TransactionFeatures features;
    Timestamp created_at{std::chrono::system_clock::now()};
    std::string notes;

    GroundTruthRecord() = default;
    GroundTruthRecord(std::string tx_id,
                      GroundTruthLabel lbl,
                      LabelSource src = LabelSource::ANALYST_INVESTIGATION,
                      double conf = 1.0,
                      TransactionFeatures feat = TransactionFeatures{},
                      std::string note_str = "")
        : transaction_id(std::move(tx_id)),
          label(lbl),
          source(src),
          confidence(conf),
          features(feat),
          created_at(std::chrono::system_clock::now()),
          notes(std::move(note_str)) {}
};

/**
 * @brief Thread-safe repository for verified ground-truth labels and records.
 * Enables offline retraining dataset generation and performance auditing.
 */
class LabelStore {
public:
    LabelStore() = default;

    bool storeLabel(const GroundTruthRecord& record);
    std::optional<GroundTruthRecord> getLabel(const std::string& tx_id) const;
    std::vector<GroundTruthRecord> getAllLabels() const;

    bool exportLabeledDatasetCsv(const std::string& filepath) const;

    size_t getFraudCount() const noexcept;
    size_t getLegitCount() const noexcept;
    size_t size() const noexcept;
    void clear();

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, GroundTruthRecord> records_;
    size_t fraud_count_{0};
    size_t legit_count_{0};
};

} // namespace epfd

#endif // EPFD_FEEDBACK_LABEL_STORE_HPP
