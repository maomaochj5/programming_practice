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
    explicit ProductDialog(const Product* product, QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ProductDialog();

    /**
     * @brief 获取商品对象
     * @return 商品智能指针
     */
    std::unique_ptr<Product> getProduct() const;

    /**
     * @brief 设置商品对象
     * @param product 商品指针
     */
    void setProduct(const Product* product);

    /**
     * @brief 接受对话框
     */
    void accept() override;

    /**
     * @brief 拒绝对话框
     */
    void reject() override;

private slots:
    void selectImage();
    void generateBarcode();
    void validateInput();
    void onAccept();

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
