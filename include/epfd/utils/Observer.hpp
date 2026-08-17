#ifndef EPFD_UTILS_OBSERVER_HPP
#define EPFD_UTILS_OBSERVER_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include <mutex>

namespace epfd {

/**
 * @brief Generic Observer Pattern for decoupled event handling.
 */
template <typename Event>
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void onNotify(const Event& event) = 0;
};

template <typename Event>
class Observable {
public:
    virtual ~Observable() = default;

    void subscribe(std::shared_ptr<IObserver<Event>> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (observer && std::find(observers_.begin(), observers_.end(), observer) == observers_.end()) {
            observers_.push_back(observer);
        }
    }

    void unsubscribe(std::shared_ptr<IObserver<Event>> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
    }

    void notifyAll(const Event& event) {
        std::vector<std::shared_ptr<IObserver<Event>>> copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            copy = observers_;
        }
        for (auto& obs : copy) {
            if (obs) {
                obs->onNotify(event);
            }
        }
    }

    size_t observerCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return observers_.size();
    }

private:
    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<IObserver<Event>>> observers_;
};

} // namespace epfd

#endif // EPFD_UTILS_OBSERVER_HPP
