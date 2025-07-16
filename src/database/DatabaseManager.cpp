#include "DatabaseManager.h"
#include "../models/Product.h"
#include "../models/Customer.h"
#include "../models/Sale.h"
#include "../models/SaleItem.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QtConcurrent>

QMutex DatabaseManager::s_mutex;

DatabaseManager& DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_connected(false)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase(const QString& path)
{
    QMutexLocker locker(&s_mutex);
    
    if (m_connected) {
        return true;
    }
    
    QDir dir(QFileInfo(path).absoluteDir());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    m_db = QSqlDatabase::addDatabase("QSQLITE", "mainConnection");
    m_db.setDatabaseName(path);
    
    if (!m_db.open()) {
        logError("openDatabase", m_db.lastError());
        return false;
    }
    
    m_db.exec("PRAGMA foreign_keys = ON;");
    
    if (!initializeTables()) {
        closeDatabase();
        return false;
    }
    
    m_connected = true;
    emit connectionStatusChanged(true);
    qDebug() << "Database connection successful:" << path;
    return true;
}

void DatabaseManager::closeDatabase()
{
    QMutexLocker locker(&s_mutex);
    
    if (m_connected) {
        m_db.close();
        m_connected = false;
        emit connectionStatusChanged(false);
        qDebug() << "Database connection closed.";
    }
    
    QSqlDatabase::removeDatabase("mainConnection");
}

bool DatabaseManager::isConnected() const
{
    return m_connected;
}

bool DatabaseManager::initializeTables()
{
    QSqlQuery query(m_db);
    const char* tables[] = {
        R"(CREATE TABLE IF NOT EXISTS Products (
            product_id INTEGER PRIMARY KEY AUTOINCREMENT,
            barcode TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            description TEXT,
            price REAL NOT NULL CHECK(price >= 0),
            stock_quantity INTEGER NOT NULL CHECK(stock_quantity >= 0),
            category TEXT,
            image_path TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        ))",
        R"(CREATE TABLE IF NOT EXISTS Customers (
            customer_id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            contact_info TEXT,
            loyalty_points INTEGER DEFAULT 0 CHECK(loyalty_points >= 0),
            registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_visit DATETIME DEFAULT CURRENT_TIMESTAMP
        ))",
        R"(CREATE TABLE IF NOT EXISTS Transactions (
            transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_id INTEGER,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            total_amount REAL NOT NULL CHECK(total_amount >= 0),
            discount_amount REAL DEFAULT 0 CHECK(discount_amount >= 0),
            payment_method TEXT NOT NULL,
            status INTEGER DEFAULT 0,
            cashier_name TEXT,
            FOREIGN KEY (customer_id) REFERENCES Customers (customer_id)
        ))",
        R"(CREATE TABLE IF NOT EXISTS TransactionItems (
            transaction_item_id INTEGER PRIMARY KEY AUTOINCREMENT,
            transaction_id INTEGER NOT NULL,
            product_id INTEGER NOT NULL,
            quantity INTEGER NOT NULL CHECK(quantity > 0),
            unit_price REAL NOT NULL CHECK(unit_price >= 0),
            subtotal REAL NOT NULL CHECK(subtotal >= 0),
            FOREIGN KEY (transaction_id) REFERENCES Transactions (transaction_id) ON DELETE CASCADE,
            FOREIGN KEY (product_id) REFERENCES Products (product_id)
        ))"
    };

    for (const char* tableSql : tables) {
        if (!query.exec(tableSql)) {
            logError("initializeTables", query.lastError());
            return false;
        }
    }

    const char* indices[] = {
        "CREATE INDEX IF NOT EXISTS idx_products_barcode ON Products (barcode)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_customer ON Transactions (customer_id)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_timestamp ON Transactions (timestamp)",
    };
    
    for (const char* indexSql : indices) {
        query.exec(indexSql);
    }
    
    return true;
}

