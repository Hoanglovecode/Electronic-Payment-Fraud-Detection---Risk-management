/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Decision Engine Implementation (Strategy & Observer Patterns)
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/decision/DecisionEngine.hpp"
#include <algorithm>

using namespace std;

namespace epfd {

DecisionEngine::DecisionEngine(shared_ptr<IDecisionPolicy> policy,
                               shared_ptr<RiskEngine> risk_engine,
                               shared_ptr<InvestigationPriorityQueue> review_queue)
    : policy_(move(policy)),
      risk_engine_(move(risk_engine)),
      review_queue_(move(review_queue)) {
    if (!policy_) {
        policy_ = make_shared<StandardDecisionPolicy>();
    }
}

// Strategy Pattern: Đổi chính sách duyệt động theo assessment
DecisionResult DecisionEngine::evaluate(const RiskAssessment& assessment) {
    shared_ptr<IDecisionPolicy> current_policy;
    shared_ptr<InvestigationPriorityQueue> current_queue;
    vector<shared_ptr<IObserver<DecisionResult>>> current_observers;

    {
        lock_guard<mutex> lock(mutex_);
        current_policy = policy_;
        current_queue = review_queue_;
        current_observers = observers_;
    }

    if (!current_policy) {
        current_policy = make_shared<StandardDecisionPolicy>();
    }

    // Thực thi chiến lược hiện tại (Polymorphic call)
    DecisionResult result = current_policy->decide(assessment);

    // Nếu kết quả là REVIEW: tự động đẩy ca điều tra vào hàng đợi ưu tiên (Priority Queue O(log N))
    if (result.isReviewed() && current_queue) {
        InvestigationCase ic;
        ic.case_id = "CASE_" + assessment.getTransactionId();
        ic.transaction_id = assessment.getTransactionId();
        ic.risk_score = assessment.getCombinedScore();
        ic.severity = assessment.getRiskLevel();
        current_queue->push(move(ic));
    }

    // Observer Pattern: Phát thông báo sự kiện quyết định tới tất cả Observer đã đăng ký
    for (const auto& obs : current_observers) {
        if (obs) {
            obs->onNotify(result);
        }
    }

    return result;
}

DecisionResult DecisionEngine::evaluate(const Transaction& tx, const RiskProfile& profile) {
    shared_ptr<RiskEngine> current_risk_engine;
    {
        lock_guard<mutex> lock(mutex_);
        current_risk_engine = risk_engine_;
    }

    RiskAssessment assessment;
    if (current_risk_engine) {
        assessment = current_risk_engine->assess(tx, profile);
    } else {
        // Fallback an toàn nếu chưa gắn RiskEngine
        assessment = RiskAssessment("ASM_" + tx.getTransactionId(), tx.getTransactionId(), 0.0, 0.0, 0.0,
                                    RiskLevel::VERY_LOW, DecisionAction::APPROVE);
    }

    return evaluate(assessment);
}

void DecisionEngine::setPolicy(shared_ptr<IDecisionPolicy> policy) {
    lock_guard<mutex> lock(mutex_);
    policy_ = move(policy);
}

shared_ptr<IDecisionPolicy> DecisionEngine::getPolicy() const {
    lock_guard<mutex> lock(mutex_);
    return policy_;
}

void DecisionEngine::setRiskEngine(shared_ptr<RiskEngine> risk_engine) {
    lock_guard<mutex> lock(mutex_);
    risk_engine_ = move(risk_engine);
}

void DecisionEngine::setInvestigationQueue(shared_ptr<InvestigationPriorityQueue> queue) {
    lock_guard<mutex> lock(mutex_);
    review_queue_ = move(queue);
}

// Observer Pattern management
void DecisionEngine::attachObserver(shared_ptr<IObserver<DecisionResult>> observer) {
    lock_guard<mutex> lock(mutex_);
    if (observer) {
        observers_.push_back(move(observer));
    }
}

void DecisionEngine::detachObserver(shared_ptr<IObserver<DecisionResult>> observer) {
    lock_guard<mutex> lock(mutex_);
    observers_.erase(remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void DecisionEngine::notifyObservers(const DecisionResult& result) {
    vector<shared_ptr<IObserver<DecisionResult>>> copy_observers;
    {
        lock_guard<mutex> lock(mutex_);
        copy_observers = observers_;
    }
    for (const auto& obs : copy_observers) {
        if (obs) {
            obs->onNotify(result);
        }
    }
}

} // namespace epfd
