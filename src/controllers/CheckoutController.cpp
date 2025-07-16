#include "CheckoutController.h"
#include "../models/Sale.h"
#include "../models/Product.h"
#include "../models/Customer.h"
#include "../database/DatabaseManager.h"
#include "../utils/ReceiptPrinter.h"
#include <QDebug>

CheckoutController::CheckoutController(QObject *parent)
    : QObject(parent)
    , m_currentSale(nullptr)
    , m_databaseManager(&DatabaseManager::getInstance())
    , m_receiptPrinter(std::make_unique<ReceiptPrinter>(this))
    , m_cashierName("收银员")
    , m_paymentProcessed(false)
    , m_changeAmount(0.0)
{
    qDebug() << "收银控制器初始化完成";
}

CheckoutController::~CheckoutController()
{
    qDebug() << "收银控制器析构";
}

void CheckoutController::setCurrentSale(Sale* sale)
{
    qDebug() << "CheckoutController::setCurrentSale called, sale:" << sale;
    qDebug() << "CheckoutController::setCurrentSale this:" << this;
    
    // 先断开之前销售的信号连接
    if (m_currentSale) {
        qDebug() << "CheckoutController::setCurrentSale disconnecting old sale:" << m_currentSale;
        // 只断开特定的信号连接，避免断开所有连接
        disconnect(m_currentSale, &Sale::saleChanged, this, &CheckoutController::onSaleChanged);
        disconnect(m_currentSale, &Sale::totalChanged, this, &CheckoutController::saleUpdated);
    }
    
    // 设置新的销售对象
    m_currentSale = sale;
    qDebug() << "CheckoutController::setCurrentSale m_currentSale set to:" << m_currentSale;
    
    // 重置状态
    m_paymentProcessed = false;
    m_changeAmount = 0.0;
    
    // 只有当新销售对象不为空时才进行连接
    if (m_currentSale) {
        qDebug() << "CheckoutController::setCurrentSale connecting signals for sale:" << m_currentSale;
        // 连接新销售的信号
        connect(m_currentSale, &Sale::saleChanged,
                this, &CheckoutController::onSaleChanged);
        connect(m_currentSale, &Sale::totalChanged,
                this, &CheckoutController::saleUpdated);
        
        // 设置收银员名称
        m_currentSale->setCashierName(m_cashierName);
        
        qDebug() << "设置当前销售，ID:" << m_currentSale->getTransactionId();
    } else {
        qDebug() << "设置当前销售为nullptr";
    }
    
    qDebug() << "CheckoutController::setCurrentSale emitting saleUpdated";
    emit saleUpdated();
    qDebug() << "CheckoutController::setCurrentSale finished";
}

Sale* CheckoutController::startNewSale(Customer* customer)
{
    qDebug() << "CheckoutController::startNewSale called, customer:" << customer;
    Sale* sale = new Sale();
    setCurrentSale(sale);
    qDebug() << "CheckoutController::startNewSale created and set new Sale:" << sale;
    return sale;
}

bool CheckoutController::addItemToSale(Product* product, int quantity, double unitPrice)
{
    if (!m_currentSale) {
        emit errorOccurred("当前没有活动的销售");
        return false;
    }
    
    if (!product) {
        emit errorOccurred("商品指针为空");
        return false;
    }
    
    if (quantity <= 0) {
        emit errorOccurred("商品数量必须大于0");
        return false;
    }
    
    // 检查库存
    if (!checkStock(product, quantity)) {
        emit errorOccurred(QString("商品 %1 库存不足，当前库存：%2")
                          .arg(product->getName())
                          .arg(product->getStockQuantity()));
        return false;
    }
    
    // 如果没有指定单价，使用商品当前价格
    if (unitPrice <= 0.0) {
        unitPrice = product->getPrice();
    }
    
    // 添加到销售
    m_currentSale->addItem(product, quantity, unitPrice);
    
    emit itemAdded(product->getName(), quantity);
    logOperation(QString("添加商品：%1 x %2").arg(product->getName()).arg(quantity));
    
    return true;
}

bool CheckoutController::removeItemFromSale(int productId)
{
    if (!m_currentSale) {
        emit errorOccurred("当前没有活动的销售");
        return false;
    }

    const auto& items = m_currentSale->getItems();
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i)->getProduct()->getProductId() == productId) {
            QString productName = items.at(i)->getProduct()->getName();
            if (m_currentSale->removeItem(i)) {
                emit itemRemoved(i);
                logOperation(QString("移除商品：%1").arg(productName));
                return true;
            }
            break; 
        }
    }

    emit errorOccurred("移除商品失败，未在购物车中找到该商品");
    return false;
}

