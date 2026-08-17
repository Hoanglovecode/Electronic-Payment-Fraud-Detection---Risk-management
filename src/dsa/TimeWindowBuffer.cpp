#include "epfd/dsa/TimeWindowBuffer.hpp"
#include <algorithm>

namespace epfd {

TimeWindowBuffer::TimeWindowBuffer(std::chrono::seconds window_duration)
    : window_duration_(window_duration) {}

void TimeWindowBuffer::add(Timestamp ts, double amount) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (entries_.empty() || ts >= entries_.back().timestamp) {
        entries_.push_back({ts, amount});
    } else {
        auto it = std::upper_bound(entries_.begin(), entries_.end(), ts, [](Timestamp val, const WindowEntry& e) {
            return val < e.timestamp;
        });
        entries_.insert(it, {ts, amount});
    }
    running_sum_ += amount;
}

void TimeWindowBuffer::prune(Timestamp current_time) {
    // Note: Assumes mutex is held or called by internal methods with lock
    auto cutoff = current_time - window_duration_;
    while (!entries_.empty() && entries_.front().timestamp < cutoff) {
        running_sum_ -= entries_.front().amount;
        entries_.pop_front();
    }
    if (entries_.empty()) {
        running_sum_ = 0.0;
    }
}

size_t TimeWindowBuffer::getCount(Timestamp current_time) {
    std::lock_guard<std::mutex> lock(mutex_);
    prune(current_time);
    return entries_.size();
}

double TimeWindowBuffer::getSum(Timestamp current_time) {
    std::lock_guard<std::mutex> lock(mutex_);
    prune(current_time);
    return running_sum_;
}

double TimeWindowBuffer::getAverage(Timestamp current_time) {
    std::lock_guard<std::mutex> lock(mutex_);
    prune(current_time);
    if (entries_.empty()) {
        return 0.0;
    }
    return running_sum_ / static_cast<double>(entries_.size());
}

double TimeWindowBuffer::getMax(Timestamp current_time) {
    std::lock_guard<std::mutex> lock(mutex_);
    prune(current_time);
    if (entries_.empty()) {
        return 0.0;
    }
    double max_val = entries_.front().amount;
    for (const auto& entry : entries_) {
        if (entry.amount > max_val) {
            max_val = entry.amount;
        }
    }
    return max_val;
}

void TimeWindowBuffer::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.clear();
    running_sum_ = 0.0;
}

} // namespace epfd
