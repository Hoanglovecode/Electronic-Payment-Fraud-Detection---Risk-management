#include "ExplainabilityDialog.hpp"
#include <QHeaderView>
#include <iomanip>
#include <sstream>

namespace epfd::gui {

ExplainabilityDialog::ExplainabilityDialog(const Transaction& tx,
                                           const DecisionResult& decision,
                                           const RiskAssessment& risk,
                                           const TransactionFeatures& features,
                                           QWidget* parent)
    : QDialog(parent), tx_(tx), decision_(decision), risk_(risk), features_(features) {
    setWindowTitle(QString::fromUtf8("Phân tích & Giải trình rủi ro giao dịch: %1").arg(QString::fromStdString(tx_.getTransactionId())));
    resize(780, 640);
    setupUi();
    populateData();
}

void ExplainabilityDialog::setupUi() {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(12);

    // Header Summary Box
    auto* grp_summary = new QGroupBox(QString::fromUtf8("Tổng kết Quyết định & Đánh giá rủi ro"), this);
    auto* lyt_summary = new QVBoxLayout(grp_summary);

    lblHeader_ = new QLabel(this);
    lblHeader_->setStyleSheet("font-size: 15px; font-weight: bold; color: #58a6ff;");
    lyt_summary->addWidget(lblHeader_);

    auto* lyt_scores = new QHBoxLayout();
    lblDecisionBadge_ = new QLabel(this);
    lblDecisionBadge_->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px 12px; border-radius: 4px;");

    lblRiskScore_ = new QLabel(this);
    lblRiskScore_->setStyleSheet("font-size: 14px; font-weight: bold;");

    pbRisk_ = new QProgressBar(this);
    pbRisk_->setRange(0, 100);
    pbRisk_->setFixedHeight(20);

    lyt_scores->addWidget(lblDecisionBadge_);
    lyt_scores->addWidget(lblRiskScore_);
    lyt_scores->addWidget(pbRisk_, 1);
    lyt_summary->addLayout(lyt_scores);

    main_layout->addWidget(grp_summary);

    // Rule Alerts Box
    auto* grp_alerts = new QGroupBox(QString::fromUtf8("Các cảnh báo gian lận & Vi phạm quy tắc được kích hoạt"), this);
    auto* lyt_alerts = new QVBoxLayout(grp_alerts);

    tblAlerts_ = new QTableWidget(this);
    tblAlerts_->setColumnCount(4);
    tblAlerts_->setHorizontalHeaderLabels({
        QString::fromUtf8("Tên quy tắc"),
        QString::fromUtf8("Mức độ nghiêm trọng"),
        QString::fromUtf8("Điểm rủi ro"),
        QString::fromUtf8("Lý do / Bằng chứng")
    });
    tblAlerts_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tblAlerts_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tblAlerts_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tblAlerts_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    tblAlerts_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblAlerts_->setAlternatingRowColors(true);
    tblAlerts_->setFixedHeight(140);
    lyt_alerts->addWidget(tblAlerts_);

    main_layout->addWidget(grp_alerts);

    // Feature Space Box (18-D)
    auto* grp_features = new QGroupBox(QString::fromUtf8("Không gian 18 đặc trưng số học trích xuất (Feature Space)"), this);
    auto* lyt_features = new QVBoxLayout(grp_features);

    tblFeatures_ = new QTableWidget(this);
    tblFeatures_->setColumnCount(4);
    tblFeatures_->setHorizontalHeaderLabels({
        QString::fromUtf8("Tên đặc trưng"),
        QString::fromUtf8("Giá trị"),
        QString::fromUtf8("Tên đặc trưng"),
        QString::fromUtf8("Giá trị")
    });
    tblFeatures_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblFeatures_->setAlternatingRowColors(true);
    tblFeatures_->setFixedHeight(180);
    lyt_features->addWidget(tblFeatures_);

    main_layout->addWidget(grp_features);

    // Action Buttons
    auto* lyt_buttons = new QHBoxLayout();
    btnOpenCase_ = new QPushButton(QString::fromUtf8("⚖️ Mở hồ sơ điều tra chuyên viên"), this);
    btnOpenCase_->setStyleSheet("background-color: #1f6feb; color: white; font-weight: bold;");
    connect(btnOpenCase_, &QPushButton::clicked, this, &ExplainabilityDialog::onOpenCaseClicked);

    btnClose_ = new QPushButton(QString::fromUtf8("Đóng"), this);
    connect(btnClose_, &QPushButton::clicked, this, &QDialog::accept);

    lyt_buttons->addWidget(btnOpenCase_);
    lyt_buttons->addStretch();
    lyt_buttons->addWidget(btnClose_);

    main_layout->addLayout(lyt_buttons);
}

void ExplainabilityDialog::populateData() {
    lblHeader_->setText(QString::fromUtf8("Giao dịch: %1 | Khách hàng: %2 | Số tiền: $%3 %4")
        .arg(QString::fromStdString(tx_.getTransactionId()))
        .arg(QString::fromStdString(tx_.getCustomerId()))
        .arg(QString::number(tx_.getAmount(), 'f', 2))
        .arg(QString::fromStdString(tx_.getCurrency())));

    QString badge_style;
    switch (decision_.action) {
        case DecisionAction::APPROVE:
            lblDecisionBadge_->setText(QString::fromUtf8("HỢP LỆ (APPROVE)"));
            badge_style = "background-color: #238636; color: white;";
            pbRisk_->setStyleSheet("QProgressBar::chunk { background-color: #238636; }");
            break;
        case DecisionAction::REVIEW:
            lblDecisionBadge_->setText(QString::fromUtf8("CẦN XEM XÉT (REVIEW)"));
            badge_style = "background-color: #d29922; color: white;";
            pbRisk_->setStyleSheet("QProgressBar::chunk { background-color: #d29922; }");
            break;
        case DecisionAction::CHALLENGE_3DS:
            lblDecisionBadge_->setText(QString::fromUtf8("XÁC THỰC 3DS (CHALLENGE)"));
            badge_style = "background-color: #db6d28; color: white;";
            pbRisk_->setStyleSheet("QProgressBar::chunk { background-color: #db6d28; }");
            break;
        case DecisionAction::BLOCK:
            lblDecisionBadge_->setText(QString::fromUtf8("CHẶN GIAN LẬN (BLOCK)"));
            badge_style = "background-color: #da3633; color: white;";
            pbRisk_->setStyleSheet("QProgressBar::chunk { background-color: #da3633; }");
            break;
    }
    lblDecisionBadge_->setStyleSheet(badge_style + " font-weight: bold; padding: 4px 10px; border-radius: 4px;");
    
    QString level_text;
    switch (decision_.risk_level) {
        case RiskLevel::LOW: level_text = QString::fromUtf8("RỦI RO THẤP"); break;
        case RiskLevel::MEDIUM: level_text = QString::fromUtf8("RỦI RO TRUNG BÌNH"); break;
        case RiskLevel::HIGH: level_text = QString::fromUtf8("RỦI RO CAO"); break;
        case RiskLevel::CRITICAL: level_text = QString::fromUtf8("RỦI RO NGHIÊM TRỌNG"); break;
    }

    lblRiskScore_->setText(QString::fromUtf8("Điểm rủi ro: %1 / 100 (%2)")
        .arg(QString::number(decision_.risk_score, 'f', 1))
        .arg(level_text));
    pbRisk_->setValue(static_cast<int>(decision_.risk_score));

    // Populate Alerts
    const auto& alerts = risk_.getAlerts();
    tblAlerts_->setRowCount(static_cast<int>(alerts.size()));
    for (size_t i = 0; i < alerts.size(); ++i) {
        const auto& alert = alerts[i];
        tblAlerts_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(alert.getRuleName())));
        
