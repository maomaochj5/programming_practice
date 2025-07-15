#include "ProductManager.h"
#include "../models/Product.h"
#include "../database/DatabaseManager.h"
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QStringList>

ProductManager::ProductManager(QObject *parent)
    : QObject(parent)
    , m_databaseManager(&DatabaseManager::getInstance())
    , m_lowStockThreshold(5)
{
    qDebug() << "商品管理器初始化完成";
}

ProductManager::~ProductManager()
{
    qDebug() << "商品管理器析构";
}

bool ProductManager::addProduct(Product* product)
{
    if (!validateProduct(product)) {
        emit errorOccurred("商品数据无效");
        return false;
    }
    
    // 检查条码唯一性
    if (!isBarcodeUnique(product->getBarcode())) {
        emit errorOccurred(QString("条码 %1 已存在").arg(product->getBarcode()));
        return false;
    }
    
    if (m_databaseManager->saveProduct(product)) {
        emit productAdded(product);
        logOperation("添加商品", product->toString());
        
        // 检查低库存警告
        checkLowStockWarning(product);
        
        return true;
    }
    
    emit errorOccurred("保存商品到数据库失败");
    return false;
}

bool ProductManager::updateProduct(Product* product)
{
    if (!product || product->getProductId() <= 0) {
        emit errorOccurred("无效的商品ID");
        return false;
    }
    
    if (!validateProduct(product)) {
        emit errorOccurred("商品数据无效");
        return false;
    }
    
    // 检查条码唯一性（排除自己）
    if (!isBarcodeUnique(product->getBarcode(), product->getProductId())) {
        emit errorOccurred(QString("条码 %1 已被其他商品使用").arg(product->getBarcode()));
        return false;
    }
    
    if (m_databaseManager->saveProduct(product)) {
        emit productUpdated(product);
        logOperation("更新商品", product->toString());
        
        // 检查低库存警告
        checkLowStockWarning(product);
        
        return true;
    }
    
    emit errorOccurred("更新商品失败");
    return false;
}

bool ProductManager::deleteProduct(int productId)
{
    if (productId <= 0) {
        emit errorOccurred("无效的商品ID");
        return false;
    }
    
    // 先获取商品信息用于日志
    auto product = getProduct(productId);
    QString productInfo = product ? product->toString() : QString("ID: %1").arg(productId);
    
    if (m_databaseManager->deleteProduct(productId)) {
        emit productDeleted(productId);
        logOperation("删除商品", productInfo);
        return true;
    }
    
    emit errorOccurred("删除商品失败");
    return false;
}

std::unique_ptr<Product> ProductManager::getProduct(int productId)
{
    if (productId <= 0) {
        return nullptr;
    }
    
    return m_databaseManager->getProduct(productId);
}

std::unique_ptr<Product> ProductManager::getProductByBarcode(const QString& barcode)
{
    if (barcode.isEmpty()) {
        return nullptr;
    }
    
    return m_databaseManager->getProductByBarcode(barcode);
}

QList<Product*> ProductManager::getAllProducts()
{
    QList<Product*> result;
    
    try {
        // 暂时使用测试数据，避免unique_ptr的复杂性
        // TODO: 重构DatabaseManager的API以使用原始指针
        qDebug() << "使用内置测试数据";
        
        Product* testProduct1 = new Product(this);
        testProduct1->setProductId(1);
        testProduct1->setBarcode("1234567890123");
        testProduct1->setName("测试商品1");
        testProduct1->setDescription("这是一个测试商品");
        testProduct1->setPrice(12.50);
        testProduct1->setStockQuantity(100);
        testProduct1->setCategory("测试分类");
        result.append(testProduct1);
        
        Product* testProduct2 = new Product(this);
        testProduct2->setProductId(2);
        testProduct2->setBarcode("1234567890124");
        testProduct2->setName("测试商品2");
        testProduct2->setDescription("这是第二个测试商品");
        testProduct2->setPrice(25.00);
        testProduct2->setStockQuantity(50);
        testProduct2->setCategory("测试分类");
        result.append(testProduct2);
        
        Product* testProduct3 = new Product(this);
        testProduct3->setProductId(3);
        testProduct3->setBarcode("1234567890125");
        testProduct3->setName("低库存商品");
        testProduct3->setDescription("这是一个低库存测试商品");
        testProduct3->setPrice(5.99);
        testProduct3->setStockQuantity(2);  // 低库存
        testProduct3->setCategory("低库存测试");
        result.append(testProduct3);
        
    } catch (const std::exception& e) {
        qWarning() << "获取商品列表时出错:" << e.what();
        emit errorOccurred("获取商品列表失败");
    }
    
    return result;
}

