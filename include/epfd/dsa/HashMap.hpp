#ifndef EPFD_DSA_HASH_MAP_HPP
#define EPFD_DSA_HASH_MAP_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <functional>
#include <initializer_list>

namespace epfd::dsa {

/**
 * @brief Custom Hash Table implementation using Separate Chaining.
 * Automatic dynamic rehashing when load factor >= 0.75.
 * @tparam Key Key type.
 * @tparam Value Value type.
 * @tparam Hash Hash function object.
 */
template <typename Key, typename Value, typename Hash = std::hash<Key>>
class HashMap {
public:
    struct Entry {
        Key first;
        Value second;
        Entry* next{nullptr};

        Entry(const Key& k, const Value& v) : first(k), second(v) {}
        Entry(Key&& k, Value&& v) : first(std::move(k)), second(std::move(v)) {}
        template <typename... Args>
        Entry(const Key& k, Args&&... args) : first(k), second(std::forward<Args>(args)...) {}
    };

    class Iterator {
    public:
        Iterator(HashMap* map, size_t bucket_idx, Entry* entry)
            : map_(map), bucket_idx_(bucket_idx), entry_(entry) {
            advanceToValid();
        }

        Entry& operator*() { return *entry_; }
        Entry* operator->() { return entry_; }

        Iterator& operator++() {
            if (entry_) {
                entry_ = entry_->next;
            }
            if (!entry_) {
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
            return map_ == other.map_ && bucket_idx_ == other.bucket_idx_ && entry_ == other.entry_;
        }

        bool operator!=(const Iterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        void advanceToValid() {
            if (entry_) return;
            if (!map_ || map_->bucket_count_ == 0) return;
            while (bucket_idx_ < map_->bucket_count_) {
                if (map_->buckets_[bucket_idx_]) {
                    entry_ = map_->buckets_[bucket_idx_];
                    return;
                }
                ++bucket_idx_;
            }
            entry_ = nullptr;
        }

        HashMap* map_{nullptr};
        size_t bucket_idx_{0};
        Entry* entry_{nullptr};
    };

    class ConstIterator {
    public:
        ConstIterator(const HashMap* map, size_t bucket_idx, const Entry* entry)
            : map_(map), bucket_idx_(bucket_idx), entry_(entry) {
            advanceToValid();
        }

        const Entry& operator*() const { return *entry_; }
        const Entry* operator->() const { return entry_; }

        ConstIterator& operator++() {
            if (entry_) {
                entry_ = entry_->next;
            }
            if (!entry_) {
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
            return map_ == other.map_ && bucket_idx_ == other.bucket_idx_ && entry_ == other.entry_;
        }

        bool operator!=(const ConstIterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        void advanceToValid() {
            if (entry_) return;
            if (!map_ || map_->bucket_count_ == 0) return;
            while (bucket_idx_ < map_->bucket_count_) {
                if (map_->buckets_[bucket_idx_]) {
                    entry_ = map_->buckets_[bucket_idx_];
                    return;
                }
                ++bucket_idx_;
            }
            entry_ = nullptr;
        }

        const HashMap* map_{nullptr};
        size_t bucket_idx_{0};
        const Entry* entry_{nullptr};
    };

    HashMap(size_t initial_buckets = 16)
        : bucket_count_(initial_buckets), size_(0), max_load_factor_(0.75f) {
        buckets_ = new Entry*[bucket_count_]();
    }

    HashMap(std::initializer_list<std::pair<Key, Value>> init) : HashMap() {
        for (const auto& item : init) {
            insert(item.first, item.second);
        }
    }

    ~HashMap() {
        clear();
        delete[] buckets_;
    }

    // Copy Constructor
    HashMap(const HashMap& other)
        : bucket_count_(other.bucket_count_), size_(0), max_load_factor_(other.max_load_factor_) {
        buckets_ = new Entry*[bucket_count_]();
        for (const auto& item : other) {
            insert(item.first, item.second);
        }
    }

    // Copy Assignment
    HashMap& operator=(const HashMap& other) {
        if (this != &other) {
            clear();
            delete[] buckets_;
            bucket_count_ = other.bucket_count_;
            max_load_factor_ = other.max_load_factor_;
            buckets_ = new Entry*[bucket_count_]();
            for (const auto& item : other) {
                insert(item.first, item.second);
            }
        }
        return *this;
    }

    // Move Constructor
    HashMap(HashMap&& other) noexcept
        : buckets_(other.buckets_), bucket_count_(other.bucket_count_),
          size_(other.size_), max_load_factor_(other.max_load_factor_) {
        other.buckets_ = nullptr;
        other.bucket_count_ = 0;
        other.size_ = 0;
    }

    // Move Assignment
    HashMap& operator=(HashMap&& other) noexcept {
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

    std::pair<Iterator, bool> insert(const Key& key, const Value& value) {
        checkAndRehash();
        size_t idx = bucketIndex(key);
        Entry* curr = buckets_[idx];
        while (curr) {
            if (curr->first == key) {
                return {Iterator(this, idx, curr), false};
            }
            curr = curr->next;
        }

        Entry* new_entry = new Entry(key, value);
        new_entry->next = buckets_[idx];
        buckets_[idx] = new_entry;
        ++size_;
        return {Iterator(this, idx, new_entry), true};
    }

    std::pair<Iterator, bool> insert(Key&& key, Value&& value) {
        checkAndRehash();
        size_t idx = bucketIndex(key);
        Entry* curr = buckets_[idx];
        while (curr) {
            if (curr->first == key) {
                return {Iterator(this, idx, curr), false};
            }
            curr = curr->next;
        }

        Entry* new_entry = new Entry(std::move(key), std::move(value));
        new_entry->next = buckets_[idx];
        buckets_[idx] = new_entry;
        ++size_;
        return {Iterator(this, idx, new_entry), true};
    }

    template <typename... Args>
    std::pair<Iterator, bool> emplace(const Key& key, Args&&... args) {
        checkAndRehash();
        size_t idx = bucketIndex(key);
        Entry* curr = buckets_[idx];
        while (curr) {
            if (curr->first == key) {
                return {Iterator(this, idx, curr), false};
            }
            curr = curr->next;
        }

        Entry* new_entry = new Entry(key, std::forward<Args>(args)...);
        new_entry->next = buckets_[idx];
        buckets_[idx] = new_entry;
        ++size_;
        return {Iterator(this, idx, new_entry), true};
    }

    Value& operator[](const Key& key) {
        checkAndRehash();
        size_t idx = bucketIndex(key);
        Entry* curr = buckets_[idx];
        while (curr) {
            if (curr->first == key) {
                return curr->second;
            }
            curr = curr->next;
        }

        Entry* new_entry = new Entry(key, Value());
        new_entry->next = buckets_[idx];
        buckets_[idx] = new_entry;
        ++size_;
        return new_entry->second;
    }

    Value& at(const Key& key) {
        size_t idx = bucketIndex(key);
        Entry* curr = buckets_[idx];
        while (curr) {
            if (curr->first == key) {
                return curr->second;
            }
            curr = curr->next;
        }
        throw std::out_of_range("HashMap key not found");
    }

    const Value& at(const Key& key) const {
        size_t idx = bucketIndex(key);
        Entry* curr = buckets_[idx];
        while (curr) {
            if (curr->first == key) {
                return curr->second;
            }
            curr = curr->next;
        }
        throw std::out_of_range("HashMap key not found");
    }

    Iterator find(const Key& key) {
        if (bucket_count_ == 0) return end();
        size_t idx = bucketIndex(key);
        Entry* curr = buckets_[idx];
        while (curr) {
            if (curr->first == key) {
                return Iterator(this, idx, curr);
            }
            curr = curr->next;
        }
        return end();
    }

    ConstIterator find(const Key& key) const {
        if (bucket_count_ == 0) return end();
        size_t idx = bucketIndex(key);
        Entry* curr = buckets_[idx];
        while (curr) {
            if (curr->first == key) {
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
        Entry* curr = buckets_[idx];
        Entry* prev = nullptr;

        while (curr) {
            if (curr->first == key) {
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
            Entry* curr = buckets_[i];
            while (curr) {
                Entry* next = curr->next;
                delete curr;
                curr = next;
            }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }

    void rehash(size_t new_bucket_count) {
        if (new_bucket_count <= bucket_count_) return;

        Entry** new_buckets = new Entry*[new_bucket_count]();
        for (size_t i = 0; i < bucket_count_; ++i) {
            Entry* curr = buckets_[i];
            while (curr) {
                Entry* next = curr->next;
                size_t new_idx = Hash{}(curr->first) % new_bucket_count;
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

    Entry** buckets_{nullptr};
    size_t bucket_count_{0};
    size_t size_{0};
    float max_load_factor_{0.75f};
};

} // namespace epfd::dsa

#endif // EPFD_DSA_HASH_MAP_HPP
