#ifndef EPFD_DATABASE_I_ACCOUNT_REPOSITORY_HPP
#define EPFD_DATABASE_I_ACCOUNT_REPOSITORY_HPP

#include <vector>
#include <string>
#include "epfd/models/Account.hpp"
#include "epfd/database/IRepository.hpp"

namespace epfd {

class IAccountRepository : public IRepository<Account, std::string> {
public:
    virtual ~IAccountRepository() = default;

    virtual std::vector<Account> findByCustomerId(const std::string& customer_id) const = 0;
};

} // namespace epfd

#endif // EPFD_DATABASE_I_ACCOUNT_REPOSITORY_HPP
