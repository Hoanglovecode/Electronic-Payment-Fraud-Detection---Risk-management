#ifndef EPFD_GUI_CASE_MANAGEMENT_WIDGET_HPP
#define EPFD_GUI_CASE_MANAGEMENT_WIDGET_HPP

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <memory>
#include "epfd/epfd.hpp"

namespace epfd::gui {

class CaseManagementWidget : public QWidget {
    Q_OBJECT

public:
    explicit CaseManagementWidget(std::shared_ptr<LabelStore> label_store,
                                  std::shared_ptr<OutcomeTracker> outcome_tracker,
                                  QWidget* parent = nullptr);

    void createCase(const QString& tx_id, const QString& cust_id, double risk_score, int decision_action);
    void refreshCaseTable();

private slots:
    void onCaseSelected(int row, int column);
    void onAssignAnalyst();
    void onAddEvidence();
    void onResolveCase();
    void onRecordOutcome();
    void onExportCsv();

private:
    void setupUi();

    std::shared_ptr<LabelStore> label_store_;
    std::shared_ptr<OutcomeTracker> outcome_tracker_;
    std::unique_ptr<CaseManager> case_manager_;

    QTableWidget* tblCases_{nullptr};
    QLabel* lblSelectedCase_{nullptr};
    QComboBox* cmbAnalyst_{nullptr};
    QPushButton* btnAssign_{nullptr};

    QTextEdit* txtEvidence_{nullptr};
    QPushButton* btnAddEvidence_{nullptr};

    QComboBox* cmbResolution_{nullptr};
    QLineEdit* txtResolutionNotes_{nullptr};
    QPushButton* btnResolve_{nullptr};

    QDoubleSpinBox* spnLossAmount_{nullptr};
    QLineEdit* txtOutcomeNotes_{nullptr};
    QPushButton* btnRecordOutcome_{nullptr};

    QPushButton* btnExportCsv_{nullptr};
    QLabel* lblStatus_{nullptr};

    std::vector<std::string> case_ids_;
};

} // namespace epfd::gui

#endif // EPFD_GUI_CASE_MANAGEMENT_WIDGET_HPP
