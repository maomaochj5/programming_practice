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
    qDebug() << "Sale 构造, this:" << this << ", parent:" << parent;
    qDebug() << "Sale 构造, m_items.size:" << m_items.size();
}

Sale::Sale(const Sale& other)
    : QObject(nullptr) // A copied sale shouldn't have a parent initially.
    , m_transactionId(other.m_transactionId)
    , m_customer(other.m_customer) // Shallow copy of customer is acceptable here.
    , m_totalAmount(other.m_totalAmount)
    , m_discountAmount(other.m_discountAmount)
    , m_paymentMethod(other.m_paymentMethod)
    , m_status(other.m_status)
    , m_timestamp(other.m_timestamp)
    , m_cashierName(other.m_cashierName)
{
    // Deep copy of the sale items
    for (SaleItem* item : other.m_items) {
        SaleItem* newItem = new SaleItem(*item);
        newItem->setParent(this); // The new Sale owns the new SaleItem
        m_items.append(newItem);
    }
}

Sale::~Sale()
{
    qDebug() << "Sale 析构, this:" << this;
    // No need to manually delete child QObjects. Qt's parent-child
    // memory management system will handle the deletion of m_items
    // automatically since they were created with this Sale object as their parent.
    // qDeleteAll(m_items); // This would cause a double-free.
    m_items.clear();
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

void Sale::setTimestamp(const QDateTime& timestamp)
{
    if (m_timestamp != timestamp) {
        m_timestamp = timestamp;
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
        calculateTotal(); // Recalculate and emit signals
    }
}

void Sale::addItem(SaleItem* item)
{
    if (!item || !item->isValid()) {
        qWarning() << "Attempt to add invalid SaleItem.";
        if (item) item->deleteLater(); // Clean up invalid item
        return;
    }

    // Ensure the item has the correct parent
    if (item->parent() != this) {
        item->setParent(this);
    }

    m_items.append(item);
    
    connect(item, &SaleItem::itemChanged, this, &Sale::calculateTotal);

    emit itemAdded(item);
    calculateTotal();
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
            // After quantity is set, we need to manually trigger the recalculation and signaling
            calculateTotal();
            return;
        }
    }
    
    // 创建新的销售项目
    SaleItem* newItem = new SaleItem(product, quantity, unitPrice, this);
    
    // 连接信号
    connect(newItem, &SaleItem::itemChanged, this, &Sale::calculateTotal);
    
    m_items.append(newItem);
    
    emit itemAdded(newItem);
    // No longer emit saleChanged() here, calculateTotal will do it.
    calculateTotal(); // Update totals
}

bool Sale::removeItem(int index)
{
    if (index < 0 || index >= m_items.size()) {
        qWarning() << "尝试移除无效索引的项目:" << index;
        return false;
    }
    
    SaleItem* item = m_items.takeAt(index);
    // 直接删除，不使用deleteLater()
    delete item;
    
    emit itemRemoved(index);
    // No longer emit saleChanged() here, calculateTotal will do it.
    calculateTotal(); // Update totals
    
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
    // setQuantity will trigger itemChanged, which calls calculateTotal.
    // So no extra call is needed here, the connection handles it.
    return true;
}

void Sale::clearItems()
{
    for (SaleItem* item : m_items) {
        // 直接删除，不使用deleteLater()
        delete item;
    }
    m_items.clear();
    
    // No longer emit saleChanged() here, calculateTotal will do it.
    calculateTotal();
    
    qDebug() << "清空所有销售项目";
}

void Sale::calculateTotal()
{
    m_totalAmount = 0.0;
    
    for (const SaleItem* item : m_items) {
        if (item->isValid()) {
            m_totalAmount += item->getSubtotal();
        }
    }
    
    // Always emit both signals to ensure all listeners are updated
    emit totalChanged(getFinalAmount());
    emit saleChanged();
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

QList<SaleItem*> Sale::getItems() const
{
    qDebug() << "Sale::getItems called, this:" << this << ", m_items.size:" << m_items.size();
    for (auto* item : m_items) {
        qDebug() << "Sale::getItems item ptr:" << item;
    }
    return m_items;
}
