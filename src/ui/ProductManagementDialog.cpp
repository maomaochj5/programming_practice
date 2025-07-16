#include "ProductManagementDialog.h"
#include "../controllers/ProductManager.h"
#include "../models/Product.h"
#include "ProductDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QMessageBox>

ProductManagementDialog::ProductManagementDialog(ProductManager* productManager, QWidget *parent)
    : QDialog(parent), m_productManager(productManager)
{
    setupUi();
    connectSignals();
    refreshProductList(); // Initial data load
}

ProductManagementDialog::~ProductManagementDialog()
{
}

void ProductManagementDialog::setupUi()
{
    setWindowTitle("商品管理");
    setMinimumSize(600, 400);

    m_productListWidget = new QListWidget(this);
    m_addButton = new QPushButton("添加商品", this);
    m_editButton = new QPushButton("编辑商品", this);
    m_deleteButton = new QPushButton("删除商品", this);
    m_closeButton = new QPushButton("关闭", this);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_productListWidget);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void ProductManagementDialog::connectSignals()
{
    connect(m_addButton, &QPushButton::clicked, this, &ProductManagementDialog::onAddProduct);
    connect(m_editButton, &QPushButton::clicked, this, &ProductManagementDialog::onEditProduct);
    connect(m_deleteButton, &QPushButton::clicked, this, &ProductManagementDialog::onDeleteProduct);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // Connect to ProductManager signals
    connect(m_productManager, &ProductManager::allProductsChanged, this, &ProductManagementDialog::onAllProductsChanged);
    connect(m_productManager, &ProductManager::productSaved, this, [this](bool success){
        onProductWriteCompleted(success, success ? "商品添加成功" : "添加商品失败");
    });
    connect(m_productManager, &ProductManager::productUpdated, this, [this](bool success){
        onProductWriteCompleted(success, success ? "商品更新成功" : "更新商品失败");
    });
    connect(m_productManager, &ProductManager::productDeleted, this, [this](bool success){
        onProductWriteCompleted(success, success ? "商品删除成功" : "删除商品失败");
    });
}

void ProductManagementDialog::refreshProductList()
{
    m_productManager->getAllProducts();
}

void ProductManagementDialog::onAllProductsChanged(const QList<Product*>& products)
{
    m_productListWidget->clear();
    for (Product* product : products) {
        if (product) {
             QListWidgetItem* item = new QListWidgetItem(product->getName(), m_productListWidget);
             item->setData(Qt::UserRole, QVariant::fromValue(product->getProductId()));
        }
    }
}


void ProductManagementDialog::onAddProduct()
{
    ProductDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        auto product = dialog.getProduct();
        if (product) {
            m_productManager->addProduct(product.release());
        }
    }
}

void ProductManagementDialog::onEditProduct()
{
    QListWidgetItem* selectedItem = m_productListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "错误", "请先选择要编辑的商品");
        return;
    }

    int productId = selectedItem->data(Qt::UserRole).toInt();
    Product* product = m_productManager->getProductById(productId);

    if (product) {
        ProductDialog dialog(product, this);
        if (dialog.exec() == QDialog::Accepted) {
            auto updatedProduct = dialog.getProduct();
            if (updatedProduct) {
                 m_productManager->updateProduct(updatedProduct.release());
            }
        }
    } else {
        QMessageBox::warning(this, "错误", "找不到所选商品。可能已被删除。");
        refreshProductList();
    }
}

void ProductManagementDialog::onDeleteProduct()
{
    QListWidgetItem* selectedItem = m_productListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "错误", "请先选择要删除的商品");
        return;
    }
    
    int productId = selectedItem->data(Qt::UserRole).toInt();
    QString productName = selectedItem->text();


    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除", QString("确定要删除商品 “%1” 吗？").arg(productName),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_productManager->deleteProduct(productId);
    }
}

void ProductManagementDialog::onProductWriteCompleted(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "成功", message);
        refreshProductList();
    } else {
        QMessageBox::warning(this, "失败", message);
    }
} 