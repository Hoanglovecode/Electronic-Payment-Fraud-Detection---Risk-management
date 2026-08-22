#ifndef EPFD_DSA_VECTOR_HPP
#define EPFD_DSA_VECTOR_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <initializer_list>
#include <algorithm>

namespace epfd::dsa {

/**
 * @brief Custom Generic Dynamic Array (Vector) implementation from scratch.
 * @tparam T Element type.
 */
template <typename T>
class Vector {
public:
    using ValueType = T;
    using Iterator = T*;
    using ConstIterator = const T*;

    Vector() : data_(nullptr), size_(0), capacity_(0) {}

    explicit Vector(size_t count, const T& value = T()) : Vector() {
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    Vector(std::initializer_list<T> init) : Vector() {
        reserve(init.size());
        for (const auto& item : init) {
            push_back(item);
        }
    }

    ~Vector() {
        clear();
        deallocate();
    }

    // Copy Constructor
    Vector(const Vector& other) : Vector() {
        reserve(other.size_);
        for (size_t i = 0; i < other.size_; ++i) {
            new (&data_[i]) T(other.data_[i]);
        }
        size_ = other.size_;
    }

    // Copy Assignment
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            clear();
            reserve(other.size_);
            for (size_t i = 0; i < other.size_; ++i) {
                new (&data_[i]) T(other.data_[i]);
            }
            size_ = other.size_;
        }
        return *this;
    }

    // Move Constructor
    Vector(Vector&& other) noexcept 
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move Assignment
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            clear();
            deallocate();
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // Capacity & Size
    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    void reserve(size_t new_cap) {
        if (new_cap <= capacity_) return;

        T* new_data = static_cast<T*>(::operator new(new_cap * sizeof(T)));
        for (size_t i = 0; i < size_; ++i) {
            new (&new_data[i]) T(std::move(data_[i]));
            data_[i].~T();
        }
        deallocate();
        data_ = new_data;
        capacity_ = new_cap;
    }

    void shrink_to_fit() {
        if (size_ == capacity_) return;
        if (size_ == 0) {
            deallocate();
            data_ = nullptr;
            capacity_ = 0;
            return;
        }
        T* new_data = static_cast<T*>(::operator new(size_ * sizeof(T)));
        for (size_t i = 0; i < size_; ++i) {
            new (&new_data[i]) T(std::move(data_[i]));
            data_[i].~T();
        }
        deallocate();
        data_ = new_data;
        capacity_ = size_;
    }

    // Element Access
    T& operator[](size_t index) noexcept { return data_[index]; }
    const T& operator[](size_t index) const noexcept { return data_[index]; }

    T& at(size_t index) {
        if (index >= size_) throw std::out_of_range("Vector index out of range");
        return data_[index];
    }
    const T& at(size_t index) const {
        if (index >= size_) throw std::out_of_range("Vector index out of range");
        return data_[index];
    }

    T& front() {
        if (empty()) throw std::out_of_range("Vector is empty");
        return data_[0];
    }
    const T& front() const {
        if (empty()) throw std::out_of_range("Vector is empty");
        return data_[0];
    }

    T& back() {
        if (empty()) throw std::out_of_range("Vector is empty");
        return data_[size_ - 1];
    }
    const T& back() const {
        if (empty()) throw std::out_of_range("Vector is empty");
        return data_[size_ - 1];
    }

    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    // Modifiers
    void push_back(const T& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        new (&data_[size_]) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        new (&data_[size_]) T(std::move(value));
        ++size_;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        new (&data_[size_]) T(std::forward<Args>(args)...);
        return data_[size_++];
    }

    void pop_back() {
        if (size_ > 0) {
            --size_;
            data_[size_].~T();
        }
    }

    void clear() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    Iterator insert(ConstIterator pos, const T& value) {
        size_t index = pos - data_;
        if (index > size_) throw std::out_of_range("Insert position out of range");

        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        }

        for (size_t i = size_; i > index; --i) {
            new (&data_[i]) T(std::move(data_[i - 1]));
            data_[i - 1].~T();
        }
        new (&data_[index]) T(value);
        ++size_;
        return data_ + index;
    }

    Iterator erase(ConstIterator pos) {
        size_t index = pos - data_;
        if (index >= size_) throw std::out_of_range("Erase position out of range");

        data_[index].~T();
        for (size_t i = index; i + 1 < size_; ++i) {
            new (&data_[i]) T(std::move(data_[i + 1]));
            data_[i + 1].~T();
        }
        --size_;
        return data_ + index;
    }

    // Iterators
    Iterator begin() noexcept { return data_; }
    Iterator end() noexcept { return data_ + size_; }
    ConstIterator begin() const noexcept { return data_; }
    ConstIterator end() const noexcept { return data_ + size_; }
    ConstIterator cbegin() const noexcept { return data_; }
    ConstIterator cend() const noexcept { return data_ + size_; }

private:
    void deallocate() {
        if (data_) {
            ::operator delete(data_);
            data_ = nullptr;
        }
    }

    T* data_{nullptr};
    size_t size_{0};
    size_t capacity_{0};
};

} // namespace epfd::dsa

#endif // EPFD_DSA_VECTOR_HPP
