#include "epfd/common/Types.hpp"

namespace epfd {

std::string_view toString(TransactionType type) {
    switch (type) {
        case TransactionType::PURCHASE:   return "PURCHASE";
        case TransactionType::TRANSFER:   return "TRANSFER";
        case TransactionType::WITHDRAWAL: return "WITHDRAWAL";
        case TransactionType::DEPOSIT:    return "DEPOSIT";
        case TransactionType::REFUND:     return "REFUND";
        case TransactionType::PAYMENT:    return "PAYMENT";
    }
    return "UNKNOWN";
}

std::string_view toString(TransactionStatus status) {
    switch (status) {
        case TransactionStatus::PENDING:    return "PENDING";
        case TransactionStatus::APPROVED:   return "APPROVED";
        case TransactionStatus::REVIEW:     return "REVIEW";
        case TransactionStatus::CHALLENGED: return "CHALLENGED";
        case TransactionStatus::REJECTED:   return "REJECTED";
        case TransactionStatus::FAILED:     return "FAILED";
        case TransactionStatus::SETTLED:    return "SETTLED";
        case TransactionStatus::CHARGEBACK: return "CHARGEBACK";
        case TransactionStatus::DISPUTED:   return "DISPUTED";
    }
    return "UNKNOWN";
}

std::string_view toString(PaymentType type) {
    switch (type) {
        case PaymentType::CREDIT_CARD:   return "CREDIT_CARD";
        case PaymentType::DEBIT_CARD:    return "DEBIT_CARD";
        case PaymentType::BANK_TRANSFER: return "BANK_TRANSFER";
        case PaymentType::E_WALLET:      return "E_WALLET";
        case PaymentType::CRYPTO:        return "CRYPTO";
    }
    return "UNKNOWN";
}

std::string_view toString(RiskLevel level) {
    switch (level) {
        case RiskLevel::VERY_LOW: return "VERY_LOW";
        case RiskLevel::LOW:      return "LOW";
        case RiskLevel::MEDIUM:   return "MEDIUM";
        case RiskLevel::HIGH:     return "HIGH";
        case RiskLevel::CRITICAL: return "CRITICAL";
    }
    return "UNKNOWN";
}

std::string_view toString(DecisionAction action) {
    switch (action) {
        case DecisionAction::APPROVE:       return "APPROVE";
        case DecisionAction::REVIEW:        return "REVIEW";
        case DecisionAction::CHALLENGE_3DS: return "CHALLENGE_3DS";
        case DecisionAction::BLOCK:         return "BLOCK";
    }
    return "UNKNOWN";
}

std::string_view toString(CaseStatus status) {
    switch (status) {
        case CaseStatus::OPEN:                     return "OPEN";
        case CaseStatus::IN_INVESTIGATION:         return "IN_INVESTIGATION";
        case CaseStatus::RESOLVED_CONFIRMED_FRAUD: return "RESOLVED_CONFIRMED_FRAUD";
        case CaseStatus::RESOLVED_FALSE_POSITIVE:  return "RESOLVED_FALSE_POSITIVE";
        case CaseStatus::CLOSED:                   return "CLOSED";
    }
    return "UNKNOWN";
}

std::string_view toString(GroundTruthLabel label) {
    switch (label) {
        case GroundTruthLabel::LEGITIMATE: return "LEGITIMATE";
        case GroundTruthLabel::FRAUD:      return "FRAUD";
        case GroundTruthLabel::UNKNOWN:    return "UNKNOWN";
    }
    return "UNKNOWN";
}

} // namespace epfd
