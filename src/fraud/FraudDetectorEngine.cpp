/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Fraud Detector Rule Engine Implementation
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/fraud/FraudDetectorEngine.hpp"
#include "epfd/fraud/ConcreteFraudRules.hpp"
#include <algorithm>

using namespace std;

namespace epfd {

FraudDetectorEngine::FraudDetectorEngine(string name, shared_ptr<FeatureExtractor> extractor)
    : name_(move(name)), extractor_(move(extractor)) {}

void FraudDetectorEngine::addRule(shared_ptr<IFraudRule> rule) {
    if (!rule) return;
    lock_guard<mutex> lock(mutex_);
    // Tránh trùng lặp ID rule
    for (auto& r : rules_) {
        if (r->getId() == rule->getId()) {
            r = rule;
            return;
        }
    }
    rules_.push_back(move(rule));
}

bool FraudDetectorEngine::removeRule(const string& rule_id) {
    lock_guard<mutex> lock(mutex_);
    auto it = remove_if(rules_.begin(), rules_.end(),
                        [&rule_id](const shared_ptr<IFraudRule>& r) {
                            return r->getId() == rule_id;
                        });
    if (it != rules_.end()) {
        rules_.erase(it, rules_.end());
        return true;
    }
    return false;
}

shared_ptr<IFraudRule> FraudDetectorEngine::getRule(const string& rule_id) const {
    lock_guard<mutex> lock(mutex_);
    for (const auto& r : rules_) {
        if (r->getId() == rule_id) {
            return r;
        }
    }
    return nullptr;
}

bool FraudDetectorEngine::enableRule(const string& rule_id, bool enabled) {
    lock_guard<mutex> lock(mutex_);
    for (const auto& r : rules_) {
        if (r->getId() == rule_id) {
            r->setEnabled(enabled);
            return true;
        }
    }
    return false;
}

size_t FraudDetectorEngine::ruleCount() const {
    lock_guard<mutex> lock(mutex_);
    return rules_.size();
}

void FraudDetectorEngine::clearRules() {
    lock_guard<mutex> lock(mutex_);
    rules_.clear();
}

void FraudDetectorEngine::setFeatureExtractor(shared_ptr<FeatureExtractor> extractor) {
    lock_guard<mutex> lock(mutex_);
    extractor_ = move(extractor);
}

// Polymorphism in action: Duyệt danh sách các con trỏ IFraudRule và gọi evaluate() đa hình
vector<FraudAlert> FraudDetectorEngine::detect(const Transaction& tx, const TransactionFeatures& features) {
    lock_guard<mutex> lock(mutex_);
    vector<FraudAlert> alerts;

    for (const auto& rule : rules_) {
        if (rule && rule->isEnabled()) {
            // Lời gọi hàm ảo: Trình biên dịch tự phân giải đúng hàm evaluate() của class con qua vtable
            auto alert_opt = rule->evaluate(tx, features);
            if (alert_opt.has_value()) {
                alerts.push_back(alert_opt.value());
            }
        }
    }

    return alerts;
}

double FraudDetectorEngine::computeFraudScore(const Transaction& tx, const TransactionFeatures& features) {
    vector<FraudAlert> alerts = detect(tx, features);
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

    // Hoang: Nếu có luật CRITICAL (ví dụ Blacklist hoặc Impossible Travel), sàn rủi ro tối thiểu là 85
    if (has_critical && total_score < 85.0) {
        total_score = 85.0;
    }

    return min(100.0, total_score);
}

vector<FraudAlert> FraudDetectorEngine::detect(const Transaction& tx) {
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

// Factory pattern: Tạo động bộ engine hoàn chỉnh với 12 luật mặc định
shared_ptr<FraudDetectorEngine> FraudDetectorEngine::createDefaultEngine(
    shared_ptr<FastLookupIndex> lookup_index,
    shared_ptr<FeatureExtractor> extractor) {
    
    auto engine = make_shared<FraudDetectorEngine>("DefaultRuleBasedFraudDetectorEngine", move(extractor));

    // Đăng ký toàn bộ các luật gian lận (12 rules)
    engine->addRule(make_shared<LargeAmountRule>(5000.0, 35.0));
    engine->addRule(make_shared<HighVelocityRule>(3, 8, 50.0));
    engine->addRule(make_shared<NewDeviceRule>(25.0));
    engine->addRule(make_shared<ImpossibleTravelRule>(800.0, 70.0));
    engine->addRule(make_shared<ForeignCountryRule>(30.0));
    engine->addRule(make_shared<UnusualTimeRule>(1.0, 5.0, 15.0));
    engine->addRule(make_shared<SuspiciousMerchantRule>(0.70, 45.0));
    engine->addRule(make_shared<BehaviorDeviationRule>(3.0, 40.0));
    engine->addRule(make_shared<CardTestingRule>(5.0, 60.0));
    engine->addRule(make_shared<AccountTakeoverSignalRule>(75.0));

    if (lookup_index) {
        engine->addRule(make_shared<BlacklistRule>(lookup_index, 100.0));
        engine->addRule(make_shared<WhitelistRule>(lookup_index, 0.0));
    }

    return engine;
}

} // namespace epfd
