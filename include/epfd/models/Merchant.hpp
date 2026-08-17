#ifndef EPFD_MODELS_MERCHANT_HPP
#define EPFD_MODELS_MERCHANT_HPP

#include <string>

namespace epfd {

class Merchant {
public:
    Merchant() = default;
    Merchant(std::string merchant_id,
             std::string name,
             std::string category_code,
             std::string country,
             double risk_rating = 0.0,
             bool is_blacklisted = false,
             bool is_whitelisted = false);

    // Getters
    const std::string& getMerchantId() const noexcept { return merchant_id_; }
    const std::string& getName() const noexcept { return name_; }
    const std::string& getCategoryCode() const noexcept { return category_code_; } // MCC
    const std::string& getCountry() const noexcept { return country_; }
    double getRiskRating() const noexcept { return risk_rating_; }
    bool isBlacklisted() const noexcept { return is_blacklisted_; }
    bool isWhitelisted() const noexcept { return is_whitelisted_; }

    // Setters
    void setMerchantId(std::string id) { merchant_id_ = std::move(id); }
    void setName(std::string name) { name_ = std::move(name); }
    void setCategoryCode(std::string mcc) { category_code_ = std::move(mcc); }
    void setCountry(std::string country) { country_ = std::move(country); }
    void setRiskRating(double rating);
    void setIsBlacklisted(bool val) noexcept { is_blacklisted_ = val; }
    void setIsWhitelisted(bool val) noexcept { is_whitelisted_ = val; }

    // Domain helpers
    bool isHighRiskMCC() const noexcept;
    std::string toString() const;

private:
    std::string merchant_id_;
    std::string name_;
    std::string category_code_; // MCC e.g. "7995", "6051"
    std::string country_;
    double risk_rating_{0.0}; // [0.0, 1.0]
    bool is_blacklisted_{false};
    bool is_whitelisted_{false};
};

} // namespace epfd

#endif // EPFD_MODELS_MERCHANT_HPP
