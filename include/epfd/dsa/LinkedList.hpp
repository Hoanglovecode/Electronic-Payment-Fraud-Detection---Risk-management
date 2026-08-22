#ifndef EPFD_DSA_LINKED_LIST_HPP
#define EPFD_DSA_LINKED_LIST_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <initializer_list>

namespace epfd::dsa {

/**
 * @brief Custom Doubly Linked List template class.
 */
template <typename T>
class LinkedList {
private:
    struct Node {
        T data;
        Node* prev{nullptr};
        Node* next{nullptr};

        template <typename... Args>
        Node(Args&&... args) : data(std::forward<Args>(args)...) {}
    };

public:
    class Iterator {
    public:
        explicit Iterator(Node* node) : node_(node) {}

        T& operator*() { return node_->data; }
        T* operator->() { return &(node_->data); }

        Iterator& operator++() {
            if (node_) node_ = node_->next;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator& operator--() {
            if (node_) node_ = node_->prev;
            return *this;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const noexcept { return node_ == other.node_; }
        bool operator!=(const Iterator& other) const noexcept { return node_ != other.node_; }

        Node* getNode() const noexcept { return node_; }

    private:
        Node* node_;
    };

    class ConstIterator {
    public:
        explicit ConstIterator(const Node* node) : node_(node) {}

        const T& operator*() const { return node_->data; }
        const T* operator->() const { return &(node_->data); }

        ConstIterator& operator++() {
            if (node_) node_ = node_->next;
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const ConstIterator& other) const noexcept { return node_ == other.node_; }
        bool operator!=(const ConstIterator& other) const noexcept { return node_ != other.node_; }

    private:
        const Node* node_;
    };

    LinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

    LinkedList(std::initializer_list<T> init) : LinkedList() {
        for (const auto& val : init) {
            push_back(val);
        }
    }

    ~LinkedList() {
        clear();
    }

    // Copy Constructor
    LinkedList(const LinkedList& other) : LinkedList() {
        for (const auto& item : other) {
            push_back(item);
        }
    }

    // Copy Assignment
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            clear();
            for (const auto& item : other) {
                push_back(item);
            }
        }
        return *this;
    }

    // Move Constructor
    LinkedList(LinkedList&& other) noexcept 
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    // Move Assignment
    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    T& front() {
        if (empty()) throw std::out_of_range("LinkedList is empty");
        return head_->data;
    }
    const T& front() const {
        if (empty()) throw std::out_of_range("LinkedList is empty");
        return head_->data;
    }

    T& back() {
        if (empty()) throw std::out_of_range("LinkedList is empty");
        return tail_->data;
    }
    const T& back() const {
        if (empty()) throw std::out_of_range("LinkedList is empty");
        return tail_->data;
    }

    void push_back(const T& value) {
        Node* node = new Node(value);
        if (empty()) {
            head_ = tail_ = node;
        } else {
            tail_->next = node;
            node->prev = tail_;
            tail_ = node;
        }
        ++size_;
    }

    void push_back(T&& value) {
        Node* node = new Node(std::move(value));
        if (empty()) {
            head_ = tail_ = node;
        } else {
            tail_->next = node;
            node->prev = tail_;
            tail_ = node;
        }
        ++size_;
    }

    void push_front(const T& value) {
        Node* node = new Node(value);
        if (empty()) {
            head_ = tail_ = node;
        } else {
            node->next = head_;
            head_->prev = node;
            head_ = node;
        }
        ++size_;
    }

    void push_front(T&& value) {
        Node* node = new Node(std::move(value));
        if (empty()) {
            head_ = tail_ = node;
        } else {
            node->next = head_;
            head_->prev = node;
            head_ = node;
        }
        ++size_;
    }

    void pop_back() {
        if (empty()) return;
        Node* old_tail = tail_;
        if (head_ == tail_) {
            head_ = tail_ = nullptr;
        } else {
            tail_ = tail_->prev;
            tail_->next = nullptr;
        }
        delete old_tail;
        --size_;
    }

    void pop_front() {
        if (empty()) return;
        Node* old_head = head_;
        if (head_ == tail_) {
            head_ = tail_ = nullptr;
        } else {
            head_ = head_->next;
            head_->prev = nullptr;
        }
        delete old_head;
        --size_;
    }

    void clear() noexcept {
        Node* curr = head_;
        while (curr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
        head_ = tail_ = nullptr;
        size_ = 0;
    }

    Iterator begin() noexcept { return Iterator(head_); }
    Iterator end() noexcept { return Iterator(nullptr); }
    ConstIterator begin() const noexcept { return ConstIterator(head_); }
    ConstIterator end() const noexcept { return ConstIterator(nullptr); }

private:
    Node* head_{nullptr};
    Node* tail_{nullptr};
    size_t size_{0};
};

} // namespace epfd::dsa

#endif // EPFD_DSA_LINKED_LIST_HPP
