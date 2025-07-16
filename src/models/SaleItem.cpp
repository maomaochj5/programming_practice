#include "SaleItem.h"
#include <QDebug>

SaleItem::SaleItem(QObject *parent)
    : QObject(parent)
    , m_product(nullptr)
    , m_quantity(0)
    , m_unitPrice(0.0)
    , m_subtotal(0.0)
{
}

SaleItem::SaleItem(const SaleItem& other)
    : QObject(other.parent())
    , m_product(new Product(*other.m_product))
    , m_quantity(other.m_quantity)
    , m_unitPrice(other.m_unitPrice)
    , m_subtotal(other.m_subtotal)
{
    // The new product needs a parent to be memory-managed correctly.
    // Setting `this` as the parent.
    m_product->setParent(this);
}

SaleItem::SaleItem(Product* product, int quantity, double unitPrice, QObject *parent)
    : QObject(parent)
    , m_product(product)
    , m_quantity(quantity)
    , m_unitPrice(unitPrice)
    , m_subtotal(0.0)
{
    calculateSubtotal();
}

void SaleItem::setProduct(Product* product)
{
    if (m_product != product) {
        m_product = product;
        calculateSubtotal();
        emit itemChanged();
    }
}

void SaleItem::setQuantity(int quantity)
{
    if (m_quantity != quantity && quantity >= 0) {
        m_quantity = quantity;
        calculateSubtotal();
        emit itemChanged();
    }
}

void SaleItem::setUnitPrice(double unitPrice)
{
    if (qAbs(m_unitPrice - unitPrice) > 0.01 && unitPrice >= 0.0) {
        m_unitPrice = unitPrice;
        calculateSubtotal();
        emit itemChanged();
    }
}

void SaleItem::calculateSubtotal()
{
    double oldSubtotal = m_subtotal;
    m_subtotal = m_quantity * m_unitPrice;
    
    if (qAbs(oldSubtotal - m_subtotal) > 0.01) {
        emit subtotalChanged(m_subtotal);
    }
}

bool SaleItem::isValid() const
{
    return m_product != nullptr && 
           m_quantity > 0 && 
           m_unitPrice >= 0.0 &&
           m_product->isValid();
}

QString SaleItem::toString() const
{
    QString productName = m_product ? m_product->getName() : "Unknown Product";
    return QString("SaleItem[Product:%1, Quantity:%2, UnitPrice:%3, Subtotal:%4]")
           .arg(productName)
           .arg(m_quantity)
           .arg(m_unitPrice, 0, 'f', 2)
           .arg(m_subtotal, 0, 'f', 2);
}