bool CheckoutController::updateItemQuantity(int productId, int quantity)
{
    if (!m_currentSale) {
        emit errorOccurred("当前没有活动的销售");
        return false;
    }

    const auto& items = m_currentSale->getItems();
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i)->getProduct()->getProductId() == productId) {
            Product* product = items.at(i)->getProduct();

            if (quantity <= 0) {
                return removeItemFromSale(productId);
            }

            int currentQuantity = items.at(i)->getQuantity();
            int additionalQuantity = quantity - currentQuantity;

            if (additionalQuantity > 0) {
                if (!checkStock(product, additionalQuantity)) {
                    emit errorOccurred(QString("商品 %1 库存不足").arg(product->getName()));
                    return false;
                }
            }

            if (m_currentSale->updateItemQuantity(i, quantity)) {
                logOperation(QString("更新商品数量：%1 -> %2").arg(product->getName()).arg(quantity));
                return true;
            }
            break;
        }
    }

    emit errorOccurred("更新商品数量失败，未在购物车中找到该商品");
    return false;
}

bool CheckoutController::applyDiscount(const QString& discountType, double discountValue)
{
    if (!m_currentSale) {
        emit errorOccurred("当前没有活动的销售");
        return false;
    }
    
    if (discountValue < 0) {
        emit errorOccurred("折扣值不能为负数");
        return false;
    }
    
    if (discountType.toLower() == "percentage") {
        if (discountValue > 100) {
            emit errorOccurred("百分比折扣不能超过100%");
            return false;
        }
        m_currentSale->applyPercentageDiscount(discountValue);
    } else if (discountType.toLower() == "fixed") {
        if (discountValue > m_currentSale->getTotalAmount()) {
            emit errorOccurred("固定折扣不能超过总金额");
            return false;
        }
        m_currentSale->applyFixedDiscount(discountValue);
    } else {
        emit errorOccurred("无效的折扣类型");
        return false;
    }
    
    logOperation(QString("应用折扣：%1 %2").arg(discountType).arg(discountValue));
    return true;
}

bool CheckoutController::processPayment(const QString& paymentMethod, double amount, double customerMoney)
{
    if (!validateSale()) {
        return false;
    }
    
    double totalAmount = m_currentSale->getFinalAmount();
    
    if (amount < totalAmount) {
        emit errorOccurred("支付金额不足");
        return false;
    }
    
    // 设置支付方式
    Sale::PaymentMethod method = Sale::stringToPaymentMethod(paymentMethod);
    m_currentSale->setPaymentMethod(method);
    
    // 计算找零（主要针对现金支付）
    if (method == Sale::Cash) {
        if (customerMoney < totalAmount) {
            emit errorOccurred("现金金额不足");
            return false;
        }
        m_changeAmount = customerMoney - totalAmount;
    } else {
        m_changeAmount = 0.0;
    }
    
    m_paymentProcessed = true;
    
    emit paymentProcessed(true, m_changeAmount);
    logOperation(QString("处理支付：%1，找零：¥%2")
                .arg(paymentMethod)
                .arg(m_changeAmount, 0, 'f', 2));
    
    return true;
}

