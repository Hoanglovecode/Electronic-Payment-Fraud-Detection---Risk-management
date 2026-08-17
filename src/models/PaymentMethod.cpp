#include "epfd/models/PaymentMethod.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace epfd {

PaymentMethod::PaymentMethod(std::string payment_id,
                             PaymentType type,
                             const std::string& raw_or_masked_pan,
                             std::string card_holder_name,
                             int expiry_month,
                             int expiry_year,
                             std::string billing_country)
    : payment_id_(std::move(payment_id)),
      type_(type),
      card_holder_name_(std::move(card_holder_name)),
      expiry_month_(expiry_month),
      expiry_year_(expiry_year),
      billing_country_(std::move(billing_country)) {
    
    std::string clean_pan;
    for (char c : raw_or_masked_pan) {
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '*') {
            clean_pan.push_back(c);
        }
    }

    if (clean_pan.length() >= 10) {
        // Extract BIN (first 6 digits)
        card_bin_ = clean_pan.substr(0, 6);
        // Extract Last 4
        last4_ = clean_pan.substr(clean_pan.length() - 4);
        // Mask the PAN
        masked_card_number_ = maskPan(clean_pan);
    } else {
        masked_card_number_ = raw_or_masked_pan;
        if (clean_pan.length() >= 4) {
            last4_ = clean_pan.substr(clean_pan.length() - 4);
        }
    }
}

PaymentMethod PaymentMethod::createBankTransfer(std::string payment_id, std::string account_number, std::string bank_name) {
    PaymentMethod pm;
    pm.payment_id_ = std::move(payment_id);
    pm.type_ = PaymentType::BANK_TRANSFER;
    pm.card_holder_name_ = std::move(bank_name);
    pm.masked_card_number_ = "ACC-" + (account_number.length() > 4 ? account_number.substr(account_number.length() - 4) : account_number);
    pm.last4_ = account_number.length() >= 4 ? account_number.substr(account_number.length() - 4) : account_number;
    return pm;
}

PaymentMethod PaymentMethod::createEWallet(std::string payment_id, std::string wallet_id, std::string provider) {
    PaymentMethod pm;
    pm.payment_id_ = std::move(payment_id);
    pm.type_ = PaymentType::E_WALLET;
    pm.card_holder_name_ = std::move(provider);
    pm.masked_card_number_ = "WALLET-" + (wallet_id.length() > 4 ? wallet_id.substr(wallet_id.length() - 4) : wallet_id);
    pm.last4_ = wallet_id.length() >= 4 ? wallet_id.substr(wallet_id.length() - 4) : wallet_id;
    return pm;
}

bool PaymentMethod::isCard() const noexcept {
    return type_ == PaymentType::CREDIT_CARD || type_ == PaymentType::DEBIT_CARD;
}

bool PaymentMethod::isExpired(int current_year, int current_month) const noexcept {
    if (!isCard() || expiry_year_ <= 0) {
        return false;
    }
    if (expiry_year_ < current_year) {
        return true;
    }
    if (expiry_year_ == current_year && expiry_month_ < current_month) {
        return true;
    }
    return false;
}

bool PaymentMethod::validateLuhn(const std::string& raw_pan) noexcept {
    std::string digits;
    for (char c : raw_pan) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
        }
    }

    if (digits.length() < 13 || digits.length() > 19) {
        return false;
    }

    int sum = 0;
    bool alternate = false;
    for (int i = static_cast<int>(digits.length()) - 1; i >= 0; --i) {
        int n = digits[i] - '0';
        if (alternate) {
            n *= 2;
            if (n > 9) {
                n = (n % 10) + 1;
            }
        }
        sum += n;
        alternate = !alternate;
    }

    return (sum % 10 == 0);
}

std::string PaymentMethod::maskPan(const std::string& raw_pan) {
    std::string digits;
    for (char c : raw_pan) {
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '*') {
            digits.push_back(c);
        }
    }

    if (digits.length() < 10) {
        return digits;
    }

    std::string masked = digits;
    size_t prefix_len = 6;
    size_t suffix_len = 4;
    for (size_t i = prefix_len; i < digits.length() - suffix_len; ++i) {
        masked[i] = '*';
    }
    return masked;
}

std::string PaymentMethod::toString() const {
    std::ostringstream oss;
    oss << "PaymentMethod[id=" << payment_id_ 
        << ", type=" << epfd::toString(type_)
        << ", masked=" << masked_card_number_ 
        << ", holder=" << card_holder_name_
        << ", exp=" << expiry_month_ << "/" << expiry_year_
        << "]";
    return oss.str();
}

} // namespace epfd
