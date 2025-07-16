#ifndef PAYMENTDIALOG_H
#define PAYMENTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTimer>
#include <QProgressBar>
#include <QMessageBox>
#include <QSoundEffect>

// 前向声明
class QSoundEffect;
class CheckoutController; // Forward declaration

class PaymentDialog : public QDialog
{
    Q_OBJECT

public:
    enum PaymentMethod {
        Cash = 0,
        Card,
        Mobile,
        Mixed
    };

    enum PaymentResult {
        Success = 0,
        Cancelled,
        Failed,
        Timeout
    };

    /**
     * @brief 构造函数
     * @param totalAmount 总金额
     * @param checkoutController 收银控制器
     * @param parent 父窗口指针
     */
    PaymentDialog(double totalAmount, CheckoutController* checkoutController, QWidget *parent = nullptr);

    PaymentMethod getPaymentMethod() const;
    double getCashAmount() const;
    double getCardAmount() const;
    double getMobileAmount() const;
    double getChangeAmount() const;
    PaymentResult getResult() const;

private slots:
    void onCashAmountChanged();
    void calculateChange();
    void processPayment();
    void cancelPayment();
    void simulateCardPayment();
    void simulateMobilePayment();
    void onPaymentTimeout();

private:
    void setupUI();
    void setupConnections();
    void setupSounds();
    void onPaymentMethodChanged();
    void updatePaymentDisplay();
    void showPaymentProcessing();
    void hidePaymentProcessing();
    void playSuccessSound();
    void playErrorSound();

    // UI Components
    QLabel *m_totalLabel;
    QLabel *m_changeLabel;
    
    // Payment method buttons
    QPushButton *m_cashButton;
    QPushButton *m_cardButton;
    QPushButton *m_mobileButton;
    
    QDoubleSpinBox *m_cashAmountSpinBox;
    
    QLabel *m_cashAmountLabel;
    
    QPushButton *m_processButton;
    QPushButton *m_cancelButton;
    
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    // Data
    double m_totalAmount;
    double m_changeAmount;
    PaymentResult m_result;
    PaymentMethod m_selectedMethod;
    CheckoutController* m_checkoutController; // Add controller member
    
    QTimer *m_paymentTimer;
    QSoundEffect *m_successSound;
    QSoundEffect *m_errorSound;
};

#endif // PAYMENTDIALOG_H
