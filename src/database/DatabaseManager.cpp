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
        qDebug() << "数据库已经连接";
        return true;
    }
    
    // 创建数据库目录（如果不存在）
    QDir dir(QFileInfo(path).absoluteDir());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    
    if (!m_db.open()) {
        QString errorMsg = QString("无法打开数据库: %1").arg(m_db.lastError().text());
        qCritical() << errorMsg;
        emit databaseError(errorMsg);
        return false;
    }
    
    // 启用外键约束
    QSqlQuery query(m_db);
    query.exec("PRAGMA foreign_keys = ON");
    
    // 初始化表结构
    if (!initializeTables()) {
        closeDatabase();
        return false;
    }
    
    m_connected = true;
    emit connectionStatusChanged(true);
    
    qDebug() << "数据库连接成功:" << path;
    return true;
}

void DatabaseManager::closeDatabase()
{
    QMutexLocker locker(&s_mutex);
    
    if (m_connected) {
        m_db.close();
        m_connected = false;
        emit connectionStatusChanged(false);
        qDebug() << "数据库连接已关闭";
    }
    
    // 移除数据库连接，防止测试中出现 "connection is still in use" 警告
    if (QSqlDatabase::contains(m_db.connectionName())) {
        QSqlDatabase::removeDatabase(m_db.connectionName());
    }
}

bool DatabaseManager::isConnected() const
{
    return m_connected && m_db.isOpen();
}

bool DatabaseManager::initializeTables()
{
    QSqlQuery query(m_db);
    
    // 创建商品表
    QString createProductsTable = R"(
        CREATE TABLE IF NOT EXISTS Products (
            product_id INTEGER PRIMARY KEY AUTOINCREMENT,
            barcode TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            description TEXT,
            price REAL NOT NULL CHECK(price >= 0),
            stock_quantity INTEGER NOT NULL CHECK(stock_quantity >= 0),
            category TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createProductsTable)) {
        logError("创建商品表", query.lastError());
        return false;
    }
    
    // 创建客户表
    QString createCustomersTable = R"(
        CREATE TABLE IF NOT EXISTS Customers (
            customer_id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            contact_info TEXT,
            loyalty_points INTEGER DEFAULT 0 CHECK(loyalty_points >= 0),
            registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_visit DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createCustomersTable)) {
        logError("创建客户表", query.lastError());
        return false;
    }
    
    // 创建交易表
    QString createTransactionsTable = R"(
        CREATE TABLE IF NOT EXISTS Transactions (
            transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_id INTEGER,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            total_amount REAL NOT NULL CHECK(total_amount >= 0),
            discount_amount REAL DEFAULT 0 CHECK(discount_amount >= 0),
            payment_method TEXT NOT NULL,
            status INTEGER DEFAULT 0,
            cashier_name TEXT,
            FOREIGN KEY (customer_id) REFERENCES Customers (customer_id)
        )
    )";
    
    if (!query.exec(createTransactionsTable)) {
        logError("创建交易表", query.lastError());
        return false;
    }
    
    // 创建交易项目表
    QString createTransactionItemsTable = R"(
        CREATE TABLE IF NOT EXISTS TransactionItems (
            transaction_item_id INTEGER PRIMARY KEY AUTOINCREMENT,
            transaction_id INTEGER NOT NULL,
            product_id INTEGER NOT NULL,
            quantity INTEGER NOT NULL CHECK(quantity > 0),
            unit_price REAL NOT NULL CHECK(unit_price >= 0),
            subtotal REAL NOT NULL CHECK(subtotal >= 0),
            FOREIGN KEY (transaction_id) REFERENCES Transactions (transaction_id) ON DELETE CASCADE,
            FOREIGN KEY (product_id) REFERENCES Products (product_id)
        )
    )";
    
    if (!query.exec(createTransactionItemsTable)) {
        logError("创建交易项目表", query.lastError());
        return false;
    }
    
    // 创建索引以提高查询性能
    query.exec("CREATE INDEX IF NOT EXISTS idx_products_barcode ON Products (barcode)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_transactions_customer ON Transactions (customer_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_transactions_timestamp ON Transactions (timestamp)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_transaction_items_transaction ON TransactionItems (transaction_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_transaction_items_product ON TransactionItems (product_id)");
    
    qDebug() << "数据库表初始化完成";
    return true;
}