void DatabaseManager::saveProduct(const Product& product)
{
    auto watcher = new QFutureWatcher<QPair<bool, int>>(this);
    connect(watcher, &QFutureWatcher<QPair<bool, int>>::finished, this, &DatabaseManager::handleProductSaved);

    QFuture<QPair<bool, int>> future = QtConcurrent::run([this, product]() {
        QMutexLocker locker(&s_mutex);
        if (!m_connected) return qMakePair(false, product.getProductId());

        QSqlQuery query(m_db);
        bool success;
        int finalId = product.getProductId();

        if (product.getProductId() <= 0) {
            query.prepare(R"(
                INSERT INTO Products (barcode, name, description, price, stock_quantity, category, image_path)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            )");
            query.addBindValue(product.getBarcode());
            query.addBindValue(product.getName());
            query.addBindValue(product.getDescription());
            query.addBindValue(product.getPrice());
            query.addBindValue(product.getStockQuantity());
            query.addBindValue(product.getCategory());
            query.addBindValue(product.getImagePath());
            success = query.exec();
            if (success) {
                finalId = query.lastInsertId().toInt();
            }
        } else {
            query.prepare(R"(
                UPDATE Products 
                SET barcode = ?, name = ?, description = ?, price = ?, 
                    stock_quantity = ?, category = ?, image_path = ?, updated_at = CURRENT_TIMESTAMP
                WHERE product_id = ?
            )");
            query.addBindValue(product.getBarcode());
            query.addBindValue(product.getName());
            query.addBindValue(product.getDescription());
            query.addBindValue(product.getPrice());
            query.addBindValue(product.getStockQuantity());
            query.addBindValue(product.getCategory());
            query.addBindValue(product.getImagePath());
            query.addBindValue(product.getProductId());
            success = query.exec();
        }

        if (!success) {
            logError("saveProduct_worker", query.lastError());
        }
        return qMakePair(success, finalId);
    });

    watcher->setFuture(future);
}

void DatabaseManager::deleteProduct(int productId)
{
    auto watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, productId]() {
        handleProductDeleted(watcher->result(), productId);
        watcher->deleteLater();
    });

    QFuture<bool> future = QtConcurrent::run([this, productId]() {
        QMutexLocker locker(&s_mutex);
        if (!m_connected) return false;

        QSqlQuery query(m_db);
        query.prepare("DELETE FROM Products WHERE product_id = ?");
        query.addBindValue(productId);
        
        if (!query.exec()) {
            logError("deleteProduct_worker", query.lastError());
            return false;
        }
        return true;
    });

    watcher->setFuture(future);
}

void DatabaseManager::getProductByBarcode(const QString& barcode)
{
    auto watcher = new QFutureWatcher<Product*>(this);
    connect(watcher, &QFutureWatcher<Product*>::finished, this, [this, watcher, barcode]() {
        handleProductReadByBarcode(watcher->result(), barcode);
        watcher->deleteLater();
    });

    QFuture<Product*> future = QtConcurrent::run([this, barcode]() {
        QMutexLocker locker(&s_mutex);
        if (!m_connected) return (Product*)nullptr;

        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM Products WHERE barcode = ?");
        query.addBindValue(barcode);

        if (!query.exec()) {
            logError("getProductByBarcode_worker", query.lastError());
            return (Product*)nullptr;
        }

        if (query.next()) {
            Product* product = new Product();
            product->setProductId(query.value("product_id").toInt());
            product->setBarcode(query.value("barcode").toString());
            product->setName(query.value("name").toString());
            product->setDescription(query.value("description").toString());
            product->setPrice(query.value("price").toDouble());
            product->setStockQuantity(query.value("stock_quantity").toInt());
            product->setCategory(query.value("category").toString());
            product->setImagePath(query.value("image_path").toString());
            return product;
        }

        return (Product*)nullptr;
    });

    watcher->setFuture(future);
}

void DatabaseManager::getAllProducts()
{
    auto watcher = new QFutureWatcher<QList<Product*>>(this);
    connect(watcher, &QFutureWatcher<QList<Product*>>::finished, this, &DatabaseManager::handleProductsRead);

    QFuture<QList<Product*>> future = QtConcurrent::run([this]() {
        QMutexLocker locker(&s_mutex);
        QList<Product*> products;
        if (!m_connected) return products;
        
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM Products ORDER BY name ASC");

        if (!query.exec()) {
            logError("getAllProducts_worker", query.lastError());
            return products;
        }

        while (query.next()) {
            Product* product = new Product();
            product->setProductId(query.value("product_id").toInt());
            product->setBarcode(query.value("barcode").toString());
            product->setName(query.value("name").toString());
            product->setDescription(query.value("description").toString());
            product->setPrice(query.value("price").toDouble());
            product->setStockQuantity(query.value("stock_quantity").toInt());
            product->setCategory(query.value("category").toString());
            product->setImagePath(query.value("image_path").toString());
            products.append(product);
        }
        return products;
    });

    watcher->setFuture(future);
}

