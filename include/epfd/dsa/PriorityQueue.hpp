#ifndef EPFD_DSA_PRIORITY_QUEUE_HPP
#define EPFD_DSA_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <functional>
#include "epfd/dsa/Vector.hpp"

namespace epfd::dsa {

/**
 * @brief Custom Binary Heap Priority Queue implementation from scratch.
 * @tparam T Element type.
 * @tparam Compare Comparator functor (default std::less<T> for Max-Heap).
 */
template <typename T, typename Compare = std::less<T>>
class PriorityQueue {
public:
    PriorityQueue() = default;

    explicit PriorityQueue(const Compare& comp) : comp_(comp) {}

    size_t size() const noexcept { return heap_.size(); }
    bool empty() const noexcept { return heap_.empty(); }

    const T& top() const {
        if (heap_.empty()) {
            throw std::out_of_range("PriorityQueue is empty");
        }
        return heap_[0];
    }

    void push(const T& value) {
        heap_.push_back(value);
        siftUp(heap_.size() - 1);
    }

    void push(T&& value) {
        heap_.push_back(std::move(value));
        siftUp(heap_.size() - 1);
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        heap_.emplace_back(std::forward<Args>(args)...);
        siftUp(heap_.size() - 1);
    }

    void pop() {
        if (heap_.empty()) return;
        if (heap_.size() == 1) {
            heap_.pop_back();
            return;
        }

        heap_[0] = std::move(heap_.back());
        heap_.pop_back();
        siftDown(0);
    }

    void clear() noexcept {
        heap_.clear();
    }

private:
    void siftUp(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (comp_(heap_[parent], heap_[index])) {
                std::swap(heap_[parent], heap_[index]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t index) {
        size_t n = heap_.size();
        while (2 * index + 1 < n) {
            size_t left = 2 * index + 1;
            size_t right = 2 * index + 2;
            size_t best = index;

            if (left < n && comp_(heap_[best], heap_[left])) {
                best = left;
            }
            if (right < n && comp_(heap_[best], heap_[right])) {
                best = right;
            }

            if (best != index) {
                std::swap(heap_[index], heap_[best]);
                index = best;
            } else {
                break;
            }
        }
    }

    Vector<T> heap_;
    Compare comp_{};
};

} // namespace epfd::dsa

#endif // EPFD_DSA_PRIORITY_QUEUE_HPP
