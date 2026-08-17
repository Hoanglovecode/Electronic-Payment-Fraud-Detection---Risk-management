#include "test_framework.hpp"
#include "epfd/common/Types.hpp"
#include "epfd/common/Constants.hpp"

EPFD_TEST(ArchitectureSuite, CommonTypesStringConversion) {
    using namespace epfd;
    ASSERT_EQ(toString(TransactionType::PURCHASE), "PURCHASE");
    ASSERT_EQ(toString(TransactionStatus::APPROVED), "APPROVED");
    ASSERT_EQ(toString(PaymentType::CREDIT_CARD), "CREDIT_CARD");
    ASSERT_EQ(toString(RiskLevel::CRITICAL), "CRITICAL");
    ASSERT_EQ(toString(DecisionAction::BLOCK), "BLOCK");
    ASSERT_EQ(toString(CaseStatus::OPEN), "OPEN");
    ASSERT_EQ(toString(GroundTruthLabel::FRAUD), "FRAUD");
}

EPFD_TEST(ArchitectureSuite, ConstantsSanityCheck) {
    using namespace epfd::constants;
    ASSERT_TRUE(RISK_SCORE_MIN == 0.0);
    ASSERT_TRUE(RISK_SCORE_MAX == 100.0);
    ASSERT_TRUE(THRESHOLD_VERY_LOW_MAX < THRESHOLD_LOW_MAX);
    ASSERT_TRUE(THRESHOLD_LOW_MAX < THRESHOLD_MEDIUM_MAX);
    ASSERT_TRUE(THRESHOLD_MEDIUM_MAX < THRESHOLD_HIGH_MAX);
}
