#ifndef PRODUCTDIALOG_H
#define PRODUCTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QRegularExpressionValidator>

#include "../models/Product.h"

class ProductDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProductDialog(QWidget *parent = nullptr);
    explicit ProductDialog(const Product &product, QWidget *parent = nullptr);

    Product getProduct() const;
    void setProduct(const Product &product);

private slots:
    void selectImage();
    void generateBarcode();
    void validateInput();

private:
    void setupUI();
    void setupValidators();
    void setupConnections();
    void updateImagePreview();

    // UI Components
    QLineEdit *m_nameEdit;
    QLineEdit *m_barcodeEdit;
    QDoubleSpinBox *m_priceSpinBox;
    QSpinBox *m_stockSpinBox;
    QSpinBox *m_minStockSpinBox;
    QComboBox *m_categoryCombo;
    QTextEdit *m_descriptionEdit;
    QLineEdit *m_supplierEdit;
    QLineEdit *m_imagePathEdit;
    QPushButton *m_selectImageButton;
    QPushButton *m_generateBarcodeButton;
    QLabel *m_imagePreview;

    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    // Data
    Product m_product;
    bool m_editMode;
};

#endif // PRODUCTDIALOG_H
