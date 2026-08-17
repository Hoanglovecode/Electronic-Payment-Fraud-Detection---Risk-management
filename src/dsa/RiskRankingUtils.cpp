#include "epfd/dsa/RiskRankingUtils.hpp"

namespace epfd {

void RiskRankingUtils::sortByRiskDescending(std::vector<RiskAssessment>& assessments) {
    std::sort(assessments.begin(), assessments.end(), [](const RiskAssessment& a, const RiskAssessment& b) {
        return a.getCombinedScore() > b.getCombinedScore();
    });
}

void RiskRankingUtils::sortByTimestampAscending(std::vector<Transaction>& transactions) {
    std::sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
        return a.getTimestamp() < b.getTimestamp();
    });
}

std::vector<Transaction> RiskRankingUtils::findTransactionsInTimeRange(
    const std::vector<Transaction>& sorted_transactions,
    Timestamp start_time,
    Timestamp end_time) {
    
    if (sorted_transactions.empty() || start_time > end_time) {
        return {};
    }

    // Binary search for lower bound
    auto low = std::lower_bound(
        sorted_transactions.begin(), sorted_transactions.end(), start_time,
        [](const Transaction& tx, Timestamp val) {
            return tx.getTimestamp() < val;
        }
    );

    // Binary search for upper bound
    auto high = std::upper_bound(
        low, sorted_transactions.end(), end_time,
        [](Timestamp val, const Transaction& tx) {
            return val < tx.getTimestamp();
        }
    );

    return std::vector<Transaction>(low, high);
}

} // namespace epfd
