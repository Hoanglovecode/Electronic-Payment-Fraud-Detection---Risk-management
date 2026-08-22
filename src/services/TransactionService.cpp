#include "epfd/services/TransactionService.hpp"

namespace epfd {

TransactionService::TransactionService(std::shared_ptr<TransactionValidator> validator,
                                       std::shared_ptr<ITransactionRepository> tx_repo,
                                       std::shared_ptr<IAccountRepository> account_repo,
                                       std::shared_ptr<ICustomerRepository> customer_repo)
    : validator_(std::move(validator)),
      tx_repo_(std::move(tx_repo)),
      account_repo_(std::move(account_repo)),
      customer_repo_(std::move(customer_repo)) {
    if (!validator_) {
        validator_ = std::make_shared<TransactionValidator>(tx_repo_, account_repo_);
    }
}

TransactionServiceResult TransactionService::processTransaction(Transaction tx) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 1. Validation phase
    ValidationResult val_res = validator_->validate(tx);
    if (!val_res.is_valid) {
        tx.setStatus(TransactionStatus::REJECTED);
        if (tx_repo_) {
            tx_repo_->save(tx);
        }
        return TransactionServiceResult{
            false,
            tx,
            val_res,
            "Transaction validation failed: " + val_res.error_message
        };
    }

    // 2. Account balance mutation (if approved)
    if (account_repo_) {
        auto acc_opt = account_repo_->findById(tx.getAccountId());
        if (acc_opt.has_value()) {
            Account acc = acc_opt.value();
            if (tx.getType() == TransactionType::PURCHASE ||
                tx.getType() == TransactionType::WITHDRAWAL ||
                tx.getType() == TransactionType::TRANSFER ||
                tx.getType() == TransactionType::PAYMENT) {
                if (!acc.withdraw(tx.getAmount())) {
                    tx.setStatus(TransactionStatus::FAILED);
                    if (tx_repo_) tx_repo_->save(tx);
                    return TransactionServiceResult{
                        false,
                        tx,
                        ValidationResult::failure(ValidationErrorCode::INSUFFICIENT_BALANCE, "Withdrawal execution failed"),
                        "Insufficient account balance at execution"
                    };
                }
            } else if (tx.getType() == TransactionType::DEPOSIT ||
                       tx.getType() == TransactionType::REFUND) {
                acc.deposit(tx.getAmount());
            }
            account_repo_->save(acc);
        }
    }

    // 3. Mark approved (ready for downstream risk / settlement)
    tx.markApproved();

    // 4. Persistence
    if (tx_repo_) {
        tx_repo_->save(tx);
    }

    return TransactionServiceResult{
        true,
        tx,
        val_res,
        "Transaction processed and approved successfully"
    };
}

bool TransactionService::settleTransaction(const std::string& transaction_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!tx_repo_) return false;

    auto tx_opt = tx_repo_->findById(transaction_id);
    if (!tx_opt.has_value()) return false;

    Transaction tx = tx_opt.value();
    if (tx.markSettled()) {
        return tx_repo_->save(tx);
    }
    return false;
}

bool TransactionService::disputeTransaction(const std::string& transaction_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!tx_repo_) return false;

    auto tx_opt = tx_repo_->findById(transaction_id);
    if (!tx_opt.has_value()) return false;

    Transaction tx = tx_opt.value();
    if (tx.markDisputed()) {
        return tx_repo_->save(tx);
    }
    return false;
}

} // namespace epfd
