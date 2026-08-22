#ifndef EPFD_DSA_RISK_RANKING_UTILS_HPP
#define EPFD_DSA_RISK_RANKING_UTILS_HPP

#include <vector>
#include <algorithm>
#include "epfd/models/Transaction.hpp"
#include "epfd/models/RiskAssessment.hpp"
#include "epfd/dsa/Vector.hpp"

namespace epfd {

class RiskRankingUtils {
public:
    /**
     * @brief Sorts RiskAssessments in descending order of combined risk score.
     */
    static void sortByRiskDescending(std::vector<RiskAssessment>& assessments);
    static void sortByRiskDescending(dsa::Vector<RiskAssessment>& assessments);

    /**
     * @brief Sorts Transactions in ascending order of timestamp.
     */
    static void sortByTimestampAscending(std::vector<Transaction>& transactions);
    static void sortByTimestampAscending(dsa::Vector<Transaction>& transactions);

    /**
     * @brief Binary search on time-sorted transactions.
     * @return Transactions whose timestamps fall within [start_time, end_time].
     */
    static std::vector<Transaction> findTransactionsInTimeRange(
        const std::vector<Transaction>& sorted_transactions,
        Timestamp start_time,
        Timestamp end_time);

    static dsa::Vector<Transaction> findTransactionsInTimeRange(
        const dsa::Vector<Transaction>& sorted_transactions,
        Timestamp start_time,
        Timestamp end_time);
};

} // namespace epfd

#endif // EPFD_DSA_RISK_RANKING_UTILS_HPP
