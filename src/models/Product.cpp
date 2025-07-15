#include "Product.h"
#include <QDebug>

Product::Product(QObject *parent)
    : QObject(parent)
    , m_productId(-1)
    , m_price(0.0)
    , m_stockQuantity(0)
{
}

Product::Product(int productId, const QString& barcode, const QString& name,
                const QString& description, double price, int stockQuantity,
                const QString& category, QObject *parent)
    : QObject(parent)
    , m_productId(productId)
    , m_barcode(barcode)
    , m_name(name)
    , m_description(description)
    , m_price(price)
    , m_stockQuantity(stockQuantity)
    , m_category(category)
{
}

Product::Product(const Product& other)
    : QObject(other.parent())
    , m_productId(other.m_productId)
    , m_barcode(other.m_barcode)
    , m_name(other.m_name)
    , m_description(other.m_description)
    , m_price(other.m_price)
    , m_stockQuantity(other.m_stockQuantity)
    , m_category(other.m_category)
{
}

Product& Product::operator=(const Product& other)
{
    if (this != &other) {
        m_productId = other.m_productId;
        m_barcode = other.m_barcode;
        m_name = other.m_name;
        m_description = other.m_description;
        m_price = other.m_price;
        m_stockQuantity = other.m_stockQuantity;
        m_category = other.m_category;
        
        emit productChanged();
    }
    return *this;
}

void Product::setProductId(int productId)
{
    if (m_productId != productId) {
        m_productId = productId;
        emit productChanged();
    }
}

void Product::setBarcode(const QString& barcode)
{
    if (m_barcode != barcode) {
        m_barcode = barcode;
        emit productChanged();
    }
}

void Product::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
        emit productChanged();
    }
}

void Product::setDescription(const QString& description)
{
    if (m_description != description) {
        m_description = description;
        emit productChanged();
    }
}

void Product::setPrice(double price)
{
    if (qAbs(m_price - price) > 0.01) { // 避免浮点数比较问题
        m_price = price;
        emit productChanged();
    }
}

void Product::setStockQuantity(int stockQuantity)
{
    if (m_stockQuantity != stockQuantity) {
        int oldStock = m_stockQuantity;
        m_stockQuantity = stockQuantity;
        emit stockChanged(m_stockQuantity);
        emit productChanged();
        
        // 低库存警告（可以根据需要调整阈值）
        if (m_stockQuantity <= 5 && oldStock > 5) {
            qWarning() << "商品库存过低:" << m_name << "当前库存:" << m_stockQuantity;
        }
    }
}

void Product::setCategory(const QString& category)
{
    if (m_category != category) {
        m_category = category;
        emit productChanged();
    }
}

bool Product::isValid() const
{
    return !m_barcode.isEmpty() && 
           !m_name.isEmpty() && 
           m_price >= 0.0 &&
           m_stockQuantity >= 0;
}

bool Product::isInStock() const
{
    return m_stockQuantity > 0;
}

bool Product::decreaseStock(int quantity)
{
    if (quantity <= 0) {
        qWarning() << "尝试减少无效数量的库存:" << quantity;
        return false;
    }
    
    if (m_stockQuantity < quantity) {
        qWarning() << "库存不足，无法减少库存。当前库存:" << m_stockQuantity 
                   << "尝试减少:" << quantity;
        return false;
    }
    
    setStockQuantity(m_stockQuantity - quantity);
    qDebug() << "商品" << m_name << "库存减少" << quantity 
             << "当前库存:" << m_stockQuantity;
    return true;
}

void Product::increaseStock(int quantity)
{
    if (quantity <= 0) {
        qWarning() << "尝试增加无效数量的库存:" << quantity;
        return;
    }
    
    setStockQuantity(m_stockQuantity + quantity);
    qDebug() << "商品" << m_name << "库存增加" << quantity 
             << "当前库存:" << m_stockQuantity;
}

QString Product::toString() const
{
    return QString("Product[ID:%1, Barcode:%2, Name:%3, Price:%4, Stock:%5, Category:%6]")
           .arg(m_productId)
           .arg(m_barcode)
           .arg(m_name)
           .arg(m_price, 0, 'f', 2)
           .arg(m_stockQuantity)
           .arg(m_category);
}