bool DatabaseManager::saveProduct(Product* product)
{
    if (!product || !product->isValid()) {
        qWarning() << "尝试保存无效的商品";
        return false;
    }
    
    QSqlQuery query(m_db);
    
    if (product->getProductId() <= 0) {
        // 新增商品
        query.prepare(R"(
            INSERT INTO Products (barcode, name, description, price, stock_quantity, category)
            VALUES (?, ?, ?, ?, ?, ?)
        )");
        query.addBindValue(product->getBarcode());
        query.addBindValue(product->getName());
        query.addBindValue(product->getDescription());
        query.addBindValue(product->getPrice());
        query.addBindValue(product->getStockQuantity());
        query.addBindValue(product->getCategory());
        
        if (!executeQuery(query)) {
            return false;
        }
        
        // 设置新生成的ID
        product->setProductId(query.lastInsertId().toInt());
        qDebug() << "新增商品成功，ID:" << product->getProductId();
    } else {
        // 更新商品
        query.prepare(R"(
            UPDATE Products 
            SET barcode = ?, name = ?, description = ?, price = ?, 
                stock_quantity = ?, category = ?, updated_at = CURRENT_TIMESTAMP
            WHERE product_id = ?
        )");
        query.addBindValue(product->getBarcode());
        query.addBindValue(product->getName());
        query.addBindValue(product->getDescription());
        query.addBindValue(product->getPrice());
        query.addBindValue(product->getStockQuantity());
        query.addBindValue(product->getCategory());
        query.addBindValue(product->getProductId());
        
        if (!executeQuery(query)) {
            return false;
        }
        
        qDebug() << "更新商品成功，ID:" << product->getProductId();
    }
    
    return true;
}

std::unique_ptr<Product> DatabaseManager::getProduct(int productId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM Products WHERE product_id = ?");
    query.addBindValue(productId);
    
    if (!executeQuery(query)) {
        return nullptr;
    }
    
    if (query.next()) {
        auto product = std::make_unique<Product>(
            query.value("product_id").toInt(),
            query.value("barcode").toString(),
            query.value("name").toString(),
            query.value("description").toString(),
            query.value("price").toDouble(),
            query.value("stock_quantity").toInt(),
            query.value("category").toString()
        );
        return product;
    }
    
    return nullptr;
}

std::unique_ptr<Product> DatabaseManager::getProductByBarcode(const QString& barcode)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM Products WHERE barcode = ?");
    query.addBindValue(barcode);
    
    if (!executeQuery(query)) {
        return nullptr;
    }
    
    if (query.next()) {
        auto product = std::make_unique<Product>(
            query.value("product_id").toInt(),
            query.value("barcode").toString(),
            query.value("name").toString(),
            query.value("description").toString(),
            query.value("price").toDouble(),
            query.value("stock_quantity").toInt(),
            query.value("category").toString()
        );
        return product;
    }
    
    return nullptr;
}

QList<Product*> DatabaseManager::getAllProducts()
{
    QList<Product*> result;
    if (!isConnected()) {
        return result;
    }
    QSqlQuery query(m_db);
    if (query.exec("SELECT id, barcode, name, description, price, stock, category FROM Products")) {
        while (query.next()) {
            Product* product = new Product();
            product->setProductId(query.value(0).toInt());
            product->setBarcode(query.value(1).toString());
            product->setName(query.value(2).toString());
            product->setDescription(query.value(3).toString());
            product->setPrice(query.value(4).toDouble());
            product->setStockQuantity(query.value(5).toInt());
            product->setCategory(query.value(6).toString());
            result.append(product);
        }
    }
    return result;
}

bool DatabaseManager::deleteProduct(int productId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM Products WHERE product_id = ?");
    query.addBindValue(productId);
    
    if (!executeQuery(query)) {
        return false;
    }
    
    qDebug() << "删除商品成功，ID:" << productId;
    return true;
}

bool DatabaseManager::updateProductStock(int productId, int newStock)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE Products 
        SET stock_quantity = ?, updated_at = CURRENT_TIMESTAMP 
        WHERE product_id = ?
    )");
    query.addBindValue(newStock);
    query.addBindValue(productId);
    
    if (!executeQuery(query)) {
        return false;
    }
    
    qDebug() << "更新商品库存成功，ID:" << productId << "新库存:" << newStock;
    return true;
}

bool DatabaseManager::saveCustomer(Customer* customer)
{
    if (!customer || !customer->isValid()) {
        qWarning() << "尝试保存无效的客户";
        return false;
    }
    
    QSqlQuery query(m_db);
    
    if (customer->getCustomerId() <= 0) {
        // 新增客户
        query.prepare(R"(
            INSERT INTO Customers (name, contact_info, loyalty_points, registration_date, last_visit)
            VALUES (?, ?, ?, ?, ?)
        )");
        query.addBindValue(customer->getName());
        query.addBindValue(customer->getContactInfo());
        query.addBindValue(customer->getLoyaltyPoints());
        query.addBindValue(customer->getRegistrationDate());
        query.addBindValue(customer->getLastVisit());
        
        if (!executeQuery(query)) {
            return false;
        }
        
        customer->setCustomerId(query.lastInsertId().toInt());
        qDebug() << "新增客户成功，ID:" << customer->getCustomerId();
    } else {
        // 更新客户
        query.prepare(R"(
            UPDATE Customers 
            SET name = ?, contact_info = ?, loyalty_points = ?, last_visit = ?
            WHERE customer_id = ?
        )");
        query.addBindValue(customer->getName());
        query.addBindValue(customer->getContactInfo());
        query.addBindValue(customer->getLoyaltyPoints());
        query.addBindValue(customer->getLastVisit());
        query.addBindValue(customer->getCustomerId());
        
        if (!executeQuery(query)) {
            return false;
        }
        
        qDebug() << "更新客户成功，ID:" << customer->getCustomerId();
    }
    
    return true;
}

