#include "epfd/dsa/RiskRankingUtils.hpp"

namespace epfd {

void RiskRankingUtils::sortByRiskDescending(std::vector<RiskAssessment>& assessments) {
    std::sort(assessments.begin(), assessments.end(), [](const RiskAssessment& a, const RiskAssessment& b) {
        return a.getCombinedScore() > b.getCombinedScore();
    });
}

void RiskRankingUtils::sortByRiskDescending(dsa::Vector<RiskAssessment>& assessments) {
    std::sort(assessments.begin(), assessments.end(), [](const RiskAssessment& a, const RiskAssessment& b) {
        return a.getCombinedScore() > b.getCombinedScore();
    });
}

void RiskRankingUtils::sortByTimestampAscending(std::vector<Transaction>& transactions) {
    std::sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
        return a.getTimestamp() < b.getTimestamp();
    });
}

void RiskRankingUtils::sortByTimestampAscending(dsa::Vector<Transaction>& transactions) {
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

    auto low = std::lower_bound(
        sorted_transactions.begin(), sorted_transactions.end(), start_time,
        [](const Transaction& tx, Timestamp val) {
            return tx.getTimestamp() < val;
        }
    );

    auto high = std::upper_bound(
        low, sorted_transactions.end(), end_time,
        [](Timestamp val, const Transaction& tx) {
            return val < tx.getTimestamp();
        }
    );

    return std::vector<Transaction>(low, high);
}

dsa::Vector<Transaction> RiskRankingUtils::findTransactionsInTimeRange(
    const dsa::Vector<Transaction>& sorted_transactions,
    Timestamp start_time,
    Timestamp end_time) {
    
    if (sorted_transactions.empty() || start_time > end_time) {
        return {};
    }

    auto low = std::lower_bound(
        sorted_transactions.begin(), sorted_transactions.end(), start_time,
        [](const Transaction& tx, Timestamp val) {
            return tx.getTimestamp() < val;
        }
    );

    auto high = std::upper_bound(
        low, sorted_transactions.end(), end_time,
        [](Timestamp val, const Transaction& tx) {
            return val < tx.getTimestamp();
        }
    );

    dsa::Vector<Transaction> res;
    for (auto it = low; it != high; ++it) {
        res.push_back(*it);
    }
    return res;
}

} // namespace epfd
