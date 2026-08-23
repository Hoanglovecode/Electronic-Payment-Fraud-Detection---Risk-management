#ifndef EPFD_DATABASE_I_RISK_ASSESSMENT_REPOSITORY_HPP
#define EPFD_DATABASE_I_RISK_ASSESSMENT_REPOSITORY_HPP

#include <vector>
#include <string>
#include "epfd/models/RiskAssessment.hpp"
#include "epfd/database/IRepository.hpp"

namespace epfd {

class IRiskAssessmentRepository : public IRepository<RiskAssessment, std::string> {
public:
    virtual ~IRiskAssessmentRepository() = default;

    virtual std::optional<RiskAssessment> findByTransactionId(const std::string& transaction_id) const = 0;
    virtual std::vector<RiskAssessment> findByRiskLevel(RiskLevel level) const = 0;
    virtual std::vector<RiskAssessment> findByDecision(DecisionAction decision) const = 0;
};

} // namespace epfd

#endif // EPFD_DATABASE_I_RISK_ASSESSMENT_REPOSITORY_HPP
