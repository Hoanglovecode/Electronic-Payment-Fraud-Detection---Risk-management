#ifndef EPFD_COMMON_CONSTANTS_HPP
#define EPFD_COMMON_CONSTANTS_HPP

#include <cstddef>
#include <chrono>

namespace epfd {
namespace constants {

// ==========================================
// Risk Score Thresholds (Default baseline)
// ==========================================
constexpr double RISK_SCORE_MIN = 0.0;
constexpr double RISK_SCORE_MAX = 100.0;

constexpr double THRESHOLD_VERY_LOW_MAX = 20.0;
constexpr double THRESHOLD_LOW_MAX = 40.0;
constexpr double THRESHOLD_MEDIUM_MAX = 60.0;
constexpr double THRESHOLD_HIGH_MAX = 80.0;

// Decision Thresholds
constexpr double DECISION_APPROVE_BELOW = 30.0;
constexpr double DECISION_CHALLENGE_BELOW = 60.0;
constexpr double DECISION_REVIEW_BELOW = 80.0; // [60, 80) -> REVIEW, >= 80 -> BLOCK

// ==========================================
// DSA Sliding Window Defaults
// ==========================================
constexpr std::chrono::minutes VELOCITY_WINDOW_SHORT{5};
constexpr std::chrono::minutes VELOCITY_WINDOW_MEDIUM{60};
constexpr std::chrono::hours VELOCITY_WINDOW_LONG{24};

// ==========================================
// Security & Masking
// ==========================================
constexpr size_t MASK_VISIBLE_PREFIX = 6; // First 6 (BIN)
constexpr size_t MASK_VISIBLE_SUFFIX = 4; // Last 4

// ==========================================
// Case Management SLAs
// ==========================================
constexpr std::chrono::hours CASE_CRITICAL_SLA_HOURS{2};
constexpr std::chrono::hours CASE_HIGH_SLA_HOURS{6};
constexpr std::chrono::hours CASE_MEDIUM_SLA_HOURS{24};

} // namespace constants
} // namespace epfd

#endif // EPFD_COMMON_CONSTANTS_HPP
