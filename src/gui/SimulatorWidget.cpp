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
    auto* grp_attacks = new QGroupBox(QString::fromUtf8("Các kịch bản mô phỏng tấn công & lưu lượng"), this);
    auto* grid = new QGridLayout(grp_attacks);
    grid->setSpacing(10);

    btnNormal_ = new QPushButton(QString::fromUtf8("🛒 Tạo luồng giao dịch thông thường hợp lệ"), this);
    btnNormal_->setStyleSheet("background-color: #238636; color: white;");
    connect(btnNormal_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::NORMAL_LEGITIMATE, static_cast<size_t>(spnCount_->value()));
    });

    btnVelocityBurst_ = new QPushButton(QString::fromUtf8("⚡ Tấn công dồn dập (Burst Velocity - 10 GD / 2s)"), this);
    btnVelocityBurst_->setStyleSheet("background-color: #d29922; color: white;");
    connect(btnVelocityBurst_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::BURST_VELOCITY_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    btnImpossibleTravel_ = new QPushButton(QString::fromUtf8("✈️ Di chuyển bất khả thi (Hà Nội ➔ Paris)"), this);
    btnImpossibleTravel_->setStyleSheet("background-color: #db6d28; color: white;");
    connect(btnImpossibleTravel_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::IMPOSSIBLE_TRAVEL_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    btnCardTesting_ = new QPushButton(QString::fromUtf8("💳 Tấn công dò thẻ (Card Testing - Vi xác thực $1.00)"), this);
    btnCardTesting_->setStyleSheet("background-color: #d29922; color: white;");
    connect(btnCardTesting_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::CARD_TESTING_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    btnMuleSmurfing_ = new QPushButton(QString::fromUtf8("🕵️ Rửa tiền tài khoản trung gian (Mule Smurfing)"), this);
    btnMuleSmurfing_->setStyleSheet("background-color: #8957e5; color: white;");
    connect(btnMuleSmurfing_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::MULE_SMURFING_ATTACK, static_cast<size_t>(spnCount_->value()));
    });

    btnATO_ = new QPushButton(QString::fromUtf8("📱 Chiếm đoạt tài khoản (Máy ảo Android đã Root)"), this);
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
    auto* grp_stream = new QGroupBox(QString::fromUtf8("Trình tạo luồng giao dịch thời gian thực liên tục"), this);
    auto* lyt_stream = new QVBoxLayout(grp_stream);

    auto* lyt_controls = new QHBoxLayout();
    lyt_controls->addWidget(new QLabel(QString::fromUtf8("Kích thước lô mỗi lần kích hoạt:"), this));
    spnCount_ = new QSpinBox(this);
    spnCount_->setRange(1, 1000);
    spnCount_->setValue(10);
    lyt_controls->addWidget(spnCount_);

    btnMixedBatch_ = new QPushButton(QString::fromUtf8("🎲 Tạo lô giao dịch thực tế hỗn hợp"), this);
    btnMixedBatch_->setStyleSheet("background-color: #1f6feb; color: white;");
    connect(btnMixedBatch_, &QPushButton::clicked, [this]() {
        emit triggerScenarioBatch(SimulationScenario::MIXED_REALISTIC_TRAFFIC, static_cast<size_t>(spnCount_->value()));
    });
    lyt_controls->addWidget(btnMixedBatch_);
    lyt_stream->addLayout(lyt_controls);

    auto* lyt_continuous = new QHBoxLayout();
    btnContinuous_ = new QPushButton(QString::fromUtf8("▶ Bắt đầu luồng thời gian thực"), this);
    btnContinuous_->setStyleSheet("background-color: #238636; color: white; font-size: 14px;");
    connect(btnContinuous_, &QPushButton::clicked, this, &SimulatorWidget::onContinuousToggled);

    lblSpeed_ = new QLabel(QString::fromUtf8("Tốc độ luồng: 100 ms/GD (10 TPS)"), this);
    sldSpeed_ = new QSlider(Qt::Horizontal, this);
    sldSpeed_->setRange(10, 1000);
    sldSpeed_->setValue(100);
    connect(sldSpeed_, &QSlider::valueChanged, [this](int val) {
        lblSpeed_->setText(QString::fromUtf8("Tốc độ luồng: %1 ms/GD (%2 TPS)")
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
        btnContinuous_->setText(QString::fromUtf8("⏸ Tạm dừng luồng thời gian thực"));
        btnContinuous_->setStyleSheet("background-color: #da3633; color: white; font-size: 14px;");
        emit startContinuousStream(sldSpeed_->value());
    } else {
        is_streaming_ = false;
        btnContinuous_->setText(QString::fromUtf8("▶ Tiếp tục luồng thời gian thực"));
        btnContinuous_->setStyleSheet("background-color: #238636; color: white; font-size: 14px;");
        emit pauseContinuousStream();
    }
}

} // namespace epfd::gui
