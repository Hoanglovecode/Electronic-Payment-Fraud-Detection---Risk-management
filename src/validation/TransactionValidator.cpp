#include "epfd/validation/TransactionValidator.hpp"
#include "epfd/database/ITransactionRepository.hpp"
#include "epfd/database/IAccountRepository.hpp"
#include <cmath>
#include <ctime>
#include <chrono>

namespace epfd {

std::string_view toString(ValidationErrorCode code) {
    switch (code) {
        case ValidationErrorCode::NONE:                     return "NONE";
        case ValidationErrorCode::MISSING_TRANSACTION_ID:   return "MISSING_TRANSACTION_ID";
        case ValidationErrorCode::MISSING_CUSTOMER_ID:      return "MISSING_CUSTOMER_ID";
        case ValidationErrorCode::MISSING_RECIPIENT_ID:     return "MISSING_RECIPIENT_ID";
        case ValidationErrorCode::MISSING_ACCOUNT_ID:       return "MISSING_ACCOUNT_ID";
        case ValidationErrorCode::MISSING_MERCHANT_ID:      return "MISSING_MERCHANT_ID";
        case ValidationErrorCode::INVALID_AMOUNT:           return "INVALID_AMOUNT";
        case ValidationErrorCode::INVALID_CURRENCY:         return "INVALID_CURRENCY";
        case ValidationErrorCode::INVALID_TIMESTAMP:        return "INVALID_TIMESTAMP";
        case ValidationErrorCode::DUPLICATE_TRANSACTION:    return "DUPLICATE_TRANSACTION";
        case ValidationErrorCode::EXPIRED_PAYMENT_METHOD:   return "EXPIRED_PAYMENT_METHOD";
        case ValidationErrorCode::INVALID_PAYMENT_DETAILS:  return "INVALID_PAYMENT_DETAILS";
        case ValidationErrorCode::ACCOUNT_NOT_FOUND:        return "ACCOUNT_NOT_FOUND";
        case ValidationErrorCode::ACCOUNT_FROZEN:           return "ACCOUNT_FROZEN";
        case ValidationErrorCode::ACCOUNT_CLOSED:           return "ACCOUNT_CLOSED";
        case ValidationErrorCode::INSUFFICIENT_BALANCE:     return "INSUFFICIENT_BALANCE";
    }
    return "UNKNOWN_ERROR";
}

std::ostream& operator<<(std::ostream& os, ValidationErrorCode code) {
    return os << toString(code);
}

TransactionValidator::TransactionValidator(std::shared_ptr<ITransactionRepository> tx_repo,
                                           std::shared_ptr<IAccountRepository> account_repo)
    : tx_repo_(std::move(tx_repo)), account_repo_(std::move(account_repo)) {}

ValidationResult TransactionValidator::validate(const Transaction& tx) const {
    // 1. Required Identifiers Check
    if (tx.getTransactionId().empty()) {
        return ValidationResult::failure(ValidationErrorCode::MISSING_TRANSACTION_ID, "Transaction ID cannot be empty");
    }
    if (tx.getCustomerId().empty()) {
        return ValidationResult::failure(ValidationErrorCode::MISSING_CUSTOMER_ID, "Customer ID cannot be empty");
    }
    if (tx.getRecipientId().empty()) {
        return ValidationResult::failure(ValidationErrorCode::MISSING_RECIPIENT_ID, "Recipient ID cannot be empty");
    }
    if (tx.getAccountId().empty()) {
        return ValidationResult::failure(ValidationErrorCode::MISSING_ACCOUNT_ID, "Account ID cannot be empty");
    }
    if (tx.getMerchantId().empty()) {
        return ValidationResult::failure(ValidationErrorCode::MISSING_MERCHANT_ID, "Merchant ID cannot be empty");
    }

    // 2. Amount Validation
    double amount = tx.getAmount();
    if (std::isnan(amount) || std::isinf(amount) || amount <= 0.0) {
        return ValidationResult::failure(ValidationErrorCode::INVALID_AMOUNT, "Transaction amount must be positive and non-zero");
    }
    if (amount > 100000000.0) { // Limit: 100 million
        return ValidationResult::failure(ValidationErrorCode::INVALID_AMOUNT, "Transaction amount exceeds system maximum threshold ($100,000,000)");
    }

    // 3. Currency Validation
    if (tx.getCurrency().length() != 3) {
        return ValidationResult::failure(ValidationErrorCode::INVALID_CURRENCY, "Currency code must be a 3-letter ISO code (e.g. USD, EUR, VND)");
    }

    // 4. Timestamp Validation (within [-30 days, +10 minutes])
    auto now = std::chrono::system_clock::now();
    auto tx_time = tx.getTimestamp();

    if (tx_time > now + std::chrono::minutes(10)) {
        return ValidationResult::failure(ValidationErrorCode::INVALID_TIMESTAMP, "Transaction timestamp is in the future (> 10 minutes)");
    }
    if (tx_time < now - std::chrono::hours(24 * 30)) {
        return ValidationResult::failure(ValidationErrorCode::INVALID_TIMESTAMP, "Transaction timestamp is too old (> 30 days)");
    }

    // 5. Payment Method & Expiration Check
    const auto& pm = tx.getPaymentMethod();
    if (pm.isCard()) {
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_buf, &t);
#else
        localtime_r(&t, &tm_buf);
#endif
        int current_year = tm_buf.tm_year + 1900;
        int current_month = tm_buf.tm_mon + 1;

        if (pm.isExpired(current_year, current_month)) {
            return ValidationResult::failure(ValidationErrorCode::EXPIRED_PAYMENT_METHOD, "Payment card is expired");
        }
    }

