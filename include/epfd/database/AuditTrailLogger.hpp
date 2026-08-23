#ifndef EPFD_DATABASE_AUDIT_TRAIL_LOGGER_HPP
#define EPFD_DATABASE_AUDIT_TRAIL_LOGGER_HPP

#include <string>
#include <mutex>
#include <fstream>
#include "epfd/common/Types.hpp"
#include "epfd/models/RiskAssessment.hpp"
#include "epfd/decision/IDecisionPolicy.hpp"

namespace epfd {

struct AuditRecord {
    std::string audit_id;
    std::string transaction_id;
    double risk_score{0.0};
    RiskLevel risk_level{RiskLevel::VERY_LOW};
    DecisionAction decision{DecisionAction::APPROVE};
    std::string rationale;
    std::string actor{"SYSTEM_AUTO"};
    Timestamp timestamp{std::chrono::system_clock::now()};
};

/**
 * @brief Thread-safe, append-only compliance audit trail logger (PCI-DSS & ISO27001).
 */
class AuditTrailLogger {
public:
    explicit AuditTrailLogger(std::string log_file_path = "epfd_audit_trail.log");
    ~AuditTrailLogger();

    bool logDecision(const DecisionResult& result, const std::string& actor = "SYSTEM_AUTO");
    bool logAssessment(const RiskAssessment& assessment, const std::string& actor = "SYSTEM_AUTO");
    bool logRecord(const AuditRecord& record);

    size_t getLoggedCount() const noexcept { return logged_count_; }
    const std::string& getLogFilePath() const noexcept { return log_file_path_; }

private:
    std::string log_file_path_;
    mutable std::mutex mutex_;
    std::ofstream log_stream_;
    size_t logged_count_{0};
};

} // namespace epfd

#endif // EPFD_DATABASE_AUDIT_TRAIL_LOGGER_HPP
