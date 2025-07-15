#include "PaymentDialog.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>
#include <QSoundEffect>

PaymentDialog::PaymentDialog(double totalAmount, QWidget *parent)
    : QDialog(parent)
    , m_totalAmount(totalAmount)
    , m_changeAmount(0.0)
    , m_result(Cancelled)
    , m_paymentTimer(new QTimer(this))
    , m_successSound(new QSoundEffect(this))
    , m_errorSound(new QSoundEffect(this))
{
    setWindowTitle(tr("支付"));
    setModal(true);
    setupUI();
    setupConnections();
    updatePaymentDisplay();
    resize(450, 500);
    
    setupSounds();
}

void PaymentDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // 总金额显示
    auto *amountGroup = new QGroupBox(tr("订单信息"), this);
    auto *amountLayout = new QFormLayout(amountGroup);

    m_totalLabel = new QLabel(QString("¥%1").arg(m_totalAmount, 0, 'f', 2), this);
    m_totalLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #d32f2f;");
    amountLayout->addRow(tr("总金额:"), m_totalLabel);

    m_changeLabel = new QLabel("¥0.00", this);
    m_changeLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #388e3c;");
    amountLayout->addRow(tr("找零:"), m_changeLabel);

    mainLayout->addWidget(amountGroup);

    // 支付方式选择
    auto *methodGroup = new QGroupBox(tr("支付方式"), this);
    auto *methodLayout = new QVBoxLayout(methodGroup);

    m_paymentMethodGroup = new QButtonGroup(this);

    m_cashRadio = new QRadioButton(tr("现金支付"), this);
    m_cashRadio->setChecked(true);
    m_paymentMethodGroup->addButton(m_cashRadio, Cash);
    methodLayout->addWidget(m_cashRadio);

    m_cardRadio = new QRadioButton(tr("银行卡支付"), this);
    m_paymentMethodGroup->addButton(m_cardRadio, Card);
    methodLayout->addWidget(m_cardRadio);

    m_mobileRadio = new QRadioButton(tr("移动支付（扫码）"), this);
    m_paymentMethodGroup->addButton(m_mobileRadio, Mobile);
    methodLayout->addWidget(m_mobileRadio);

    m_mixedRadio = new QRadioButton(tr("组合支付"), this);
    m_paymentMethodGroup->addButton(m_mixedRadio, Mixed);
    methodLayout->addWidget(m_mixedRadio);

    mainLayout->addWidget(methodGroup);

    // 金额输入区域
    auto *inputGroup = new QGroupBox(tr("支付金额"), this);
    auto *inputLayout = new QFormLayout(inputGroup);

    // 现金金额
    m_cashAmountLabel = new QLabel(tr("现金金额:"), this);
    m_cashAmountSpinBox = new QDoubleSpinBox(this);
    m_cashAmountSpinBox->setRange(0.0, 999999.99);
    m_cashAmountSpinBox->setDecimals(2);
    m_cashAmountSpinBox->setSuffix(" 元");
    m_cashAmountSpinBox->setValue(m_totalAmount);
    inputLayout->addRow(m_cashAmountLabel, m_cashAmountSpinBox);

    // 银行卡金额
    m_cardAmountLabel = new QLabel(tr("银行卡金额:"), this);
    m_cardAmountSpinBox = new QDoubleSpinBox(this);
    m_cardAmountSpinBox->setRange(0.0, 999999.99);
    m_cardAmountSpinBox->setDecimals(2);
    m_cardAmountSpinBox->setSuffix(" 元");
    m_cardAmountSpinBox->setVisible(false);
    m_cardAmountLabel->setVisible(false);
    inputLayout->addRow(m_cardAmountLabel, m_cardAmountSpinBox);

    // 移动支付金额
    m_mobileAmountLabel = new QLabel(tr("移动支付金额:"), this);
    m_mobileAmountSpinBox = new QDoubleSpinBox(this);
    m_mobileAmountSpinBox->setRange(0.0, 999999.99);
    m_mobileAmountSpinBox->setDecimals(2);
    m_mobileAmountSpinBox->setSuffix(" 元");
    m_mobileAmountSpinBox->setVisible(false);
    m_mobileAmountLabel->setVisible(false);
    inputLayout->addRow(m_mobileAmountLabel, m_mobileAmountSpinBox);

    mainLayout->addWidget(inputGroup);

    // 支付进度显示
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setVisible(false);
    mainLayout->addWidget(m_statusLabel);

    // 按钮区域
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(tr("取消"), this);
    buttonLayout->addWidget(m_cancelButton);

    m_processButton = new QPushButton(tr("确认支付"), this);
    m_processButton->setDefault(true);
    m_processButton->setStyleSheet("QPushButton { background-color: #4caf50; color: white; font-weight: bold; }");
    buttonLayout->addWidget(m_processButton);

    mainLayout->addLayout(buttonLayout);
}

