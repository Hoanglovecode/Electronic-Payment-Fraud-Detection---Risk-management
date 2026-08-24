#include "MainWindow.hpp"
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

namespace epfd::gui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System");
    resize(1200, 780);

    initCoreEngine();
    setupUi();

    stream_timer_ = new QTimer(this);
    connect(stream_timer_, &QTimer::timeout, this, &MainWindow::onStreamTimerTick);
}

void MainWindow::initCoreEngine() {
    tx_repo_ = std::make_shared<InMemoryTransactionRepository>();
    cust_repo_ = std::make_shared<InMemoryCustomerRepository>();
    acc_repo_ = std::make_shared<InMemoryAccountRepository>();
    validator_ = std::make_shared<TransactionValidator>(tx_repo_, acc_repo_);
    service_ = std::make_shared<TransactionService>(validator_, tx_repo_, acc_repo_, cust_repo_);

    lookup_index_ = std::make_shared<FastLookupIndex>();
    lookup_index_->addBlacklist("195.154.120.10");

    fraud_engine_ = FraudDetectorEngine::createDefaultEngine(lookup_index_);
    extractor_ = std::make_shared<FeatureExtractor>();
    risk_policy_ = std::make_shared<StandardWeightedRiskPolicy>();
    risk_aggregator_ = std::make_shared<RiskAggregator>();
    ml_predictor_ = std::make_shared<MockModelPredictor>("NativeXGBoostCalibrated", "v1.2.0", 18, true, 0.05);

    risk_engine_ = std::make_shared<RiskEngine>(risk_policy_, risk_aggregator_, ml_predictor_, fraud_engine_, extractor_);
    decision_engine_ = std::make_shared<DecisionEngine>(std::make_shared<StandardDecisionPolicy>(), risk_engine_);

    label_store_ = std::make_shared<LabelStore>();
    outcome_tracker_ = std::make_shared<OutcomeTracker>();
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    auto* root_layout = new QVBoxLayout(central);
    root_layout->setContentsMargins(8, 8, 8, 8);
    root_layout->setSpacing(8);

    // KPI Summary Header Cards
    auto* grp_kpi = new QGroupBox("Real-Time System Overview & Key Performance Indicators (KPI)", this);
    auto* lyt_kpi = new QHBoxLayout(grp_kpi);

    lblTotalTx_ = new QLabel("📊 Total Processed: 0", this);
    lblTotalTx_->setStyleSheet("font-size: 14px; font-weight: bold; color: #58a6ff;");

    lblApprovedTx_ = new QLabel("🟢 Approved: 0", this);
    lblApprovedTx_->setStyleSheet("font-size: 14px; font-weight: bold; color: #2ea043;");

    lblBlockedTx_ = new QLabel("🔴 Blocked Fraud: 0", this);
    lblBlockedTx_->setStyleSheet("font-size: 14px; font-weight: bold; color: #f85149;");

    lblBlockedLoss_ = new QLabel("🛡️ Mitigated Loss: $0.00", this);
    lblBlockedLoss_->setStyleSheet("font-size: 14px; font-weight: bold; color: #d29922;");

    lyt_kpi->addWidget(lblTotalTx_);
    lyt_kpi->addWidget(lblApprovedTx_);
    lyt_kpi->addWidget(lblBlockedTx_);
    lyt_kpi->addWidget(lblBlockedLoss_);
    root_layout->addWidget(grp_kpi);

    // Main Tab Widget
    tabWidget_ = new QTabWidget(this);

    streamWidget_ = new TransactionStreamWidget(this);
    connect(streamWidget_, &TransactionStreamWidget::openCaseFromStream, this, &MainWindow::onOpenCaseRequested);

    simulatorWidget_ = new SimulatorWidget(this);
    connect(simulatorWidget_, &SimulatorWidget::triggerScenarioBatch, this, &MainWindow::onTriggerScenarioBatch);
    connect(simulatorWidget_, &SimulatorWidget::startContinuousStream, this, &MainWindow::onStartContinuousStream);
    connect(simulatorWidget_, &SimulatorWidget::pauseContinuousStream, this, &MainWindow::onPauseContinuousStream);

    caseWidget_ = new CaseManagementWidget(label_store_, outcome_tracker_, this);
    benchmarkWidget_ = new BenchmarkWidget(decision_engine_, service_, this);

    tabWidget_->addTab(streamWidget_, "📡 Live Transaction Stream Monitor");
    tabWidget_->addTab(simulatorWidget_, "⚡ Attack & Traffic Simulator");
    tabWidget_->addTab(caseWidget_, "⚖️ Analyst Case Management & Labels");
    tabWidget_->addTab(benchmarkWidget_, "🚀 High-Throughput & DSA Benchmarks");

    root_layout->addWidget(tabWidget_, 1);
    setCentralWidget(central);

    statusBar()->showMessage("EPFD-RAS Engine Online | Ready to process transaction stream.");
}

