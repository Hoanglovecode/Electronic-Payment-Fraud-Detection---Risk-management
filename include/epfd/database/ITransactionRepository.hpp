#ifndef EPFD_DATABASE_I_TRANSACTION_REPOSITORY_HPP
#define EPFD_DATABASE_I_TRANSACTION_REPOSITORY_HPP

#include <vector>
#include <string>
#include <chrono>
#include "epfd/models/Transaction.hpp"
#include "epfd/database/IRepository.hpp"

namespace epfd {

class ITransactionRepository : public IRepository<Transaction, std::string> {
public:
    virtual ~ITransactionRepository() = default;

    virtual std::vector<Transaction> findByCustomerId(const std::string& customer_id) const = 0;
    virtual std::vector<Transaction> findByStatus(TransactionStatus status) const = 0;
    virtual std::vector<Transaction> findRecentByCustomer(const std::string& customer_id, std::chrono::seconds duration) const = 0;
};

} // namespace epfd

#endif // EPFD_DATABASE_I_TRANSACTION_REPOSITORY_HPP