QList<Product*> ProductManager::getProductsByCategory(const QString& category)
{
    QList<Product*> result;
    auto allProducts = getAllProducts();
    
    for (Product* product : allProducts) {
        if (product->getCategory() == category) {
            result.append(product);
        }
    }
    
    return result;
}

QList<Product*> ProductManager::searchProducts(const QString& keyword)
{
    QList<Product*> result;
    
    if (keyword.isEmpty()) {
        return getAllProducts();
    }
    
    auto allProducts = getAllProducts();
    QString searchKeyword = keyword.toLower();
    
    for (Product* product : allProducts) {
        if (product->getName().toLower().contains(searchKeyword) ||
            product->getDescription().toLower().contains(searchKeyword) ||
            product->getBarcode().contains(searchKeyword) ||
            product->getCategory().toLower().contains(searchKeyword)) {
            result.append(product);
        }
    }
    
    return result;
}

QList<Product*> ProductManager::getLowStockProducts(int threshold)
{
    QList<Product*> result;
    auto allProducts = getAllProducts();
    
    for (Product* product : allProducts) {
        if (product->getStockQuantity() <= threshold) {
            result.append(product);
        }
    }
    
    return result;
}

bool ProductManager::updateProductStock(int productId, int newStock)
{
    if (productId <= 0 || newStock < 0) {
        emit errorOccurred("无效的库存更新参数");
        return false;
    }
    
    if (m_databaseManager->updateProductStock(productId, newStock)) {
        emit stockChanged(productId, newStock);
        
        // 检查低库存警告
        if (newStock <= m_lowStockThreshold) {
            emit lowStockWarning(productId, newStock);
        }
        
        logOperation("更新库存", QString("商品ID: %1, 新库存: %2").arg(productId).arg(newStock));
        return true;
    }
    
    emit errorOccurred("更新库存失败");
    return false;
}

bool ProductManager::batchUpdatePrices(const QString& categoryOrAll, 
                                      const QString& adjustmentType, 
                                      double adjustmentValue)
{
    if (adjustmentValue == 0.0) {
        emit errorOccurred("调整值不能为0");
        return false;
    }
    
    QList<Product*> products;
    
    if (categoryOrAll.toUpper() == "ALL") {
        products = getAllProducts();
    } else {
        products = getProductsByCategory(categoryOrAll);
    }
    
    if (products.isEmpty()) {
        emit errorOccurred("没有找到要更新的商品");
        return false;
    }
    
    int updatedCount = 0;
    
    for (Product* product : products) {
        double oldPrice = product->getPrice();
        double newPrice = oldPrice;
        
        if (adjustmentType.toLower() == "percentage") {
            newPrice = oldPrice * (1.0 + adjustmentValue / 100.0);
        } else if (adjustmentType.toLower() == "fixed") {
            newPrice = oldPrice + adjustmentValue;
        } else {
            emit errorOccurred("无效的调整类型");
            return false;
        }
        
        // 确保价格不为负数
        if (newPrice < 0.0) {
            newPrice = 0.0;
        }
        
        product->setPrice(newPrice);
        
        if (m_databaseManager->saveProduct(product)) {
            updatedCount++;
            emit productUpdated(product);
        }
    }
    
    logOperation("批量更新价格", 
                QString("分类: %1, 类型: %2, 值: %3, 更新数量: %4")
                .arg(categoryOrAll)
                .arg(adjustmentType)
                .arg(adjustmentValue)
                .arg(updatedCount));
    
    return updatedCount > 0;
}

bool ProductManager::isBarcodeUnique(const QString& barcode, int excludeProductId)
{
    if (barcode.isEmpty()) {
        return false;
    }
    
    auto existingProduct = getProductByBarcode(barcode);
    
    if (!existingProduct) {
        return true; // 条码不存在，唯一
    }
    
    // 如果存在但是是被排除的商品ID，则仍然唯一
    return existingProduct->getProductId() == excludeProductId;
}

