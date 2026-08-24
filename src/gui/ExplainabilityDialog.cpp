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
    setWindowTitle(QString("Transaction Risk Analysis: %1").arg(QString::fromStdString(tx_.getTransactionId())));
    resize(780, 640);
    setupUi();
    populateData();
}

void ExplainabilityDialog::setupUi() {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(12);

    // Header Summary Box
    auto* grp_summary = new QGroupBox("Decision & Risk Assessment Summary", this);
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
    auto* grp_alerts = new QGroupBox("Triggered Fraud Alerts & Violations", this);
    auto* lyt_alerts = new QVBoxLayout(grp_alerts);

    tblAlerts_ = new QTableWidget(this);
    tblAlerts_->setColumnCount(4);
    tblAlerts_->setHorizontalHeaderLabels({"Rule Name", "Severity", "Risk Score", "Reason / Evidence"});
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
    auto* grp_features = new QGroupBox("Extracted 18-Dimensional Feature Space", this);
    auto* lyt_features = new QVBoxLayout(grp_features);

    tblFeatures_ = new QTableWidget(this);
    tblFeatures_->setColumnCount(4);
    tblFeatures_->setHorizontalHeaderLabels({"Feature Name", "Value", "Feature Name", "Value"});
    tblFeatures_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblFeatures_->setAlternatingRowColors(true);
    tblFeatures_->setFixedHeight(180);
    lyt_features->addWidget(tblFeatures_);

    main_layout->addWidget(grp_features);

    // Action Buttons
    auto* lyt_buttons = new QHBoxLayout();
    btnOpenCase_ = new QPushButton("⚖️ Open Investigation Review Case", this);
    btnOpenCase_->setStyleSheet("background-color: #1f6feb; color: white; font-weight: bold;");
    connect(btnOpenCase_, &QPushButton::clicked, this, &ExplainabilityDialog::onOpenCaseClicked);

    btnClose_ = new QPushButton("Close", this);
    connect(btnClose_, &QPushButton::clicked, this, &QDialog::accept);

    lyt_buttons->addWidget(btnOpenCase_);
    lyt_buttons->addStretch();
    lyt_buttons->addWidget(btnClose_);

    main_layout->addLayout(lyt_buttons);
}

void ExplainabilityDialog::populateData() {
    lblHeader_->setText(QString("Transaction: %1 | Customer: %2 | Amount: $%3 %4")
        .arg(QString::fromStdString(tx_.getTransactionId()))
        .arg(QString::fromStdString(tx_.getCustomerId()))
        .arg(QString::number(tx_.getAmount(), 'f', 2))
        .arg(QString::fromStdString(tx_.getCurrency())));

    QString badge_style;
    switch (decision_.action) {
        case DecisionAction::APPROVE:
            lblDecisionBadge_->setText("APPROVE");
            badge_style = "background-color: #238636; color: white;";
            pbRisk_->setStyleSheet("QProgressBar::chunk { background-color: #238636; }");
            break;
        case DecisionAction::REVIEW:
            lblDecisionBadge_->setText("REVIEW");
            badge_style = "background-color: #d29922; color: white;";
            pbRisk_->setStyleSheet("QProgressBar::chunk { background-color: #d29922; }");
            break;
        case DecisionAction::CHALLENGE_3DS:
            lblDecisionBadge_->setText("CHALLENGE 3DS");
            badge_style = "background-color: #db6d28; color: white;";
            pbRisk_->setStyleSheet("QProgressBar::chunk { background-color: #db6d28; }");
            break;
        case DecisionAction::BLOCK:
            lblDecisionBadge_->setText("BLOCK / DECLINED");
            badge_style = "background-color: #da3633; color: white;";
            pbRisk_->setStyleSheet("QProgressBar::chunk { background-color: #da3633; }");
            break;
    }
    lblDecisionBadge_->setStyleSheet(badge_style + " font-weight: bold; padding: 4px 10px; border-radius: 4px;");
    lblRiskScore_->setText(QString("Risk Score: %1 / 100 (%2)")
        .arg(QString::number(decision_.risk_score, 'f', 1))
        .arg(QString::fromStdString(toString(decision_.risk_level))));
    pbRisk_->setValue(static_cast<int>(decision_.risk_score));

    // Populate Alerts
    tblAlerts_->setRowCount(static_cast<int>(risk_.triggered_alerts.size()));
    for (size_t i = 0; i < risk_.triggered_alerts.size(); ++i) {
        const auto& alert = risk_.triggered_alerts[i];
        tblAlerts_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(alert.rule_name)));
        tblAlerts_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(toString(alert.severity))));
        tblAlerts_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(alert.risk_score, 'f', 1)));
        tblAlerts_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::fromStdString(alert.reason)));
    }
    if (risk_.triggered_alerts.empty()) {
        tblAlerts_->setRowCount(1);
        tblAlerts_->setItem(0, 0, new QTableWidgetItem("No rules triggered"));
        tblAlerts_->setItem(0, 1, new QTableWidgetItem("NONE"));
        tblAlerts_->setItem(0, 2, new QTableWidgetItem("0.0"));
        tblAlerts_->setItem(0, 3, new QTableWidgetItem("Clean baseline transaction"));
    }

    // Populate 18 Features (in 2-column layout)
    std::vector<std::pair<QString, QString>> feat_pairs = {
        {"Amount ($)", QString::number(features_.amount, 'f', 2)},
        {"Hour of Day", QString::number(features_.hour_of_day)},
        {"Day of Week", QString::number(features_.day_of_week)},
        {"Is Weekend", features_.is_weekend ? "Yes (1.0)" : "No (0.0)"},
        {"Velocity (5 min)", QString::number(features_.tx_velocity_5m)},
        {"Velocity (1 hour)", QString::number(features_.tx_velocity_1h)},
        {"Velocity (24 hour)", QString::number(features_.tx_velocity_24h)},
        {"Amount Sum (24h)", QString::number(features_.amount_sum_24h, 'f', 2)},
        {"Deviation Ratio", QString::number(features_.amount_to_avg_ratio, 'f', 2)},
        {"Is New Device", features_.is_new_device ? "Yes (1.0)" : "No (0.0)"},
        {"Device Risk Flag", features_.device_risk_flag ? "ROOTED/EMULATOR (1.0)" : "Safe (0.0)"},
        {"IP Diversity (24h)", QString::number(features_.ip_diversity_24h)},
        {"Device Diversity (24h)", QString::number(features_.device_diversity_24h)},
        {"Geo Distance (km)", QString::number(features_.geo_distance_km, 'f', 2)},
        {"Speed (km/h)", QString::number(features_.speed_kmh, 'f', 2)},
        {"Is Cross-Border", features_.is_cross_border ? "Yes (1.0)" : "No (0.0)"},
        {"Merchant Risk Score", QString::number(features_.merchant_risk_score, 'f', 2)},
        {"Is High-Risk MCC", features_.is_high_risk_mcc ? "Yes (1.0)" : "No (0.0)"}
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
    btnOpenCase_->setText("Case Created");
}

} // namespace epfd::gui
