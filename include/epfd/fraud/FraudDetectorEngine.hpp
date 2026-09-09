/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Fraud Detector Rule Engine (Polymorphism & Open/Closed Principle)
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 *
 * OOP Design Notes:
 * - Polymorphism (Tính đa hình): Quản lý tập hợp các con trỏ đa hình std::shared_ptr<IFraudRule>.
 *   Khi thực thi hàm detect(), engine gọi rule->evaluate() và C++ tự động kích hoạt logic của
 *   lớp con tương ứng qua con trỏ vptr / bảng vtable.
 * - Open/Closed Principle (OCP): Đóng cho sửa đổi (không sửa core engine), mở cho mở rộng
 *   (dễ dàng thêm luật gian lận mới bằng addRule()).
 */

#ifndef EPFD_FRAUD_FRAUD_DETECTOR_ENGINE_HPP
#define EPFD_FRAUD_FRAUD_DETECTOR_ENGINE_HPP

#include <vector>
#include <memory>
#include <mutex>
#include "epfd/fraud/IFraudDetector.hpp"
#include "epfd/fraud/IFraudRule.hpp"
#include "epfd/features/FeatureExtractor.hpp"

namespace epfd {

class FraudDetectorEngine : public IFraudDetector {
public:
    explicit FraudDetectorEngine(std::string name = "RuleBasedFraudDetectorEngine",
                                 std::shared_ptr<FeatureExtractor> extractor = nullptr);

    const std::string& getName() const noexcept override { return name_; }

    // Rule management
    void addRule(std::shared_ptr<IFraudRule> rule);
    bool removeRule(const std::string& rule_id);
    std::shared_ptr<IFraudRule> getRule(const std::string& rule_id) const;
    bool enableRule(const std::string& rule_id, bool enabled);
    size_t ruleCount() const;
    void clearRules();

    // Feature extractor management
    void setFeatureExtractor(std::shared_ptr<FeatureExtractor> extractor);

    // Detection & Scoring with pre-extracted features
    std::vector<FraudAlert> detect(const Transaction& tx, const TransactionFeatures& features);
    double computeFraudScore(const Transaction& tx, const TransactionFeatures& features);

    // IFraudDetector Interface overrides
    std::vector<FraudAlert> detect(const Transaction& tx) override;
    double computeFraudScore(const Transaction& tx) override;

    /**
     * @brief Factory method that initializes an engine loaded with default fraud rules.
     */
    static std::shared_ptr<FraudDetectorEngine> createDefaultEngine(
        std::shared_ptr<FastLookupIndex> lookup_index = nullptr,
        std::shared_ptr<FeatureExtractor> extractor = nullptr);

private:
    std::string name_;
    std::shared_ptr<FeatureExtractor> extractor_;
    std::vector<std::shared_ptr<IFraudRule>> rules_;
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_FRAUD_FRAUD_DETECTOR_ENGINE_HPP