bool CheckoutController::completeSale()
{
    qDebug() << "CheckoutController::completeSale called, m_currentSale:" << m_currentSale << ", m_databaseManager:" << m_databaseManager << ", m_receiptPrinter:" << (m_receiptPrinter ? m_receiptPrinter.get() : nullptr);
    if (!validateSale()) {
        qDebug() << "CheckoutController::completeSale validateSale failed";
        return false;
    }
    if (!m_paymentProcessed) {
        emit errorOccurred("支付尚未处理");
        qDebug() << "CheckoutController::completeSale payment not processed";
        return false;
    }
    if (!m_currentSale) {
        emit errorOccurred("当前没有活动的销售");
        qDebug() << "CheckoutController::completeSale m_currentSale is null";
        return false;
    }
    if (!m_databaseManager) {
        emit errorOccurred("数据库管理器未初始化");
        qDebug() << "CheckoutController::completeSale m_databaseManager is null";
        return false;
    }
    if (!m_receiptPrinter) {
        emit errorOccurred("票据打印器未初始化");
        qDebug() << "CheckoutController::completeSale m_receiptPrinter is null";
        return false;
    }
    // 设置交易状态为已完成
    m_currentSale->setStatus(Sale::Completed);
    // 保存到数据库
    int transactionId = m_databaseManager->saveTransaction(m_currentSale);
    qDebug() << "CheckoutController::completeSale after saveTransaction, transactionId:" << transactionId;
    if (transactionId < 0) {
        emit errorOccurred("保存交易到数据库失败");
        return false;
    }
    // 打印票据
    if (!m_receiptPrinter->printReceipt(*m_currentSale, m_currentSale->getItems())) {
        qWarning() << "打印票据失败，但交易已保存";
        // 注意：这里不返回false，因为交易已经成功保存
    }
    emit saleCompleted(transactionId);
    emit saleSuccessfullyCompleted(m_currentSale); // 发射带有完整销售信息的信号
    logOperation(QString("完成销售，交易ID：%1").arg(transactionId));
    // 重置状态
    m_paymentProcessed = false;
    m_changeAmount = 0.0;
    return true;
}

void CheckoutController::cancelSale()
{
    if (m_currentSale) {
        m_currentSale->setStatus(Sale::Cancelled);
        m_currentSale->clearItems();
        
        m_paymentProcessed = false;
        m_changeAmount = 0.0;
        
        emit saleCancelled();
        logOperation("取消销售");
    }
}

void CheckoutController::clearSale()
{
    if (m_currentSale) {
        m_currentSale->clearItems();
        m_paymentProcessed = false;
        m_changeAmount = 0.0;
        
        logOperation("清空销售");
    }
}

double CheckoutController::getChangeAmount(double paymentAmount) const
{
    if (!m_currentSale) {
        return 0.0;
    }
    
    double total = m_currentSale->getFinalAmount();
    return qMax(0.0, paymentAmount - total);
}

bool CheckoutController::checkStock(Product* product, int quantity) const
{
    if (!product) {
        return false;
    }
    
    // 检查商品库存是否足够
    int availableStock = product->getStockQuantity();
    
    // 如果当前销售中已经包含了这个商品，需要考虑已使用的数量
    if (m_currentSale) {
        for (auto* item : m_currentSale->getItems()) {
            if (item->getProduct() == product) {
                availableStock -= item->getQuantity();
            }
        }
    }
    
    return availableStock >= quantity;
}

void CheckoutController::setCashierName(const QString& cashierName)
{
    m_cashierName = cashierName;
    
    if (m_currentSale) {
        m_currentSale->setCashierName(cashierName);
    }
    
    qDebug() << "设置收银员：" << cashierName;
}

void CheckoutController::onSaleChanged()
{
    emit saleUpdated();
}

bool CheckoutController::validateSale() const
{
    if (!m_currentSale) {
        emit const_cast<CheckoutController*>(this)->errorOccurred("当前没有活动的销售");
        return false;
    }
    
    if (m_currentSale->isEmpty()) {
        emit const_cast<CheckoutController*>(this)->errorOccurred("购物车为空");
        return false;
    }
    
    if (m_currentSale->getFinalAmount() <= 0) {
        emit const_cast<CheckoutController*>(this)->errorOccurred("销售金额无效");
        return false;
    }
    
    return true;
}

bool CheckoutController::updateInventory()
{
    qDebug() << "CheckoutController::updateInventory called, m_currentSale:" << m_currentSale << ", m_databaseManager:" << m_databaseManager;
    if (!m_currentSale) {
        qDebug() << "CheckoutController::updateInventory m_currentSale is null";
        return false;
    }
    if (!m_databaseManager) {
        qDebug() << "CheckoutController::updateInventory m_databaseManager is null";
        return false;
    }
    // 更新所有商品的库存
    for (auto* item : m_currentSale->getItems()) {
        Product* product = item->getProduct();
        int newStock = product->getStockQuantity() - item->getQuantity();
        if (!m_databaseManager->updateProductStock(product->getProductId(), newStock)) {
            qCritical() << "更新商品库存失败：" << product->getName();
            return false;
        }
        // 更新内存中的商品对象
        product->setStockQuantity(newStock);
    }
    return true;
}

void CheckoutController::logOperation(const QString& message)
{
    QString logMessage = QString("[%1] %2: %3")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                        .arg(m_cashierName)
                        .arg(message);
    qDebug() << logMessage;
}
