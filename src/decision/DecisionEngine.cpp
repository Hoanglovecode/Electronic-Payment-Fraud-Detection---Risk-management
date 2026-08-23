#include "epfd/decision/DecisionEngine.hpp"
#include <algorithm>

namespace epfd {

DecisionEngine::DecisionEngine(std::shared_ptr<IDecisionPolicy> policy,
                               std::shared_ptr<RiskEngine> risk_engine,
                               std::shared_ptr<InvestigationPriorityQueue> review_queue)
    : policy_(std::move(policy)),
      risk_engine_(std::move(risk_engine)),
      review_queue_(std::move(review_queue)) {
    if (!policy_) {
        policy_ = std::make_shared<StandardDecisionPolicy>();
    }
}

DecisionResult DecisionEngine::evaluate(const RiskAssessment& assessment) {
    std::shared_ptr<IDecisionPolicy> current_policy;
    std::shared_ptr<InvestigationPriorityQueue> current_queue;
    std::vector<std::shared_ptr<IObserver<DecisionResult>>> current_observers;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_policy = policy_;
        current_queue = review_queue_;
        current_observers = observers_;
    }

    if (!current_policy) {
        current_policy = std::make_shared<StandardDecisionPolicy>();
    }

    DecisionResult result = current_policy->decide(assessment);

    // If decision mandates manual investigation, automatically push to InvestigationPriorityQueue
    if (result.isReviewed() && current_queue) {
        InvestigationCase ic;
        ic.case_id = "CASE_" + assessment.getTransactionId();
        ic.transaction_id = assessment.getTransactionId();
        ic.risk_score = assessment.getCombinedScore();
        ic.severity = assessment.getRiskLevel();
        current_queue->push(std::move(ic));
    }

    // Broadcast decision to all attached observers
    for (const auto& obs : current_observers) {
        if (obs) {
            obs->onNotify(result);
        }
    }

    return result;
}

DecisionResult DecisionEngine::evaluate(const Transaction& tx, const RiskProfile& profile) {
    std::shared_ptr<RiskEngine> current_risk_engine;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_risk_engine = risk_engine_;
    }

    RiskAssessment assessment;
    if (current_risk_engine) {
        assessment = current_risk_engine->assess(tx, profile);
    } else {
        // Fallback default assessment if risk engine not wired
        assessment = RiskAssessment("ASM_" + tx.getTransactionId(), tx.getTransactionId(), 0.0, 0.0, 0.0,
                                    RiskLevel::VERY_LOW, DecisionAction::APPROVE);
    }

    return evaluate(assessment);
}

void DecisionEngine::setPolicy(std::shared_ptr<IDecisionPolicy> policy) {
    std::lock_guard<std::mutex> lock(mutex_);
    policy_ = std::move(policy);
}

std::shared_ptr<IDecisionPolicy> DecisionEngine::getPolicy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return policy_;
}

void DecisionEngine::setRiskEngine(std::shared_ptr<RiskEngine> risk_engine) {
    std::lock_guard<std::mutex> lock(mutex_);
    risk_engine_ = std::move(risk_engine);
}

void DecisionEngine::setInvestigationQueue(std::shared_ptr<InvestigationPriorityQueue> queue) {
    std::lock_guard<std::mutex> lock(mutex_);
    review_queue_ = std::move(queue);
}

void DecisionEngine::attachObserver(std::shared_ptr<IObserver<DecisionResult>> observer) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (observer) {
        observers_.push_back(std::move(observer));
    }
}

void DecisionEngine::detachObserver(std::shared_ptr<IObserver<DecisionResult>> observer) {
    std::lock_guard<std::mutex> lock(mutex_);
    observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void DecisionEngine::notifyObservers(const DecisionResult& result) {
    std::vector<std::shared_ptr<IObserver<DecisionResult>>> copy_observers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy_observers = observers_;
    }
    for (const auto& obs : copy_observers) {
        if (obs) {
            obs->onNotify(result);
        }
    }
}

} // namespace epfd