        QString sev_text;
        switch (alert.getSeverity()) {
            case RiskLevel::LOW: sev_text = QString::fromUtf8("THẤP"); break;
            case RiskLevel::MEDIUM: sev_text = QString::fromUtf8("TRUNG BÌNH"); break;
            case RiskLevel::HIGH: sev_text = QString::fromUtf8("CAO"); break;
            case RiskLevel::CRITICAL: sev_text = QString::fromUtf8("NGHIÊM TRỌNG"); break;
            default: sev_text = QString::fromUtf8("THÔNG TIN"); break;
        }
        tblAlerts_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(sev_text));
        tblAlerts_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(alert.getScoreContribution(), 'f', 1)));
        tblAlerts_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::fromStdString(alert.getReason())));
    }
    if (alerts.empty()) {
        tblAlerts_->setRowCount(1);
        tblAlerts_->setItem(0, 0, new QTableWidgetItem(QString::fromUtf8("Không kích hoạt quy tắc nào")));
        tblAlerts_->setItem(0, 1, new QTableWidgetItem(QString::fromUtf8("KHÔNG")));
        tblAlerts_->setItem(0, 2, new QTableWidgetItem("0.0"));
        tblAlerts_->setItem(0, 3, new QTableWidgetItem(QString::fromUtf8("Giao dịch an toàn, phù hợp hồ sơ cơ bản")));
    }

    // Populate 18 Features (in 2-column layout)
    std::vector<std::pair<QString, QString>> feat_pairs = {
        {QString::fromUtf8("Số tiền ($)"), QString::number(features_.transaction_amount, 'f', 2)},
        {QString::fromUtf8("Khung giờ trong ngày"), QString::number(features_.hour_of_day, 'f', 0)},
        {QString::fromUtf8("Là ngày cuối tuần"), features_.is_weekend > 0.5 ? QString::fromUtf8("Có (1.0)") : QString::fromUtf8("Không (0.0)")},
        {QString::fromUtf8("Số GD (5 phút gần nhất)"), QString::number(features_.transactions_last_5min, 'f', 0)},
        {QString::fromUtf8("Tổng tiền (5 phút gần nhất)"), QString::number(features_.amount_sum_last_5min, 'f', 2)},
        {QString::fromUtf8("Số GD (1 giờ gần nhất)"), QString::number(features_.transactions_last_1hour, 'f', 0)},
        {QString::fromUtf8("Tổng tiền (1 giờ gần nhất)"), QString::number(features_.amount_sum_last_1hour, 'f', 2)},
        {QString::fromUtf8("Số GD (24 giờ gần nhất)"), QString::number(features_.transactions_last_24hours, 'f', 0)},
        {QString::fromUtf8("Tổng tiền (24 giờ gần nhất)"), QString::number(features_.amount_sum_last_24hours, 'f', 2)},
        {QString::fromUtf8("Số tiền trung bình (24h)"), QString::number(features_.average_amount_24h, 'f', 2)},
        {QString::fromUtf8("Tỷ lệ lệch so với TB"), QString::number(features_.amount_deviation_ratio, 'f', 2)},
        {QString::fromUtf8("Thiết bị mới"), features_.is_new_device > 0.5 ? QString::fromUtf8("Có (1.0)") : QString::fromUtf8("Không (0.0)")},
        {QString::fromUtf8("Cảnh báo rủi ro thiết bị"), features_.is_high_risk_device > 0.5 ? QString::fromUtf8("MÁY ẢO / ROOT (1.0)") : QString::fromUtf8("An toàn (0.0)")},
        {QString::fromUtf8("Số tài khoản trên thiết bị"), QString::number(features_.accounts_on_device_count, 'f', 0)},
        {QString::fromUtf8("Quốc gia mới"), features_.is_new_country > 0.5 ? QString::fromUtf8("Có (1.0)") : QString::fromUtf8("Không (0.0)")},
        {QString::fromUtf8("Khoảng cách địa lý (km)"), QString::number(features_.distance_from_home_km, 'f', 2)},
        {QString::fromUtf8("Vận tốc di chuyển (km/h)"), QString::number(features_.speed_from_last_tx_kmh, 'f', 2)},
        {QString::fromUtf8("Mã ngành (MCC) rủi ro"), features_.is_high_risk_mcc > 0.5 ? QString::fromUtf8("Có (1.0)") : QString::fromUtf8("Không (0.0)")}
    };

    tblFeatures_->setRowCount(9);
    for (int r = 0; r < 9; ++r) {
        tblFeatures_->setItem(r, 0, new QTableWidgetItem(feat_pairs[r].first));
        tblFeatures_->setItem(r, 1, new QTableWidgetItem(feat_pairs[r].second));
        tblFeatures_->setItem(r, 2, new QTableWidgetItem(feat_pairs[r + 9].first));
        tblFeatures_->setItem(r, 3, new QTableWidgetItem(feat_pairs[r + 9].second));
    }
}

void ExplainabilityDialog::onOpenCaseClicked() {
    emit openCaseRequested(QString::fromStdString(tx_.getTransactionId()),
                           QString::fromStdString(tx_.getCustomerId()),
                           decision_.risk_score,
                           static_cast<int>(decision_.action));
    btnOpenCase_->setEnabled(false);
    btnOpenCase_->setText(QString::fromUtf8("Đã tạo hồ sơ"));
}

} // namespace epfd::gui
