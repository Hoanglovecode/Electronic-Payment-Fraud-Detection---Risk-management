#ifndef EPFD_MODELS_ACCOUNT_HPP
#define EPFD_MODELS_ACCOUNT_HPP

#include <string>
#include <chrono>
#include "epfd/common/Types.hpp"

namespace epfd {

class Account {
public:
    Account() = default;
    Account(std::string account_id,
            std::string customer_id,
            double initial_balance,
            std::string currency = "USD",
            Timestamp created_at = std::chrono::system_clock::now());

    // Getters
    const std::string& getAccountId() const noexcept { return account_id_; }
    const std::string& getCustomerId() const noexcept { return customer_id_; }
    double getBalance() const noexcept { return balance_; }
    const std::string& getCurrency() const noexcept { return currency_; }
    bool isFrozen() const noexcept { return is_frozen_; }
    bool isClosed() const noexcept { return is_closed_; }
    Timestamp getCreatedAt() const noexcept { return created_at_; }

    // Account Operations
    void deposit(double amount);
    bool withdraw(double amount);
    bool hasSufficientBalance(double amount) const noexcept;
    void freeze() noexcept { is_frozen_ = true; }
    void unfreeze() noexcept { is_frozen_ = false; }
    void close() noexcept { is_closed_ = true; }

    std::string toString() const;

private:
    std::string account_id_;
    std::string customer_id_;
    double balance_{0.0};
    std::string currency_{"USD"};
    bool is_frozen_{false};
    bool is_closed_{false};
    Timestamp created_at_{std::chrono::system_clock::now()};
};

} // namespace epfd

#endif // EPFD_MODELS_ACCOUNT_HPP
