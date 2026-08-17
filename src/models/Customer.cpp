#include "epfd/models/Customer.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace epfd {

Customer::Customer(std::string customer_id,
                   std::string full_name,
                   std::string email,
                   std::string phone,
                   Location home_location,
                   bool is_kyc_verified,
                   bool is_vip,
                   Timestamp created_at)
    : customer_id_(std::move(customer_id)),
      full_name_(std::move(full_name)),
      email_(std::move(email)),
      phone_(std::move(phone)),
      home_location_(std::move(home_location)),
      is_kyc_verified_(is_kyc_verified),
      is_vip_(is_vip),
      created_at_(created_at) {}

void Customer::setRiskScore(double score) {
    if (score < 0.0 || score > 100.0) {
        throw std::invalid_argument("Customer risk score must be between 0.0 and 100.0");
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

void Customer::addAccountId(std::string account_id) {
    if (std::find(account_ids_.begin(), account_ids_.end(), account_id) == account_ids_.end()) {
        account_ids_.push_back(std::move(account_id));
    }
}

void Customer::addKnownDeviceId(std::string device_id) {
    if (std::find(known_device_ids_.begin(), known_device_ids_.end(), device_id) == known_device_ids_.end()) {
        known_device_ids_.push_back(std::move(device_id));
    }
}

void Customer::addKnownPaymentMethodId(std::string payment_method_id) {
    if (std::find(known_payment_method_ids_.begin(), known_payment_method_ids_.end(), payment_method_id) == known_payment_method_ids_.end()) {
        known_payment_method_ids_.push_back(std::move(payment_method_id));
    }
}

bool Customer::isKnownDevice(const std::string& device_id) const noexcept {
    return std::find(known_device_ids_.begin(), known_device_ids_.end(), device_id) != known_device_ids_.end();
}

bool Customer::isKnownPaymentMethod(const std::string& payment_method_id) const noexcept {
    return std::find(known_payment_method_ids_.begin(), known_payment_method_ids_.end(), payment_method_id) != known_payment_method_ids_.end();
}

std::string Customer::toString() const {
    std::ostringstream oss;
    oss << "Customer[id=" << customer_id_ 
        << ", name=" << full_name_ 
        << ", risk=" << risk_score_ 
        << " (" << epfd::toString(risk_level_) << ")"
        << ", kyc=" << (is_kyc_verified_ ? "true" : "false")
        << ", vip=" << (is_vip_ ? "true" : "false")
        << "]";
    return oss.str();
}

} // namespace epfd
