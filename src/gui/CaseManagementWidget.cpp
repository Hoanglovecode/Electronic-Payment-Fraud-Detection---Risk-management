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
    auto* grp_table = new QGroupBox(QString::fromUtf8("Hàng đợi hồ sơ điều tra gian lận (Đang mở)"), this);
    auto* lyt_grp_tbl = new QVBoxLayout(grp_table);

    tblCases_ = new QTableWidget(this);
    tblCases_->setColumnCount(7);
    tblCases_->setHorizontalHeaderLabels({
        QString::fromUtf8("Mã hồ sơ"),
        QString::fromUtf8("Mã GD"),
        QString::fromUtf8("Mã khách hàng"),
        QString::fromUtf8("Điểm rủi ro"),
        QString::fromUtf8("Hành động gốc"),
        QString::fromUtf8("Trạng thái"),
        QString::fromUtf8("Chuyên viên")
    });
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
    btnExportCsv_ = new QPushButton(QString::fromUtf8("📥 Xuất tập nhãn chuẩn Ground Truth sang CSV (LabelStore)"), this);
    btnExportCsv_->setStyleSheet("background-color: #238636; color: white; font-weight: bold;");
    connect(btnExportCsv_, &QPushButton::clicked, this, &CaseManagementWidget::onExportCsv);

    lblStatus_ = new QLabel(QString::fromUtf8("Hồ sơ trong hàng đợi: 0 | Bản ghi nhãn chuẩn: 0"), this);
    lblStatus_->setStyleSheet("color: #8b949e; font-weight: bold;");

    lyt_export->addWidget(btnExportCsv_);
    lyt_export->addStretch();
    lyt_export->addWidget(lblStatus_);
    lyt_grp_tbl->addLayout(lyt_export);

    lyt_left->addWidget(grp_table);
    main_layout->addLayout(lyt_left, 3);

    // Right: Investigation Workbench Panel
    auto* lyt_right = new QVBoxLayout();
    auto* grp_bench = new QGroupBox(QString::fromUtf8("Bàn làm việc điều tra & Xử lý hồ sơ chuyên viên"), this);
    auto* lyt_grp_bench = new QVBoxLayout(grp_bench);
    lyt_grp_bench->setSpacing(10);

    lblSelectedCase_ = new QLabel(QString::fromUtf8("Hồ sơ đang chọn: Chưa chọn"), this);
    lblSelectedCase_->setStyleSheet("font-size: 14px; font-weight: bold; color: #58a6ff;");
    lyt_grp_bench->addWidget(lblSelectedCase_);

    // Analyst assignment
    auto* lyt_assign = new QHBoxLayout();
    cmbAnalyst_ = new QComboBox(this);
    cmbAnalyst_->addItems({"analyst_sarah", "analyst_john", "analyst_alex", "analyst_triet"});
    btnAssign_ = new QPushButton(QString::fromUtf8("Phân công"), this);
    connect(btnAssign_, &QPushButton::clicked, this, &CaseManagementWidget::onAssignAnalyst);
    lyt_assign->addWidget(new QLabel(QString::fromUtf8("Chuyên viên:"), this));
    lyt_assign->addWidget(cmbAnalyst_, 1);
    lyt_assign->addWidget(btnAssign_);
    lyt_grp_bench->addLayout(lyt_assign);

    // Digital Evidence Box
    lyt_grp_bench->addWidget(new QLabel(QString::fromUtf8("Bằng chứng kỹ thuật số & Ghi chú điều tra:"), this));
    txtEvidence_ = new QTextEdit(this);
    txtEvidence_->setPlaceholderText(QString::fromUtf8("Nhập hiện vật kỹ thuật số, dữ liệu nhảy IP bất thường, liên hệ khách hàng..."));
    txtEvidence_->setFixedHeight(70);
    lyt_grp_bench->addWidget(txtEvidence_);

    btnAddEvidence_ = new QPushButton(QString::fromUtf8("➕ Thêm bằng chứng điều tra"), this);
    connect(btnAddEvidence_, &QPushButton::clicked, this, &CaseManagementWidget::onAddEvidence);
    lyt_grp_bench->addWidget(btnAddEvidence_);

    // Case Resolution
    lyt_grp_bench->addWidget(new QLabel(QString::fromUtf8("Kết luận xử lý hồ sơ cuối cùng:"), this));
    cmbResolution_ = new QComboBox(this);
    cmbResolution_->addItem(QString::fromUtf8("XÁC NHẬN GIAN LẬN (CONFIRMED_FRAUD)"));
    cmbResolution_->addItem(QString::fromUtf8("BÁO ĐỘNG GIẢ (FALSE_POSITIVE)"));
    cmbResolution_->addItem(QString::fromUtf8("ĐÓNG HỒ SƠ (CLOSED)"));
    lyt_grp_bench->addWidget(cmbResolution_);

    txtResolutionNotes_ = new QLineEdit(this);
    txtResolutionNotes_->setPlaceholderText(QString::fromUtf8("Tóm tắt lý do kết luận..."));
    lyt_grp_bench->addWidget(txtResolutionNotes_);

    btnResolve_ = new QPushButton(QString::fromUtf8("✅ Gửi kết luận hồ sơ & Cập nhật nhãn chuẩn"), this);
    btnResolve_->setStyleSheet("background-color: #1f6feb; color: white; font-weight: bold;");
    connect(btnResolve_, &QPushButton::clicked, this, &CaseManagementWidget::onResolveCase);
    lyt_grp_bench->addWidget(btnResolve_);

    // Outcome & Loss Tracking
    auto* grp_outcome = new QGroupBox(QString::fromUtf8("Theo dõi kết quả thực tế & Tổn thất"), this);
    auto* lyt_outcome = new QVBoxLayout(grp_outcome);

    auto* lyt_loss = new QHBoxLayout();
    lyt_loss->addWidget(new QLabel(QString::fromUtf8("Tổn thất tài chính ($):"), this));
    spnLossAmount_ = new QDoubleSpinBox(this);
    spnLossAmount_->setRange(0.0, 1000000.0);
    spnLossAmount_->setValue(0.0);
    lyt_loss->addWidget(spnLossAmount_, 1);
    lyt_outcome->addLayout(lyt_loss);

    txtOutcomeNotes_ = new QLineEdit(this);
    txtOutcomeNotes_->setPlaceholderText(QString::fromUtf8("Ghi chú kết quả (VD: Nhận khiếu nại bồi hoàn từ ngân hàng phát hành thẻ)..."));
    lyt_outcome->addWidget(txtOutcomeNotes_);

    btnRecordOutcome_ = new QPushButton(QString::fromUtf8("💳 Ghi nhận sự kiện Khiếu nại / Bồi hoàn (Chargeback)"), this);
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
        
        QString orig_dec_text;
        switch (c.getOriginalDecision()) {
            case DecisionAction::APPROVE: orig_dec_text = QString::fromUtf8("HỢP LỆ"); break;
            case DecisionAction::REVIEW: orig_dec_text = QString::fromUtf8("XEM XÉT"); break;
            case DecisionAction::CHALLENGE_3DS: orig_dec_text = QString::fromUtf8("XÁC THỰC 3DS"); break;
            case DecisionAction::BLOCK: orig_dec_text = QString::fromUtf8("CHẶN"); break;
        }
        tblCases_->setItem(row, 4, new QTableWidgetItem(orig_dec_text));

        QString st_text;
        QColor st_color;
        switch (c.getStatus()) {
            case CaseStatus::RESOLVED_CONFIRMED_FRAUD:
                st_text = QString::fromUtf8("XÁC NHẬN GIAN LẬN");
                st_color = QColor("#f85149"); // Red
                break;
            case CaseStatus::RESOLVED_FALSE_POSITIVE:
                st_text = QString::fromUtf8("BÁO ĐỘNG GIẢ");
                st_color = QColor("#2ea043"); // Green
                break;
            case CaseStatus::CLOSED:
                st_text = QString::fromUtf8("ĐÃ ĐÓNG");
                st_color = QColor("#8b949e");
                break;
            case CaseStatus::IN_INVESTIGATION:
                st_text = QString::fromUtf8("ĐANG ĐIỀU TRA");
                st_color = QColor("#d29922"); // Yellow
                break;
            default:
                st_text = QString::fromUtf8("CHỜ XỬ LÝ");
                st_color = QColor("#d29922"); // Yellow
                break;
        }
        auto* itemStatus = new QTableWidgetItem(st_text);
        itemStatus->setForeground(st_color);
        tblCases_->setItem(row, 5, itemStatus);
        tblCases_->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(c.getAssignedAnalystId())));
    }
    lblStatus_->setText(QString::fromUtf8("Hồ sơ trong hàng đợi: %1 | Bản ghi nhãn chuẩn: %2")
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

    QString st_text;
    switch (c.getStatus()) {
        case CaseStatus::RESOLVED_CONFIRMED_FRAUD: st_text = QString::fromUtf8("XÁC NHẬN GIAN LẬN"); break;
        case CaseStatus::RESOLVED_FALSE_POSITIVE: st_text = QString::fromUtf8("BÁO ĐỘNG GIẢ"); break;
        case CaseStatus::CLOSED: st_text = QString::fromUtf8("ĐÃ ĐÓNG"); break;
        case CaseStatus::IN_INVESTIGATION: st_text = QString::fromUtf8("ĐANG ĐIỀU TRA"); break;
        default: st_text = QString::fromUtf8("CHỜ XỬ LÝ"); break;
    }

    lblSelectedCase_->setText(QString::fromUtf8("Đang chọn: %1 (%2)")
        .arg(QString::fromStdString(c.getCaseId()))
        .arg(st_text));

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
    QMessageBox::information(this, QString::fromUtf8("Đã thêm bằng chứng"), QString::fromUtf8("Bằng chứng kỹ thuật số đã được gắn thành công vào hồ sơ điều tra."));
}

