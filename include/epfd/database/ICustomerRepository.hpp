#ifndef EPFD_DATABASE_I_CUSTOMER_REPOSITORY_HPP
#define EPFD_DATABASE_I_CUSTOMER_REPOSITORY_HPP

#include <vector>
#include <string>
#include "epfd/models/Customer.hpp"
#include "epfd/database/IRepository.hpp"

namespace epfd {

class ICustomerRepository : public IRepository<Customer, std::string> {
public:
    virtual ~ICustomerRepository() = default;

    virtual std::optional<Customer> findByEmail(const std::string& email) const = 0;
    virtual std::vector<Customer> findByRiskLevel(RiskLevel level) const = 0;
    virtual std::vector<Customer> findBlacklisted() const = 0;
};

} // namespace epfd

#endif // EPFD_DATABASE_I_CUSTOMER_REPOSITORY_HPP