void PaymentDialog::setupConnections()
{
    connect(m_paymentMethodGroup, &QButtonGroup::buttonClicked,
            this, &PaymentDialog::onPaymentMethodChanged);
    
    connect(m_cashAmountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PaymentDialog::onCashAmountChanged);
    connect(m_cardAmountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PaymentDialog::calculateChange);
    connect(m_mobileAmountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PaymentDialog::calculateChange);

    connect(m_processButton, &QPushButton::clicked, this, &PaymentDialog::processPayment);
    connect(m_cancelButton, &QPushButton::clicked, this, &PaymentDialog::cancelPayment);

    connect(m_paymentTimer, &QTimer::timeout, this, &PaymentDialog::onPaymentTimeout);
}

void PaymentDialog::onPaymentMethodChanged()
{
    PaymentMethod method = static_cast<PaymentMethod>(m_paymentMethodGroup->checkedId());
    
    // 隐藏所有金额输入
    m_cardAmountLabel->setVisible(false);
    m_cardAmountSpinBox->setVisible(false);
    m_mobileAmountLabel->setVisible(false);
    m_mobileAmountSpinBox->setVisible(false);
    
    // 重置金额
    m_cardAmountSpinBox->setValue(0.0);
    m_mobileAmountSpinBox->setValue(0.0);
    
    switch (method) {
    case Cash:
        m_cashAmountSpinBox->setValue(m_totalAmount);
        break;
        
    case Card:
        m_cashAmountSpinBox->setValue(0.0);
        m_cardAmountSpinBox->setValue(m_totalAmount);
        m_cardAmountLabel->setVisible(true);
        m_cardAmountSpinBox->setVisible(true);
        break;
        
    case Mobile:
        m_cashAmountSpinBox->setValue(0.0);
        m_mobileAmountSpinBox->setValue(m_totalAmount);
        m_mobileAmountLabel->setVisible(true);
        m_mobileAmountSpinBox->setVisible(true);
        break;
        
    case Mixed:
        m_cardAmountLabel->setVisible(true);
        m_cardAmountSpinBox->setVisible(true);
        m_mobileAmountLabel->setVisible(true);
        m_mobileAmountSpinBox->setVisible(true);
        break;
    }
    
    calculateChange();
    updatePaymentDisplay();
}

void PaymentDialog::onCashAmountChanged()
{
    calculateChange();
}

void PaymentDialog::calculateChange()
{
    double totalPaid = getCashAmount() + getCardAmount() + getMobileAmount();
    m_changeAmount = totalPaid - m_totalAmount;
    
    if (m_changeAmount < 0) {
        m_changeLabel->setText(QString("还需: ¥%1").arg(-m_changeAmount, 0, 'f', 2));
        m_changeLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #d32f2f;");
        m_processButton->setEnabled(false);
    } else {
        m_changeLabel->setText(QString("¥%1").arg(m_changeAmount, 0, 'f', 2));
        m_changeLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #388e3c;");
        m_processButton->setEnabled(true);
    }
}

void PaymentDialog::updatePaymentDisplay()
{
    PaymentMethod method = getPaymentMethod();
    
    switch (method) {
    case Cash:
        m_processButton->setText(tr("收款"));
        break;
    case Card:
        m_processButton->setText(tr("刷卡支付"));
        break;
    case Mobile:
        m_processButton->setText(tr("扫码支付"));
        break;
    case Mixed:
        m_processButton->setText(tr("组合支付"));
        break;
    }
}

void PaymentDialog::processPayment()
{
    PaymentMethod method = getPaymentMethod();
    
    showPaymentProcessing();
    
    switch (method) {
    case Cash:
        // 现金支付立即完成
        m_result = Success;
        playSuccessSound();
        QTimer::singleShot(1000, this, &QDialog::accept);
        break;
        
    case Card:
        simulateCardPayment();
        break;
        
    case Mobile:
        simulateMobilePayment();
        break;
        
    case Mixed:
        // 组合支付模拟
        m_statusLabel->setText(tr("处理组合支付..."));
        QTimer::singleShot(3000, [this]() {
            m_result = Success;
            playSuccessSound();
            accept();
        });
        break;
    }
}

