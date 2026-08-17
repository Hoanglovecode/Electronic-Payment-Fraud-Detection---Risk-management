#include "epfd/dsa/FastLookupIndex.hpp"

namespace epfd {

void FastLookupIndex::recordDeviceUsage(const std::string& device_fingerprint, const std::string& customer_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    device_to_customers_[device_fingerprint].insert(customer_id);
}

void FastLookupIndex::recordIpUsage(const std::string& ip_address, const std::string& customer_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    ip_to_customers_[ip_address].insert(customer_id);
}

void FastLookupIndex::recordCardUsage(const std::string& card_bin_or_hash, const std::string& customer_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    card_to_customers_[card_bin_or_hash].insert(customer_id);
}

size_t FastLookupIndex::getAccountCountOnDevice(const std::string& device_fingerprint) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = device_to_customers_.find(device_fingerprint);
    if (it != device_to_customers_.end()) {
        return it->second.size();
    }
    return 0;
}

size_t FastLookupIndex::getAccountCountOnIp(const std::string& ip_address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = ip_to_customers_.find(ip_address);
    if (it != ip_to_customers_.end()) {
        return it->second.size();
    }
    return 0;
}

size_t FastLookupIndex::getAccountCountOnCard(const std::string& card_bin_or_hash) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = card_to_customers_.find(card_bin_or_hash);
    if (it != card_to_customers_.end()) {
        return it->second.size();
    }
    return 0;
}

std::unordered_set<std::string> FastLookupIndex::getCustomersOnDevice(const std::string& device_fingerprint) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = device_to_customers_.find(device_fingerprint);
    if (it != device_to_customers_.end()) {
        return it->second;
    }
    return {};
}

void FastLookupIndex::addBlacklist(const std::string& entity_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    blacklist_.insert(entity_key);
}

void FastLookupIndex::removeBlacklist(const std::string& entity_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    blacklist_.erase(entity_key);
}

bool FastLookupIndex::isBlacklisted(const std::string& entity_key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return blacklist_.find(entity_key) != blacklist_.end();
}

void FastLookupIndex::addWhitelist(const std::string& entity_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    whitelist_.insert(entity_key);
}

void FastLookupIndex::removeWhitelist(const std::string& entity_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    whitelist_.erase(entity_key);
}

bool FastLookupIndex::isWhitelisted(const std::string& entity_key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return whitelist_.find(entity_key) != whitelist_.end();
}

void FastLookupIndex::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    device_to_customers_.clear();
    ip_to_customers_.clear();
    card_to_customers_.clear();
    blacklist_.clear();
    whitelist_.clear();
}

} // namespace epfd
