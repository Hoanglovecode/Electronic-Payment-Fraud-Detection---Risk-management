#include "epfd/database/FileRepositories.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace epfd {

FileTransactionRepository::FileTransactionRepository(std::string file_path)
    : file_path_(std::move(file_path)) {
    reloadFromDisk();
}

std::string FileTransactionRepository::serializeTransaction(const Transaction& tx) const {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        tx.getTimestamp().time_since_epoch()).count();

    std::ostringstream oss;
    oss << tx.getTransactionId() << ","
        << static_cast<int>(tx.getType()) << ","
        << tx.getCustomerId() << ","
        << tx.getRecipientId() << ","
        << tx.getAccountId() << ","
        << std::fixed << std::setprecision(2) << tx.getAmount() << ","
        << tx.getCurrency() << ","
        << ms << ","
        << tx.getLocation().getLatitude() << ","
        << tx.getLocation().getLongitude() << ","
        << tx.getLocation().getCity() << ","
        << tx.getLocation().getCountry() << ","
        << tx.getIpAddress() << ","
        << tx.getDevice().getDeviceId() << ","
        << tx.getDevice().getDeviceFingerprint() << ","
        << tx.getDevice().getIpAddress() << ","
        << tx.getMerchantId() << ","
        << static_cast<int>(tx.getStatus());
    return oss.str();
}

std::optional<Transaction> FileTransactionRepository::deserializeTransaction(const std::string& line) const {
    if (line.empty() || line[0] == '#') {
        return std::nullopt;
    }

    std::stringstream ss(line);
    std::string tx_id, type_str, cust_id, rec_id, acc_id, amount_str, currency, ts_str;
    std::string lat_str, lon_str, city, country, ip, dev_id, dev_fp, dev_ip, merchant_id, status_str;

    if (!std::getline(ss, tx_id, ',') ||
        !std::getline(ss, type_str, ',') ||
        !std::getline(ss, cust_id, ',') ||
        !std::getline(ss, rec_id, ',') ||
        !std::getline(ss, acc_id, ',') ||
        !std::getline(ss, amount_str, ',') ||
        !std::getline(ss, currency, ',') ||
        !std::getline(ss, ts_str, ',') ||
        !std::getline(ss, lat_str, ',') ||
        !std::getline(ss, lon_str, ',') ||
        !std::getline(ss, city, ',') ||
        !std::getline(ss, country, ',') ||
        !std::getline(ss, ip, ',') ||
        !std::getline(ss, dev_id, ',') ||
        !std::getline(ss, dev_fp, ',') ||
        !std::getline(ss, dev_ip, ',') ||
        !std::getline(ss, merchant_id, ',') ||
        !std::getline(ss, status_str, ',')) {
        return std::nullopt;
    }

    try {
        TransactionType type = static_cast<TransactionType>(std::stoi(type_str));
        double amount = std::stod(amount_str);
        long long ms = std::stoll(ts_str);
        auto ts = Timestamp(std::chrono::milliseconds(ms));
        double lat = std::stod(lat_str);
        double lon = std::stod(lon_str);
        Location loc(lat, lon, city, country);
        Device dev(dev_id, dev_fp, dev_ip);
        PaymentMethod pm("pm_" + tx_id, PaymentType::CREDIT_CARD, "411111******1111", "Cardholder", 12, 2028);
        TransactionStatus status = static_cast<TransactionStatus>(std::stoi(status_str));

        Transaction tx(tx_id, type, cust_id, rec_id, acc_id, amount, currency, ts, loc, ip, dev, merchant_id, pm);
        tx.setStatus(status);
        return tx;
    } catch (...) {
        return std::nullopt;
    }
}

bool FileTransactionRepository::syncToDisk() const {
    std::ofstream ofs(file_path_, std::ios::trunc);
    if (!ofs.is_open()) {
        return false;
    }

    ofs << "# tx_id,type,cust_id,rec_id,acc_id,amount,currency,ts_ms,lat,lon,city,country,ip,dev_id,dev_fp,dev_ip,merchant_id,status\n";
    for (const auto& tx : in_memory_cache_) {
        ofs << serializeTransaction(tx) << "\n";
    }
    return true;
}

bool FileTransactionRepository::reloadFromDisk() {
    std::lock_guard<std::mutex> lock(mutex_);
    in_memory_cache_.clear();

    std::ifstream ifs(file_path_);
    if (!ifs.is_open()) {
        return false; // File doesn't exist yet, clean start
    }

    std::string line;
    while (std::getline(ifs, line)) {
        auto tx_opt = deserializeTransaction(line);
        if (tx_opt.has_value()) {
            in_memory_cache_.push_back(tx_opt.value());
        }
    }
    return true;
}

bool FileTransactionRepository::save(const Transaction& entity) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(in_memory_cache_.begin(), in_memory_cache_.end(),
                           [&entity](const Transaction& t) {
                               return t.getTransactionId() == entity.getTransactionId();
                           });
    if (it != in_memory_cache_.end()) {
        *it = entity;
    } else {
        in_memory_cache_.push_back(entity);
    }
    return syncToDisk();
}

std::optional<Transaction> FileTransactionRepository::findById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& tx : in_memory_cache_) {
        if (tx.getTransactionId() == id) {
            return tx;
        }
    }
    return std::nullopt;
}

std::vector<Transaction> FileTransactionRepository::findAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return in_memory_cache_;
}

bool FileTransactionRepository::remove(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(in_memory_cache_.begin(), in_memory_cache_.end(),
                             [&id](const Transaction& t) {
                                 return t.getTransactionId() == id;
                             });
    if (it != in_memory_cache_.end()) {
        in_memory_cache_.erase(it, in_memory_cache_.end());
        syncToDisk();
        return true;
    }
    return false;
}

size_t FileTransactionRepository::count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return in_memory_cache_.size();
}

void FileTransactionRepository::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    in_memory_cache_.clear();
    syncToDisk();
}

std::vector<Transaction> FileTransactionRepository::findByCustomerId(const std::string& customer_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Transaction> result;
    for (const auto& tx : in_memory_cache_) {
        if (tx.getCustomerId() == customer_id) {
            result.push_back(tx);
        }
    }
    return result;
}

std::vector<Transaction> FileTransactionRepository::findByStatus(TransactionStatus status) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Transaction> result;
    for (const auto& tx : in_memory_cache_) {
        if (tx.getStatus() == status) {
            result.push_back(tx);
        }
    }
    return result;
}

std::vector<Transaction> FileTransactionRepository::findRecentByCustomer(
    const std::string& customer_id, std::chrono::seconds duration) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Transaction> result;
    auto now = std::chrono::system_clock::now();
    for (const auto& tx : in_memory_cache_) {
        if (tx.getCustomerId() == customer_id) {
            if ((now - tx.getTimestamp()) <= duration) {
                result.push_back(tx);
            }
        }
    }
    return result;
}

} // namespace epfd
