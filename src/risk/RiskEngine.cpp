#include "epfd/risk/RiskEngine.hpp"
#include "epfd/risk/ConcreteRiskPolicies.hpp"

namespace epfd {

RiskEngine::RiskEngine(std::shared_ptr<IRiskPolicy> policy,
                       std::shared_ptr<RiskAggregator> aggregator,
                       std::shared_ptr<IModelPredictor> predictor,
                       std::shared_ptr<FraudDetectorEngine> detector,
                       std::shared_ptr<FeatureExtractor> extractor)
    : policy_(std::move(policy)),
      aggregator_(std::move(aggregator)),
      predictor_(std::move(predictor)),
      detector_(std::move(detector)),
      extractor_(std::move(extractor)) {
    if (!policy_) {
        policy_ = std::make_shared<StandardWeightedRiskPolicy>();
    }
    if (!aggregator_) {
        aggregator_ = std::make_shared<RiskAggregator>();
    }
}

RiskAssessment RiskEngine::assess(const Transaction& tx,
                                  const TransactionFeatures& features,
                                  const std::vector<FraudAlert>& alerts,
                                  const RiskProfile& profile) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 1. ML inference (if predictor available)
    double ml_prob = 0.0;
    if (predictor_ && predictor_->isAvailable()) {
        try {
            auto pred = predictor_->predict(features.toVector());
            ml_prob = pred.fraud_probability;
        } catch (...) {
            ml_prob = 0.0; // Graceful degradation on ML failure
        }
    }

    // 2. Risk Aggregation
    RiskAggregationResult agg_res = aggregator_->aggregate(alerts, ml_prob, features, profile);

    // 3. Map to Risk Level via active Policy
    RiskLevel level = policy_->mapToRiskLevel(agg_res.combined_score);

    // 4. Provisional Decision mapping
    DecisionAction action = DecisionAction::APPROVE;
    if (level == RiskLevel::CRITICAL) {
        action = DecisionAction::BLOCK;
    } else if (level == RiskLevel::HIGH) {
        action = DecisionAction::REVIEW;
    } else if (level == RiskLevel::MEDIUM) {
        action = DecisionAction::CHALLENGE_3DS;
    }

    // 5. Reasons compilation
    std::vector<std::string> reasons;
    reasons.push_back(agg_res.explanation);
    for (const auto& factor : agg_res.factors) {
        reasons.push_back(factor.toString());
    }

    std::string asm_id = "ASM_" + tx.getTransactionId();
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
    std::vector<FraudAlert> alerts;
    if (detector_) {
        alerts = detector_->detect(tx, features);
    }
    return assess(tx, features, alerts, profile);
}

void RiskEngine::setPolicy(std::shared_ptr<IRiskPolicy> policy) {
    std::lock_guard<std::mutex> lock(mutex_);
    policy_ = std::move(policy);
}

void RiskEngine::setAggregator(std::shared_ptr<RiskAggregator> aggregator) {
    std::lock_guard<std::mutex> lock(mutex_);
    aggregator_ = std::move(aggregator);
}

void RiskEngine::setModelPredictor(std::shared_ptr<IModelPredictor> predictor) {
    std::lock_guard<std::mutex> lock(mutex_);
    predictor_ = std::move(predictor);
}

void RiskEngine::setFraudDetector(std::shared_ptr<FraudDetectorEngine> detector) {
    std::lock_guard<std::mutex> lock(mutex_);
    detector_ = std::move(detector);
}

void RiskEngine::setFeatureExtractor(std::shared_ptr<FeatureExtractor> extractor) {
    std::lock_guard<std::mutex> lock(mutex_);
    extractor_ = std::move(extractor);
}

} // namespace epfd