int DatabaseManager::saveTransaction(Sale* sale)
{
    QMutexLocker locker(&s_mutex);
    if (!sale || sale->isEmpty() || !m_connected) return -1;
    
    if (!m_db.transaction()) {
        logError("saveTransaction_begin", m_db.lastError());
        return -1;
    }
    
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO Transactions (customer_id, total_amount, discount_amount, payment_method, status, cashier_name)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(sale->getCustomer() ? QVariant(sale->getCustomer()->getCustomerId()) : QVariant());
    query.addBindValue(sale->getTotalAmount());
    query.addBindValue(sale->getDiscountAmount());
    query.addBindValue(Sale::paymentMethodToString(sale->getPaymentMethod()));
    query.addBindValue(static_cast<int>(sale->getStatus()));
    query.addBindValue(sale->getCashierName());
    
    if (!query.exec()) {
        logError("saveTransaction_main", query.lastError());
        m_db.rollback();
        return -1;
    }
    
    int transactionId = query.lastInsertId().toInt();
    
    for (SaleItem* item : sale->getItems()) {
        query.prepare(R"(
            INSERT INTO TransactionItems (transaction_id, product_id, quantity, unit_price, subtotal)
            VALUES (?, ?, ?, ?, ?)
        )");
        query.addBindValue(transactionId);
        query.addBindValue(item->getProduct()->getProductId());
        query.addBindValue(item->getQuantity());
        query.addBindValue(item->getUnitPrice());
        query.addBindValue(item->getSubtotal());
        
        if (!query.exec()) {
            logError("saveTransaction_item", query.lastError());
            m_db.rollback();
            return -1;
        }
        
        if (!updateProductStock(item->getProduct()->getProductId(), item->getProduct()->getStockQuantity() - item->getQuantity())) {
            logError("saveTransaction_stockUpdate", m_db.lastError());
            m_db.rollback();
            return -1;
        }
    }
    
    if (!m_db.commit()) {
        logError("saveTransaction_commit", m_db.lastError());
        m_db.rollback();
        return -1;
    }
    
    sale->setTransactionId(transactionId);
    return transactionId;
}

bool DatabaseManager::updateProductStock(int productId, int newStock)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE Products SET stock_quantity = ? WHERE product_id = ?");
    query.addBindValue(newStock);
    query.addBindValue(productId);
    return query.exec();
}

QList<int> DatabaseManager::getPopularProducts(int limit, int days)
{
    // TODO: Implement this properly
    Q_UNUSED(limit);
    Q_UNUSED(days);
    return {};
}

QHash<int, int> DatabaseManager::getProductSalesStats(int days)
{
    // TODO: Implement this properly
    Q_UNUSED(days);
    return {};
}

void DatabaseManager::logError(const QString& context, const QSqlError& error)
{
    QString errorMsg = QString("%1: %2").arg(context, error.text());
    qCritical() << "Database Error -" << errorMsg;
    emit databaseError(errorMsg);
}

void DatabaseManager::handleProductsRead()
{
    auto* watcher = static_cast<QFutureWatcher<QList<Product*>>*>(sender());
    if (!watcher) return;
    emit productsRead(watcher->result());
    watcher->deleteLater();
}

void DatabaseManager::handleProductReadByBarcode(Product* product, const QString& barcode)
{
    emit productReadByBarcode(product, barcode);
}

void DatabaseManager::handleProductSaved()
{
    auto* watcher = static_cast<QFutureWatcher<QPair<bool, int>>*>(sender());
    if (!watcher) return;
    auto result = watcher->result();
    emit productSaved(result.first, result.second);
    watcher->deleteLater();
}

void DatabaseManager::handleProductDeleted(bool success, int productId)
{
    emit productDeleted(success, productId);
}

// Synchronous methods for contexts that require it (e.g., exiting)
// These are not yet implemented as they are not currently required by the async flow.
std::unique_ptr<Product> DatabaseManager::getProduct(int productId) { return nullptr; }
bool DatabaseManager::saveCustomer(Customer* customer) { return false; }
std::unique_ptr<Customer> DatabaseManager::getCustomer(int customerId) { return nullptr; }
QList<std::unique_ptr<Customer>> DatabaseManager::getAllCustomers() { return {}; }
bool DatabaseManager::deleteCustomer(int customerId) { return false; }
std::unique_ptr<Sale> DatabaseManager::getTransaction(int transactionId) { return nullptr; }
QList<std::unique_ptr<Sale>> DatabaseManager::getCustomerTransactions(int customerId, int limit) { return {}; }
QList<std::unique_ptr<Sale>> DatabaseManager::getTransactionsByDateRange(const QDateTime& startDate, const QDateTime& endDate) { return {}; }
double DatabaseManager::getRevenueStats(int days) { return 0.0; }
