#include "epfd/utils/SecurityUtils.hpp"
#include <algorithm>
#include <cctype>

namespace epfd {

std::string SecurityUtils::maskPan(const std::string& pan) {
    std::string digits;
    digits.reserve(pan.size());
    for (char c : pan) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
        }
    }

    if (digits.length() < 4) {
        return "****";
    }

    std::string last4 = digits.substr(digits.length() - 4);
    return "**** **** **** " + last4;
}

std::string SecurityUtils::maskPanBinAndLast4(const std::string& pan) {
    std::string digits;
    digits.reserve(pan.size());
    for (char c : pan) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
        }
    }

    if (digits.length() < 10) {
        return maskPan(pan);
    }

    std::string bin = digits.substr(0, 6);
    std::string last4 = digits.substr(digits.length() - 4);
    return bin.substr(0, 4) + " " + bin.substr(4, 2) + "** **** " + last4;
}

std::string SecurityUtils::redactCvv(const std::string& text) {
    // Redact CVV patterns (e.g., CVV: 123, CVC=456)
    static const std::regex cvv_regex(R"((cvv|cvc|security_code)\s*[:=]\s*\d{3,4})", std::regex::icase);
    return std::regex_replace(text, cvv_regex, "$1: [REDACTED]");
}

std::string SecurityUtils::maskEmail(const std::string& email) {
    auto at_pos = email.find('@');
    if (at_pos == std::string::npos || at_pos < 2) {
        return "***@***";
    }

    std::string user = email.substr(0, at_pos);
    std::string domain = email.substr(at_pos);

    if (user.length() <= 2) {
        return user.front() + std::string("***") + domain;
    }

    return user.front() + std::string("***") + user.back() + domain;
}

std::string SecurityUtils::maskIp(const std::string& ip) {
    auto first_dot = ip.find('.');
    if (first_dot == std::string::npos) return "*.*.*.*";
    auto second_dot = ip.find('.', first_dot + 1);
    if (second_dot == std::string::npos) return "*.*.*.*";

    return ip.substr(0, second_dot) + ".*.*";
}

std::string SecurityUtils::sanitizeLogPayload(const std::string& payload) {
    // 1. Redact CVVs first
    std::string sanitized = redactCvv(payload);

    // 2. Redact 13-19 digit card numbers
    static const std::regex pan_regex(R"(\b\d{4}[ -]?\d{4}[ -]?\d{4}[ -]?\d{1,7}\b)");
    sanitized = std::regex_replace(sanitized, pan_regex, "****-****-****-XXXX");

    return sanitized;
}

} // namespace epfd