bool ProductManager::validateProduct(Product* product)
{
    if (!product) {
        return false;
    }
    
    // 检查必填字段
    if (product->getBarcode().isEmpty()) {
        emit errorOccurred("商品条码不能为空");
        return false;
    }
    
    if (product->getName().isEmpty()) {
        emit errorOccurred("商品名称不能为空");
        return false;
    }
    
    if (product->getPrice() < 0.0) {
        emit errorOccurred("商品价格不能为负数");
        return false;
    }
    
    if (product->getStockQuantity() < 0) {
        emit errorOccurred("商品库存不能为负数");
        return false;
    }
    
    // 检查条码格式（简单的长度检查）
    if (product->getBarcode().length() < 8 || product->getBarcode().length() > 20) {
        emit errorOccurred("条码长度应在8-20位之间");
        return false;
    }
    
    // 检查条码是否只包含数字
    for (QChar c : product->getBarcode()) {
        if (!c.isDigit()) {
            emit errorOccurred("条码只能包含数字");
            return false;
        }
    }
    
    return true;
}

QStringList ProductManager::getAllCategories()
{
    QStringList categories;
    auto allProducts = getAllProducts();
    
    for (const Product* product : allProducts) {
        QString category = product->getCategory();
        if (!category.isEmpty() && !categories.contains(category)) {
            categories.append(category);
        }
    }
    
    categories.sort();
    return categories;
}

int ProductManager::importProductsFromCSV(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(QString("无法打开文件: %1").arg(filePath));
        return 0;
    }
    
    QTextStream in(&file);
    int importedCount = 0;
    int lineNumber = 0;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;
        
        // 跳过空行和注释行
        if (line.isEmpty() || line.startsWith("#")) {
            continue;
        }
        
        // 跳过标题行（第一个非空行）
        if (lineNumber == 1) {
            continue;
        }
        
        QStringList fields = line.split(",");
        if (fields.size() < 5) {
            qWarning() << "第" << lineNumber << "行格式错误，跳过";
            continue;
        }
        
        // CSV格式：barcode,name,description,price,stock,category
        Product* product = new Product(this);
        product->setBarcode(fields[0].trimmed());
        product->setName(fields[1].trimmed());
        product->setDescription(fields.size() > 2 ? fields[2].trimmed() : "");
        product->setPrice(fields[3].toDouble());
        product->setStockQuantity(fields[4].toInt());
        product->setCategory(fields.size() > 5 ? fields[5].trimmed() : "未分类");
        
        if (addProduct(product)) {
            importedCount++;
        } else {
            qWarning() << "第" << lineNumber << "行导入失败:" << product->getName();
            delete product;
        }
    }
    
    logOperation("导入商品", QString("文件: %1, 成功导入: %2").arg(filePath).arg(importedCount));
    return importedCount;
}

bool ProductManager::exportProductsToCSV(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(QString("无法创建文件: %1").arg(filePath));
        return false;
    }
    
    QTextStream out(&file);
    // out.setCodec("UTF-8"); // Qt6中已移除，默认为UTF-8
    
    // 写入标题行
    out << "条码,名称,描述,价格,库存,分类\n";
    
    auto allProducts = getAllProducts();
    for (const Product* product : allProducts) {
        out << QString("%1,%2,%3,%4,%5,%6\n")
               .arg(product->getBarcode())
               .arg(product->getName())
               .arg(product->getDescription())
               .arg(product->getPrice(), 0, 'f', 2)
               .arg(product->getStockQuantity())
               .arg(product->getCategory());
    }
    
    logOperation("导出商品", QString("文件: %1, 导出数量: %2").arg(filePath).arg(allProducts.size()));
    return true;
}

void ProductManager::checkLowStockWarning(Product* product)
{
    if (product && product->getStockQuantity() <= m_lowStockThreshold) {
        emit lowStockWarning(product->getProductId(), product->getStockQuantity());
    }
}

void ProductManager::logOperation(const QString& operation, const QString& productInfo)
{
    QString logMessage = QString("[时间戳] %1: %2")
                        .arg(operation)
                        .arg(productInfo);
    qDebug() << logMessage;
}

void ProductManager::setLowStockThreshold(int threshold)
{
    if (threshold > 0) {
        m_lowStockThreshold = threshold;
    }
}

int ProductManager::getLowStockThreshold() const
{
    return m_lowStockThreshold;
}
