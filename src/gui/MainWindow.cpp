#include "MainWindow.hpp"
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

namespace epfd::gui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QString::fromUtf8("EPFD-RAS: Hệ thống Phát hiện Gian lận Thanh toán Điện tử & Quản trị Rủi ro"));
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
    auto* grp_kpi = new QGroupBox(QString::fromUtf8("Tổng quan hệ thống theo thời gian thực & Chỉ số hiệu suất chính (KPI)"), this);
    auto* lyt_kpi = new QHBoxLayout(grp_kpi);

    lblTotalTx_ = new QLabel(QString::fromUtf8("📊 Tổng đã xử lý: 0"), this);
    lblTotalTx_->setStyleSheet("font-size: 14px; font-weight: bold; color: #58a6ff;");

    lblApprovedTx_ = new QLabel(QString::fromUtf8("🟢 Hợp lệ (Đã duyệt): 0"), this);
    lblApprovedTx_->setStyleSheet("font-size: 14px; font-weight: bold; color: #2ea043;");

    lblBlockedTx_ = new QLabel(QString::fromUtf8("🔴 Gian lận đã chặn: 0"), this);
    lblBlockedTx_->setStyleSheet("font-size: 14px; font-weight: bold; color: #f85149;");

    lblBlockedLoss_ = new QLabel(QString::fromUtf8("🛡️ Tổn thất đã ngăn chặn: $0.00"), this);
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

    tabWidget_->addTab(streamWidget_, QString::fromUtf8("📡 Giám sát luồng giao dịch trực tiếp"));
    tabWidget_->addTab(simulatorWidget_, QString::fromUtf8("⚡ Trình mô phỏng lưu lượng & Tấn công"));
    tabWidget_->addTab(caseWidget_, QString::fromUtf8("⚖️ Quản lý hồ sơ & Gán nhãn điều tra"));
    tabWidget_->addTab(benchmarkWidget_, QString::fromUtf8("🚀 Điểm chuẩn hiệu năng & Cấu trúc dữ liệu (DSA)"));

    root_layout->addWidget(tabWidget_, 1);
    setCentralWidget(central);

    statusBar()->showMessage(QString::fromUtf8("Hệ thống EPFD-RAS đang trực tuyến | Sẵn sàng xử lý luồng giao dịch."));
}

void MainWindow::onProcessSingleTransaction(const Transaction& tx) {
    if (!acc_repo_->findById(tx.getAccountId()).has_value()) {
        acc_repo_->save(Account(tx.getAccountId(), tx.getCustomerId(), 50000.0, "USD"));
    }
    if (!cust_repo_->findById(tx.getCustomerId()).has_value()) {
        cust_repo_->save(Customer(tx.getCustomerId(), "Người dùng luồng", "user@stream.com", "0900000000"));
    }

    auto feat = extractor_->extract(tx);
    auto risk = risk_engine_->assess(tx);
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

static QString scenarioToString(SimulationScenario s) {
    switch (s) {
        case SimulationScenario::NORMAL_LEGITIMATE: return QString::fromUtf8("Giao dịch thông thường hợp lệ");
        case SimulationScenario::BURST_VELOCITY_ATTACK: return QString::fromUtf8("Tấn công dồn dập (Burst Velocity)");
        case SimulationScenario::IMPOSSIBLE_TRAVEL_ATTACK: return QString::fromUtf8("Di chuyển bất khả thi (Impossible Travel)");
        case SimulationScenario::CARD_TESTING_ATTACK: return QString::fromUtf8("Tấn công dò thẻ (Card Testing)");
        case SimulationScenario::MULE_SMURFING_ATTACK: return QString::fromUtf8("Rửa tiền tài khoản trung gian (Mule Smurfing)");
        case SimulationScenario::ACCOUNT_TAKEOVER_ATTACK: return QString::fromUtf8("Chiếm đoạt tài khoản (Account Takeover)");
        case SimulationScenario::MIXED_REALISTIC_TRAFFIC: return QString::fromUtf8("Lưu lượng thực tế hỗn hợp");
    }
    return QString::fromUtf8("Không xác định");
}

void MainWindow::onTriggerScenarioBatch(SimulationScenario scenario, size_t count) {
    auto batch = simulator_.generateBatch(count, scenario);
    for (const auto& tx : batch) {
        onProcessSingleTransaction(tx);
    }
    statusBar()->showMessage(QString::fromUtf8("Đã xử lý lô %1 giao dịch cho kịch bản: %2")
        .arg(count)
        .arg(scenarioToString(scenario)));
}

void MainWindow::onStartContinuousStream(int interval_ms) {
    stream_timer_->start(interval_ms);
    statusBar()->showMessage(QString::fromUtf8("Đang chạy luồng giao dịch liên tục với chu kỳ %1 ms.").arg(interval_ms));
}

void MainWindow::onPauseContinuousStream() {
    stream_timer_->stop();
    statusBar()->showMessage(QString::fromUtf8("Đã tạm dừng luồng giao dịch liên tục."));
}

void MainWindow::onStreamTimerTick() {
    Transaction tx = simulator_.generateSingleTransaction(SimulationScenario::MIXED_REALISTIC_TRAFFIC);
    onProcessSingleTransaction(tx);
}

void MainWindow::onOpenCaseRequested(const QString& tx_id, const QString& cust_id, double risk_score, int decision_action) {
    caseWidget_->createCase(tx_id, cust_id, risk_score, decision_action);
    tabWidget_->setCurrentWidget(caseWidget_);
    statusBar()->showMessage(QString::fromUtf8("Đã mở hồ sơ điều tra cho giao dịch: %1").arg(tx_id));
}

void MainWindow::updateKpiBanner() {
    lblTotalTx_->setText(QString::fromUtf8("📊 Tổng đã xử lý: %1").arg(total_processed_));
    lblApprovedTx_->setText(QString::fromUtf8("🟢 Hợp lệ (Đã duyệt): %1").arg(total_approved_));
    lblBlockedTx_->setText(QString::fromUtf8("🔴 Gian lận đã chặn: %1").arg(total_blocked_));
    lblBlockedLoss_->setText(QString::fromUtf8("🛡️ Tổn thất đã ngăn chặn: $%1").arg(QString::number(total_blocked_amount_, 'f', 2)));
}

} // namespace epfd::gui
