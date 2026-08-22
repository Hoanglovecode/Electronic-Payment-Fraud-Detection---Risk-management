#ifndef EPFD_VALIDATION_VALIDATION_RESULT_HPP
#define EPFD_VALIDATION_VALIDATION_RESULT_HPP

#include <string>
#include <string_view>

namespace epfd {

enum class ValidationErrorCode {
    NONE = 0,
    MISSING_TRANSACTION_ID,
    MISSING_CUSTOMER_ID,
    MISSING_RECIPIENT_ID,
    MISSING_ACCOUNT_ID,
    MISSING_MERCHANT_ID,
    INVALID_AMOUNT,
    INVALID_CURRENCY,
    INVALID_TIMESTAMP,
    DUPLICATE_TRANSACTION,
    EXPIRED_PAYMENT_METHOD,
    INVALID_PAYMENT_DETAILS,
    ACCOUNT_NOT_FOUND,
    ACCOUNT_FROZEN,
    ACCOUNT_CLOSED,
    INSUFFICIENT_BALANCE
};

std::string_view toString(ValidationErrorCode code);
std::ostream& operator<<(std::ostream& os, ValidationErrorCode code);

struct ValidationResult {
    bool is_valid{true};
    ValidationErrorCode error_code{ValidationErrorCode::NONE};
    std::string error_message;

    static ValidationResult success() {
        return ValidationResult{true, ValidationErrorCode::NONE, ""};
    }

    static ValidationResult failure(ValidationErrorCode code, std::string message) {
        return ValidationResult{false, code, std::move(message)};
    }
};

} // namespace epfd

#endif // EPFD_VALIDATION_VALIDATION_RESULT_HPP
