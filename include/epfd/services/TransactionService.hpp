#ifndef EPFD_SERVICES_TRANSACTION_SERVICE_HPP
#define EPFD_SERVICES_TRANSACTION_SERVICE_HPP

#include <memory>
#include <mutex>
#include "epfd/models/Transaction.hpp"
#include "epfd/validation/TransactionValidator.hpp"
#include "epfd/database/ITransactionRepository.hpp"
#include "epfd/database/IAccountRepository.hpp"
#include "epfd/database/ICustomerRepository.hpp"

namespace epfd {

struct TransactionServiceResult {
    bool is_success{true};
    Transaction transaction;
    ValidationResult validation_result;
    std::string message;
};

/**
 * @brief Coordinates end-to-end transaction processing lifecycle (Orchestration Service).
 * Interacts with Validator, Account balances, and Persistence Repositories.
 */
class TransactionService {
public:
    TransactionService(std::shared_ptr<TransactionValidator> validator,
                       std::shared_ptr<ITransactionRepository> tx_repo,
                       std::shared_ptr<IAccountRepository> account_repo = nullptr,
                       std::shared_ptr<ICustomerRepository> customer_repo = nullptr);

    /**
     * @brief Ingests and processes a transaction through validation and lifecycle orchestration.
     */
    TransactionServiceResult processTransaction(Transaction tx);

    /**
     * @brief Manually settles or updates an existing transaction state.
     */
    bool settleTransaction(const std::string& transaction_id);
    bool disputeTransaction(const std::string& transaction_id);

private:
    std::shared_ptr<TransactionValidator> validator_;
    std::shared_ptr<ITransactionRepository> tx_repo_;
    std::shared_ptr<IAccountRepository> account_repo_;
    std::shared_ptr<ICustomerRepository> customer_repo_;
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_SERVICES_TRANSACTION_SERVICE_HPP
