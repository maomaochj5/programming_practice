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
// #include <QSoundEffect>

// 前向声明
class QSoundEffect;

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

    explicit PaymentDialog(double totalAmount, QWidget *parent = nullptr);

    PaymentMethod getPaymentMethod() const;
    double getCashAmount() const;
    double getCardAmount() const;
    double getMobileAmount() const;
    double getChangeAmount() const;
    PaymentResult getResult() const;

private slots:
    void onPaymentMethodChanged();
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
    void updatePaymentDisplay();
    void showPaymentProcessing();
    void hidePaymentProcessing();
    void playSuccessSound();
    void playErrorSound();

    // UI Components
    QLabel *m_totalLabel;
    QLabel *m_changeLabel;
    
    QButtonGroup *m_paymentMethodGroup;
    QRadioButton *m_cashRadio;
    QRadioButton *m_cardRadio;
    QRadioButton *m_mobileRadio;
    QRadioButton *m_mixedRadio;
    
    QDoubleSpinBox *m_cashAmountSpinBox;
    QDoubleSpinBox *m_cardAmountSpinBox;
    QDoubleSpinBox *m_mobileAmountSpinBox;
    
    QLabel *m_cashAmountLabel;
    QLabel *m_cardAmountLabel;
    QLabel *m_mobileAmountLabel;
    
    QPushButton *m_processButton;
    QPushButton *m_cancelButton;
    
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    // Data
    double m_totalAmount;
    double m_changeAmount;
    PaymentResult m_result;
    
    QTimer *m_paymentTimer;
    QSoundEffect *m_successSound;
    QSoundEffect *m_errorSound;
};

#endif // PAYMENTDIALOG_H
