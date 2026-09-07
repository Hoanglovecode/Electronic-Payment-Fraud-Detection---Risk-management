#include "CaseManagementWidget.hpp"
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>

namespace epfd::gui {

CaseManagementWidget::CaseManagementWidget(std::shared_ptr<LabelStore> label_store,
                                           std::shared_ptr<OutcomeTracker> outcome_tracker,
                                           QWidget* parent)
    : QWidget(parent), label_store_(label_store), outcome_tracker_(outcome_tracker) {
    case_manager_ = std::make_unique<CaseManager>(label_store_, outcome_tracker_);
    setupUi();
}

void CaseManagementWidget::setupUi() {
    auto* main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(12);

    // Left: Case List Table
    auto* lyt_left = new QVBoxLayout();
    auto* grp_table = new QGroupBox("Active Fraud Investigation Cases (Queue)", this);
    auto* lyt_grp_tbl = new QVBoxLayout(grp_table);

    tblCases_ = new QTableWidget(this);
    tblCases_->setColumnCount(7);
    tblCases_->setHorizontalHeaderLabels({"Case ID", "Tx ID", "Customer ID", "Risk Score", "Original Action", "Status", "Analyst"});
    tblCases_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tblCases_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tblCases_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tblCases_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tblCases_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tblCases_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    tblCases_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);

    tblCases_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblCases_->setSelectionMode(QAbstractItemView::SingleSelection);
    tblCases_->setAlternatingRowColors(true);
    tblCases_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(tblCases_, &QTableWidget::cellClicked, this, &CaseManagementWidget::onCaseSelected);

    lyt_grp_tbl->addWidget(tblCases_);

    auto* lyt_export = new QHBoxLayout();
    btnExportCsv_ = new QPushButton("📥 Export Ground Truth Labels to CSV (LabelStore)", this);
    btnExportCsv_->setStyleSheet("background-color: #238636; color: white; font-weight: bold;");
    connect(btnExportCsv_, &QPushButton::clicked, this, &CaseManagementWidget::onExportCsv);

    lblStatus_ = new QLabel("Cases in queue: 0", this);
    lblStatus_->setStyleSheet("color: #8b949e; font-weight: bold;");

    lyt_export->addWidget(btnExportCsv_);
    lyt_export->addStretch();
    lyt_export->addWidget(lblStatus_);
    lyt_grp_tbl->addLayout(lyt_export);

    lyt_left->addWidget(grp_table);
    main_layout->addLayout(lyt_left, 3);

    // Right: Investigation Workbench Panel
    auto* lyt_right = new QVBoxLayout();
    auto* grp_bench = new QGroupBox("Analyst Investigation & Resolution Workbench", this);
    auto* lyt_grp_bench = new QVBoxLayout(grp_bench);
    lyt_grp_bench->setSpacing(10);

    lblSelectedCase_ = new QLabel("Selected Case: None", this);
    lblSelectedCase_->setStyleSheet("font-size: 14px; font-weight: bold; color: #58a6ff;");
    lyt_grp_bench->addWidget(lblSelectedCase_);

    // Analyst assignment
    auto* lyt_assign = new QHBoxLayout();
    cmbAnalyst_ = new QComboBox(this);
    cmbAnalyst_->addItems({"analyst_sarah", "analyst_john", "analyst_alex", "analyst_triet"});
    btnAssign_ = new QPushButton("Assign Analyst", this);
    connect(btnAssign_, &QPushButton::clicked, this, &CaseManagementWidget::onAssignAnalyst);
    lyt_assign->addWidget(new QLabel("Assignee:", this));
    lyt_assign->addWidget(cmbAnalyst_, 1);
    lyt_assign->addWidget(btnAssign_);
    lyt_grp_bench->addLayout(lyt_assign);

    // Digital Evidence Box
    lyt_grp_bench->addWidget(new QLabel("Forensics Evidence & Investigation Notes:", this));
    txtEvidence_ = new QTextEdit(this);
    txtEvidence_->setPlaceholderText("Enter digital forensic artifacts, IP jump evidence, or user communications...");
    txtEvidence_->setFixedHeight(70);
    lyt_grp_bench->addWidget(txtEvidence_);

    btnAddEvidence_ = new QPushButton("➕ Append Forensic Evidence", this);
    connect(btnAddEvidence_, &QPushButton::clicked, this, &CaseManagementWidget::onAddEvidence);
    lyt_grp_bench->addWidget(btnAddEvidence_);

