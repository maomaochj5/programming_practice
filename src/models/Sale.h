#ifndef SALE_H
#define SALE_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include "SaleItem.h"
#include "Customer.h"

/**
 * @brief Sale类 - 销售交易数据模型
 * 
 * 代表一笔完整的销售交易，包含多个销售项目
 */
class Sale : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 支付方式枚举
     */
    enum PaymentMethod {
        Cash = 0,           ///< 现金
        Card,               ///< 银行卡
        MobilePay,          ///< 移动支付
        GiftCard,           ///< 礼品卡
        LoyaltyPoints,      ///< 积分
        Mixed               ///< 混合支付
    };
    Q_ENUM(PaymentMethod)
    
    /**
     * @brief 交易状态枚举
     */
    enum TransactionStatus {
        InProgress = 0,     ///< 进行中
        Completed,          ///< 已完成
        Cancelled,          ///< 已取消
        Refunded            ///< 已退款
    };
    Q_ENUM(TransactionStatus)
    
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit Sale(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~Sale();
    
    /**
     * @brief 参数化构造函数
     * @param transactionId 交易ID
     * @param customer 客户指针（可为nullptr）
     * @param parent 父对象指针
     */
    Sale(int transactionId, Customer* customer = nullptr, QObject *parent = nullptr);
    
    // Getter 方法
    int getTransactionId() const { return m_transactionId; }
    Customer* getCustomer() const { return m_customer; }
    QList<SaleItem*> getItems() const { return m_items; }
    double getTotalAmount() const { return m_totalAmount; }
    double getDiscountAmount() const { return m_discountAmount; }
    double getFinalAmount() const { return m_totalAmount - m_discountAmount; }
    PaymentMethod getPaymentMethod() const { return m_paymentMethod; }
    TransactionStatus getStatus() const { return m_status; }
    QDateTime getTimestamp() const { return m_timestamp; }
    QString getCashierName() const { return m_cashierName; }
    
    // Setter 方法
    void setTransactionId(int transactionId);
    void setCustomer(Customer* customer);
    void setPaymentMethod(PaymentMethod method);
    void setStatus(TransactionStatus status);
    void setCashierName(const QString& cashierName);
    void setDiscountAmount(double discount);
    
    /**
     * @brief 添加销售项目
     * @param product 商品指针
     * @param quantity 数量
     * @param unitPrice 单价（如果为0则使用商品当前价格）
     */
    void addItem(Product* product, int quantity, double unitPrice = 0.0);
    
    /**
     * @brief 移除销售项目
     * @param index 项目索引
     * @return 如果移除成功返回true
     */
    bool removeItem(int index);
    
    /**
     * @brief 移除销售项目
     * @param product 要移除的商品
     * @return 如果移除成功返回true
     */
    bool removeItem(Product* product);
    
    /**
     * @brief 更新销售项目数量
     * @param index 项目索引
     * @param quantity 新数量
     * @return 如果更新成功返回true
     */
    bool updateItemQuantity(int index, int quantity);
    
    /**
     * @brief 清空所有销售项目
     */
    void clearItems();
    
    /**
     * @brief 计算总金额
     */
    void calculateTotal();
    
    /**
     * @brief 检查销售是否为空
     * @return 如果没有销售项目返回true
     */
    bool isEmpty() const;
    
    /**
     * @brief 获取总商品数量
     * @return 所有项目的数量总和
     */
    int getTotalItemCount() const;
    
    /**
     * @brief 应用百分比折扣
     * @param percentage 折扣百分比（0-100）
     */
    void applyPercentageDiscount(double percentage);
    
    /**
     * @brief 应用固定金额折扣
     * @param amount 折扣金额
     */
    void applyFixedDiscount(double amount);
    
    /**
     * @brief 支付方式转字符串
     * @param method 支付方式
     * @return 支付方式的字符串表示
     */
    static QString paymentMethodToString(PaymentMethod method);
    
    /**
     * @brief 字符串转支付方式
     * @param str 支付方式字符串
     * @return 对应的支付方式枚举
     */
    static PaymentMethod stringToPaymentMethod(const QString& str);
    
    /**
     * @brief 转换为字符串表示
     * @return 销售信息的字符串表示
     */
    QString toString() const;

signals:
    /**
     * @brief 销售信息发生变化时发射的信号
     */
    void saleChanged();
    
    /**
     * @brief 添加项目时发射的信号
     * @param item 新添加的项目
     */
    void itemAdded(SaleItem* item);
    
    /**
     * @brief 移除项目时发射的信号
     * @param index 被移除项目的索引
     */
    void itemRemoved(int index);
    
    /**
     * @brief 总金额变化时发射的信号
     * @param newTotal 新的总金额
     */
    void totalChanged(double newTotal);

private:
    int m_transactionId;            ///< 交易ID
    Customer* m_customer;           ///< 客户指针
    QList<SaleItem*> m_items;       ///< 销售项目列表
    double m_totalAmount;           ///< 总金额
    double m_discountAmount;        ///< 折扣金额
    PaymentMethod m_paymentMethod;  ///< 支付方式
    TransactionStatus m_status;     ///< 交易状态
    QDateTime m_timestamp;          ///< 交易时间
    QString m_cashierName;          ///< 收银员姓名
};

#endif // SALE_H
