#ifndef EPFD_GUI_BENCHMARK_WIDGET_HPP
#define EPFD_GUI_BENCHMARK_WIDGET_HPP

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <memory>
#include "epfd/epfd.hpp"

namespace epfd::gui {

class BenchmarkWidget : public QWidget {
    Q_OBJECT

public:
    explicit BenchmarkWidget(std::shared_ptr<DecisionEngine> decision_engine,
                             std::shared_ptr<TransactionService> service,
                             QWidget* parent = nullptr);

private slots:
    void onRunPipelineBenchmark();
    void onRunDsaBenchmark();

private:
    void setupUi();

    std::shared_ptr<DecisionEngine> decision_engine_;
    std::shared_ptr<TransactionService> service_;

    QSpinBox* spnTxCount_{nullptr};
    QPushButton* btnRunPipeline_{nullptr};
    QLabel* lblTps_{nullptr};
    QLabel* lblMean_{nullptr};
    QLabel* lblP50_{nullptr};
    QLabel* lblP95_{nullptr};
    QLabel* lblP99_{nullptr};

    QPushButton* btnRunDsa_{nullptr};
    QTableWidget* tblDsa_{nullptr};
};

} // namespace epfd::gui

#endif // EPFD_GUI_BENCHMARK_WIDGET_HPP
