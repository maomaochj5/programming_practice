#include "Sale.h"
#include <QDebug>
#include <algorithm>

Sale::Sale(QObject *parent)
    : QObject(parent)
    , m_transactionId(-1)
    , m_customer(nullptr)
    , m_totalAmount(0.0)
    , m_discountAmount(0.0)
    , m_paymentMethod(Cash)
    , m_status(InProgress)
    , m_timestamp(QDateTime::currentDateTime())
{
}

Sale::Sale(int transactionId, Customer* customer, QObject *parent)
    : QObject(parent)
    , m_transactionId(transactionId)
    , m_customer(customer)
    , m_totalAmount(0.0)
    , m_discountAmount(0.0)
    , m_paymentMethod(Cash)
    , m_status(InProgress)
    , m_timestamp(QDateTime::currentDateTime())
{
}

void Sale::setTransactionId(int transactionId)
{
    if (m_transactionId != transactionId) {
        m_transactionId = transactionId;
        emit saleChanged();
    }
}

void Sale::setCustomer(Customer* customer)
{
    if (m_customer != customer) {
        m_customer = customer;
        emit saleChanged();
    }
}

void Sale::setPaymentMethod(PaymentMethod method)
{
    if (m_paymentMethod != method) {
        m_paymentMethod = method;
        emit saleChanged();
    }
}

void Sale::setStatus(TransactionStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit saleChanged();
    }
}

void Sale::setCashierName(const QString& cashierName)
{
    if (m_cashierName != cashierName) {
        m_cashierName = cashierName;
        emit saleChanged();
    }
}

void Sale::setDiscountAmount(double discount)
{
    if (qAbs(m_discountAmount - discount) > 0.01 && discount >= 0.0) {
        m_discountAmount = discount;
        emit saleChanged();
        emit totalChanged(getFinalAmount());
    }
}

void Sale::addItem(Product* product, int quantity, double unitPrice)
{
    if (!product || quantity <= 0) {
        qWarning() << "尝试添加无效的销售项目";
        return;
    }
    
    // 如果没有指定单价，使用商品当前价格
    if (unitPrice <= 0.0) {
        unitPrice = product->getPrice();
    }
    
    // 检查是否已存在相同商品的项目
    for (SaleItem* item : m_items) {
        if (item->getProduct() == product && 
            qAbs(item->getUnitPrice() - unitPrice) < 0.01) {
            // 增加数量而不是创建新项目
            item->setQuantity(item->getQuantity() + quantity);
            calculateTotal();
            return;
        }
    }
    
    // 创建新的销售项目
    SaleItem* newItem = new SaleItem(product, quantity, unitPrice, this);
    
    // 连接信号
    connect(newItem, &SaleItem::itemChanged, 
            this, &Sale::calculateTotal);
    
    m_items.append(newItem);
    calculateTotal();
    
    emit itemAdded(newItem);
    emit saleChanged();
    
    qDebug() << "添加销售项目:" << newItem->toString();
}

bool Sale::removeItem(int index)
{
    if (index < 0 || index >= m_items.size()) {
        qWarning() << "尝试移除无效索引的项目:" << index;
        return false;
    }
    
    SaleItem* item = m_items.takeAt(index);
    item->deleteLater();
    
    calculateTotal();
    emit itemRemoved(index);
    emit saleChanged();
    
    qDebug() << "移除销售项目，索引:" << index;
    return true;
}

bool Sale::removeItem(Product* product)
{
    if (!product) {
        return false;
    }
    
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i]->getProduct() == product) {
            return removeItem(i);
        }
    }
    
    qWarning() << "未找到要移除的商品:" << product->getName();
    return false;
}

bool Sale::updateItemQuantity(int index, int quantity)
{
    if (index < 0 || index >= m_items.size()) {
        qWarning() << "尝试更新无效索引的项目数量:" << index;
        return false;
    }
    
    if (quantity <= 0) {
        // 数量为0或负数，移除项目
        return removeItem(index);
    }
    
    m_items[index]->setQuantity(quantity);
    return true;
}

void Sale::clearItems()
{
    for (SaleItem* item : m_items) {
        item->deleteLater();
    }
    m_items.clear();
    
    calculateTotal();
    emit saleChanged();
    
    qDebug() << "清空所有销售项目";
}

void Sale::calculateTotal()
{
    double oldTotal = m_totalAmount;
    m_totalAmount = 0.0;
    
    for (const SaleItem* item : m_items) {
        if (item->isValid()) {
            m_totalAmount += item->getSubtotal();
        }
    }
    
    if (qAbs(oldTotal - m_totalAmount) > 0.01) {
        emit totalChanged(getFinalAmount());
    }
}

bool Sale::isEmpty() const
{
    return m_items.isEmpty();
}

int Sale::getTotalItemCount() const
{
    int total = 0;
    for (const SaleItem* item : m_items) {
        total += item->getQuantity();
    }
    return total;
}

void Sale::applyPercentageDiscount(double percentage)
{
    if (percentage < 0.0 || percentage > 100.0) {
        qWarning() << "无效的折扣百分比:" << percentage;
        return;
    }
    
    double discount = m_totalAmount * (percentage / 100.0);
    setDiscountAmount(discount);
    
    qDebug() << "应用百分比折扣:" << percentage << "% 折扣金额:" << discount;
}

void Sale::applyFixedDiscount(double amount)
{
    if (amount < 0.0 || amount > m_totalAmount) {
        qWarning() << "无效的折扣金额:" << amount;
        return;
    }
    
    setDiscountAmount(amount);
    qDebug() << "应用固定金额折扣:" << amount;
}

QString Sale::paymentMethodToString(PaymentMethod method)
{
    switch (method) {
        case Cash: return "现金";
        case Card: return "银行卡";
        case MobilePay: return "移动支付";
        case GiftCard: return "礼品卡";
        case LoyaltyPoints: return "积分";
        case Mixed: return "混合支付";
        default: return "未知";
    }
}

Sale::PaymentMethod Sale::stringToPaymentMethod(const QString& str)
{
    if (str == "现金" || str.toLower() == "cash") return Cash;
    if (str == "银行卡" || str.toLower() == "card") return Card;
    if (str == "移动支付" || str.toLower() == "mobilepay") return MobilePay;
    if (str == "礼品卡" || str.toLower() == "giftcard") return GiftCard;
    if (str == "积分" || str.toLower() == "loyaltypoints") return LoyaltyPoints;
    if (str == "混合支付" || str.toLower() == "mixed") return Mixed;
    return Cash; // 默认返回现金
}

QString Sale::toString() const
{
    return QString("Sale[ID:%1, Items:%2, Total:%3, Discount:%4, Final:%5, Payment:%6, Status:%7]")
           .arg(m_transactionId)
           .arg(m_items.size())
           .arg(m_totalAmount, 0, 'f', 2)
           .arg(m_discountAmount, 0, 'f', 2)
           .arg(getFinalAmount(), 0, 'f', 2)
           .arg(paymentMethodToString(m_paymentMethod))
           .arg(static_cast<int>(m_status));
}