    // 6. Duplicate / Idempotency Check
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (seen_transaction_ids_.contains(tx.getTransactionId())) {
            return ValidationResult::failure(ValidationErrorCode::DUPLICATE_TRANSACTION, "Duplicate transaction ID detected in validator cache");
        }
    }
    if (tx_repo_ && tx_repo_->findById(tx.getTransactionId()).has_value()) {
        return ValidationResult::failure(ValidationErrorCode::DUPLICATE_TRANSACTION, "Duplicate transaction ID found in repository");
    }

    // 7. Account Status & Balance Check
    if (account_repo_) {
        auto acc_opt = account_repo_->findById(tx.getAccountId());
        if (!acc_opt.has_value()) {
            return ValidationResult::failure(ValidationErrorCode::ACCOUNT_NOT_FOUND, "Account " + tx.getAccountId() + " not found");
        }

        const auto& acc = acc_opt.value();
        if (acc.isClosed()) {
            return ValidationResult::failure(ValidationErrorCode::ACCOUNT_CLOSED, "Account " + tx.getAccountId() + " is closed");
        }
        if (acc.isFrozen()) {
            return ValidationResult::failure(ValidationErrorCode::ACCOUNT_FROZEN, "Account " + tx.getAccountId() + " is frozen");
        }

        // Check balance for debit operations
        if (tx.getType() == TransactionType::PURCHASE || 
            tx.getType() == TransactionType::WITHDRAWAL || 
            tx.getType() == TransactionType::TRANSFER || 
            tx.getType() == TransactionType::PAYMENT) {
            if (!acc.hasSufficientBalance(tx.getAmount())) {
                return ValidationResult::failure(ValidationErrorCode::INSUFFICIENT_BALANCE, 
                    "Insufficient account balance (Available: " + std::to_string(acc.getBalance()) + ", Required: " + std::to_string(tx.getAmount()) + ")");
            }
        }
    }

    // Record as seen
    {
        std::lock_guard<std::mutex> lock(mutex_);
        seen_transaction_ids_.insert(tx.getTransactionId());
    }

    return ValidationResult::success();
}

void TransactionValidator::clearSeenTransactions() {
    std::lock_guard<std::mutex> lock(mutex_);
    seen_transaction_ids_.clear();
}

} // namespace epfd