    // Case Resolution
    lyt_grp_bench->addWidget(new QLabel("Final Case Resolution:", this));
    cmbResolution_ = new QComboBox(this);
    cmbResolution_->addItems({"RESOLVED_CONFIRMED_FRAUD", "RESOLVED_FALSE_POSITIVE", "CLOSED"});
    lyt_grp_bench->addWidget(cmbResolution_);

    txtResolutionNotes_ = new QLineEdit(this);
    txtResolutionNotes_->setPlaceholderText("Summary reason for resolution...");
    lyt_grp_bench->addWidget(txtResolutionNotes_);

    btnResolve_ = new QPushButton("✅ Submit Case Resolution & Update Ground Truth", this);
    btnResolve_->setStyleSheet("background-color: #1f6feb; color: white; font-weight: bold;");
    connect(btnResolve_, &QPushButton::clicked, this, &CaseManagementWidget::onResolveCase);
    lyt_grp_bench->addWidget(btnResolve_);

    // Outcome & Loss Tracking
    auto* grp_outcome = new QGroupBox("Real-World Outcome Tracking", this);
    auto* lyt_outcome = new QVBoxLayout(grp_outcome);

    auto* lyt_loss = new QHBoxLayout();
    lyt_loss->addWidget(new QLabel("Financial Loss ($):", this));
    spnLossAmount_ = new QDoubleSpinBox(this);
    spnLossAmount_->setRange(0.0, 1000000.0);
    spnLossAmount_->setValue(0.0);
    lyt_loss->addWidget(spnLossAmount_, 1);
    lyt_outcome->addLayout(lyt_loss);

    txtOutcomeNotes_ = new QLineEdit(this);
    txtOutcomeNotes_->setPlaceholderText("Outcome note (e.g. Chargeback filed by card issuer)...");
    lyt_outcome->addWidget(txtOutcomeNotes_);

    btnRecordOutcome_ = new QPushButton("💳 Record Chargeback / Dispute Event", this);
    btnRecordOutcome_->setStyleSheet("background-color: #d29922; color: white;");
    connect(btnRecordOutcome_, &QPushButton::clicked, this, &CaseManagementWidget::onRecordOutcome);
    lyt_outcome->addWidget(btnRecordOutcome_);

    lyt_grp_bench->addWidget(grp_outcome);
    lyt_grp_bench->addStretch();

    lyt_right->addWidget(grp_bench);
    main_layout->addLayout(lyt_right, 2);
}

void CaseManagementWidget::createCase(const QString& tx_id, const QString& cust_id, double risk_score, int decision_action) {
    std::string cid = case_manager_->createCase(tx_id.toStdString(),
                                                cust_id.toStdString(),
                                                risk_score,
                                                static_cast<DecisionAction>(decision_action));
    case_ids_.push_back(cid);
    refreshCaseTable();
}

void CaseManagementWidget::refreshCaseTable() {
    tblCases_->setRowCount(static_cast<int>(case_ids_.size()));
    for (size_t i = 0; i < case_ids_.size(); ++i) {
        auto c_opt = case_manager_->getCase(case_ids_[i]);
        if (!c_opt.has_value()) continue;
        const auto& c = c_opt.value();

        int row = static_cast<int>(i);
        tblCases_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(c.getCaseId())));
        tblCases_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(c.getTransactionId())));
        tblCases_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(c.getCustomerId())));
        tblCases_->setItem(row, 3, new QTableWidgetItem(QString::number(c.getInitialRiskScore(), 'f', 1)));
        auto orig_dec_str = toString(c.getOriginalDecision());
        tblCases_->setItem(row, 4, new QTableWidgetItem(QString::fromUtf8(orig_dec_str.data(), static_cast<qsizetype>(orig_dec_str.size()))));

        auto st_str = toString(c.getStatus());
        auto* itemStatus = new QTableWidgetItem(QString::fromUtf8(st_str.data(), static_cast<qsizetype>(st_str.size())));
        if (c.getStatus() == CaseStatus::RESOLVED_CONFIRMED_FRAUD) {
            itemStatus->setForeground(QColor("#f85149")); // Red
        } else if (c.getStatus() == CaseStatus::RESOLVED_FALSE_POSITIVE) {
            itemStatus->setForeground(QColor("#2ea043")); // Green
        } else {
            itemStatus->setForeground(QColor("#d29922")); // Yellow
        }
        tblCases_->setItem(row, 5, itemStatus);
        tblCases_->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(c.getAssignedAnalystId())));
    }
    lblStatus_->setText(QString("Cases in queue: %1 | Ground Truth Records: %2")
        .arg(case_ids_.size())
        .arg(label_store_->size()));
}

