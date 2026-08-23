#include "epfd/database/AuditTrailLogger.hpp"
#include <iomanip>
#include <sstream>

namespace epfd {

AuditTrailLogger::AuditTrailLogger(std::string log_file_path)
    : log_file_path_(std::move(log_file_path)) {
    log_stream_.open(log_file_path_, std::ios::app);
}

AuditTrailLogger::~AuditTrailLogger() {
    if (log_stream_.is_open()) {
        log_stream_.flush();
        log_stream_.close();
    }
}

bool AuditTrailLogger::logRecord(const AuditRecord& record) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!log_stream_.is_open()) {
        log_stream_.open(log_file_path_, std::ios::app);
        if (!log_stream_.is_open()) return false;
    }

    std::time_t t = std::chrono::system_clock::to_time_t(record.timestamp);
    std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif

    char time_str[64];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

    log_stream_ << "[" << time_str << "]"
                << " AUDIT_ID=" << (record.audit_id.empty() ? "AUD_" + record.transaction_id : record.audit_id)
                << " TX_ID=" << record.transaction_id
                << " SCORE=" << std::fixed << std::setprecision(1) << record.risk_score
                << " LEVEL=" << epfd::toString(record.risk_level)
                << " ACTION=" << (record.decision == DecisionAction::APPROVE ? "APPROVE" :
                                  record.decision == DecisionAction::CHALLENGE_3DS ? "CHALLENGE_3DS" :
                                  record.decision == DecisionAction::REVIEW ? "REVIEW" : "BLOCK")
                << " ACTOR=" << record.actor
                << " RATIONALE=\"" << record.rationale << "\""
                << std::endl; // flush on write

    logged_count_++;
    return true;
}

bool AuditTrailLogger::logDecision(const DecisionResult& result, const std::string& actor) {
    AuditRecord rec;
    rec.audit_id = result.decision_id;
    rec.transaction_id = result.transaction_id;
    rec.risk_score = result.risk_score;
    rec.risk_level = result.risk_level;
    rec.decision = result.action;
    rec.rationale = result.rationale;
    rec.actor = actor;
    rec.timestamp = result.evaluated_at;
    return logRecord(rec);
}

bool AuditTrailLogger::logAssessment(const RiskAssessment& assessment, const std::string& actor) {
    AuditRecord rec;
    rec.audit_id = assessment.getAssessmentId();
    rec.transaction_id = assessment.getTransactionId();
    rec.risk_score = assessment.getCombinedScore();
    rec.risk_level = assessment.getRiskLevel();
    rec.decision = assessment.getDecision();
    rec.rationale = assessment.getReasons().empty() ? "Standard assessment" : assessment.getReasons()[0];
    rec.actor = actor;
    rec.timestamp = assessment.getEvaluatedAt();
    return logRecord(rec);
}

} // namespace epfd
