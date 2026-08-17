#ifndef EPFD_DSA_FAST_LOOKUP_INDEX_HPP
#define EPFD_DSA_FAST_LOOKUP_INDEX_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include "epfd/models/Customer.hpp"

namespace epfd {

/**
 * @brief O(1) Fast in-memory lookup index using std::unordered_map & std::unordered_set.
 * Tracks multi-entity associations and blacklists/whitelists.
 */
class FastLookupIndex {
public:
    FastLookupIndex() = default;

    // Association indexing
    void recordDeviceUsage(const std::string& device_fingerprint, const std::string& customer_id);
    void recordIpUsage(const std::string& ip_address, const std::string& customer_id);
    void recordCardUsage(const std::string& card_bin_or_hash, const std::string& customer_id);

    size_t getAccountCountOnDevice(const std::string& device_fingerprint) const;
    size_t getAccountCountOnIp(const std::string& ip_address) const;
    size_t getAccountCountOnCard(const std::string& card_bin_or_hash) const;

    std::unordered_set<std::string> getCustomersOnDevice(const std::string& device_fingerprint) const;

    // Blacklist & Whitelist management
    void addBlacklist(const std::string& entity_key);
    void removeBlacklist(const std::string& entity_key);
    bool isBlacklisted(const std::string& entity_key) const;

    void addWhitelist(const std::string& entity_key);
    void removeWhitelist(const std::string& entity_key);
    bool isWhitelisted(const std::string& entity_key) const;

    void clear();

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unordered_set<std::string>> device_to_customers_;
    std::unordered_map<std::string, std::unordered_set<std::string>> ip_to_customers_;
    std::unordered_map<std::string, std::unordered_set<std::string>> card_to_customers_;

    std::unordered_set<std::string> blacklist_;
    std::unordered_set<std::string> whitelist_;
};

} // namespace epfd

#endif // EPFD_DSA_FAST_LOOKUP_INDEX_HPP