void CaseManagementWidget::onCaseSelected(int row, int column) {
    (void)column;
    if (row < 0 || row >= static_cast<int>(case_ids_.size())) return;

    std::string cid = case_ids_[static_cast<size_t>(row)];
    auto c_opt = case_manager_->getCase(cid);
    if (!c_opt.has_value()) return;
    const auto& c = c_opt.value();

    auto sel_st_str = toString(c.getStatus());
    lblSelectedCase_->setText(QString("Selected: %1 (%2)")
        .arg(QString::fromStdString(c.getCaseId()))
        .arg(QString::fromUtf8(sel_st_str.data(), static_cast<qsizetype>(sel_st_str.size()))));

    if (!c.getAssignedAnalystId().empty()) {
        cmbAnalyst_->setCurrentText(QString::fromStdString(c.getAssignedAnalystId()));
    }
}

void CaseManagementWidget::onAssignAnalyst() {
    int row = tblCases_->currentRow();
    if (row < 0 || row >= static_cast<int>(case_ids_.size())) return;

    std::string cid = case_ids_[static_cast<size_t>(row)];
    case_manager_->assignCase(cid, cmbAnalyst_->currentText().toStdString());
    refreshCaseTable();
}

void CaseManagementWidget::onAddEvidence() {
    int row = tblCases_->currentRow();
    if (row < 0 || row >= static_cast<int>(case_ids_.size())) return;

    QString note = txtEvidence_->toPlainText().trimmed();
    if (note.isEmpty()) return;

    std::string cid = case_ids_[static_cast<size_t>(row)];
    case_manager_->addEvidence(cid, note.toStdString());
    txtEvidence_->clear();
    QMessageBox::information(this, "Evidence Added", "Forensic evidence successfully attached to investigation case.");
}

void CaseManagementWidget::onResolveCase() {
    int row = tblCases_->currentRow();
    if (row < 0 || row >= static_cast<int>(case_ids_.size())) return;

    std::string cid = case_ids_[static_cast<size_t>(row)];
    QString res_str = cmbResolution_->currentText();
    CaseStatus st = CaseStatus::CLOSED;
    if (res_str == "RESOLVED_CONFIRMED_FRAUD") st = CaseStatus::RESOLVED_CONFIRMED_FRAUD;
    else if (res_str == "RESOLVED_FALSE_POSITIVE") st = CaseStatus::RESOLVED_FALSE_POSITIVE;

    TransactionFeatures dummy_feat;
    case_manager_->resolveCase(cid, st, txtResolutionNotes_->text().toStdString(), dummy_feat);
    refreshCaseTable();
    QMessageBox::information(this, "Case Resolved", QString("Case %1 resolved as %2. Ground Truth label recorded.")
        .arg(QString::fromStdString(cid))
        .arg(res_str));
}

void CaseManagementWidget::onRecordOutcome() {
    int row = tblCases_->currentRow();
    if (row < 0 || row >= static_cast<int>(case_ids_.size())) return;

    std::string cid = case_ids_[static_cast<size_t>(row)];
    auto c_opt = case_manager_->getCase(cid);
    if (!c_opt.has_value()) return;

    outcome_tracker_->recordOutcome("ev_" + cid,
                                    c_opt->getTransactionId(),
                                    OutcomeType::CHARGEBACK_RECEIVED,
                                    spnLossAmount_->value(),
                                    txtOutcomeNotes_->text().toStdString());
    QMessageBox::information(this, "Outcome Recorded", "Dispute / Chargeback event recorded in OutcomeTracker.");
}

void CaseManagementWidget::onExportCsv() {
    QString path = QFileDialog::getSaveFileName(this, "Export Ground Truth Dataset", "ground_truth_labels.csv", "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    if (label_store_->exportLabeledDatasetCsv(path.toStdString())) {
        QMessageBox::information(this, "Export Succeeded", QString("Dataset successfully exported to:\n%1").arg(path));
    } else {
        QMessageBox::warning(this, "Export Failed", "Could not export CSV file.");
    }
}

} // namespace epfd::gui
