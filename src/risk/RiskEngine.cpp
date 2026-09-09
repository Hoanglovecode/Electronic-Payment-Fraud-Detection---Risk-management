/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Risk Management Engine Implementation
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/risk/RiskEngine.hpp"
#include "epfd/risk/ConcreteRiskPolicies.hpp"

using namespace std;

namespace epfd {

// Constructor Injection: Khởi tạo và thiết lập các thành phần phụ thuộc mặc định nếu truyền nullptr
RiskEngine::RiskEngine(shared_ptr<IRiskPolicy> policy,
                       shared_ptr<RiskAggregator> aggregator,
                       shared_ptr<IModelPredictor> predictor,
                       shared_ptr<FraudDetectorEngine> detector,
                       shared_ptr<FeatureExtractor> extractor)
    : policy_(move(policy)),
      aggregator_(move(aggregator)),
      predictor_(move(predictor)),
      detector_(move(detector)),
      extractor_(move(extractor)) {
    if (!policy_) {
        policy_ = make_shared<StandardWeightedRiskPolicy>();
    }
    if (!aggregator_) {
        aggregator_ = make_shared<RiskAggregator>();
    }
}

RiskAssessment RiskEngine::assess(const Transaction& tx,
                                  const TransactionFeatures& features,
                                  const vector<FraudAlert>& alerts,
                                  const RiskProfile& profile) {
    lock_guard<mutex> lock(mutex_);

    // 1. Dự đoán xác suất rủi ro bằng mô hình ML (nếu có sẵn)
    double ml_prob = 0.0;
    if (predictor_ && predictor_->isAvailable()) {
        try {
            auto pred = predictor_->predict(features.toVector());
            ml_prob = pred.fraud_probability;
        } catch (...) {
            // Khiem: Graceful Degradation - nếu ML gặp sự cố, hệ thống tự động fallback về 0 và dựa vào Rule Engine
            ml_prob = 0.0;
        }
    }

    // 2. Tổng hợp rủi ro đa yếu tố (Rules + ML + Lịch sử khách hàng)
    RiskAggregationResult agg_res = aggregator_->aggregate(alerts, ml_prob, features, profile);

    // 3. Quy đổi điểm số thành mức độ rủi ro (RiskLevel) theo chính sách
    RiskLevel level = policy_->mapToRiskLevel(agg_res.combined_score);

    // 4. Xác định hành động đề xuất ban đầu (Provisional Decision)
    DecisionAction action = DecisionAction::APPROVE;
    if (level == RiskLevel::CRITICAL) {
        action = DecisionAction::BLOCK;
    } else if (level == RiskLevel::HIGH) {
        action = DecisionAction::REVIEW;
    } else if (level == RiskLevel::MEDIUM) {
        action = DecisionAction::CHALLENGE_3DS;
    }

    // 5. Tổng hợp các lý do giải trình (Explainability factors)
    vector<string> reasons;
    reasons.push_back(agg_res.explanation);
    for (const auto& factor : agg_res.factors) {
        reasons.push_back(factor.toString());
    }

    string asm_id = "ASM_" + tx.getTransactionId();
    return RiskAssessment(asm_id,
                          tx.getTransactionId(),
                          agg_res.rule_score,
                          agg_res.ml_score,
                          agg_res.combined_score,
                          level,
                          action,
                          alerts,
                          reasons);
}

RiskAssessment RiskEngine::assess(const Transaction& tx, const RiskProfile& profile) {
    TransactionFeatures features;
    if (extractor_) {
        features = extractor_->extract(tx);
    }
    vector<FraudAlert> alerts;
    if (detector_) {
        alerts = detector_->detect(tx, features);
    }
    return assess(tx, features, alerts, profile);
}

void RiskEngine::setPolicy(shared_ptr<IRiskPolicy> policy) {
    lock_guard<mutex> lock(mutex_);
    policy_ = move(policy);
}

void RiskEngine::setAggregator(shared_ptr<RiskAggregator> aggregator) {
    lock_guard<mutex> lock(mutex_);
    aggregator_ = move(aggregator);
}

void RiskEngine::setModelPredictor(shared_ptr<IModelPredictor> predictor) {
    lock_guard<mutex> lock(mutex_);
    predictor_ = move(predictor);
}

void RiskEngine::setFraudDetector(shared_ptr<FraudDetectorEngine> detector) {
    lock_guard<mutex> lock(mutex_);
    detector_ = move(detector);
}

void RiskEngine::setFeatureExtractor(shared_ptr<FeatureExtractor> extractor) {
    lock_guard<mutex> lock(mutex_);
    extractor_ = move(extractor);
}

} // namespace epfd
