#ifndef EPFD_COMMON_TYPES_HPP
#define EPFD_COMMON_TYPES_HPP

#include <string>
#include <cstdint>
#include <chrono>
#include <string_view>

namespace epfd {

// ==========================================
// 1. Transaction Types & Status
// ==========================================

enum class TransactionType {
    PURCHASE,
    TRANSFER,
    WITHDRAWAL,
    DEPOSIT,
    REFUND,
    PAYMENT
};

enum class TransactionStatus {
    PENDING,
    APPROVED,
    REVIEW,
    CHALLENGED,
    REJECTED,
    FAILED,
    SETTLED,
    CHARGEBACK,
    DISPUTED
};

enum class PaymentType {
    CREDIT_CARD,
    DEBIT_CARD,
    BANK_TRANSFER,
    E_WALLET,
    CRYPTO
};

// ==========================================
// 2. Risk & Decision Enums
// ==========================================

enum class RiskLevel {
    VERY_LOW,   // [0, 20)
    LOW,        // [20, 40)
    MEDIUM,     // [40, 60)
    HIGH,       // [60, 80)
    CRITICAL    // [80, 100]
};

enum class DecisionAction {
    APPROVE,
    REVIEW,
    CHALLENGE_3DS,
    BLOCK
};

enum class FraudRuleCategory {
    VELOCITY,
    AMOUNT_DEVIATION,
    GEO_LOCATION,
    DEVICE_INTEGRITY,
    CARD_TESTING,
    ACCOUNT_TAKEOVER,
    LIST_MATCHING,
    BEHAVIORAL
};

// ==========================================
// 3. Case Management & Feedback Enums
// ==========================================

enum class CaseStatus {
    OPEN,
    IN_INVESTIGATION,
    RESOLVED_CONFIRMED_FRAUD,
    RESOLVED_FALSE_POSITIVE,
    CLOSED
};

enum class GroundTruthLabel {
    LEGITIMATE = 0,
    FRAUD = 1,
    UNKNOWN = -1
};

// ==========================================
// 4. Utility Timestamp Definition
// ==========================================

using Timestamp = std::chrono::system_clock::time_point;

// ==========================================
// 5. String Helper Conversions & Stream Operators
// ==========================================

std::string_view toString(TransactionType type);
std::string_view toString(TransactionStatus status);
std::string_view toString(PaymentType type);
std::string_view toString(RiskLevel level);
std::string_view toString(DecisionAction action);
std::string_view toString(FraudRuleCategory category);
std::string_view toString(CaseStatus status);
std::string_view toString(GroundTruthLabel label);

std::ostream& operator<<(std::ostream& os, TransactionType type);
std::ostream& operator<<(std::ostream& os, TransactionStatus status);
std::ostream& operator<<(std::ostream& os, PaymentType type);
std::ostream& operator<<(std::ostream& os, RiskLevel level);
std::ostream& operator<<(std::ostream& os, DecisionAction action);
std::ostream& operator<<(std::ostream& os, FraudRuleCategory category);
std::ostream& operator<<(std::ostream& os, CaseStatus status);
std::ostream& operator<<(std::ostream& os, GroundTruthLabel label);

} // namespace epfd

#endif // EPFD_COMMON_TYPES_HPP
