#ifndef EPFD_DSA_CUSTOMER_VELOCITY_TRACKER_HPP
#define EPFD_DSA_CUSTOMER_VELOCITY_TRACKER_HPP

#include <memory>
#include <mutex>
#include "epfd/dsa/TimeWindowBuffer.hpp"
#include "epfd/dsa/HashMap.hpp"
#include "epfd/common/Constants.hpp"

namespace epfd {

struct VelocityMetrics {
    size_t count_5m{0};
    double sum_5m{0.0};
    size_t count_1h{0};
    double sum_1h{0.0};
    size_t count_24h{0};
    double sum_24h{0.0};
    double average_amount_24h{0.0};
};

struct EntityVelocityProfile {
    TimeWindowBuffer window_5m{constants::VELOCITY_WINDOW_SHORT};
    TimeWindowBuffer window_1h{constants::VELOCITY_WINDOW_MEDIUM};
    TimeWindowBuffer window_24h{constants::VELOCITY_WINDOW_LONG};
};

/**
 * @brief High-performance tracker for customer and card velocity signals.
 * Uses custom epfd::dsa::HashMap of Sliding Windows with thread-safety.
 */
class CustomerVelocityTracker {
public:
    CustomerVelocityTracker() = default;

    void recordTransaction(const std::string& entity_id, Timestamp ts, double amount);
    VelocityMetrics getMetrics(const std::string& entity_id, Timestamp current_time);
    void clear();

private:
    dsa::HashMap<std::string, std::shared_ptr<EntityVelocityProfile>> profiles_;
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_DSA_CUSTOMER_VELOCITY_TRACKER_HPP