int DatabaseManager::saveTransaction(Sale* sale)
{
    if (!sale || sale->isEmpty()) {
        qWarning() << "尝试保存无效的交易";
        return -1;
    }
    
    if (!m_db.transaction()) {
        logError("开始事务", m_db.lastError());
        return -1;
    }
    
    QSqlQuery query(m_db);
    
    try {
        // 保存交易主记录
        query.prepare(R"(
            INSERT INTO Transactions (customer_id, total_amount, discount_amount, 
                                    payment_method, status, cashier_name)
            VALUES (?, ?, ?, ?, ?, ?)
        )");
        
        int customerId = -1;
        if (sale->getCustomer()) {
            customerId = sale->getCustomer()->getCustomerId();
        }
        query.addBindValue(customerId > 0 ? customerId : QVariant());
        query.addBindValue(sale->getTotalAmount());
        query.addBindValue(sale->getDiscountAmount());
        query.addBindValue(Sale::paymentMethodToString(sale->getPaymentMethod()));
        query.addBindValue(static_cast<int>(sale->getStatus()));
        query.addBindValue(sale->getCashierName());
        
        if (!executeQuery(query)) {
            throw std::runtime_error("保存交易主记录失败");
        }
        
        int transactionId = query.lastInsertId().toInt();
        
        // 保存交易项目
        for (SaleItem* item : sale->getItems()) {
            if (!item->isValid()) {
                continue;
            }
            
            int productId = item->getProduct()->getProductId();
            if (productId <= 0) {
                throw std::runtime_error("商品ID无效，无法保存交易项目");
            }
            
            // 验证商品是否存在于数据库中
            QSqlQuery checkQuery(m_db);
            checkQuery.prepare("SELECT COUNT(*) FROM Products WHERE product_id = ?");
            checkQuery.addBindValue(productId);
            
            if (!executeQuery(checkQuery) || !checkQuery.next() || checkQuery.value(0).toInt() == 0) {
                throw std::runtime_error(QString("商品ID %1 不存在于数据库中").arg(productId).toStdString());
            }
            
            query.prepare(R"(
                INSERT INTO TransactionItems (transaction_id, product_id, quantity, unit_price, subtotal)
                VALUES (?, ?, ?, ?, ?)
            )");
            query.addBindValue(transactionId);
            query.addBindValue(productId);
            query.addBindValue(item->getQuantity());
            query.addBindValue(item->getUnitPrice());
            query.addBindValue(item->getSubtotal());
            
            if (!executeQuery(query)) {
                throw std::runtime_error("保存交易项目失败");
            }
            
            // 更新商品库存
            int newStock = item->getProduct()->getStockQuantity() - item->getQuantity();
            if (!updateProductStock(productId, newStock)) {
                throw std::runtime_error("更新商品库存失败");
            }
            
            // 同步内存中的库存数量
            item->getProduct()->setStockQuantity(newStock);
        }
        
        // 提交事务
        if (!m_db.commit()) {
            throw std::runtime_error("提交事务失败");
        }
        
        sale->setTransactionId(transactionId);
        qDebug() << "保存交易成功，ID:" << transactionId;
        return transactionId;
        
    } catch (const std::exception& e) {
        m_db.rollback();
        qCritical() << "保存交易失败:" << e.what();
        return -1;
    }
}

QHash<int, int> DatabaseManager::getProductSalesStats(int days)
{
    QHash<int, int> stats;
    
    // 简化实现：返回空的统计数据，避免编译错误
    // TODO: 实现真实的销售统计查询
    qDebug() << "DatabaseManager::getProductSalesStats called for" << days << "days (returning empty stats)";
    
    return stats;
}

QList<int> DatabaseManager::getPopularProducts(int limit, int days)
{
    QList<int> popularProducts;
    
    // 简化实现：返回空的热门商品列表，避免编译错误
    // TODO: 实现真实的热门商品查询
    qDebug() << "DatabaseManager::getPopularProducts called with limit" << limit << "days" << days << "(returning empty list)";
    
    return popularProducts;
}

bool DatabaseManager::executeQuery(QSqlQuery& query)
{
    if (!query.exec()) {
        logError("执行查询", query.lastError());
        return false;
    }
    return true;
}

void DatabaseManager::logError(const QString& context, const QSqlError& error)
{
    QString errorMsg = QString("%1: %2").arg(context, error.text());
    qCritical() << errorMsg;
    emit databaseError(errorMsg);
}
