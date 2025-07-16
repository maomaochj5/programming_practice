#ifndef PRODUCTMANAGEMENTDIALOG_H
#define PRODUCTMANAGEMENTDIALOG_H

#include <QDialog>
#include <QList>

class Product;
class ProductManager;
class QListWidget;
class QPushButton;

class ProductManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProductManagementDialog(ProductManager* productManager, QWidget *parent = nullptr);
    ~ProductManagementDialog();

private slots:
    void onAddProduct();
    void onEditProduct();
    void onDeleteProduct();
    void refreshProductList();
    void onAllProductsChanged(const QList<Product*>& products);
    void onProductWriteCompleted(bool success, const QString& message);


private:
    void setupUi();
    void connectSignals();

    ProductManager* m_productManager; // Non-owning pointer

    QListWidget* m_productListWidget;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_closeButton;
};

#endif // PRODUCTMANAGEMENTDIALOG_H 