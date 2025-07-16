#ifndef CHECKOUTCONTROLLER_H
#define CHECKOUTCONTROLLER_H

#include <QObject>
#include <memory>

// 前向声明
class Sale;
class Product;
class Customer;
class DatabaseManager;
class ReceiptPrinter;

/**
 * @brief CheckoutController类 - 收银流程控制器
 * 
 * 管理整个销售和结账流程的控制器，协调各个模块的工作
 */
class CheckoutController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit CheckoutController(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~CheckoutController();

    /**
     * @brief 设置当前销售
     * @param sale 销售对象指针
     */
    void setCurrentSale(Sale* sale);

    /**
     * @brief 获取当前销售
     * @return 当前销售对象指针
     */
    Sale* getCurrentSale() const { return m_currentSale; }

    /**
     * @brief 开始新的销售
     * @param customer 客户指针（可为nullptr）
     * @return 新创建的销售对象指针
     */
    Sale* startNewSale(Customer* customer = nullptr);

    /**
     * @brief 添加商品到销售
     * @param product 商品指针
     * @param quantity 数量
     * @param unitPrice 单价（如果为0则使用商品当前价格）
     * @return 如果添加成功返回true
     */
    bool addItemToSale(Product* product, int quantity, double unitPrice = 0.0);

    /**
     * @brief 从销售中移除商品
     * @param productId 商品ID
     * @return 如果移除成功返回true
     */
    bool removeItemFromSale(int productId);

    /**
     * @brief 更新销售项目数量
     * @param productId 商品ID
     * @param quantity 新数量
     * @return 如果更新成功返回true
     */
    bool updateItemQuantity(int productId, int quantity);

    /**
     * @brief 应用折扣
     * @param discountType 折扣类型（"percentage" 或 "fixed"）
     * @param discountValue 折扣值
     * @return 如果应用成功返回true
     */
    bool applyDiscount(const QString& discountType, double discountValue);

    /**
     * @brief 处理支付
     * @param paymentMethod 支付方式
     * @param amount 支付金额
     * @param customerMoney 客户给的钱（现金支付时）
     * @return 如果支付处理成功返回true
     */
    bool processPayment(const QString& paymentMethod, double amount, double customerMoney = 0.0);

    /**
     * @brief 完成销售（保存到数据库并打印票据）
     * @return 如果完成成功返回true
     */
    bool completeSale();

    /**
     * @brief 取消当前销售
     */
    void cancelSale();

    /**
     * @brief 清空当前销售
     */
    void clearSale();

    /**
     * @brief 获取找零金额
     * @param paymentAmount 支付金额
     * @return 找零金额
     */
    double getChangeAmount(double paymentAmount) const;

    /**
     * @brief 检查库存是否足够
     * @param product 商品指针
     * @param quantity 需要的数量
     * @return 如果库存足够返回true
     */
    bool checkStock(Product* product, int quantity) const;

    /**
     * @brief 设置收银员名称
     * @param cashierName 收银员名称
     */
    void setCashierName(const QString& cashierName);

signals:
    /**
     * @brief 销售更新时发射的信号
     */
    void saleUpdated();

    /**
     * @brief 商品添加到销售时发射的信号
     * @param productName 商品名称
     * @param quantity 数量
     */
    void itemAdded(const QString& productName, int quantity);

    /**
     * @brief 商品从销售中移除时发射的信号
     * @param index 被移除项目的索引
     */
    void itemRemoved(int index);

    /**
     * @brief 支付处理完成时发射的信号
     * @param success 是否成功
     * @param changeAmount 找零金额
     */
    void paymentProcessed(bool success, double changeAmount);

    /**
     * @brief 销售完成时发射的信号
     * @param transactionId 交易ID
     */
    void saleCompleted(int transactionId);

    /**
     * @brief 销售成功完成后，将完整的销售信息发出
     * @param completedSale 刚刚完成的销售对象的副本
    */
    void saleSuccessfullyCompleted(Sale* completedSale);

    /**
     * @brief 销售被取消时发射的信号
     */
    void saleCancelled();

    /**
     * @brief 错误发生时发射的信号
     * @param errorMessage 错误消息
     */
    void errorOccurred(const QString& errorMessage);

private slots:
    /**
     * @brief 处理销售变化
     */
    void onSaleChanged();

private:
    /**
     * @brief 验证销售有效性
     * @return 如果销售有效返回true
     */
    bool validateSale() const;

    /**
     * @brief 更新商品库存
     * @return 如果更新成功返回true
     */
    bool updateInventory();

    /**
     * @brief 记录日志
     * @param message 日志消息
     */
    void logOperation(const QString& message);

private:
    Sale* m_currentSale;                        ///< 当前销售对象
    DatabaseManager* m_databaseManager;        ///< 数据库管理器
    std::unique_ptr<ReceiptPrinter> m_receiptPrinter;  ///< 票据打印器
    QString m_cashierName;                      ///< 收银员名称
    bool m_paymentProcessed;                    ///< 支付是否已处理
    double m_changeAmount;                      ///< 找零金额
};

#endif // CHECKOUTCONTROLLER_H
