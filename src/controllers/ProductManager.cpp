#include "ProductManager.h"
#include "../database/DatabaseManager.h"
#include "../models/Product.h"
#include <QDebug>

ProductManager::ProductManager(QObject *parent)
    : QObject(parent), m_databaseManager(&DatabaseManager::getInstance())
{
    connect(m_databaseManager, &DatabaseManager::productsRead, this, &ProductManager::onProductsRead);
    connect(m_databaseManager, &DatabaseManager::productReadByBarcode, this, &ProductManager::onProductReadByBarcode);
    
    // Connect DB write operations to PM slots
    connect(m_databaseManager, &DatabaseManager::productSaved, this, &ProductManager::onProductSaved);
    connect(m_databaseManager, &DatabaseManager::productDeleted, this, &ProductManager::onProductDeleted);

    // Trigger the initial asynchronous load
    getAllProducts();
}

ProductManager::~ProductManager()
{
    qDeleteAll(m_productCache);
}

void ProductManager::getAllProducts()
{
    m_databaseManager->getAllProducts();
}

QList<Product*> ProductManager::getAllProductsSync() const
{
    return m_productCache.values();
}

void ProductManager::onProductsRead(const QList<Product*>& products)
{
    qDeleteAll(m_productCache);
    m_productCache.clear();

    for (Product* product : products) {
        m_productCache.insert(product->getProductId(), product);
    }

    emit allProductsChanged(m_productCache.values());
}

void ProductManager::onProductSaved(bool success, int productId)
{
    if (success) {
        getAllProducts(); // Refresh cache
    }
    emit productSaved(success);
    emit productUpdated(success);
}

void ProductManager::onProductDeleted(bool success, int productId)
{
    if (success) {
        // The item is gone from DB, now remove from cache
        if(m_productCache.contains(productId)) {
            delete m_productCache.take(productId);
        }
        emit allProductsChanged(m_productCache.values());
    }
    emit productDeleted(success);
}


Product* ProductManager::getProductById(int id)
{
    return m_productCache.value(id, nullptr);
}

QList<Product*> ProductManager::getProductsByIds(const QList<int>& ids)
{
    QList<Product*> products;
    for (int id : ids) {
        Product* product = getProductById(id);
        if (product) {
            products.append(product);
        }
    }
    return products;
}

Product* ProductManager::getProductByName(const QString& name)
{
    for (Product* product : m_productCache.values()) {
        if (product->getName().compare(name, Qt::CaseInsensitive) == 0) {
            return product;
        }
    }
    return nullptr;
}

void ProductManager::getProductByBarcode(const QString& barcode)
{
    // First, check the cache
    for (Product* product : m_productCache.values()) {
        if (product->getBarcode() == barcode) {
            emit productFoundByBarcode(product, barcode);
            return;
        }
    }
    
    // If not in cache, ask the database asynchronously
    m_databaseManager->getProductByBarcode(barcode);
}

void ProductManager::onProductReadByBarcode(Product* product, const QString& barcode)
{
    if (product) {
        // Add to cache if it's not already there (it shouldn't be)
        if (!m_productCache.contains(product->getProductId())) {
             m_productCache.insert(product->getProductId(), product);
        }
    }
    // Emit the result, whether it's a valid product or nullptr
    emit productFoundByBarcode(product, barcode);
}

void ProductManager::addProduct(Product* product)
{
    if (product) {
        m_databaseManager->saveProduct(*product);
    }
}

void ProductManager::updateProduct(Product* product)
{
    if (product) {
        m_databaseManager->saveProduct(*product);
    }
}

void ProductManager::deleteProduct(int id)
{
    m_databaseManager->deleteProduct(id);
}

QList<Product*> ProductManager::searchProducts(const QString& searchTerm)
{
    QList<Product*> results;
    if (searchTerm.isEmpty()) {
        return m_productCache.values();
    }

    for (Product* product : m_productCache.values()) {
        if (product->getName().contains(searchTerm, Qt::CaseInsensitive) ||
            product->getBarcode().contains(searchTerm)) {
            results.append(product);
        }
    }
    return results;
}
