#ifndef EPFD_DATABASE_FILE_REPOSITORIES_HPP
#define EPFD_DATABASE_FILE_REPOSITORIES_HPP

#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include "epfd/database/ITransactionRepository.hpp"

namespace epfd {

/**
 * @brief Persistent File-backed Transaction Repository supporting CSV/JSON serialization.
 * Demonstrates Repository abstraction (DIP) and cold-start state persistence.
 */
class FileTransactionRepository : public ITransactionRepository {
public:
    explicit FileTransactionRepository(std::string file_path = "transactions_store.csv");

    bool save(const Transaction& entity) override;
    std::optional<Transaction> findById(const std::string& id) const override;
    std::vector<Transaction> findAll() const override;
    bool remove(const std::string& id) override;
    size_t count() const override;
    void clear() override;

    std::vector<Transaction> findByCustomerId(const std::string& customer_id) const override;
    std::vector<Transaction> findByStatus(TransactionStatus status) const override;
    std::vector<Transaction> findRecentByCustomer(const std::string& customer_id, std::chrono::seconds duration) const override;

    // Explicit disk sync methods
    bool syncToDisk() const;
    bool reloadFromDisk();
    const std::string& getFilePath() const noexcept { return file_path_; }

private:
    std::string serializeTransaction(const Transaction& tx) const;
    std::optional<Transaction> deserializeTransaction(const std::string& line) const;

    std::string file_path_;
    mutable std::mutex mutex_;
    std::vector<Transaction> in_memory_cache_;
};

} // namespace epfd

#endif // EPFD_DATABASE_FILE_REPOSITORIES_HPP