void PaymentDialog::cancelPayment()
{
    m_result = Cancelled;
    reject();
}

void PaymentDialog::simulateCardPayment()
{
    m_statusLabel->setText(tr("请插入银行卡..."));
    m_progressBar->setValue(25);
    
    QTimer::singleShot(2000, [this]() {
        m_statusLabel->setText(tr("请输入密码..."));
        m_progressBar->setValue(50);
        
        QTimer::singleShot(2000, [this]() {
            m_statusLabel->setText(tr("正在处理..."));
            m_progressBar->setValue(75);
            
            QTimer::singleShot(1500, [this]() {
                // 模拟90%成功率
                if (QRandomGenerator::global()->bounded(100) < 90) {
                    m_result = Success;
                    m_statusLabel->setText(tr("支付成功！"));
                    m_progressBar->setValue(100);
                    playSuccessSound();
                    QTimer::singleShot(1000, this, &QDialog::accept);
                } else {
                    m_result = Failed;
                    m_statusLabel->setText(tr("支付失败，请重试"));
                    playErrorSound();
                    hidePaymentProcessing();
                }
            });
        });
    });
    
    // 设置超时
    m_paymentTimer->start(30000); // 30秒超时
}

void PaymentDialog::simulateMobilePayment()
{
    m_statusLabel->setText(tr("请使用手机扫描二维码..."));
    m_progressBar->setValue(20);
    
    QTimer::singleShot(3000, [this]() {
        m_statusLabel->setText(tr("等待确认支付..."));
        m_progressBar->setValue(60);
        
        QTimer::singleShot(2000, [this]() {
            m_statusLabel->setText(tr("正在处理..."));
            m_progressBar->setValue(80);
            
            QTimer::singleShot(1000, [this]() {
                // 模拟95%成功率
                if (QRandomGenerator::global()->bounded(100) < 95) {
                    m_result = Success;
                    m_statusLabel->setText(tr("支付成功！"));
                    m_progressBar->setValue(100);
                    playSuccessSound();
                    QTimer::singleShot(1000, this, &QDialog::accept);
                } else {
                    m_result = Failed;
                    m_statusLabel->setText(tr("支付失败，请重试"));
                    playErrorSound();
                    hidePaymentProcessing();
                }
            });
        });
    });
    
    // 设置超时
    m_paymentTimer->start(60000); // 60秒超时
}

void PaymentDialog::onPaymentTimeout()
{
    m_paymentTimer->stop();
    m_result = Timeout;
    m_statusLabel->setText(tr("支付超时"));
    playErrorSound();
    hidePaymentProcessing();
}

void PaymentDialog::showPaymentProcessing()
{
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_statusLabel->setVisible(true);
    m_processButton->setEnabled(false);
}

void PaymentDialog::hidePaymentProcessing()
{
    m_progressBar->setVisible(false);
    m_statusLabel->setVisible(false);
    m_processButton->setEnabled(true);
    m_paymentTimer->stop();
}

void PaymentDialog::setupSounds()
{
    m_successSound->setSource(QUrl::fromLocalFile(":/sounds/success.wav"));
    m_successSound->setVolume(0.8);
    m_errorSound->setSource(QUrl::fromLocalFile(":/sounds/error.wav"));
    m_errorSound->setVolume(0.8);
}

void PaymentDialog::playSuccessSound()
{
    m_successSound->play();
}

void PaymentDialog::playErrorSound()
{
    m_errorSound->play();
}

PaymentDialog::PaymentMethod PaymentDialog::getPaymentMethod() const
{
    return static_cast<PaymentMethod>(m_paymentMethodGroup->checkedId());
}

double PaymentDialog::getCashAmount() const
{
    return m_cashAmountSpinBox->value();
}

double PaymentDialog::getCardAmount() const
{
    return m_cardAmountSpinBox->value();
}

double PaymentDialog::getMobileAmount() const
{
    return m_mobileAmountSpinBox->value();
}

double PaymentDialog::getChangeAmount() const
{
    return m_changeAmount;
}

PaymentDialog::PaymentResult PaymentDialog::getResult() const
{
    return m_result;
}
