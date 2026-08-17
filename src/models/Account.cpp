#include "epfd/models/Account.hpp"
#include <sstream>
#include <stdexcept>

namespace epfd {

Account::Account(std::string account_id,
                 std::string customer_id,
                 double initial_balance,
                 std::string currency,
                 Timestamp created_at)
    : account_id_(std::move(account_id)),
      customer_id_(std::move(customer_id)),
      balance_(initial_balance),
      currency_(std::move(currency)),
      created_at_(created_at) {
    if (balance_ < 0.0) {
        throw std::invalid_argument("Initial account balance cannot be negative");
    }
}

void Account::deposit(double amount) {
    if (amount <= 0.0) {
        throw std::invalid_argument("Deposit amount must be strictly positive");
    }
    if (is_frozen_ || is_closed_) {
        throw std::runtime_error("Cannot deposit to a frozen or closed account");
    }
    balance_ += amount;
}

bool Account::withdraw(double amount) {
    if (amount <= 0.0) {
        throw std::invalid_argument("Withdrawal amount must be strictly positive");
    }
    if (is_frozen_ || is_closed_) {
        return false;
    }
    if (balance_ < amount) {
        return false;
    }
    balance_ -= amount;
    return true;
}

bool Account::hasSufficientBalance(double amount) const noexcept {
    if (amount <= 0.0 || is_frozen_ || is_closed_) {
        return false;
    }
    return balance_ >= amount;
}

std::string Account::toString() const {
    std::ostringstream oss;
    oss << "Account[id=" << account_id_ 
        << ", customer=" << customer_id_ 
        << ", balance=" << balance_ << " " << currency_ 
        << ", frozen=" << (is_frozen_ ? "true" : "false")
        << ", closed=" << (is_closed_ ? "true" : "false")
        << "]";
    return oss.str();
}

} // namespace epfd
