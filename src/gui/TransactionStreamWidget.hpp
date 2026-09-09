#ifndef EPFD_GUI_TRANSACTION_STREAM_WIDGET_HPP
#define EPFD_GUI_TRANSACTION_STREAM_WIDGET_HPP

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include "epfd/epfd.hpp"
#include "ExplainabilityDialog.hpp"

namespace epfd::gui {

struct EvaluatedTxRecord {
    Transaction tx;
    DecisionResult decision;
    RiskAssessment risk;
    TransactionFeatures features;
};

class TransactionStreamWidget : public QWidget {
    Q_OBJECT

public:
    explicit TransactionStreamWidget(QWidget* parent = nullptr);

    void addEvaluatedTransaction(const Transaction& tx,
                                 const DecisionResult& decision,
                                 const RiskAssessment& risk,
                                 const TransactionFeatures& features);
    void clearStream();

signals:
    void openCaseFromStream(const QString& tx_id, const QString& cust_id, double risk_score, int decision_action);

private slots:
    void onCellDoubleClicked(int row, int column);
    void onFilterChanged();

private:
    void setupUi();
    void updateTableRow(int row, const EvaluatedTxRecord& rec);

    QTableWidget* table_{nullptr};
    QLineEdit* txtSearch_{nullptr};
    QComboBox* cmbFilterAction_{nullptr};
    QLabel* lblCount_{nullptr};
    QPushButton* btnClear_{nullptr};

    std::vector<EvaluatedTxRecord> records_;
};

} // namespace epfd::gui

#endif // EPFD_GUI_TRANSACTION_STREAM_WIDGET_HPP
