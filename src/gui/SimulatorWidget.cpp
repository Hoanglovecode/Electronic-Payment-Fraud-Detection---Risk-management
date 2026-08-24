#include "SimulatorWidget.hpp"

namespace epfd::gui {

SimulatorWidget::SimulatorWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void SimulatorWidget::setupUi() {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(12, 12, 12, 12);
    main_layout->setSpacing(12);

    // Attack Scenario Triggers
    auto* grp_attacks = new QGroupBox("Synthetic Attack & Traffic Scenarios", this);
    auto* grid = new QGridLayout(grp_attacks);
    grid->setSpacing(10);

    btnNormal_ = new QPushButton("🛒 Generate Normal Legitimate Stream", this);
    btnNormal_->setStyleSheet("background-color: #238636; color: white;");
    connect(btnNormal_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::NORMAL_LEGITIMATE, static_cast<size_t>(spnCount_->value()));
    });

    btnVelocityBurst_ = new QPushButton("⚡ Burst Velocity Attack (10 Tx in 2s)", this);
    btnVelocityBurst_->setStyleSheet("background-color: #d29922; color: white;");
    connect(btnVelocityBurst_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::BURST_VELOCITY_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    btnImpossibleTravel_ = new QPushButton("✈️ Impossible Travel (Hanoi ➔ Paris)", this);
    btnImpossibleTravel_->setStyleSheet("background-color: #db6d28; color: white;");
    connect(btnImpossibleTravel_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::IMPOSSIBLE_TRAVEL_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    btnCardTesting_ = new QPushButton("💳 Card Testing Attack ($1.00 Micro-auths)", this);
    btnCardTesting_->setStyleSheet("background-color: #d29922; color: white;");
    connect(btnCardTesting_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::CARD_TESTING_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    btnMuleSmurfing_ = new QPushButton("🕵️ Mule Smurfing AML Structuring", this);
    btnMuleSmurfing_->setStyleSheet("background-color: #8957e5; color: white;");
    connect(btnMuleSmurfing_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::MULE_SMURFING_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    btnATO_ = new QPushButton("📱 Account Takeover (Rooted Emulator)", this);
    btnATO_->setStyleSheet("background-color: #da3633; color: white; font-weight: bold;");
    connect(btnATO_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::ACCOUNT_TAKEOVER_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    grid->addWidget(btnNormal_, 0, 0);
    grid->addWidget(btnVelocityBurst_, 0, 1);
    grid->addWidget(btnImpossibleTravel_, 1, 0);
    grid->addWidget(btnCardTesting_, 1, 1);
    grid->addWidget(btnMuleSmurfing_, 2, 0);
    grid->addWidget(btnATO_, 2, 1);

    main_layout->addWidget(grp_attacks);

    // Batch & Continuous Streaming Controls
    auto* grp_stream = new QGroupBox("Continuous Live Stream Generator", this);
    auto* lyt_stream = new QVBoxLayout(grp_stream);

    auto* lyt_controls = new QHBoxLayout();
    lyt_controls->addWidget(new QLabel("Batch Size per Trigger:", this));
    spnCount_ = new QSpinBox(this);
    spnCount_->setRange(1, 1000);
    spnCount_->setValue(10);
    lyt_controls->addWidget(spnCount_);

    btnMixedBatch_ = new QPushButton("🎲 Generate Mixed Realistic Traffic Batch", this);
    btnMixedBatch_->setStyleSheet("background-color: #1f6feb; color: white;");
    connect(btnMixedBatch_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::MIXED_REALISTIC_TRAFFIC, static_cast<size_t>(spnCount_->value()));
    });
    lyt_controls->addWidget(btnMixedBatch_);
    lyt_stream->addLayout(lyt_controls);

    auto* lyt_continuous = new QHBoxLayout();
    btnContinuous_ = new QPushButton("▶ Start Continuous Real-time Stream", this);
    btnContinuous_->setStyleSheet("background-color: #238636; color: white; font-size: 14px;");
    connect(btnContinuous_, &QPushButton::clicked, this, &SimulatorWidget::onContinuousToggled);

    lblSpeed_ = new QLabel("Stream Speed: 100 ms/tx (10 TPS)", this);
    sldSpeed_ = new QSlider(Qt::Horizontal, this);
    sldSpeed_->setRange(10, 1000);
    sldSpeed_->setValue(100);
    connect(sldSpeed_, &QSlider::valueChanged, [this](int val) {
        lblSpeed_->setText(QString("Stream Speed: %1 ms/tx (%2 TPS)")
            .arg(val)
            .arg(1000 / (val > 0 ? val : 1)));
        if (is_streaming_) {
            emit startContinuousStream(val);
        }
    });

    lyt_continuous->addWidget(btnContinuous_);
    lyt_continuous->addWidget(lblSpeed_);
    lyt_continuous->addWidget(sldSpeed_, 1);
    lyt_stream->addLayout(lyt_continuous);

    main_layout->addWidget(grp_stream);
    main_layout->addStretch();
}

void SimulatorWidget::onContinuousToggled() {
    if (!is_streaming_) {
        is_streaming_ = true;
        btnContinuous_->setText("⏸ Pause Continuous Stream");
        btnContinuous_->setStyleSheet("background-color: #da3633; color: white; font-size: 14px;");
        emit startContinuousStream(sldSpeed_->value());
    } else {
        is_streaming_ = false;
        btnContinuous_->setText("▶ Resume Continuous Stream");
        btnContinuous_->setStyleSheet("background-color: #238636; color: white; font-size: 14px;");
        emit pauseContinuousStream();
    }
}

} // namespace epfd::gui
