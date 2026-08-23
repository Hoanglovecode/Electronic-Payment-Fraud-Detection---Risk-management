#ifndef EPFD_DECISION_DECISION_ENGINE_HPP
#define EPFD_DECISION_DECISION_ENGINE_HPP

#include <memory>
#include <mutex>
#include <vector>
#include "epfd/common/Types.hpp"
#include "epfd/models/Transaction.hpp"
#include "epfd/models/RiskAssessment.hpp"
#include "epfd/risk/RiskEngine.hpp"
#include "epfd/decision/IDecisionPolicy.hpp"
#include "epfd/decision/ConcreteDecisionPolicies.hpp"
#include "epfd/dsa/InvestigationPriorityQueue.hpp"
#include "epfd/utils/Observer.hpp"

namespace epfd {

/**
 * @brief Master Business Decision Engine converting Risk Assessments into operational actions.
 * Supports dynamic Policy swapping (Strategy pattern) and real-time Event Publishing (Observer pattern).
 */
class DecisionEngine {
public:
    explicit DecisionEngine(std::shared_ptr<IDecisionPolicy> policy = nullptr,
                           std::shared_ptr<RiskEngine> risk_engine = nullptr,
                           std::shared_ptr<InvestigationPriorityQueue> review_queue = nullptr);

    /**
     * @brief Evaluates an existing RiskAssessment and applies business policy.
     */
    DecisionResult evaluate(const RiskAssessment& assessment);

    /**
     * @brief End-to-end evaluation: Runs RiskEngine to assess risk, then applies decision policy.
     */
    DecisionResult evaluate(const Transaction& tx, const RiskProfile& profile = RiskProfile{});

    // Dependency injection & configuration
    void setPolicy(std::shared_ptr<IDecisionPolicy> policy);
    std::shared_ptr<IDecisionPolicy> getPolicy() const;

    void setRiskEngine(std::shared_ptr<RiskEngine> risk_engine);
    void setInvestigationQueue(std::shared_ptr<InvestigationPriorityQueue> queue);

    // Observer management (Event broadcasting)
    void attachObserver(std::shared_ptr<IObserver<DecisionResult>> observer);
    void detachObserver(std::shared_ptr<IObserver<DecisionResult>> observer);
    void notifyObservers(const DecisionResult& result);

private:
    std::shared_ptr<IDecisionPolicy> policy_;
    std::shared_ptr<RiskEngine> risk_engine_;
    std::shared_ptr<InvestigationPriorityQueue> review_queue_;
    std::vector<std::shared_ptr<IObserver<DecisionResult>>> observers_;
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_DECISION_DECISION_ENGINE_HPP
