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
        // Find insert position in circular deque
        size_t idx = 0;
        while (idx < entries_.size() && entries_[idx].timestamp <= ts) {
            ++idx;
        }
        // Shift or append
        entries_.push_back({ts, amount});
        for (size_t i = entries_.size() - 1; i > idx; --i) {
            std::swap(entries_[i], entries_[i - 1]);
        }
    }
    running_sum_ += amount;
}

void TimeWindowBuffer::prune(Timestamp current_time) {
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
    for (size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].amount > max_val) {
            max_val = entries_[i].amount;
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
