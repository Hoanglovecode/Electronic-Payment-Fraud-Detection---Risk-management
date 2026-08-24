#ifndef EPFD_GUI_EXPLAINABILITY_DIALOG_HPP
#define EPFD_GUI_EXPLAINABILITY_DIALOG_HPP

#include <QDialog>
#include <QLabel>
#include <QTableWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include "epfd/epfd.hpp"

namespace epfd::gui {

class ExplainabilityDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExplainabilityDialog(const Transaction& tx,
                                  const DecisionResult& decision,
                                  const RiskAssessment& risk,
                                  const TransactionFeatures& features,
                                  QWidget* parent = nullptr);

signals:
    void openCaseRequested(const QString& tx_id, const QString& cust_id, double risk_score, int decision_action);

private slots:
    void onOpenCaseClicked();

private:
    void setupUi();
    void populateData();

    Transaction tx_;
    DecisionResult decision_;
    RiskAssessment risk_;
    TransactionFeatures features_;

    QLabel* lblHeader_{nullptr};
    QLabel* lblDecisionBadge_{nullptr};
    QLabel* lblRiskScore_{nullptr};
    QProgressBar* pbRisk_{nullptr};
    QTableWidget* tblFeatures_{nullptr};
    QTableWidget* tblAlerts_{nullptr};
    QPushButton* btnOpenCase_{nullptr};
    QPushButton* btnClose_{nullptr};
};

} // namespace epfd::gui

#endif // EPFD_GUI_EXPLAINABILITY_DIALOG_HPP
