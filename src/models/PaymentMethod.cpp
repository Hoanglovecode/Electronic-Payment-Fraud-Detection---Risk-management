/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Payment Method Implementation (Factory Pattern & Luhn Check)
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/models/PaymentMethod.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

namespace epfd {

PaymentMethod::PaymentMethod(string payment_id,
                             PaymentType type,
                             const string& raw_or_masked_pan,
                             string card_holder_name,
                             int expiry_month,
                             int expiry_year,
                             string billing_country)
    : payment_id_(move(payment_id)),
      type_(type),
      card_holder_name_(move(card_holder_name)),
      expiry_month_(expiry_month),
      expiry_year_(expiry_year),
      billing_country_(move(billing_country)) {
    
    // Làm sạch chuỗi số thẻ: chỉ giữ lại chữ số và dấu *
    string clean_pan;
    for (char c : raw_or_masked_pan) {
        if (isdigit(static_cast<unsigned char>(c)) || c == '*') {
            clean_pan.push_back(c);
        }
    }

    if (clean_pan.length() >= 10) {
        // Trích xuất BIN (6 chữ số đầu) để nhận diện ngân hàng phát hành
        card_bin_ = clean_pan.substr(0, 6);
        // Trích xuất 4 chữ số cuối (Last4)
        last4_ = clean_pan.substr(clean_pan.length() - 4);
        // Tự động mask các chữ số ở giữa theo chuẩn PCI-DSS
        masked_card_number_ = maskPan(clean_pan);
    } else {
        masked_card_number_ = raw_or_masked_pan;
        if (clean_pan.length() >= 4) {
            last4_ = clean_pan.substr(clean_pan.length() - 4);
        }
    }
}

// Factory Method: Tạo đối tượng thanh toán chuyển khoản ngân hàng
PaymentMethod PaymentMethod::createBankTransfer(string payment_id, string account_number, string bank_name) {
    PaymentMethod pm;
    pm.payment_id_ = move(payment_id);
    pm.type_ = PaymentType::BANK_TRANSFER;
    pm.card_holder_name_ = move(bank_name);
    pm.masked_card_number_ = "ACC-" + (account_number.length() > 4 ? account_number.substr(account_number.length() - 4) : account_number);
    pm.last4_ = account_number.length() >= 4 ? account_number.substr(account_number.length() - 4) : account_number;
    return pm;
}

// Factory Method: Tạo đối tượng ví điện tử (MoMo, ZaloPay, ApplePay...)
PaymentMethod PaymentMethod::createEWallet(string payment_id, string wallet_id, string provider) {
    PaymentMethod pm;
    pm.payment_id_ = move(payment_id);
    pm.type_ = PaymentType::E_WALLET;
    pm.card_holder_name_ = move(provider);
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

// Thuật toán kiểm tra số thẻ hợp lệ Luhn (Mod 10 Checksum)
bool PaymentMethod::validateLuhn(const string& raw_pan) noexcept {
    string digits;
    for (char c : raw_pan) {
        if (isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
        }
    }

    // Độ dài tiêu chuẩn thẻ tín dụng quốc tế (13-19 chữ số)
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

// Hàm mask số thẻ: Giữ 6 số đầu, 4 số cuối, thay thế các số ở giữa bằng dấu *
string PaymentMethod::maskPan(const string& raw_pan) {
    string digits;
    for (char c : raw_pan) {
        if (isdigit(static_cast<unsigned char>(c)) || c == '*') {
            digits.push_back(c);
        }
    }

    if (digits.length() < 10) {
        return digits;
    }

    string masked = digits;
    size_t prefix_len = 6;
    size_t suffix_len = 4;
    for (size_t i = prefix_len; i < digits.length() - suffix_len; ++i) {
        masked[i] = '*';
    }
    return masked;
}

string PaymentMethod::toString() const {
    ostringstream oss;
    oss << "PaymentMethod[id=" << payment_id_ 
        << ", type=" << epfd::toString(type_)
        << ", masked=" << masked_card_number_ 
        << ", holder=" << card_holder_name_
        << ", exp=" << expiry_month_ << "/" << expiry_year_
        << "]";
    return oss.str();
}

} // namespace epfd
