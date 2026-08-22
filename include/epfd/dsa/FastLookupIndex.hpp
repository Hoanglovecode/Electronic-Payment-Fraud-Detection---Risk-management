#ifndef EPFD_DSA_FAST_LOOKUP_INDEX_HPP
#define EPFD_DSA_FAST_LOOKUP_INDEX_HPP

#include <string>
#include <mutex>
#include "epfd/models/Customer.hpp"
#include "epfd/dsa/HashMap.hpp"
#include "epfd/dsa/HashSet.hpp"

namespace epfd {

/**
 * @brief O(1) Fast in-memory lookup index using custom HashMap & HashSet.
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

    dsa::HashSet<std::string> getCustomersOnDevice(const std::string& device_fingerprint) const;

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
    dsa::HashMap<std::string, dsa::HashSet<std::string>> device_to_customers_;
    dsa::HashMap<std::string, dsa::HashSet<std::string>> ip_to_customers_;
    dsa::HashMap<std::string, dsa::HashSet<std::string>> card_to_customers_;

    dsa::HashSet<std::string> blacklist_;
    dsa::HashSet<std::string> whitelist_;
};

} // namespace epfd

#endif // EPFD_DSA_FAST_LOOKUP_INDEX_HPP
