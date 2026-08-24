#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <fstream>
#include <cstdio>

using namespace epfd;

// ==========================================
// 1. Review Case Lifecycle Suite
// ==========================================
EPFD_TEST(FeedbackSuite, ReviewCaseLifecycle) {
    ReviewCase rc("CASE_001", "tx_100", "c_1", 85.0, DecisionAction::REVIEW, {"Impossible travel detected"});

    ASSERT_EQ(rc.getCaseId(), "CASE_001");
    ASSERT_EQ(rc.getTransactionId(), "tx_100");
    ASSERT_EQ(rc.getCustomerId(), "c_1");
    ASSERT_TRUE(rc.getStatus() == CaseStatus::OPEN);
    ASSERT_FALSE(rc.isResolved());

    // Assign to analyst
    ASSERT_TRUE(rc.assignTo("ANALYST_BOB"));
    ASSERT_TRUE(rc.getStatus() == CaseStatus::IN_INVESTIGATION);
    ASSERT_EQ(rc.getAssignedAnalystId(), "ANALYST_BOB");

    // Add evidence and note
    rc.addEvidence("Cardholder reported phone stolen");
    rc.addNote("Contacted customer via phone; confirmed fraudulent transaction");
    ASSERT_EQ(rc.getEvidence().size(), 2);
    ASSERT_EQ(rc.getNotes().size(), 1);

    // Resolve as confirmed fraud
    ASSERT_TRUE(rc.resolve(CaseStatus::RESOLVED_CONFIRMED_FRAUD, "Stolen device account takeover"));
    ASSERT_TRUE(rc.getStatus() == CaseStatus::RESOLVED_CONFIRMED_FRAUD);
    ASSERT_TRUE(rc.isResolved());
    ASSERT_EQ(rc.getResolutionSummary(), "Stolen device account takeover");

    // Close case
    ASSERT_TRUE(rc.close());
    ASSERT_TRUE(rc.getStatus() == CaseStatus::CLOSED);
}

// ==========================================
// 2. Outcome Tracker Suite
// ==========================================
EPFD_TEST(FeedbackSuite, OutcomeTrackerEventsAndMetrics) {
    OutcomeTracker tracker;

    tracker.recordOutcome("EVT_1", "tx_101", OutcomeType::CHARGEBACK_RECEIVED, 350.0, "Fraud dispute from issuer");
    tracker.recordOutcome("EVT_2", "tx_102", OutcomeType::CUSTOMER_CONFIRMED_LEGIT, 50.0, "SMS OTP confirmed");
    tracker.recordOutcome("EVT_3", "tx_101", OutcomeType::MERCHANT_REFUND, 350.0, "Refund processed");

    ASSERT_EQ(tracker.totalCount(), 3);
    ASSERT_EQ(tracker.getChargebackCount(), 1);
    ASSERT_EQ(tracker.getLegitConfirmationCount(), 1);
    ASSERT_NEAR(tracker.getTotalChargebackLoss(), 350.0, 0.01);

    auto tx101_events = tracker.getOutcomesForTransaction("tx_101");
    ASSERT_EQ(tx101_events.size(), 2);
}

// ==========================================
// 3. Label Store Ground-Truth Suite
// ==========================================
EPFD_TEST(FeedbackSuite, LabelStoreGroundTruthStorageAndCsvExport) {
    LabelStore store;

    TransactionFeatures f1;
    f1.transaction_amount = 50.0;
    f1.hour_of_day = 14.0;
    GroundTruthRecord lbl1("tx_clean", GroundTruthLabel::LEGITIMATE, LabelSource::CUSTOMER_DISPUTE, 1.0, f1, "Customer verified");

    TransactionFeatures f2;
    f2.transaction_amount = 8900.0;
    f2.hour_of_day = 3.0;
    GroundTruthRecord lbl2("tx_fraud", GroundTruthLabel::FRAUD, LabelSource::CHARGEBACK_CONFIRMED, 1.0, f2, "Chargeback confirmed");

    ASSERT_TRUE(store.storeLabel(lbl1));
    ASSERT_TRUE(store.storeLabel(lbl2));

    ASSERT_EQ(store.size(), 2);
    ASSERT_EQ(store.getFraudCount(), 1);
    ASSERT_EQ(store.getLegitCount(), 1);

    auto found = store.getLabel("tx_fraud");
    ASSERT_TRUE(found.has_value());
    ASSERT_TRUE(found.value().label == GroundTruthLabel::FRAUD);
    ASSERT_TRUE(found.value().source == LabelSource::CHARGEBACK_CONFIRMED);

    // Export dataset to CSV
    const std::string export_csv = "test_labeled_export.csv";
    std::remove(export_csv.c_str());

    ASSERT_TRUE(store.exportLabeledDatasetCsv(export_csv));

    std::ifstream ifs(export_csv);
    ASSERT_TRUE(ifs.is_open());
    std::string header;
    ASSERT_TRUE(std::getline(ifs, header));
    ASSERT_TRUE(header.find("transaction_amount") != std::string::npos);
    ASSERT_TRUE(header.find("is_fraud") != std::string::npos);

    std::remove(export_csv.c_str());
}

// ==========================================
// 4. Case Manager End-to-End Suite
// ==========================================
EPFD_TEST(FeedbackSuite, CaseManagerEndToEndInvestigation) {
    auto label_store = std::make_shared<LabelStore>();
    auto outcome_tracker = std::make_shared<OutcomeTracker>();
    CaseManager case_mgr(label_store, outcome_tracker);

    // Create review case
    std::string case_id = case_mgr.createCase("tx_eval_1", "c_eval_1", 92.0, DecisionAction::REVIEW,
                                              {"High velocity burst", "New foreign country"});

    ASSERT_FALSE(case_id.empty());
    ASSERT_EQ(case_mgr.totalCases(), 1);
    ASSERT_EQ(case_mgr.getOpenCaseCount(), 1);
    ASSERT_EQ(case_mgr.getResolvedCaseCount(), 0);

    // Assign to analyst
    ASSERT_TRUE(case_mgr.assignCase(case_id, "ANALYST_SARAH"));
    ASSERT_TRUE(case_mgr.addNote(case_id, "Verified with card issuing bank: card reported compromised"));

    // Resolve as confirmed fraud
    TransactionFeatures f;
    f.transaction_amount = 4500.0;
    f.transactions_last_1hour = 5.0;

    ASSERT_TRUE(case_mgr.resolveCase(case_id, CaseStatus::RESOLVED_CONFIRMED_FRAUD,
                                     "Confirmed fraud by issuer", f));

    ASSERT_EQ(case_mgr.getOpenCaseCount(), 0);
    ASSERT_EQ(case_mgr.getResolvedCaseCount(), 1);

    // Check automatic label creation in LabelStore
    ASSERT_EQ(label_store->size(), 1);
    ASSERT_EQ(label_store->getFraudCount(), 1);

    auto lbl = label_store->getLabel("tx_eval_1");
    ASSERT_TRUE(lbl.has_value());
    ASSERT_TRUE(lbl.value().label == GroundTruthLabel::FRAUD);
    ASSERT_TRUE(lbl.value().source == LabelSource::ANALYST_INVESTIGATION);
}
