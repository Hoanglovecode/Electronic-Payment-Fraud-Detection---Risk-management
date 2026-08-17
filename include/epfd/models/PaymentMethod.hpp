#ifndef EPFD_MODELS_PAYMENT_METHOD_HPP
#define EPFD_MODELS_PAYMENT_METHOD_HPP

#include <string>
#include "epfd/common/Types.hpp"

namespace epfd {

class PaymentMethod {
public:
    PaymentMethod() = default;

    /**
     * @brief Safe constructor that automatically masks PAN and extracts BIN / Last4.
     * Raw CVV is never stored.
     */
    PaymentMethod(std::string payment_id,
                  PaymentType type,
                  const std::string& raw_or_masked_pan,
                  std::string card_holder_name,
                  int expiry_month,
                  int expiry_year,
                  std::string billing_country = "");

    // Factory method for non-card methods
    static PaymentMethod createBankTransfer(std::string payment_id, std::string account_number, std::string bank_name);
    static PaymentMethod createEWallet(std::string payment_id, std::string wallet_id, std::string provider);

    // Getters
    const std::string& getPaymentId() const noexcept { return payment_id_; }
    PaymentType getType() const noexcept { return type_; }
    const std::string& getMaskedCardNumber() const noexcept { return masked_card_number_; }
    const std::string& getCardBin() const noexcept { return card_bin_; }
    const std::string& getLast4() const noexcept { return last4_; }
    const std::string& getCardHolderName() const noexcept { return card_holder_name_; }
    int getExpiryMonth() const noexcept { return expiry_month_; }
    int getExpiryYear() const noexcept { return expiry_year_; }
    const std::string& getBillingCountry() const noexcept { return billing_country_; }

    // Domain validation & helpers
    bool isCard() const noexcept;
    bool isExpired(int current_year, int current_month) const noexcept;
    static bool validateLuhn(const std::string& raw_pan) noexcept;
    static std::string maskPan(const std::string& raw_pan);

    std::string toString() const;

private:
    std::string payment_id_;
    PaymentType type_{PaymentType::CREDIT_CARD};
    std::string masked_card_number_;
    std::string card_bin_;
    std::string last4_;
    std::string card_holder_name_;
    int expiry_month_{0};
    int expiry_year_{0};
    std::string billing_country_;
};

} // namespace epfd

#endif // EPFD_MODELS_PAYMENT_METHOD_HPP