void CaseManagementWidget::onResolveCase() {
    int row = tblCases_->currentRow();
    if (row < 0 || row >= static_cast<int>(case_ids_.size())) return;

    std::string cid = case_ids_[static_cast<size_t>(row)];
    int res_idx = cmbResolution_->currentIndex();
    CaseStatus st = CaseStatus::CLOSED;
    QString res_str = QString::fromUtf8("ĐÃ ĐÓNG");
    if (res_idx == 0) {
        st = CaseStatus::RESOLVED_CONFIRMED_FRAUD;
        res_str = QString::fromUtf8("XÁC NHẬN GIAN LẬN");
    } else if (res_idx == 1) {
        st = CaseStatus::RESOLVED_FALSE_POSITIVE;
        res_str = QString::fromUtf8("BÁO ĐỘNG GIẢ");
    }

    TransactionFeatures dummy_feat;
    case_manager_->resolveCase(cid, st, txtResolutionNotes_->text().toStdString(), dummy_feat);
    refreshCaseTable();
    QMessageBox::information(this, QString::fromUtf8("Đã xử lý hồ sơ"), QString::fromUtf8("Hồ sơ %1 đã được kết luận là %2. Nhãn chuẩn Ground Truth đã được cập nhật.")
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
    QMessageBox::information(this, QString::fromUtf8("Đã ghi nhận kết quả"), QString::fromUtf8("Sự kiện Khiếu nại / Bồi hoàn (Chargeback) đã được ghi nhận vào OutcomeTracker."));
}

void CaseManagementWidget::onExportCsv() {
    QString path = QFileDialog::getSaveFileName(this, QString::fromUtf8("Xuất tập nhãn chuẩn Ground Truth"), "ground_truth_labels.csv", "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    if (label_store_->exportLabeledDatasetCsv(path.toStdString())) {
        QMessageBox::information(this, QString::fromUtf8("Xuất tệp thành công"), QString::fromUtf8("Tập dữ liệu đã được xuất thành công tới:\n%1").arg(path));
    } else {
        QMessageBox::warning(this, QString::fromUtf8("Xuất tệp thất bại"), QString::fromUtf8("Không thể xuất tệp tin CSV."));
    }
}

} // namespace epfd::gui
