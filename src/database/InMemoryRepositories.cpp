#include "epfd/database/InMemoryRepositories.hpp"
#include <algorithm>

namespace epfd {

// ==========================================
// InMemoryTransactionRepository
// ==========================================

bool InMemoryTransactionRepository::save(const Transaction& entity) {
    std::lock_guard<std::mutex> lock(mutex_);
    storage_[entity.getTransactionId()] = entity;
    return true;
}

std::optional<Transaction> InMemoryTransactionRepository::findById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = storage_.find(id);
    if (it != storage_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Transaction> InMemoryTransactionRepository::findAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Transaction> result;
    result.reserve(storage_.size());
    for (const auto& [_, tx] : storage_) {
        result.push_back(tx);
    }
    return result;
}

bool InMemoryTransactionRepository::remove(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return storage_.erase(id) > 0;
}

size_t InMemoryTransactionRepository::count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return storage_.size();
}

void InMemoryTransactionRepository::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    storage_.clear();
}

std::vector<Transaction> InMemoryTransactionRepository::findByCustomerId(const std::string& customer_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Transaction> result;
    for (const auto& [_, tx] : storage_) {
        if (tx.getCustomerId() == customer_id) {
            result.push_back(tx);
        }
    }
    return result;
}

std::vector<Transaction> InMemoryTransactionRepository::findByStatus(TransactionStatus status) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Transaction> result;
    for (const auto& [_, tx] : storage_) {
        if (tx.getStatus() == status) {
            result.push_back(tx);
        }
    }
    return result;
}

std::vector<Transaction> InMemoryTransactionRepository::findRecentByCustomer(
    const std::string& customer_id, std::chrono::seconds duration) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Transaction> result;
    auto now = std::chrono::system_clock::now();
    for (const auto& [_, tx] : storage_) {
        if (tx.getCustomerId() == customer_id) {
            if ((now - tx.getTimestamp()) <= duration) {
                result.push_back(tx);
            }
        }
    }
    return result;
}

// ==========================================
// InMemoryCustomerRepository
// ==========================================

bool InMemoryCustomerRepository::save(const Customer& entity) {
    std::lock_guard<std::mutex> lock(mutex_);
    storage_[entity.getCustomerId()] = entity;
    return true;
}

std::optional<Customer> InMemoryCustomerRepository::findById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = storage_.find(id);
    if (it != storage_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Customer> InMemoryCustomerRepository::findAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Customer> result;
    result.reserve(storage_.size());
    for (const auto& [_, cust] : storage_) {
        result.push_back(cust);
    }
    return result;
}

bool InMemoryCustomerRepository::remove(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return storage_.erase(id) > 0;
}

size_t InMemoryCustomerRepository::count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return storage_.size();
}

void InMemoryCustomerRepository::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    storage_.clear();
}

std::optional<Customer> InMemoryCustomerRepository::findByEmail(const std::string& email) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [_, cust] : storage_) {
        if (cust.getEmail() == email) {
            return cust;
        }
    }
    return std::nullopt;
}

std::vector<Customer> InMemoryCustomerRepository::findByRiskLevel(RiskLevel level) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Customer> result;
    for (const auto& [_, cust] : storage_) {
        if (cust.getRiskLevel() == level) {
            result.push_back(cust);
        }
    }
    return result;
}

std::vector<Customer> InMemoryCustomerRepository::findBlacklisted() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Customer> result;
    for (const auto& [_, cust] : storage_) {
        if (cust.isBlacklisted()) {
            result.push_back(cust);
        }
    }
    return result;
}

} // namespace epfd
