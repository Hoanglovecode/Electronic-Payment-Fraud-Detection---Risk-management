#ifndef EPFD_MODELS_CUSTOMER_HPP
#define EPFD_MODELS_CUSTOMER_HPP

#include <string>
#include <vector>
#include <chrono>
#include "epfd/common/Types.hpp"
#include "epfd/models/Location.hpp"

namespace epfd {

class Customer {
public:
    Customer() = default;
    Customer(std::string customer_id,
             std::string full_name,
             std::string email,
             std::string phone,
             Location home_location = Location{},
             bool is_kyc_verified = false,
             bool is_vip = false,
             Timestamp created_at = std::chrono::system_clock::now());

    // Getters
    const std::string& getCustomerId() const noexcept { return customer_id_; }
    const std::string& getFullName() const noexcept { return full_name_; }
    const std::string& getEmail() const noexcept { return email_; }
    const std::string& getPhone() const noexcept { return phone_; }
    const Location& getHomeLocation() const noexcept { return home_location_; }
    double getRiskScore() const noexcept { return risk_score_; }
    RiskLevel getRiskLevel() const noexcept { return risk_level_; }
    bool isKycVerified() const noexcept { return is_kyc_verified_; }
    bool isVip() const noexcept { return is_vip_; }
    bool isBlacklisted() const noexcept { return is_blacklisted_; }
    Timestamp getCreatedAt() const noexcept { return created_at_; }
    
    const std::vector<std::string>& getAccountIds() const noexcept { return account_ids_; }
    const std::vector<std::string>& getKnownDeviceIds() const noexcept { return known_device_ids_; }
    const std::vector<std::string>& getKnownPaymentMethodIds() const noexcept { return known_payment_method_ids_; }

    // Setters
    void setRiskScore(double score);
    void setRiskLevel(RiskLevel level) noexcept { risk_level_ = level; }
    void setIsKycVerified(bool val) noexcept { is_kyc_verified_ = val; }
    void setIsVip(bool val) noexcept { is_vip_ = val; }
    void setIsBlacklisted(bool val) noexcept { is_blacklisted_ = val; }
    void setHomeLocation(Location loc) { home_location_ = std::move(loc); }

    // Association management
    void addAccountId(std::string account_id);
    void addKnownDeviceId(std::string device_id);
    void addKnownPaymentMethodId(std::string payment_method_id);

    bool isKnownDevice(const std::string& device_id) const noexcept;
    bool isKnownPaymentMethod(const std::string& payment_method_id) const noexcept;

    std::string toString() const;

private:
    std::string customer_id_;
    std::string full_name_;
    std::string email_;
    std::string phone_;
    Location home_location_;
    double risk_score_{0.0};
    RiskLevel risk_level_{RiskLevel::VERY_LOW};
    bool is_kyc_verified_{false};
    bool is_vip_{false};
    bool is_blacklisted_{false};
    Timestamp created_at_{std::chrono::system_clock::now()};

    std::vector<std::string> account_ids_;
    std::vector<std::string> known_device_ids_;
    std::vector<std::string> known_payment_method_ids_;
};

} // namespace epfd

#endif // EPFD_MODELS_CUSTOMER_HPP
