#ifndef EPFD_DATABASE_I_FRAUD_CASE_REPOSITORY_HPP
#define EPFD_DATABASE_I_FRAUD_CASE_REPOSITORY_HPP

#include <vector>
#include <string>
#include "epfd/models/Dispute.hpp"
#include "epfd/database/IRepository.hpp"

namespace epfd {

class IFraudCaseRepository : public IRepository<Dispute, std::string> {
public:
    virtual ~IFraudCaseRepository() = default;

    virtual std::vector<Dispute> findByTransactionId(const std::string& transaction_id) const = 0;
    virtual std::vector<Dispute> findByCustomerId(const std::string& customer_id) const = 0;
    virtual std::vector<Dispute> findByStatus(CaseStatus status) const = 0;
};

} // namespace epfd

#endif // EPFD_DATABASE_I_FRAUD_CASE_REPOSITORY_HPP
