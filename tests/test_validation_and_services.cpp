#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>

using namespace epfd;
using namespace std::chrono_literals;

// ==========================================
// 1. TransactionValidator Unit Tests
// ==========================================
EPFD_TEST(ValidationSuite, ValidTransactionPasses) {
    auto txRepo = std::make_shared<InMemoryTransactionRepository>();
    auto accRepo = std::make_shared<InMemoryAccountRepository>();
    TransactionValidator validator(txRepo, accRepo);

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("d1", "fp1", "192.168.1.1", "Chrome", false, false, loc);
    PaymentMethod pm("pm1", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen Van A", 12, 2028);

    Account acc("acc_val_1", "cust_1", 1000.0, "USD");
    accRepo->save(acc);

    Transaction tx("tx_val_ok", TransactionType::PURCHASE, "cust_1", "m_01", "acc_val_1", 150.0, "USD",
                    std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_01", pm);

    auto res = validator.validate(tx);
    ASSERT_TRUE(res.is_valid);
    ASSERT_EQ(res.error_code, ValidationErrorCode::NONE);
}

EPFD_TEST(ValidationSuite, MissingRequiredIdentifiers) {
    TransactionValidator validator;
    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    // Empty Transaction ID
    Transaction tx1("", TransactionType::PURCHASE, "cust_1", "rec_1", "acc_1", 100.0, "USD",
                    std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m_1", pm);
    auto res1 = validator.validate(tx1);
    ASSERT_FALSE(res1.is_valid);
    ASSERT_EQ(res1.error_code, ValidationErrorCode::MISSING_TRANSACTION_ID);

    // Empty Customer ID
    Transaction tx2("tx2", TransactionType::PURCHASE, "", "rec_1", "acc_1", 100.0, "USD",
                    std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m_1", pm);
    auto res2 = validator.validate(tx2);
    ASSERT_FALSE(res2.is_valid);
    ASSERT_EQ(res2.error_code, ValidationErrorCode::MISSING_CUSTOMER_ID);

    // Empty Account ID
    Transaction tx3("tx3", TransactionType::PURCHASE, "cust_1", "rec_1", "", 100.0, "USD",
                    std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m_1", pm);
    auto res3 = validator.validate(tx3);
    ASSERT_FALSE(res3.is_valid);
    ASSERT_EQ(res3.error_code, ValidationErrorCode::MISSING_ACCOUNT_ID);
}

EPFD_TEST(ValidationSuite, InvalidAmountAndCurrency) {
    TransactionValidator validator;
    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    // Extreme negative amount
    try {
        Transaction tx_neg("tx_neg", TransactionType::PURCHASE, "c1", "r1", "a1", -50.0, "USD",
                            std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
        ASSERT_TRUE(false);
    } catch (const std::invalid_argument&) {
        ASSERT_TRUE(true);
    }

    // Invalid currency code
    Transaction tx_curr("tx_curr", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "US_DOLLAR",
                         std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    auto res = validator.validate(tx_curr);
    ASSERT_FALSE(res.is_valid);
    ASSERT_EQ(res.error_code, ValidationErrorCode::INVALID_CURRENCY);
}

EPFD_TEST(ValidationSuite, TimestampBoundaries) {
    TransactionValidator validator;
    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    auto now = std::chrono::system_clock::now();

    // Future timestamp (+ 1 hour)
    Transaction tx_fut("tx_fut", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "USD",
                       now + 1h, loc, "1.1.1.1", dev, "m1", pm);
    auto res_fut = validator.validate(tx_fut);
    ASSERT_FALSE(res_fut.is_valid);
    ASSERT_EQ(res_fut.error_code, ValidationErrorCode::INVALID_TIMESTAMP);

    // Stale timestamp (60 days old)
    Transaction tx_old("tx_old", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "USD",
                       now - std::chrono::hours(24 * 60), loc, "1.1.1.1", dev, "m1", pm);
    auto res_old = validator.validate(tx_old);
    ASSERT_FALSE(res_old.is_valid);
    ASSERT_EQ(res_old.error_code, ValidationErrorCode::INVALID_TIMESTAMP);
}

EPFD_TEST(ValidationSuite, DuplicateTransactionIdempotency) {
    TransactionValidator validator;
    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    Transaction tx("tx_dup_1", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "USD",
                   std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);

    auto res1 = validator.validate(tx);
    ASSERT_TRUE(res1.is_valid);

    // Re-sending same transaction
    auto res2 = validator.validate(tx);
    ASSERT_FALSE(res2.is_valid);
    ASSERT_EQ(res2.error_code, ValidationErrorCode::DUPLICATE_TRANSACTION);
}

EPFD_TEST(ValidationSuite, AccountStatusAndBalanceValidation) {
    auto accRepo = std::make_shared<InMemoryAccountRepository>();
    TransactionValidator validator(nullptr, accRepo);

    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    // 1. Account not found
    Transaction tx1("tx_acc_1", TransactionType::PURCHASE, "c1", "r1", "acc_unknown", 100.0, "USD",
                    std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    auto res1 = validator.validate(tx1);
    ASSERT_FALSE(res1.is_valid);
    ASSERT_EQ(res1.error_code, ValidationErrorCode::ACCOUNT_NOT_FOUND);

    // 2. Account frozen
    Account frozen_acc("acc_frozen", "c1", 500.0, "USD");
    frozen_acc.freeze();
    accRepo->save(frozen_acc);

    Transaction tx2("tx_acc_2", TransactionType::PURCHASE, "c1", "r1", "acc_frozen", 100.0, "USD",
                    std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    auto res2 = validator.validate(tx2);
    ASSERT_FALSE(res2.is_valid);
    ASSERT_EQ(res2.error_code, ValidationErrorCode::ACCOUNT_FROZEN);

    // 3. Insufficient balance
    Account low_bal_acc("acc_low", "c1", 50.0, "USD");
    accRepo->save(low_bal_acc);

    Transaction tx3("tx_acc_3", TransactionType::PURCHASE, "c1", "r1", "acc_low", 200.0, "USD",
                    std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    auto res3 = validator.validate(tx3);
    ASSERT_FALSE(res3.is_valid);
    ASSERT_EQ(res3.error_code, ValidationErrorCode::INSUFFICIENT_BALANCE);
}

// ==========================================
// 2. TransactionService Orchestration Tests
// ==========================================
EPFD_TEST(ServiceSuite, TransactionServiceFullLifecycle) {
    auto txRepo = std::make_shared<InMemoryTransactionRepository>();
    auto accRepo = std::make_shared<InMemoryAccountRepository>();
    auto validator = std::make_shared<TransactionValidator>(txRepo, accRepo);

    TransactionService service(validator, txRepo, accRepo);

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("d1", "fp1", "192.168.1.1", "Chrome", false, false, loc);
    PaymentMethod pm("pm1", PaymentType::CREDIT_CARD, "4111111111111111", "Le Van C", 12, 2028);

    Account acc("acc_svc_1", "cust_10", 1000.0, "USD");
    accRepo->save(acc);

    // Process valid purchase of $300
    Transaction tx("tx_svc_100", TransactionType::PURCHASE, "cust_10", "m_01", "acc_svc_1", 300.0, "USD",
                    std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_01", pm);

    auto result = service.processTransaction(tx);
    ASSERT_TRUE(result.is_success);
    ASSERT_EQ(result.transaction.getStatus(), TransactionStatus::APPROVED);

    // Check account balance deducted from $1000 to $700
    auto updated_acc = accRepo->findById("acc_svc_1");
    ASSERT_TRUE(updated_acc.has_value());
    ASSERT_EQ(updated_acc->getBalance(), 700.0);

    // Check repository saved
    auto saved_tx = txRepo->findById("tx_svc_100");
    ASSERT_TRUE(saved_tx.has_value());
    ASSERT_EQ(saved_tx->getStatus(), TransactionStatus::APPROVED);

    // Settle transaction
    ASSERT_TRUE(service.settleTransaction("tx_svc_100"));
    ASSERT_EQ(txRepo->findById("tx_svc_100")->getStatus(), TransactionStatus::SETTLED);

    // Dispute transaction
    ASSERT_TRUE(service.disputeTransaction("tx_svc_100"));
    ASSERT_EQ(txRepo->findById("tx_svc_100")->getStatus(), TransactionStatus::DISPUTED);
}
