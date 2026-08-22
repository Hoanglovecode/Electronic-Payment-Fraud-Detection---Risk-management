#ifndef EPFD_DSA_DEQUE_HPP
#define EPFD_DSA_DEQUE_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <initializer_list>

namespace epfd::dsa {

/**
 * @brief Custom Circular Double-Ended Queue (Deque) implementation.
 * Supports O(1) push_front, push_back, pop_front, pop_back, and random access.
 */
template <typename T>
class Deque {
public:
    class Iterator {
    public:
        Iterator(Deque* deque, size_t index) : deque_(deque), index_(index) {}

        T& operator*() { return (*deque_)[index_]; }
        T* operator->() { return &((*deque_)[index_]); }

        Iterator& operator++() {
            ++index_;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++index_;
            return tmp;
        }

        bool operator==(const Iterator& other) const noexcept {
            return deque_ == other.deque_ && index_ == other.index_;
        }

        bool operator!=(const Iterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        Deque* deque_;
        size_t index_;
    };

    class ConstIterator {
    public:
        ConstIterator(const Deque* deque, size_t index) : deque_(deque), index_(index) {}

        const T& operator*() const { return (*deque_)[index_]; }
        const T* operator->() const { return &((*deque_)[index_]); }

        ConstIterator& operator++() {
            ++index_;
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator tmp = *this;
            ++index_;
            return tmp;
        }

        bool operator==(const ConstIterator& other) const noexcept {
            return deque_ == other.deque_ && index_ == other.index_;
        }

        bool operator!=(const ConstIterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        const Deque* deque_;
        size_t index_;
    };

    Deque() : buffer_(nullptr), head_(0), size_(0), capacity_(0) {}

    explicit Deque(size_t count, const T& value = T()) : Deque() {
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    Deque(std::initializer_list<T> init) : Deque() {
        reserve(init.size());
        for (const auto& item : init) {
            push_back(item);
        }
    }

    ~Deque() {
        clear();
        deallocate();
    }

    // Copy Constructor
    Deque(const Deque& other) : Deque() {
        reserve(other.size_);
        for (size_t i = 0; i < other.size_; ++i) {
            push_back(other[i]);
        }
    }

    // Copy Assignment
    Deque& operator=(const Deque& other) {
        if (this != &other) {
            clear();
            reserve(other.size_);
            for (size_t i = 0; i < other.size_; ++i) {
                push_back(other[i]);
            }
        }
        return *this;
    }

    // Move Constructor
    Deque(Deque&& other) noexcept
        : buffer_(other.buffer_), head_(other.head_), size_(other.size_), capacity_(other.capacity_) {
        other.buffer_ = nullptr;
        other.head_ = 0;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move Assignment
    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            clear();
            deallocate();
            buffer_ = other.buffer_;
            head_ = other.head_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.buffer_ = nullptr;
            other.head_ = 0;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // Capacity
    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    void reserve(size_t new_cap) {
        if (new_cap <= capacity_) return;

        T* new_buf = static_cast<T*>(::operator new(new_cap * sizeof(T)));
        for (size_t i = 0; i < size_; ++i) {
            size_t old_idx = (head_ + i) % capacity_;
            new (&new_buf[i]) T(std::move(buffer_[old_idx]));
            buffer_[old_idx].~T();
        }
        deallocate();
        buffer_ = new_buf;
        head_ = 0;
        capacity_ = new_cap;
    }

    // Element Access
    T& operator[](size_t index) noexcept {
        return buffer_[(head_ + index) % capacity_];
    }

    const T& operator[](size_t index) const noexcept {
        return buffer_[(head_ + index) % capacity_];
    }

    T& at(size_t index) {
        if (index >= size_) throw std::out_of_range("Deque index out of range");
        return (*this)[index];
    }

    const T& at(size_t index) const {
        if (index >= size_) throw std::out_of_range("Deque index out of range");
        return (*this)[index];
    }

    T& front() {
        if (empty()) throw std::out_of_range("Deque is empty");
        return buffer_[head_];
    }

    const T& front() const {
        if (empty()) throw std::out_of_range("Deque is empty");
        return buffer_[head_];
    }

    T& back() {
        if (empty()) throw std::out_of_range("Deque is empty");
        return buffer_[(head_ + size_ - 1) % capacity_];
    }

    const T& back() const {
        if (empty()) throw std::out_of_range("Deque is empty");
        return buffer_[(head_ + size_ - 1) % capacity_];
    }

    // Modifiers
    void push_back(const T& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        size_t tail_idx = (head_ + size_) % capacity_;
        new (&buffer_[tail_idx]) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        size_t tail_idx = (head_ + size_) % capacity_;
        new (&buffer_[tail_idx]) T(std::move(value));
        ++size_;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        size_t tail_idx = (head_ + size_) % capacity_;
        new (&buffer_[tail_idx]) T(std::forward<Args>(args)...);
        ++size_;
        return buffer_[tail_idx];
    }

    void push_front(const T& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        head_ = (head_ == 0 ? capacity_ - 1 : head_ - 1);
        new (&buffer_[head_]) T(value);
        ++size_;
    }

    void push_front(T&& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        head_ = (head_ == 0 ? capacity_ - 1 : head_ - 1);
        new (&buffer_[head_]) T(std::move(value));
        ++size_;
    }

    void pop_front() {
        if (size_ > 0) {
            buffer_[head_].~T();
            head_ = (head_ + 1) % capacity_;
            --size_;
        }
    }

    void pop_back() {
        if (size_ > 0) {
            size_t tail_idx = (head_ + size_ - 1) % capacity_;
            buffer_[tail_idx].~T();
            --size_;
        }
    }

    void clear() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            size_t idx = (head_ + i) % capacity_;
            buffer_[idx].~T();
        }
        head_ = 0;
        size_ = 0;
    }

    // Iterators
    Iterator begin() noexcept { return Iterator(this, 0); }
    Iterator end() noexcept { return Iterator(this, size_); }
    ConstIterator begin() const noexcept { return ConstIterator(this, 0); }
    ConstIterator end() const noexcept { return ConstIterator(this, size_); }

private:
    void deallocate() {
        if (buffer_) {
            ::operator delete(buffer_);
            buffer_ = nullptr;
        }
    }

    T* buffer_{nullptr};
    size_t head_{0};
    size_t size_{0};
    size_t capacity_{0};
};

} // namespace epfd::dsa

#endif // EPFD_DSA_DEQUE_HPP
