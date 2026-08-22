#ifndef EPFD_DSA_INVESTIGATION_PRIORITY_QUEUE_HPP
#define EPFD_DSA_INVESTIGATION_PRIORITY_QUEUE_HPP

#include <string>
#include <chrono>
#include <mutex>
#include "epfd/common/Types.hpp"
#include "epfd/dsa/PriorityQueue.hpp"

namespace epfd {

struct InvestigationCase {
    std::string case_id;
    std::string transaction_id;
    std::string customer_id;
    double risk_score{0.0};
    double amount{0.0};
    RiskLevel severity{RiskLevel::MEDIUM};
    Timestamp created_at{std::chrono::system_clock::now()};
    Timestamp sla_deadline{std::chrono::system_clock::now() + std::chrono::hours(24)};

    double getPriorityRank() const {
        double base = risk_score * 10.0 + (amount > 1000.0 ? 50.0 : 0.0);
        if (severity == RiskLevel::CRITICAL) base += 200.0;
        else if (severity == RiskLevel::HIGH) base += 100.0;
        return base;
    }
};

struct InvestigationCaseComparator {
    bool operator()(const InvestigationCase& a, const InvestigationCase& b) const {
        // Max-heap: highest priority rank comes out first
        return a.getPriorityRank() < b.getPriorityRank();
    }
};

/**
 * @brief Thread-safe Priority Queue for triaging high-risk fraud cases using custom PriorityQueue.
 */
class InvestigationPriorityQueue {
public:
    InvestigationPriorityQueue() = default;

    void push(InvestigationCase item) {
        std::lock_guard<std::mutex> lock(mutex_);
        pq_.push(std::move(item));
    }

    bool pop(InvestigationCase& out_case) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pq_.empty()) {
            return false;
        }
        out_case = pq_.top();
        pq_.pop();
        return true;
    }

    bool peek(InvestigationCase& out_case) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pq_.empty()) {
            return false;
        }
        out_case = pq_.top();
        return true;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pq_.size();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pq_.empty();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        pq_.clear();
    }

private:
    mutable std::mutex mutex_;
    dsa::PriorityQueue<InvestigationCase, InvestigationCaseComparator> pq_;
};

} // namespace epfd

#endif // EPFD_DSA_INVESTIGATION_PRIORITY_QUEUE_HPP