void MainWindow::onProcessSingleTransaction(const Transaction& tx) {
    if (!acc_repo_->findById(tx.getAccountId()).has_value()) {
        acc_repo_->save(Account(tx.getAccountId(), tx.getCustomerId(), 50000.0, "USD"));
    }
    if (!cust_repo_->findById(tx.getCustomerId()).has_value()) {
        cust_repo_->save(Customer(tx.getCustomerId(), "Stream User", "user@stream.com", "0900000000"));
    }

    auto feat = extractor_->extract(tx);
    auto risk = risk_engine_->assessRisk(tx);
    auto decision = decision_engine_->evaluate(tx);
    service_->processTransaction(tx);

    streamWidget_->addEvaluatedTransaction(tx, decision, risk, feat);

    total_processed_++;
    if (decision.action == DecisionAction::APPROVE) {
        total_approved_++;
    } else if (decision.action == DecisionAction::BLOCK) {
        total_blocked_++;
        total_blocked_amount_ += tx.getAmount();

        // Automatically open review case on Blocked transaction
        caseWidget_->createCase(QString::fromStdString(tx.getTransactionId()),
                                QString::fromStdString(tx.getCustomerId()),
                                decision.risk_score,
                                static_cast<int>(decision.action));
    }

    updateKpiBanner();
}

void MainWindow::onTriggerScenarioBatch(SimulationScenario scenario, size_t count) {
    auto batch = simulator_.generateBatch(count, scenario);
    for (const auto& tx : batch) {
        onProcessSingleTransaction(tx);
    }
    statusBar()->showMessage(QString("Processed batch of %1 transactions for scenario: %2")
        .arg(count)
        .arg(QString::fromStdString(toString(scenario))));
}

void MainWindow::onStartContinuousStream(int interval_ms) {
    stream_timer_->start(interval_ms);
    statusBar()->showMessage(QString("Continuous stream active at %1 ms interval.").arg(interval_ms));
}

void MainWindow::onPauseContinuousStream() {
    stream_timer_->stop();
    statusBar()->showMessage("Continuous stream paused.");
}

void MainWindow::onStreamTimerTick() {
    Transaction tx = simulator_.generateTransaction(SimulationScenario::MIXED_REALISTIC_TRAFFIC);
    onProcessSingleTransaction(tx);
}

void MainWindow::onOpenCaseRequested(const QString& tx_id, const QString& cust_id, double risk_score, int decision_action) {
    caseWidget_->createCase(tx_id, cust_id, risk_score, decision_action);
    tabWidget_->setCurrentWidget(caseWidget_);
    statusBar()->showMessage(QString("Opened Investigation Case for Transaction: %1").arg(tx_id));
}

void MainWindow::updateKpiBanner() {
    lblTotalTx_->setText(QString("📊 Total Processed: %1").arg(total_processed_));
    lblApprovedTx_->setText(QString("🟢 Approved: %1").arg(total_approved_));
    lblBlockedTx_->setText(QString("🔴 Blocked Fraud: %1").arg(total_blocked_));
    lblBlockedLoss_->setText(QString("🛡️ Mitigated Loss: $%1").arg(QString::number(total_blocked_amount_, 'f', 2)));
}

} // namespace epfd::gui
