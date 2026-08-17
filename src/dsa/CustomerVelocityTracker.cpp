#include "epfd/dsa/CustomerVelocityTracker.hpp"

namespace epfd {

void CustomerVelocityTracker::recordTransaction(const std::string& entity_id, Timestamp ts, double amount) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = profiles_.find(entity_id);
    if (it == profiles_.end()) {
        it = profiles_.emplace(entity_id, std::make_unique<EntityVelocityProfile>()).first;
    }

    it->second->window_5m.add(ts, amount);
    it->second->window_1h.add(ts, amount);
    it->second->window_24h.add(ts, amount);
}

VelocityMetrics CustomerVelocityTracker::getMetrics(const std::string& entity_id, Timestamp current_time) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = profiles_.find(entity_id);
    if (it == profiles_.end()) {
        return VelocityMetrics{};
    }

    VelocityMetrics m;
    m.count_5m = it->second->window_5m.getCount(current_time);
    m.sum_5m = it->second->window_5m.getSum(current_time);

    m.count_1h = it->second->window_1h.getCount(current_time);
    m.sum_1h = it->second->window_1h.getSum(current_time);

    m.count_24h = it->second->window_24h.getCount(current_time);
    m.sum_24h = it->second->window_24h.getSum(current_time);
    m.average_amount_24h = it->second->window_24h.getAverage(current_time);

    return m;
}

void CustomerVelocityTracker::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    profiles_.clear();
}

} // namespace epfd
