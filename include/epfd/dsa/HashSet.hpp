#ifndef EPFD_DSA_HASH_SET_HPP
#define EPFD_DSA_HASH_SET_HPP

#include <cstddef>
#include <utility>
#include <functional>
#include <initializer_list>

namespace epfd::dsa {

/**
 * @brief Custom Hash Set implementation with Separate Chaining.
 * @tparam Key Element type.
 * @tparam Hash Hash function object.
 */
template <typename Key, typename Hash = std::hash<Key>>
class HashSet {
public:
    struct Node {
        Key key;
        Node* next{nullptr};
        explicit Node(const Key& k) : key(k) {}
        explicit Node(Key&& k) : key(std::move(k)) {}
    };

    class Iterator {
    public:
        Iterator(HashSet* set, size_t bucket_idx, Node* node)
            : set_(set), bucket_idx_(bucket_idx), node_(node) {
            advanceToValid();
        }

        const Key& operator*() const { return node_->key; }
        const Key* operator->() const { return &(node_->key); }

        Iterator& operator++() {
            if (node_) {
                node_ = node_->next;
            }
            if (!node_) {
                ++bucket_idx_;
                advanceToValid();
            }
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const noexcept {
            return set_ == other.set_ && bucket_idx_ == other.bucket_idx_ && node_ == other.node_;
        }

        bool operator!=(const Iterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        void advanceToValid() {
            if (node_) return;
            if (!set_ || set_->bucket_count_ == 0) return;
            while (bucket_idx_ < set_->bucket_count_) {
                if (set_->buckets_[bucket_idx_]) {
                    node_ = set_->buckets_[bucket_idx_];
                    return;
                }
                ++bucket_idx_;
            }
            node_ = nullptr;
        }

        HashSet* set_{nullptr};
        size_t bucket_idx_{0};
        Node* node_{nullptr};
    };

    class ConstIterator {
    public:
        ConstIterator(const HashSet* set, size_t bucket_idx, const Node* node)
            : set_(set), bucket_idx_(bucket_idx), node_(node) {
            advanceToValid();
        }

        const Key& operator*() const { return node_->key; }
        const Key* operator->() const { return &(node_->key); }

        ConstIterator& operator++() {
            if (node_) {
                node_ = node_->next;
            }
            if (!node_) {
                ++bucket_idx_;
                advanceToValid();
            }
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const ConstIterator& other) const noexcept {
            return set_ == other.set_ && bucket_idx_ == other.bucket_idx_ && node_ == other.node_;
        }

        bool operator!=(const ConstIterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        void advanceToValid() {
            if (node_) return;
            if (!set_ || set_->bucket_count_ == 0) return;
            while (bucket_idx_ < set_->bucket_count_) {
                if (set_->buckets_[bucket_idx_]) {
                    node_ = set_->buckets_[bucket_idx_];
                    return;
                }
                ++bucket_idx_;
            }
            node_ = nullptr;
        }

        const HashSet* set_{nullptr};
        size_t bucket_idx_{0};
        const Node* node_{nullptr};
    };

    HashSet(size_t initial_buckets = 16)
        : bucket_count_(initial_buckets), size_(0), max_load_factor_(0.75f) {
        buckets_ = new Node*[bucket_count_]();
    }

    HashSet(std::initializer_list<Key> init) : HashSet() {
        for (const auto& item : init) {
            insert(item);
        }
    }

    ~HashSet() {
        clear();
        delete[] buckets_;
    }

    // Copy Constructor
    HashSet(const HashSet& other)
        : bucket_count_(other.bucket_count_), size_(0), max_load_factor_(other.max_load_factor_) {
        buckets_ = new Node*[bucket_count_]();
        for (const auto& item : other) {
            insert(item);
        }
    }

    // Copy Assignment
    HashSet& operator=(const HashSet& other) {
        if (this != &other) {
            clear();
            delete[] buckets_;
            bucket_count_ = other.bucket_count_;
            max_load_factor_ = other.max_load_factor_;
            buckets_ = new Node*[bucket_count_]();
            for (const auto& item : other) {
                insert(item);
            }
        }
        return *this;
    }

    // Move Constructor
    HashSet(HashSet&& other) noexcept
        : buckets_(other.buckets_), bucket_count_(other.bucket_count_),
          size_(other.size_), max_load_factor_(other.max_load_factor_) {
        other.buckets_ = nullptr;
        other.bucket_count_ = 0;
        other.size_ = 0;
    }

    // Move Assignment
    HashSet& operator=(HashSet&& other) noexcept {
        if (this != &other) {
            clear();
            delete[] buckets_;
            buckets_ = other.buckets_;
            bucket_count_ = other.bucket_count_;
            size_ = other.size_;
            max_load_factor_ = other.max_load_factor_;
            other.buckets_ = nullptr;
            other.bucket_count_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    size_t size() const noexcept { return size_; }
    size_t bucket_count() const noexcept { return bucket_count_; }
    bool empty() const noexcept { return size_ == 0; }
    float load_factor() const noexcept { return bucket_count_ == 0 ? 0.0f : static_cast<float>(size_) / bucket_count_; }

    std::pair<Iterator, bool> insert(const Key& key) {
        checkAndRehash();
        size_t idx = bucketIndex(key);
        Node* curr = buckets_[idx];
        while (curr) {
            if (curr->key == key) {
                return {Iterator(this, idx, curr), false};
            }
            curr = curr->next;
        }

        Node* new_node = new Node(key);
        new_node->next = buckets_[idx];
        buckets_[idx] = new_node;
        ++size_;
        return {Iterator(this, idx, new_node), true};
    }

    std::pair<Iterator, bool> insert(Key&& key) {
        checkAndRehash();
        size_t idx = bucketIndex(key);
        Node* curr = buckets_[idx];
        while (curr) {
            if (curr->key == key) {
                return {Iterator(this, idx, curr), false};
            }
            curr = curr->next;
        }

        Node* new_node = new Node(std::move(key));
        new_node->next = buckets_[idx];
        buckets_[idx] = new_node;
        ++size_;
        return {Iterator(this, idx, new_node), true};
    }

    Iterator find(const Key& key) {
        if (bucket_count_ == 0) return end();
        size_t idx = bucketIndex(key);
        Node* curr = buckets_[idx];
        while (curr) {
            if (curr->key == key) {
                return Iterator(this, idx, curr);
            }
            curr = curr->next;
        }
        return end();
    }

    ConstIterator find(const Key& key) const {
        if (bucket_count_ == 0) return end();
        size_t idx = bucketIndex(key);
        Node* curr = buckets_[idx];
        while (curr) {
            if (curr->key == key) {
                return ConstIterator(this, idx, curr);
            }
            curr = curr->next;
        }
        return end();
    }

    bool contains(const Key& key) const noexcept {
        return find(key) != end();
    }

    bool erase(const Key& key) {
        if (bucket_count_ == 0) return false;
        size_t idx = bucketIndex(key);
        Node* curr = buckets_[idx];
        Node* prev = nullptr;

        while (curr) {
            if (curr->key == key) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    buckets_[idx] = curr->next;
                }
                delete curr;
                --size_;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    void clear() noexcept {
        if (!buckets_) return;
        for (size_t i = 0; i < bucket_count_; ++i) {
            Node* curr = buckets_[i];
            while (curr) {
                Node* next = curr->next;
                delete curr;
                curr = next;
            }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }

    void rehash(size_t new_bucket_count) {
        if (new_bucket_count <= bucket_count_) return;

        Node** new_buckets = new Node*[new_bucket_count]();
        for (size_t i = 0; i < bucket_count_; ++i) {
            Node* curr = buckets_[i];
            while (curr) {
                Node* next = curr->next;
                size_t new_idx = Hash{}(curr->key) % new_bucket_count;
                curr->next = new_buckets[new_idx];
                new_buckets[new_idx] = curr;
                curr = next;
            }
        }

        delete[] buckets_;
        buckets_ = new_buckets;
        bucket_count_ = new_bucket_count;
    }

    Iterator begin() noexcept { return Iterator(this, 0, nullptr); }
    Iterator end() noexcept { return Iterator(this, bucket_count_, nullptr); }
    ConstIterator begin() const noexcept { return ConstIterator(this, 0, nullptr); }
    ConstIterator end() const noexcept { return ConstIterator(this, bucket_count_, nullptr); }

private:
    size_t bucketIndex(const Key& key) const noexcept {
        return Hash{}(key) % bucket_count_;
    }

    void checkAndRehash() {
        if (load_factor() >= max_load_factor_) {
            rehash(bucket_count_ == 0 ? 16 : bucket_count_ * 2);
        }
    }

    Node** buckets_{nullptr};
    size_t bucket_count_{0};
    size_t size_{0};
    float max_load_factor_{0.75f};
};

} // namespace epfd::dsa

#endif // EPFD_DSA_HASH_SET_HPP
