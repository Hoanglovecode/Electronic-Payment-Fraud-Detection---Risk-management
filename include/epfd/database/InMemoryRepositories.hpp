#ifndef EPFD_DATABASE_IN_MEMORY_REPOSITORIES_HPP
#define EPFD_DATABASE_IN_MEMORY_REPOSITORIES_HPP

#include <unordered_map>
#include <mutex>
#include "epfd/database/ITransactionRepository.hpp"
#include "epfd/database/ICustomerRepository.hpp"
#include "epfd/database/IAccountRepository.hpp"
#include "epfd/database/IFraudAlertRepository.hpp"
#include "epfd/database/IFraudCaseRepository.hpp"
#include "epfd/database/IRiskAssessmentRepository.hpp"

namespace epfd {

class InMemoryTransactionRepository : public ITransactionRepository {
public:
    bool save(const Transaction& entity) override;
    std::optional<Transaction> findById(const std::string& id) const override;
    std::vector<Transaction> findAll() const override;
    bool remove(const std::string& id) override;
    size_t count() const override;
    void clear() override;

    std::vector<Transaction> findByCustomerId(const std::string& customer_id) const override;
    std::vector<Transaction> findByStatus(TransactionStatus status) const override;
    std::vector<Transaction> findRecentByCustomer(const std::string& customer_id, std::chrono::seconds duration) const override;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Transaction> storage_;
};

class InMemoryCustomerRepository : public ICustomerRepository {
public:
    bool save(const Customer& entity) override;
    std::optional<Customer> findById(const std::string& id) const override;
    std::vector<Customer> findAll() const override;
    bool remove(const std::string& id) override;
    size_t count() const override;
    void clear() override;

    std::optional<Customer> findByEmail(const std::string& email) const override;
    std::vector<Customer> findByRiskLevel(RiskLevel level) const override;
    std::vector<Customer> findBlacklisted() const override;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Customer> storage_;
};

class InMemoryAccountRepository : public IAccountRepository {
public:
    bool save(const Account& entity) override;
    std::optional<Account> findById(const std::string& id) const override;
    std::vector<Account> findAll() const override;
    bool remove(const std::string& id) override;
    size_t count() const override;
    void clear() override;

    std::vector<Account> findByCustomerId(const std::string& customer_id) const override;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Account> storage_;
};

class InMemoryFraudAlertRepository : public IFraudAlertRepository {
public:
    bool save(const FraudAlert& entity) override;
    std::optional<FraudAlert> findById(const std::string& id) const override;
    std::vector<FraudAlert> findAll() const override;
    bool remove(const std::string& id) override;
    size_t count() const override;
    void clear() override;

    std::vector<FraudAlert> findByTransactionId(const std::string& transaction_id) const override;
    std::vector<FraudAlert> findByRuleId(const std::string& rule_id) const override;
    std::vector<FraudAlert> findBySeverity(RiskLevel severity) const override;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, FraudAlert> storage_;
};

class InMemoryFraudCaseRepository : public IFraudCaseRepository {
public:
    bool save(const Dispute& entity) override;
    std::optional<Dispute> findById(const std::string& id) const override;
    std::vector<Dispute> findAll() const override;
    bool remove(const std::string& id) override;
    size_t count() const override;
    void clear() override;

    std::vector<Dispute> findByTransactionId(const std::string& transaction_id) const override;
    std::vector<Dispute> findByCustomerId(const std::string& customer_id) const override;
    std::vector<Dispute> findByStatus(CaseStatus status) const override;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Dispute> storage_;
};

class InMemoryRiskAssessmentRepository : public IRiskAssessmentRepository {
public:
    bool save(const RiskAssessment& entity) override;
    std::optional<RiskAssessment> findById(const std::string& id) const override;
    std::vector<RiskAssessment> findAll() const override;
    bool remove(const std::string& id) override;
    size_t count() const override;
    void clear() override;

    std::optional<RiskAssessment> findByTransactionId(const std::string& transaction_id) const override;
    std::vector<RiskAssessment> findByRiskLevel(RiskLevel level) const override;
    std::vector<RiskAssessment> findByDecision(DecisionAction decision) const override;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, RiskAssessment> storage_;
};

} // namespace epfd

#endif // EPFD_DATABASE_IN_MEMORY_REPOSITORIES_HPP
