#ifndef EPFD_DSA_TIME_WINDOW_BUFFER_HPP
#define EPFD_DSA_TIME_WINDOW_BUFFER_HPP

#include <deque>
#include <chrono>
#include <mutex>
#include "epfd/common/Types.hpp"

namespace epfd {

struct WindowEntry {
    Timestamp timestamp;
    double amount{0.0};
};

/**
 * @brief Thread-safe Sliding Window Buffer using std::deque.
 * Maintains running sum and count with O(1) amortized additions and queries.
 */
class TimeWindowBuffer {
public:
    explicit TimeWindowBuffer(std::chrono::seconds window_duration);

    void add(Timestamp ts, double amount);
    void prune(Timestamp current_time);

    size_t getCount(Timestamp current_time);
    double getSum(Timestamp current_time);
    double getAverage(Timestamp current_time);
    double getMax(Timestamp current_time);

    std::chrono::seconds getWindowDuration() const noexcept { return window_duration_; }
    void clear();

private:
    std::chrono::seconds window_duration_;
    std::deque<WindowEntry> entries_;
    double running_sum_{0.0};
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_DSA_TIME_WINDOW_BUFFER_HPP
