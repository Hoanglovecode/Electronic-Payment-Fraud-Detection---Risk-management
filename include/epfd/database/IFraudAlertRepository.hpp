#ifndef EPFD_DATABASE_I_FRAUD_ALERT_REPOSITORY_HPP
#define EPFD_DATABASE_I_FRAUD_ALERT_REPOSITORY_HPP

#include <vector>
#include <string>
#include "epfd/models/FraudAlert.hpp"
#include "epfd/database/IRepository.hpp"

namespace epfd {

class IFraudAlertRepository : public IRepository<FraudAlert, std::string> {
public:
    virtual ~IFraudAlertRepository() = default;

    virtual std::vector<FraudAlert> findByTransactionId(const std::string& transaction_id) const = 0;
    virtual std::vector<FraudAlert> findByRuleId(const std::string& rule_id) const = 0;
    virtual std::vector<FraudAlert> findBySeverity(RiskLevel severity) const = 0;
};

} // namespace epfd

#endif // EPFD_DATABASE_I_FRAUD_ALERT_REPOSITORY_HPP
