#ifndef EPFD_UTILS_SECURITY_UTILS_HPP
#define EPFD_UTILS_SECURITY_UTILS_HPP

#include <string>
#include <string_view>
#include <regex>

namespace epfd {

/**
 * @brief PCI-DSS compliant Data Masking and Security Utilities.
 * Enforces masking of sensitive payment data (PAN, CVV, PII) across logs, repositories, and UI.
 */
class SecurityUtils {
public:
    /**
     * @brief Masks a Primary Account Number (PAN), revealing only the last 4 digits.
     * Example: "4111111111111234" -> "**** **** **** 1234"
     */
    static std::string maskPan(const std::string& pan);

    /**
     * @brief Masks a Primary Account Number with first 6 (BIN) and last 4 digits.
     * Example: "4111111111111234" -> "4111 11** **** 1234"
     */
    static std::string maskPanBinAndLast4(const std::string& pan);

    /**
     * @brief Strips/redacts any 3 or 4 digit CVV/CVC security code from text.
     */
    static std::string redactCvv(const std::string& text);

    /**
     * @brief Masks email address for PII privacy.
     * Example: "nguyen.van.a@bank.com" -> "n***a@bank.com"
     */
    static std::string maskEmail(const std::string& email);

    /**
     * @brief Masks IP address (anonymizes last octets).
     * Example: "192.168.1.105" -> "192.168.*.*"
     */
    static std::string maskIp(const std::string& ip);

    /**
     * @brief Sanitizes any general log payload by redacting PANs, CVVs, and sensitive tokens.
     */
    static std::string sanitizeLogPayload(const std::string& payload);
};

} // namespace epfd

#endif // EPFD_UTILS_SECURITY_UTILS_HPP
