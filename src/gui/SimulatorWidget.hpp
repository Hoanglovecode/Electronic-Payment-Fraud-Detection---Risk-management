#ifndef EPFD_GUI_SIMULATOR_WIDGET_HPP
#define EPFD_GUI_SIMULATOR_WIDGET_HPP

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include "epfd/epfd.hpp"

namespace epfd::gui {

class SimulatorWidget : public QWidget {
    Q_OBJECT

public:
    explicit SimulatorWidget(QWidget* parent = nullptr);

signals:
    void triggerScenarioBatch(SimulationScenario scenario, size_t count);
    void startContinuousStream(int interval_ms);
    void pauseContinuousStream();

private slots:
    void onContinuousToggled();

private:
    void setupUi();

    QPushButton* btnNormal_{nullptr};
    QPushButton* btnVelocityBurst_{nullptr};
    QPushButton* btnImpossibleTravel_{nullptr};
    QPushButton* btnCardTesting_{nullptr};
    QPushButton* btnMuleSmurfing_{nullptr};
    QPushButton* btnATO_{nullptr};
    QPushButton* btnMixedBatch_{nullptr};

    QSpinBox* spnCount_{nullptr};
    QPushButton* btnContinuous_{nullptr};
    QSlider* sldSpeed_{nullptr};
    QLabel* lblSpeed_{nullptr};

    bool is_streaming_{false};
};

} // namespace epfd::gui

#endif // EPFD_GUI_SIMULATOR_WIDGET_HPP
