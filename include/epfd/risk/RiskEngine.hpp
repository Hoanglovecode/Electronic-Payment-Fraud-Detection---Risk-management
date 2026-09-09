/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Risk Management Engine (Composition & Dependency Injection)
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 *
 * OOP Design Notes:
 * - Composition (Quan hệ Has-A): RiskEngine tổng hợp năng lực từ 4 component độc lập:
 *   + FraudDetectorEngine: Động cơ đánh giá tập luật chuyên gia.
 *   + IModelPredictor: Mô hình học máy dự đoán xác suất rủi ro.
 *   + RiskAggregator: Bộ tổng hợp trọng số đa yếu tố.
 *   + FeatureExtractor: Bộ trích xuất 18 đặc trưng giao dịch.
 * - Dependency Injection: Nhận các thành phần phụ thuộc qua constructor, giảm sự phụ thuộc cứng (loose coupling)
 *   và giúp viết unit test dễ dàng.
 */

#ifndef EPFD_RISK_RISK_ENGINE_HPP
#define EPFD_RISK_RISK_ENGINE_HPP

#include <memory>
#include <mutex>
#include "epfd/models/Transaction.hpp"
#include "epfd/models/RiskAssessment.hpp"
#include "epfd/models/FraudAlert.hpp"
#include "epfd/features/TransactionFeatures.hpp"
#include "epfd/features/FeatureExtractor.hpp"
#include "epfd/fraud/FraudDetectorEngine.hpp"
#include "epfd/ml/IModelPredictor.hpp"
#include "epfd/risk/IRiskPolicy.hpp"
#include "epfd/risk/RiskAggregator.hpp"
#include "epfd/risk/RiskProfile.hpp"

namespace epfd {

class RiskEngine {
public:
    RiskEngine(std::shared_ptr<IRiskPolicy> policy = nullptr,
               std::shared_ptr<RiskAggregator> aggregator = nullptr,
               std::shared_ptr<IModelPredictor> predictor = nullptr,
               std::shared_ptr<FraudDetectorEngine> detector = nullptr,
               std::shared_ptr<FeatureExtractor> extractor = nullptr);

    /**
     * @brief Performs full end-to-end risk assessment for a transaction.
     */
    RiskAssessment assess(const Transaction& tx,
                          const TransactionFeatures& features,
                          const std::vector<FraudAlert>& alerts,
                          const RiskProfile& profile = RiskProfile{});

    RiskAssessment assess(const Transaction& tx, const RiskProfile& profile = RiskProfile{});

    // Configuration setters
    void setPolicy(std::shared_ptr<IRiskPolicy> policy);
    void setAggregator(std::shared_ptr<RiskAggregator> aggregator);
    void setModelPredictor(std::shared_ptr<IModelPredictor> predictor);
    void setFraudDetector(std::shared_ptr<FraudDetectorEngine> detector);
    void setFeatureExtractor(std::shared_ptr<FeatureExtractor> extractor);

private:
    std::shared_ptr<IRiskPolicy> policy_;
    std::shared_ptr<RiskAggregator> aggregator_;
    std::shared_ptr<IModelPredictor> predictor_;
    std::shared_ptr<FraudDetectorEngine> detector_;
    std::shared_ptr<FeatureExtractor> extractor_;
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_RISK_RISK_ENGINE_HPP
