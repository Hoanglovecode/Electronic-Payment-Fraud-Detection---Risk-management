#ifndef EPFD_MODELS_TRANSACTION_HPP
#define EPFD_MODELS_TRANSACTION_HPP

#include <string>
#include <chrono>
#include "epfd/common/Types.hpp"
#include "epfd/models/Location.hpp"
#include "epfd/models/Device.hpp"
#include "epfd/models/PaymentMethod.hpp"

namespace epfd {

class Transaction {
public:
    Transaction() = default;
    Transaction(std::string transaction_id,
                TransactionType type,
                std::string customer_id,
                std::string recipient_id,
                std::string account_id,
                double amount,
                std::string currency,
                Timestamp timestamp,
                Location location,
                std::string ip_address,
                Device device,
                std::string merchant_id,
                PaymentMethod payment_method);

    // Getters
    const std::string& getTransactionId() const noexcept { return transaction_id_; }
    TransactionType getType() const noexcept { return type_; }
    const std::string& getCustomerId() const noexcept { return customer_id_; }
    const std::string& getRecipientId() const noexcept { return recipient_id_; }
    const std::string& getAccountId() const noexcept { return account_id_; }
    double getAmount() const noexcept { return amount_; }
    const std::string& getCurrency() const noexcept { return currency_; }
    Timestamp getTimestamp() const noexcept { return timestamp_; }
    const Location& getLocation() const noexcept { return location_; }
    const std::string& getIpAddress() const noexcept { return ip_address_; }
    const Device& getDevice() const noexcept { return device_; }
    const std::string& getMerchantId() const noexcept { return merchant_id_; }
    const PaymentMethod& getPaymentMethod() const noexcept { return payment_method_; }
    TransactionStatus getStatus() const noexcept { return status_; }
    Timestamp getCreatedAt() const noexcept { return created_at_; }
    Timestamp getUpdatedAt() const noexcept { return updated_at_; }

    // Setters / State updates
    void setStatus(TransactionStatus new_status);
    void setAmount(double amount);
    void setCurrency(std::string currency) { currency_ = std::move(currency); }
    void setLocation(Location loc) { location_ = std::move(loc); }
    void setDevice(Device dev) { device_ = std::move(dev); }

    // Lifecycle state transitions
    bool markApproved();
    bool markReview();
    bool markChallenged();
    bool markRejected();
    bool markSettled();
    bool markChargeback();
    bool markDisputed();

    // Domain helpers
    bool isCrossBorder(const std::string& home_country) const noexcept;
    bool isHighAmount(double threshold) const noexcept;

    std::string toString() const;

private:
    std::string transaction_id_;
    TransactionType type_{TransactionType::PURCHASE};
    std::string customer_id_;
    std::string recipient_id_;
    std::string account_id_;
    double amount_{0.0};
    std::string currency_{"USD"};
    Timestamp timestamp_{std::chrono::system_clock::now()};
    Location location_;
    std::string ip_address_;
    Device device_;
    std::string merchant_id_;
    PaymentMethod payment_method_;
    TransactionStatus status_{TransactionStatus::PENDING};
    Timestamp created_at_{std::chrono::system_clock::now()};
    Timestamp updated_at_{std::chrono::system_clock::now()};
};

} // namespace epfd

#endif // EPFD_MODELS_TRANSACTION_HPP
