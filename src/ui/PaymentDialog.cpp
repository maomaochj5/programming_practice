#include "PaymentDialog.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>
#include <QSoundEffect>
#include <QIcon>
#include <QMessageBox>

#include "../controllers/CheckoutController.h"

PaymentDialog::PaymentDialog(double totalAmount, CheckoutController* checkoutController, QWidget *parent)
    : QDialog(parent)
    , m_totalAmount(totalAmount)
    , m_changeAmount(0.0)
    , m_result(Cancelled)
    , m_selectedMethod(Cash)
    , m_checkoutController(checkoutController) // Initialize controller
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
    amountLayout->addRow(tr("总金额:"), m_totalLabel);

    m_changeLabel = new QLabel("¥0.00", this);
    amountLayout->addRow(tr("找零:"), m_changeLabel);

    mainLayout->addWidget(amountGroup);

    // 支付方式选择
    auto *methodGroup = new QGroupBox(tr("选择支付方式"), this);
    auto *methodLayout = new QHBoxLayout(methodGroup);
    methodLayout->setSpacing(15);

    m_cashButton = new QPushButton(QIcon(":/resources/cash.png"), tr("现金"), this);
    m_cardButton = new QPushButton(QIcon(":/resources/card.png"), tr("银行卡"), this);
    m_mobileButton = new QPushButton(QIcon(":/resources/mobile_pay.png"), tr("移动支付"), this);

    QPushButton* buttons[] = { m_cashButton, m_cardButton, m_mobileButton };
    for(auto* button : buttons) {
        button->setIconSize(QSize(48, 48));
        button->setMinimumHeight(80);
        button->setCheckable(true);
        methodLayout->addWidget(button);
    }
    
    m_cashButton->setChecked(true);

    mainLayout->addWidget(methodGroup);

    // 金额输入区域
    auto *inputGroup = new QGroupBox(tr("支付金额"), this);
    auto *inputLayout = new QFormLayout(inputGroup);

    // 现金金额
    m_cashAmountLabel = new QLabel(tr("支付现金:"), this);
    m_cashAmountSpinBox = new QDoubleSpinBox(this);
    m_cashAmountSpinBox->setRange(0.0, 999999.99);
    m_cashAmountSpinBox->setDecimals(2);
    m_cashAmountSpinBox->setSuffix(" 元");
    m_cashAmountSpinBox->setValue(m_totalAmount);
    m_cashAmountSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    inputLayout->addRow(m_cashAmountLabel, m_cashAmountSpinBox);

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
    m_processButton->setObjectName("processButton"); // for specific styling
    buttonLayout->addWidget(m_processButton);

    mainLayout->addLayout(buttonLayout);
}

void PaymentDialog::setupConnections()
{
    connect(m_cashButton, &QPushButton::clicked, this, [this](){
        m_selectedMethod = Cash;
        m_cardButton->setChecked(false);
        m_mobileButton->setChecked(false);
        onPaymentMethodChanged();
    });
    connect(m_cardButton, &QPushButton::clicked, this, [this](){
        m_selectedMethod = Card;
        m_cashButton->setChecked(false);
        m_mobileButton->setChecked(false);
        onPaymentMethodChanged();
    });
    connect(m_mobileButton, &QPushButton::clicked, this, [this](){
        m_selectedMethod = Mobile;
        m_cashButton->setChecked(false);
        m_cardButton->setChecked(false);
        onPaymentMethodChanged();
    });
    
    connect(m_cashAmountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PaymentDialog::onCashAmountChanged);

    connect(m_processButton, &QPushButton::clicked, this, &PaymentDialog::processPayment);
    connect(m_cancelButton, &QPushButton::clicked, this, &PaymentDialog::cancelPayment);

    connect(m_paymentTimer, &QTimer::timeout, this, &PaymentDialog::onPaymentTimeout);
}

void PaymentDialog::onPaymentMethodChanged()
{
    bool isCash = (m_selectedMethod == Cash);
    m_cashAmountLabel->setVisible(isCash);
    m_cashAmountSpinBox->setVisible(isCash);

    if (isCash) {
        m_cashAmountSpinBox->setValue(m_totalAmount);
        m_cashAmountSpinBox->setFocus();
        m_cashAmountSpinBox->selectAll();
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
    double paidAmount = 0;
    if (m_selectedMethod == Cash) {
        paidAmount = getCashAmount();
    } else {
        paidAmount = m_totalAmount; // For Card and Mobile, assume exact amount is paid
    }

    m_changeAmount = paidAmount - m_totalAmount;
    
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

    bool hasChange = m_changeAmount > 0;
    m_changeLabel->setText(QString("¥%1").arg(m_changeAmount, 0, 'f', 2));
}

void PaymentDialog::processPayment()
{
    if (!m_checkoutController) {
        QMessageBox::critical(this, "严重错误", "收银控制器未初始化！");
        return;
    }

    QString methodStr;
    switch (m_selectedMethod) {
        case Cash: methodStr = "Cash"; break;
        case Card: methodStr = "Card"; break;
        case Mobile: methodStr = "Mobile"; break;
        default: methodStr = "Unknown";
    }

    double paidAmount = (m_selectedMethod == Cash) ? getCashAmount() : m_totalAmount;

    // Delegate payment processing to the controller
    bool success = m_checkoutController->processPayment(methodStr, m_totalAmount, paidAmount);

    if (success) {
        m_result = Success;
        playSuccessSound();
        accept(); // Close dialog on success
    } else {
        m_result = Failed;
        playErrorSound();
        // The controller should have emitted an error signal with a message.
        // We can optionally show a generic message here too.
        QMessageBox::warning(this, "支付失败", "支付处理失败，请检查金额或联系管理员。");
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
    return m_selectedMethod;
}

double PaymentDialog::getCashAmount() const
{
    return m_cashAmountSpinBox->value();
}

double PaymentDialog::getCardAmount() const
{
    return (m_selectedMethod == Card) ? m_totalAmount : 0.0;
}

double PaymentDialog::getMobileAmount() const
{
    return (m_selectedMethod == Mobile) ? m_totalAmount : 0.0;
}

double PaymentDialog::getChangeAmount() const
{
    return m_changeAmount;
}

PaymentDialog::PaymentResult PaymentDialog::getResult() const
{
    return m_result;
}
