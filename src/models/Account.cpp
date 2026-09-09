/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Bank Account Implementation
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/models/Account.hpp"
#include <sstream>
#include <stdexcept>

using namespace std;

namespace epfd {

Account::Account(string account_id,
                 string customer_id,
                 double initial_balance,
                 string currency,
                 Timestamp created_at)
    : account_id_(move(account_id)),
      customer_id_(move(customer_id)),
      balance_(initial_balance),
      currency_(move(currency)),
      created_at_(created_at) {
    if (balance_ < 0.0) {
        throw invalid_argument("Initial account balance cannot be negative");
    }
}

void Account::deposit(double amount) {
    if (amount <= 0.0) {
        throw invalid_argument("Deposit amount must be strictly positive");
    }
    if (is_frozen_ || is_closed_) {
        throw runtime_error("Cannot deposit to a frozen or closed account");
    }
    balance_ += amount;
}

bool Account::withdraw(double amount) {
    if (amount <= 0.0) {
        throw invalid_argument("Withdrawal amount must be strictly positive");
    }
    // Không thể rút tiền nếu tài khoản bị đóng hoặc đóng băng
    if (is_frozen_ || is_closed_) {
        return false;
    }
    // Đảm bảo số dư khả dụng
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

string Account::toString() const {
    ostringstream oss;
    oss << "Account[id=" << account_id_ 
        << ", customer=" << customer_id_ 
        << ", balance=" << balance_ << " " << currency_ 
        << ", frozen=" << (is_frozen_ ? "true" : "false")
        << ", closed=" << (is_closed_ ? "true" : "false")
        << "]";
    return oss.str();
}

} // namespace epfd
