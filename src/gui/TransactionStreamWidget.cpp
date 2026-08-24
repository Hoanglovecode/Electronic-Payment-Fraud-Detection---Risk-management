#include "TransactionStreamWidget.hpp"
#include <QHeaderView>
#include <QDateTime>

namespace epfd::gui {

TransactionStreamWidget::TransactionStreamWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void TransactionStreamWidget::setupUi() {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(8);

    // Filter toolbar
    auto* lyt_tools = new QHBoxLayout();

    auto* lblFilter = new QLabel("Filter Action:", this);
    cmbFilterAction_ = new QComboBox(this);
    cmbFilterAction_->addItems({"ALL DECISIONS", "APPROVE", "REVIEW", "CHALLENGE_3DS", "BLOCK"});
    connect(cmbFilterAction_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TransactionStreamWidget::onFilterChanged);

    txtSearch_ = new QLineEdit(this);
    txtSearch_->setPlaceholderText("🔍 Search Transaction ID, Customer, or Card...");
    connect(txtSearch_, &QLineEdit::textChanged, this, &TransactionStreamWidget::onFilterChanged);

    lblCount_ = new QLabel("Total Streamed: 0 | Displayed: 0", this);
    lblCount_->setStyleSheet("color: #8b949e; font-weight: bold;");

    btnClear_ = new QPushButton("Clear Feed", this);
    connect(btnClear_, &QPushButton::clicked, this, &TransactionStreamWidget::clearStream);

    lyt_tools->addWidget(lblFilter);
    lyt_tools->addWidget(cmbFilterAction_);
    lyt_tools->addWidget(txtSearch_, 1);
    lyt_tools->addWidget(lblCount_);
    lyt_tools->addWidget(btnClear_);

    main_layout->addLayout(lyt_tools);

    // Table
    table_ = new QTableWidget(this);
    table_->setColumnCount(8);
    table_->setHorizontalHeaderLabels({"Timestamp", "Tx ID", "Customer ID", "Amount", "Country", "Risk Score", "Decision", "Masked Card"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);

    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setAlternatingRowColors(true);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(table_, &QTableWidget::cellDoubleClicked, this, &TransactionStreamWidget::onCellDoubleClicked);

    main_layout->addWidget(table_);
}

void TransactionStreamWidget::addEvaluatedTransaction(const Transaction& tx,
                                                     const DecisionResult& decision,
                                                     const RiskAssessment& risk,
                                                     const TransactionFeatures& features) {
    EvaluatedTxRecord rec{tx, decision, risk, features};
    records_.push_back(rec);

    int row = table_->rowCount();
    table_->insertRow(row);
    updateTableRow(row, rec);

    lblCount_->setText(QString("Total Streamed: %1 | Displayed: %2")
        .arg(records_.size())
        .arg(table_->rowCount()));

    table_->scrollToBottom();
}

void TransactionStreamWidget::updateTableRow(int row, const EvaluatedTxRecord& rec) {
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(rec.tx.getTimestamp().time_since_epoch()).count();
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(time_ms);

    table_->setItem(row, 0, new QTableWidgetItem(dt.toString("hh:mm:ss.zzz")));
    table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(rec.tx.getTransactionId())));
    table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(rec.tx.getCustomerId())));
    table_->setItem(row, 3, new QTableWidgetItem(QString("$%1 %2")
        .arg(QString::number(rec.tx.getAmount(), 'f', 2))
        .arg(QString::fromStdString(rec.tx.getCurrency()))));
    table_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(rec.tx.getLocation().getCountry())));

    // Risk Score
    auto* itemScore = new QTableWidgetItem(QString::number(rec.decision.risk_score, 'f', 1));
    itemScore->setTextAlignment(Qt::AlignCenter);
    table_->setItem(row, 5, itemScore);

    // Decision Badge
    auto* itemDecision = new QTableWidgetItem(QString::fromStdString(toString(rec.decision.action)));
    itemDecision->setTextAlignment(Qt::AlignCenter);

    switch (rec.decision.action) {
        case DecisionAction::APPROVE:
            itemDecision->setForeground(QColor("#2ea043")); // Green
            break;
        case DecisionAction::REVIEW:
            itemDecision->setForeground(QColor("#d29922")); // Yellow
            break;
        case DecisionAction::CHALLENGE_3DS:
            itemDecision->setForeground(QColor("#db6d28")); // Orange
            break;
        case DecisionAction::BLOCK:
            itemDecision->setForeground(QColor("#f85149")); // Red
            break;
    }
    table_->setItem(row, 6, itemDecision);

    // Masked Card
    std::string masked = SecurityUtils::maskPan(rec.tx.getPaymentMethod().getMaskedCardNumber());
    table_->setItem(row, 7, new QTableWidgetItem(QString::fromStdString(masked)));
}

void TransactionStreamWidget::onCellDoubleClicked(int row, int column) {
    (void)column;
    if (row < 0 || row >= static_cast<int>(records_.size())) return;

    const auto& rec = records_[static_cast<size_t>(row)];
    auto* dialog = new ExplainabilityDialog(rec.tx, rec.decision, rec.risk, rec.features, this);
    connect(dialog, &ExplainabilityDialog::openCaseRequested, this, &TransactionStreamWidget::openCaseFromStream);
    dialog->exec();
}

void TransactionStreamWidget::onFilterChanged() {
    QString filter = cmbFilterAction_->currentText();
    QString query = txtSearch_->text().trimmed().toLower();

    int visible_count = 0;
    for (int r = 0; r < table_->rowCount(); ++r) {
        bool match_action = (filter == "ALL DECISIONS") || (table_->item(r, 6)->text() == filter);
        bool match_query = query.isEmpty() ||
                           table_->item(r, 1)->text().toLower().contains(query) ||
                           table_->item(r, 2)->text().toLower().contains(query) ||
                           table_->item(r, 7)->text().toLower().contains(query);

        bool show = match_action && match_query;
        table_->setRowHidden(r, !show);
        if (show) visible_count++;
    }

    lblCount_->setText(QString("Total Streamed: %1 | Displayed: %2")
        .arg(records_.size())
        .arg(visible_count));
}

void TransactionStreamWidget::clearStream() {
    records_.clear();
    table_->setRowCount(0);
    lblCount_->setText("Total Streamed: 0 | Displayed: 0");
}

} // namespace epfd::gui
