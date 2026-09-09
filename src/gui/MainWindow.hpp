#ifndef EPFD_GUI_MAIN_WINDOW_HPP
#define EPFD_GUI_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QTabWidget>
#include <QTimer>
#include <QLabel>
#include <QToolBar>
#include <memory>
#include "epfd/epfd.hpp"
#include "TransactionStreamWidget.hpp"
#include "SimulatorWidget.hpp"
#include "CaseManagementWidget.hpp"
#include "BenchmarkWidget.hpp"

namespace epfd::gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onProcessSingleTransaction(const Transaction& tx);
    void onTriggerScenarioBatch(SimulationScenario scenario, size_t count);
    void onStartContinuousStream(int interval_ms);
    void onPauseContinuousStream();
    void onStreamTimerTick();
    void onOpenCaseRequested(const QString& tx_id, const QString& cust_id, double risk_score, int decision_action);

private:
    void initCoreEngine();
    void setupUi();
    void updateKpiBanner();

    // Core Services & Engines
    std::shared_ptr<InMemoryTransactionRepository> tx_repo_;
    std::shared_ptr<InMemoryCustomerRepository> cust_repo_;
    std::shared_ptr<InMemoryAccountRepository> acc_repo_;
    std::shared_ptr<TransactionValidator> validator_;
    std::shared_ptr<TransactionService> service_;

    std::shared_ptr<FastLookupIndex> lookup_index_;
    std::shared_ptr<FraudDetectorEngine> fraud_engine_;
    std::shared_ptr<FeatureExtractor> extractor_;
    std::shared_ptr<StandardWeightedRiskPolicy> risk_policy_;
    std::shared_ptr<RiskAggregator> risk_aggregator_;
    std::shared_ptr<MockModelPredictor> ml_predictor_;
    std::shared_ptr<RiskEngine> risk_engine_;
    std::shared_ptr<DecisionEngine> decision_engine_;

    std::shared_ptr<LabelStore> label_store_;
    std::shared_ptr<OutcomeTracker> outcome_tracker_;

    TransactionSimulator simulator_;
    QTimer* stream_timer_{nullptr};

    // UI Components
    QTabWidget* tabWidget_{nullptr};
    TransactionStreamWidget* streamWidget_{nullptr};
    SimulatorWidget* simulatorWidget_{nullptr};
    CaseManagementWidget* caseWidget_{nullptr};
    BenchmarkWidget* benchmarkWidget_{nullptr};

    // KPI Banner Labels
    QLabel* lblTotalTx_{nullptr};
    QLabel* lblApprovedTx_{nullptr};
    QLabel* lblBlockedTx_{nullptr};
    QLabel* lblBlockedLoss_{nullptr};

    size_t total_processed_{0};
    size_t total_approved_{0};
    size_t total_blocked_{0};
    double total_blocked_amount_{0.0};
};

} // namespace epfd::gui

#endif // EPFD_GUI_MAIN_WINDOW_HPP
