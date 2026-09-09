/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Customer Implementation
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/models/Customer.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace epfd {

Customer::Customer(string customer_id,
                   string full_name,
                   string email,
                   string phone,
                   Location home_location,
                   bool is_kyc_verified,
                   bool is_vip,
                   Timestamp created_at)
    : customer_id_(move(customer_id)),
      full_name_(move(full_name)),
      email_(move(email)),
      phone_(move(phone)),
      home_location_(move(home_location)),
      is_kyc_verified_(is_kyc_verified),
      is_vip_(is_vip),
      created_at_(created_at) {}

// Business Logic: Đồng bộ điểm rủi ro và RiskLevel tương ứng
void Customer::setRiskScore(double score) {
    if (score < 0.0 || score > 100.0) {
        throw invalid_argument("Customer risk score must be between 0.0 and 100.0");
    }
    risk_score_ = score;
    if (score < 20.0) {
        risk_level_ = RiskLevel::VERY_LOW;
    } else if (score < 40.0) {
        risk_level_ = RiskLevel::LOW;
    } else if (score < 60.0) {
        risk_level_ = RiskLevel::MEDIUM;
    } else if (score < 80.0) {
        risk_level_ = RiskLevel::HIGH;
    } else {
        risk_level_ = RiskLevel::CRITICAL;
    }
}

// Composition management: Quản lý các tài khoản ngân hàng của khách
void Customer::addAccountId(string account_id) {
    if (find(account_ids_.begin(), account_ids_.end(), account_id) == account_ids_.end()) {
        account_ids_.push_back(move(account_id));
    }
}

// Aggregation management: Liên kết thiết bị và thẻ thanh toán
void Customer::addKnownDeviceId(string device_id) {
    if (find(known_device_ids_.begin(), known_device_ids_.end(), device_id) == known_device_ids_.end()) {
        known_device_ids_.push_back(move(device_id));
    }
}

void Customer::addKnownPaymentMethodId(string payment_method_id) {
    if (find(known_payment_method_ids_.begin(), known_payment_method_ids_.end(), payment_method_id) == known_payment_method_ids_.end()) {
        known_payment_method_ids_.push_back(move(payment_method_id));
    }
}

bool Customer::isKnownDevice(const string& device_id) const noexcept {
    return find(known_device_ids_.begin(), known_device_ids_.end(), device_id) != known_device_ids_.end();
}

bool Customer::isKnownPaymentMethod(const string& payment_method_id) const noexcept {
    return find(known_payment_method_ids_.begin(), known_payment_method_ids_.end(), payment_method_id) != known_payment_method_ids_.end();
}

string Customer::toString() const {
    ostringstream oss;
    oss << "Customer[id=" << customer_id_
        << ", name=" << full_name_
        << ", email=" << email_
        << ", score=" << risk_score_
        << ", level=" << epfd::toString(risk_level_)
        << ", kyc=" << (is_kyc_verified_ ? "true" : "false")
        << ", vip=" << (is_vip_ ? "true" : "false")
        << "]";
    return oss.str();
}

} // namespace epfd
