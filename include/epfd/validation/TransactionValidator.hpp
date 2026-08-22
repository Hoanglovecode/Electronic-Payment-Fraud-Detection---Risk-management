#ifndef EPFD_VALIDATION_TRANSACTION_VALIDATOR_HPP
#define EPFD_VALIDATION_TRANSACTION_VALIDATOR_HPP

#include <memory>
#include <mutex>
#include "epfd/models/Transaction.hpp"
#include "epfd/validation/ValidationResult.hpp"
#include "epfd/dsa/HashSet.hpp"

namespace epfd {

class ITransactionRepository;
class IAccountRepository;

/**
 * @brief Strictly validates transaction data integrity and business invariants (SRP).
 * Does NOT contain risk scoring or fraud detection logic.
 */
class TransactionValidator {
public:
    TransactionValidator(std::shared_ptr<ITransactionRepository> tx_repo = nullptr,
                         std::shared_ptr<IAccountRepository> account_repo = nullptr);

    /**
     * @brief Validates all transaction fields and constraints.
     */
    ValidationResult validate(const Transaction& tx) const;

    /**
     * @brief Clears in-memory duplicate check cache if any.
     */
    void clearSeenTransactions();

private:
    std::shared_ptr<ITransactionRepository> tx_repo_;
    std::shared_ptr<IAccountRepository> account_repo_;
    mutable dsa::HashSet<std::string> seen_transaction_ids_;
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_VALIDATION_TRANSACTION_VALIDATOR_HPP
