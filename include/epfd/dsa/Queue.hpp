#ifndef EPFD_DSA_QUEUE_HPP
#define EPFD_DSA_QUEUE_HPP

#include "epfd/dsa/Deque.hpp"

namespace epfd::dsa {

/**
 * @brief Custom FIFO Queue implementation adapter over Deque.
 * @tparam T Element type.
 */
template <typename T>
class Queue {
public:
    Queue() = default;

    void push(const T& value) { container_.push_back(value); }
    void push(T&& value) { container_.push_back(std::move(value)); }

    template <typename... Args>
    void emplace(Args&&... args) { container_.emplace_back(std::forward<Args>(args)...); }

    void pop() { container_.pop_front(); }

    T& front() { return container_.front(); }
    const T& front() const { return container_.front(); }

    T& back() { return container_.back(); }
    const T& back() const { return container_.back(); }

    size_t size() const noexcept { return container_.size(); }
    bool empty() const noexcept { return container_.empty(); }
    void clear() noexcept { container_.clear(); }

private:
    Deque<T> container_;
};

} // namespace epfd::dsa

#endif // EPFD_DSA_QUEUE_HPP
