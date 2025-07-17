#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H

#include <QObject>
#include <QHash>

class Product;
class DatabaseManager;

class ProductManager : public QObject
{
    Q_OBJECT
public:
    explicit ProductManager(QObject *parent = nullptr);
    ~ProductManager();

    // Public API for the UI
    void getAllProducts(); // Asynchronous trigger
    QList<Product*> getAllProductsSync() const;
    Product* getProductById(int id);
    Product* getProductByName(const QString& name);
    QList<Product*> getProductsByIds(const QList<int>& ids);
    void getProductByBarcode(const QString& barcode);
    void addProduct(Product* product);
    void updateProduct(Product* product);
    void deleteProduct(int id);
    QList<Product*> searchProducts(const QString& searchTerm);

signals:
    void allProductsChanged(const QList<Product*>& products);
    void productFoundByBarcode(Product* product, const QString& barcode);
    void productSaved(bool success);
    void productUpdated(bool success);
    void productDeleted(bool success);

private slots:
    void onProductsRead(const QList<Product*>& products);
    void onProductReadByBarcode(Product* product, const QString& barcode);
    void onProductSaved(bool success, int productId);
    void onProductDeleted(bool success, int productId);

private:
    DatabaseManager* m_databaseManager;
    QHash<int, Product*> m_productCache;
};

#endif // PRODUCTMANAGER_H
