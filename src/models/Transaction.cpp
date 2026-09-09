/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Transaction Implementation
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/models/Transaction.hpp"
#include <sstream>
#include <stdexcept>

using namespace std;

namespace epfd {

Transaction::Transaction(string transaction_id,
                         TransactionType type,
                         string customer_id,
                         string recipient_id,
                         string account_id,
                         double amount,
                         string currency,
                         Timestamp timestamp,
                         Location location,
                         string ip_address,
                         Device device,
                         string merchant_id,
                         PaymentMethod payment_method)
    : transaction_id_(move(transaction_id)),
      type_(type),
      customer_id_(move(customer_id)),
      recipient_id_(move(recipient_id)),
      account_id_(move(account_id)),
      currency_(move(currency)),
      timestamp_(timestamp),
      location_(move(location)),
      ip_address_(move(ip_address)),
      device_(move(device)),
      merchant_id_(move(merchant_id)),
      payment_method_(move(payment_method)),
      status_(TransactionStatus::PENDING),
      created_at_(chrono::system_clock::now()),
      updated_at_(chrono::system_clock::now()) {
    // Encapsulation validation: ensure amount is strictly valid
    setAmount(amount);
}

void Transaction::setAmount(double amount) {
    if (amount <= 0.0) {
        throw invalid_argument("Transaction amount must be strictly positive");
    }
    amount_ = amount;
    updated_at_ = chrono::system_clock::now();
}

void Transaction::setStatus(TransactionStatus new_status) {
    status_ = new_status;
    updated_at_ = chrono::system_clock::now();
}

// State transition methods (Finite State Machine pattern)
bool Transaction::markApproved() {
    if (status_ == TransactionStatus::PENDING || status_ == TransactionStatus::REVIEW || status_ == TransactionStatus::CHALLENGED) {
        setStatus(TransactionStatus::APPROVED);
        return true;
    }
    return false;
}

bool Transaction::markReview() {
    if (status_ == TransactionStatus::PENDING) {
        setStatus(TransactionStatus::REVIEW);
        return true;
    }
    return false;
}

bool Transaction::markChallenged() {
    if (status_ == TransactionStatus::PENDING) {
        setStatus(TransactionStatus::CHALLENGED);
        return true;
    }
    return false;
}

bool Transaction::markRejected() {
    if (status_ == TransactionStatus::PENDING || status_ == TransactionStatus::REVIEW || status_ == TransactionStatus::CHALLENGED) {
        setStatus(TransactionStatus::REJECTED);
        return true;
    }
    return false;
}

bool Transaction::markSettled() {
    if (status_ == TransactionStatus::APPROVED) {
        setStatus(TransactionStatus::SETTLED);
        return true;
    }
    return false;
}

bool Transaction::markChargeback() {
    if (status_ == TransactionStatus::SETTLED || status_ == TransactionStatus::APPROVED) {
        setStatus(TransactionStatus::CHARGEBACK);
        return true;
    }
    return false;
}

bool Transaction::markDisputed() {
    if (status_ == TransactionStatus::SETTLED || status_ == TransactionStatus::APPROVED) {
        setStatus(TransactionStatus::DISPUTED);
        return true;
    }
    return false;
}

bool Transaction::isCrossBorder(const string& home_country) const noexcept {
    if (home_country.empty() || location_.getCountry().empty()) {
        return false;
    }
    return location_.getCountry() != home_country;
}

bool Transaction::isHighAmount(double threshold) const noexcept {
    return amount_ >= threshold;
}

string Transaction::toString() const {
    ostringstream oss;
    oss << "Transaction[id=" << transaction_id_
        << ", type=" << epfd::toString(type_)
        << ", sender=" << customer_id_
        << ", recipient=" << recipient_id_
        << ", amount=" << amount_ << " " << currency_
        << ", status=" << epfd::toString(status_)
        << ", location=" << location_.toString()
        << ", merchant=" << merchant_id_
        << "]";
    return oss.str();
}

} // namespace epfd
