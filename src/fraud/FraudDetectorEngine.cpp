#include "epfd/fraud/FraudDetectorEngine.hpp"
#include "epfd/fraud/ConcreteFraudRules.hpp"
#include <algorithm>

namespace epfd {

FraudDetectorEngine::FraudDetectorEngine(std::string name, std::shared_ptr<FeatureExtractor> extractor)
    : name_(std::move(name)), extractor_(std::move(extractor)) {}

void FraudDetectorEngine::addRule(std::shared_ptr<IFraudRule> rule) {
    if (!rule) return;
    std::lock_guard<std::mutex> lock(mutex_);
    // Avoid duplicate rule ID
    for (auto& r : rules_) {
        if (r->getId() == rule->getId()) {
            r = rule;
            return;
        }
    }
    rules_.push_back(std::move(rule));
}

bool FraudDetectorEngine::removeRule(const std::string& rule_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(rules_.begin(), rules_.end(),
                             [&rule_id](const std::shared_ptr<IFraudRule>& r) {
                                 return r->getId() == rule_id;
                             });
    if (it != rules_.end()) {
        rules_.erase(it, rules_.end());
        return true;
    }
    return false;
}

std::shared_ptr<IFraudRule> FraudDetectorEngine::getRule(const std::string& rule_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& r : rules_) {
        if (r->getId() == rule_id) {
            return r;
        }
    }
    return nullptr;
}

bool FraudDetectorEngine::enableRule(const std::string& rule_id, bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& r : rules_) {
        if (r->getId() == rule_id) {
            r->setEnabled(enabled);
            return true;
        }
    }
    return false;
}

size_t FraudDetectorEngine::ruleCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return rules_.size();
}

void FraudDetectorEngine::clearRules() {
    std::lock_guard<std::mutex> lock(mutex_);
    rules_.clear();
}

void FraudDetectorEngine::setFeatureExtractor(std::shared_ptr<FeatureExtractor> extractor) {
    std::lock_guard<std::mutex> lock(mutex_);
    extractor_ = std::move(extractor);
}

std::vector<FraudAlert> FraudDetectorEngine::detect(const Transaction& tx, const TransactionFeatures& features) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<FraudAlert> alerts;

    for (const auto& rule : rules_) {
        if (rule && rule->isEnabled()) {
            auto alert_opt = rule->evaluate(tx, features);
            if (alert_opt.has_value()) {
                alerts.push_back(alert_opt.value());
            }
        }
    }

    return alerts;
}

double FraudDetectorEngine::computeFraudScore(const Transaction& tx, const TransactionFeatures& features) {
    std::vector<FraudAlert> alerts = detect(tx, features);
    if (alerts.empty()) {
        return 0.0;
    }

    double total_score = 0.0;
    bool has_critical = false;

    for (const auto& alert : alerts) {
        total_score += alert.getScoreContribution();
        if (alert.getSeverity() == RiskLevel::CRITICAL) {
            has_critical = true;
        }
    }

    if (has_critical && total_score < 85.0) {
        total_score = 85.0;
    }

    return std::min(100.0, total_score);
}

std::vector<FraudAlert> FraudDetectorEngine::detect(const Transaction& tx) {
    TransactionFeatures features;
    if (extractor_) {
        features = extractor_->extract(tx);
    }
    return detect(tx, features);
}

double FraudDetectorEngine::computeFraudScore(const Transaction& tx) {
    TransactionFeatures features;
    if (extractor_) {
        features = extractor_->extract(tx);
    }
    return computeFraudScore(tx, features);
}

std::shared_ptr<FraudDetectorEngine> FraudDetectorEngine::createDefaultEngine(
    std::shared_ptr<FastLookupIndex> lookup_index,
    std::shared_ptr<FeatureExtractor> extractor) {
    
    auto engine = std::make_shared<FraudDetectorEngine>("DefaultRuleBasedFraudDetectorEngine", std::move(extractor));

    // Register all initial & advanced rules
    engine->addRule(std::make_shared<LargeAmountRule>(5000.0, 35.0));
    engine->addRule(std::make_shared<HighVelocityRule>(3, 8, 50.0));
    engine->addRule(std::make_shared<NewDeviceRule>(25.0));
    engine->addRule(std::make_shared<ImpossibleTravelRule>(800.0, 70.0));
    engine->addRule(std::make_shared<ForeignCountryRule>(30.0));
    engine->addRule(std::make_shared<UnusualTimeRule>(1.0, 5.0, 15.0));
    engine->addRule(std::make_shared<SuspiciousMerchantRule>(0.70, 45.0));
    engine->addRule(std::make_shared<BehaviorDeviationRule>(3.0, 40.0));
    engine->addRule(std::make_shared<CardTestingRule>(5.0, 60.0));
    engine->addRule(std::make_shared<AccountTakeoverSignalRule>(75.0));

    if (lookup_index) {
        engine->addRule(std::make_shared<BlacklistRule>(lookup_index, 100.0));
        engine->addRule(std::make_shared<WhitelistRule>(lookup_index, 0.0));
    }

    return engine;
}

} // namespace epfd
