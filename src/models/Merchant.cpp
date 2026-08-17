#include "epfd/models/Merchant.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace epfd {

Merchant::Merchant(std::string merchant_id,
                   std::string name,
                   std::string category_code,
                   std::string country,
                   double risk_rating,
                   bool is_blacklisted,
                   bool is_whitelisted)
    : merchant_id_(std::move(merchant_id)),
      name_(std::move(name)),
      category_code_(std::move(category_code)),
      country_(std::move(country)),
      is_blacklisted_(is_blacklisted),
      is_whitelisted_(is_whitelisted) {
    setRiskRating(risk_rating);
}

void Merchant::setRiskRating(double rating) {
    if (rating < 0.0 || rating > 1.0) {
        throw std::invalid_argument("Merchant risk rating must be between 0.0 and 1.0");
    }
    risk_rating_ = rating;
}

bool Merchant::isHighRiskMCC() const noexcept {
    // 7995: Gambling, 6051: Quasi-Cash/Crypto, 5993: Cigar Stores, 7273: Dating, 5967: Direct Marketing
    return (category_code_ == "7995" || 
            category_code_ == "6051" || 
            category_code_ == "5993" || 
            category_code_ == "7273" || 
            category_code_ == "5967");
}

std::string Merchant::toString() const {
    std::ostringstream oss;
    oss << "Merchant[id=" << merchant_id_ 
        << ", name=" << name_ 
        << ", mcc=" << category_code_ 
        << ", country=" << country_ 
        << ", risk=" << risk_rating_ 
        << ", blacklisted=" << (is_blacklisted_ ? "true" : "false")
        << "]";
    return oss.str();
}

} // namespace epfd
